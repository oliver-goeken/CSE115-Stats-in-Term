#include "integration.h"
#include "log.h"

#define DEFAULT_CODE_FILE  "spotify_code.JSON"

// as per https://developer.spotify.com/documentation/web-api/tutorials/code-pkce-flow

void free_tokens(spotify_tokens* token_list)
{
    // NOTE: i'm also setting them to null after freeing just in case

    if (!token_list) return ; 

    free (token_list->access_token);
    token_list->access_token = NULL ;
    free (token_list->refresh_token) ; 
    token_list->refresh_token = NULL ; 
    free (token_list->token_type) ; 
    token_list->token_type = NULL ; 
    token_list-> expires_in = 0 ; 
}

size_t write_to_curl_buf(char* ptr, size_t size, size_t num_elems, void* userdata){
    size_t total_bytes = size * num_elems;
    curl_buffer* buff = (curl_buffer*)userdata;
    
    char* temp = realloc(buff->data, buff->len + total_bytes + 1);
    if (!temp)
        return 0;

    buff->data = temp;
    memcpy(buff->data + buff->len, ptr, total_bytes);

    buff->len += total_bytes;
    buff->data[buff->len] = '\0';
    return total_bytes;

}

int load_tokens(const char* path, spotify_tokens* tokens_output) {
    FILE* f = fopen(path, "r");
    if (!f) return IO_ERR;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* raw = malloc(size + 1);
    if (!raw) { fclose(f); return NO_MEM_ERR; }
    fread(raw, 1, size, f);
    fclose(f);
    raw[size] = '\0';

    cJSON* root = cJSON_Parse(raw);
    free(raw);
    if (!root) return JSON_ERR;

    const cJSON* access  = cJSON_GetObjectItemCaseSensitive(root, "access_token");
    const cJSON* refresh = cJSON_GetObjectItemCaseSensitive(root, "refresh_token");
    const cJSON* expires = cJSON_GetObjectItemCaseSensitive(root, "expires_in");

    if (!cJSON_IsString(access) || !cJSON_IsString(refresh) || !cJSON_IsNumber(expires)) {
        cJSON_Delete(root);
        return JSON_ERR;
    }

    tokens_output->access_token  = strdup(access->valuestring);
    tokens_output->refresh_token = strdup(refresh->valuestring);
    tokens_output->token_type    = strdup("Bearer");
    tokens_output->expires_in    = (int)expires->valuedouble;

    cJSON_Delete(root);

    if (!tokens_output->access_token || !tokens_output->refresh_token || !tokens_output->token_type) {
        free_tokens(tokens_output);
        return NO_MEM_ERR;
    }

    return OK;
}

int spotify_api_get(const char* access_token, const char* endpoint, cJSON** response_out) {
    CURL* curl = curl_easy_init();
    if (!curl) return CURL_ERR;

    curl_buffer response = {0};
    struct curl_slist* headers = NULL;

    // build the Authorization header with the access token
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", access_token);
    headers = curl_slist_append(headers, auth_header);

    char url[512];
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/%s", endpoint);

    curl_easy_setopt(curl, CURLOPT_URL,           url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_curl_buf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,     "spotify-pkce-c/1.0");

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) { free(response.data); return CURL_ERR; }
    if (http_code != 200) { free(response.data); return AUTH_ERR; }

    *response_out = cJSON_Parse(response.data);
    free(response.data);
    if (!*response_out) return JSON_ERR;

    return OK;
}



api_track_list api_get_top_tracks(const char* access_token) {
    api_track_list list = {0};  // return this on failure
    
    char endpoint[128];
    snprintf(endpoint, sizeof(endpoint), "me/top/tracks?limit=50&time_range=medium_term");

    cJSON* response = NULL;
    int rc = spotify_api_get(access_token, endpoint, &response);
    if (rc != OK) return list;

    const cJSON* items = cJSON_GetObjectItemCaseSensitive(response, "items");
    if (!cJSON_IsArray(items)) {
        cJSON_Delete(response);
        return list;
    }

    int count = cJSON_GetArraySize(items);
    list.root = malloc(sizeof(track) * count);
    if (!list.root) { cJSON_Delete(response); return list; }
    list.len = 0;

    const cJSON* item = NULL;
    cJSON_ArrayForEach(item, items) {
        const cJSON* name       = cJSON_GetObjectItemCaseSensitive(item, "name");
        const cJSON* artists    = cJSON_GetObjectItemCaseSensitive(item, "artists");
        const cJSON* album      = cJSON_GetObjectItemCaseSensitive(item, "album");
        const cJSON* album_name = cJSON_GetObjectItemCaseSensitive(album, "name");
        const cJSON* uri        = cJSON_GetObjectItemCaseSensitive(item, "uri");

        char artist_names[512] = {0};
        const cJSON* artist = NULL;
        cJSON_ArrayForEach(artist, artists) {
            const cJSON* artist_name = cJSON_GetObjectItemCaseSensitive(artist, "name");
            if (cJSON_IsString(artist_name)) {
                if (artist_names[0]) strncat(artist_names, ", ", sizeof(artist_names) - strlen(artist_names) - 1);
                strncat(artist_names, artist_name->valuestring, sizeof(artist_names) - strlen(artist_names) - 1);
            }
        }

        api_track* t = &list.root[list.len];
        t->name      = cJSON_IsString(name)       ? strdup(name->valuestring)       : strdup("unknown");
        t->album     = cJSON_IsString(album_name) ? strdup(album_name->valuestring) : strdup("unknown");
        t->artist    = artist_names[0]            ? strdup(artist_names)            : strdup("unknown");
        t->track_uri = cJSON_IsString(uri)        ? strdup(uri->valuestring)        : strdup("unknown");

        list.len++;
    }

    cJSON_Delete(response);
    return list;
}   

api_track_list open_auth(){
    api_track_list empty = {0};

    int rc = system("/Users/oliverdgoeken/.local/pipx/venvs/python-dotenv/bin/python3 /Users/oliverdgoeken/school/25-26/q2/cse/CSE115-Stats-in-Term/src/pkce_auth.py");
    if (rc != 0) {
        log_msg("Auth failed");
        return empty;
    }

    spotify_tokens tok = {0};
    if (load_tokens("tokens.json", &tok) != OK) {
        log_msg("Failed to load tokens");
        return empty;
    }

    api_track_list top = api_get_top_tracks(tok.access_token);
    free_tokens(&tok);
    return top;
}

void api_free_track_list(api_track_list list) {
    if (!list.root) return;
    for (int i = 0; i < list.len; i++) {
        free(list.root[i].name);
        free(list.root[i].album);
        free(list.root[i].artist);
        free(list.root[i].track_uri);
    }
    free(list.root);
}
