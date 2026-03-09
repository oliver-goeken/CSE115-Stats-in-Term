#include "integration.h"

#define DEFAULT_CODE_FILE  "spotify_code.JSON"

// as per https://developer.spotify.com/documentation/web-api/tutorials/code-pkce-flow

// after authentication on browser, user will see a 127.0.0.1 error page
// this is expected, the code is sent to the redirect uri which is localhost
// http://127.0.0.1:8080/?code=<the_string_the_user_needs_to_copy>&state=<other_stuff_we_dont_use>
// user MUST copy the string after code= and before &state=
// in your terminal in the same directory as integration.c, run: echo '{"code": "the_string_the_user_copied"}' > spotify_code.JSON


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

// PKCE conversion functions 
static char* encode(const unsigned char* str, size_t len){
    size_t base64_len = ((len+2)/3)*4+1 ; 
    char* buffer = malloc(base64_len); 
    if (!buffer) return NULL ; 

    EVP_EncodeBlock((unsigned char* )buffer, str, (int) len);
    for (char* c = buffer; *c; c++){
        if (*c == '+')
            *c = '-';
        else if (*c == '/')
            *c = '_';
        else if (*c == '='){ // end of data string!
            *c = '\0';
            break;
        }
    }

    return buffer;
}

static int code_verifier(char* data, size_t data_size){
    unsigned char raw_data[32];
    if(RAND_bytes(raw_data, sizeof(raw_data)) != 1)
        return CRYPTO_ERR;

    char* encoded_data = encode(raw_data, sizeof(raw_data));
    if (!encoded_data)
        return NO_MEM_ERR;

    strncpy(data, encoded_data, data_size-1);
    data[data_size-1] = '\0';

    free(encoded_data);
    return OK;
}

static int code_challenge(const char* verifier, char* data, size_t data_size){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*) verifier, strlen(verifier), hash);
    
    char* encoded_data = encode(hash, sizeof(hash));
    if (!encoded_data) 
        return NO_MEM_ERR;

    strncpy(data, encoded_data, data_size-1);
    data[data_size-1] = '\0';

    free(encoded_data);
    return OK;
}

static int generate_state(char* data, size_t data_size){
    unsigned char raw_data[16];
    if (RAND_bytes(raw_data, sizeof(raw_data)) != 1)
        return CRYPTO_ERR;

    char* encoded_data = encode(raw_data, sizeof(raw_data));
    if (!encoded_data)
        return NO_MEM_ERR;

    strncpy(data, encoded_data, data_size-1);
    data[data_size-1] = '\0';

    free(encoded_data);
    return OK;
}

// URL encoding and authorization section
static char* encode_url(const char* url){
    CURL* curl = curl_easy_init();
    if (!curl)
        return NULL ;

    char* output = curl_easy_escape(curl, url, 0);
    curl_easy_cleanup(curl);
    if(!output)
        return NULL;

    char* copy_of_output = strdup(output);
    curl_free(output);
    return copy_of_output;
}




static char* url_auth_build(const char* client_id, const char* redirect_uri, const char* scopes, const char* state, const char* challenge){
    char* encode_redirect = encode_url(redirect_uri);
    char* encode_scopes = encode_url(scopes);
    char* encode_state = encode_url(state);
    char* encode_challenge = encode_url(challenge);

    if(!encode_redirect || !encode_scopes || !encode_state || !encode_challenge){
        free(encode_redirect);
        free(encode_scopes);
        free(encode_state);
        free(encode_challenge);
        return NULL;
    }
    //make sure url size is big enough!!!
    size_t url_size = strlen(SPOTIFY_AUTH_URL) + strlen(client_id) + strlen(encode_redirect) + strlen(encode_scopes) + strlen(encode_state) + strlen(encode_challenge) + 256; 

    char* url = malloc(url_size);
    if(!url) 
        return NULL;
    else{ // format the url here
        snprintf(url, url_size, "%s" "?response_type=code" "&client_id=%s" "&scope=%s" "&redirect_uri=%s" "&state=%s" "&code_challenge_method=S256" "&code_challenge=%s", SPOTIFY_AUTH_URL, client_id, encode_scopes, encode_redirect, encode_state, encode_challenge);
    }

    free(encode_redirect);
    free(encode_scopes);
    free(encode_state);
    free(encode_challenge);
    return url;
}

static const char* open_url_on_browser(const char* url) {
    // for testing purposes, uncomment the printf
    //printf("Please open the following URL in your browser to authenticate:\n%s\n", url); 
    return url ; // in pkce_auth, the url is built: char* auth_url = url_auth_build(client_id, redirect_uri, scopes, state_str, code_challenge_str);

}

static int read_code_from_json(const char* path, char* out, size_t out_size){
    FILE* f = fopen(path, "r");
    if (!f)
        return IO_ERR;

    //use cJSON to do this
    // get the length of the file and then rewind back
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);

    if (file_size <= 0) {
        fclose(f);
        return JSON_ERR;
    }

    char *raw = malloc((size_t)file_size + 1);
    if (!raw) {
        fclose(f); 
        return JSON_ERR; 
    }

    size_t read_len = fread(raw, 1, (size_t)file_size, f);
    fclose(f);
    raw[read_len] = '\0';

    cJSON *root = cJSON_Parse(raw);
    free(raw);

    if (!root) {
        fprintf(stderr, "[spotify_pkce] cJSON parse error: %s\n", cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() : "unknown");
        return JSON_ERR;
    }

    const cJSON *code_item = cJSON_GetObjectItemCaseSensitive(root, "code");
    if (!cJSON_IsString(code_item) || !code_item->valuestring || !code_item->valuestring[0]) {
        cJSON_Delete(root);
        return JSON_ERR;
    }

    strncpy(out, code_item->valuestring, out_size - 1);
    out[out_size - 1] = '\0';

    cJSON_Delete(root);
    return OK;
}

static int wait_til_json_populated(const char* json, char* code, size_t code_size){
    // check the json until it's populated, will be different for each os
    time_t time_limit = time(NULL) + WAIT_FOR_CODE_TIME_MS;

    while(time(NULL) < time_limit){
#ifdef __WIN32
        Sleep(POLL_INTERVAL_MS);
#else 
        usleep(POLL_INTERVAL_MS * 1000); // just incase it needs more polling time
#endif
        if(read_code_from_json(json, code, code_size) == OK)
            return OK;
    }

    return TIMEOUT_ERR; // making this the default instead of assuming it'll work
}


static size_t write_to_curl_buf(char* ptr, size_t size, size_t num_elems, void* userdata){
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


// tokenize and write to file
static int code_to_tokens(const char* code, const char* client_id, const char* redirect_uri, const char* verifier, spotify_tokens *tokens_output){
    char* encode_redirect = encode_url(redirect_uri);
    char* encode_code = encode_url(code);
    char* encode_client = encode_url(client_id);
    char* encode_verifier= encode_url(verifier);

    if (!encode_redirect|| !encode_code || !encode_client || !encode_verifier){
        free(encode_redirect);
        free(encode_code);
        free(encode_client);
        free(encode_verifier);
        return NO_MEM_ERR;
    }

    //allocate mem for post auth
    char post_auth[8192];
    snprintf(post_auth, sizeof(post_auth), "grant_type=authorization_code" "&code=%s" "&redirect_uri=%s" "&client_id=%s" "&code_verifier=%s", encode_code, encode_redirect, encode_client, encode_verifier);
    free(encode_redirect);
    free(encode_code);
    free(encode_client);
    free(encode_verifier);

    //now set up curl resp
    CURL* curl = curl_easy_init();
    if (!curl)
        return CURL_ERR;
    curl_buffer response = {0};
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    curl_easy_setopt(curl, CURLOPT_URL,            SPOTIFY_TOKEN_URL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,     post_auth);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_to_curl_buf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,      "spotify-pkce-c/1.0");
    CURLcode resp = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if(resp != CURLE_OK) {
        free(response.data);
        return CURL_ERR;
    }
    if (http_code != 200) {
        free(response.data);
        return AUTH_ERR;
    }

    const char* json = response.data;
    cJSON* root = cJSON_Parse(json);
    free(response.data);
    if(!root){
        //log_msg("[spotify_pkce] cJSON response parse error");
        return JSON_ERR;
    }

    const cJSON* check_error = cJSON_GetObjectItemCaseSensitive(root, "error");
    if (cJSON_IsString(check_error) && check_error->valuestring) {
        //log_msg("[spotify_pkce] Spotify API error: %s", cJSON_GetObjectItemCaseSensitive(root, "error_description")->valuestring);
        cJSON_Delete(root);
        return AUTH_ERR;
    }

    const cJSON* access_token = cJSON_GetObjectItemCaseSensitive(root, "access_token");
    const cJSON* refresh_token = cJSON_GetObjectItemCaseSensitive(root, "refresh_token");
    const cJSON* token_type = cJSON_GetObjectItemCaseSensitive(root, "token_type");
    const cJSON* expires_in = cJSON_GetObjectItemCaseSensitive(root, "expires_in");

    if (!cJSON_IsString(access_token) || !access_token->valuestring ||
        !cJSON_IsString(refresh_token) || !refresh_token->valuestring ||
        !cJSON_IsString(token_type) || !token_type->valuestring ||
        !cJSON_IsNumber(expires_in)) {
        //log_msg("[spotify_pkce] cJSON response missing fields");
        cJSON_Delete(root);
        return JSON_ERR;
    }

    tokens_output->access_token = strdup(access_token->valuestring);
    tokens_output->refresh_token = strdup(refresh_token->valuestring);
    tokens_output->token_type = strdup(token_type->valuestring);
    tokens_output->expires_in = expires_in->valuedouble;

    cJSON_Delete(root);
    if (!tokens_output->access_token || !tokens_output->refresh_token || !tokens_output->token_type) {
        free_tokens(tokens_output);
        return JSON_ERR;
    }
    return OK;
}

int write_code_file(const char* code_file, const char* code){
    FILE* f = fopen(code_file, "w");
    if (!f)
        return IO_ERR;
    fprintf(f, "{\"code\": \"%s\"}\n", code);
    fclose(f);
    return OK;
}

// pkce authentication
int pkce_auth(const char* client_id, const char* redirect_uri, const char* scopes, const char* code_file, spotify_tokens* tokens_output){
    if (!client_id || !redirect_uri || !tokens_output)
        return JSON_ERR;

    if (!code_file)
        code_file = DEFAULT_CODE_FILE;
    if(!scopes)
        scopes = DEFAULT_SCOPES;

    memset(tokens_output, 0, sizeof(spotify_tokens));
    char code_verifier_str[128];
    char code_challenge_str[128];
    char state_str[64] = {};
    int rc;

    if ((rc = code_verifier(code_verifier_str, sizeof(code_verifier_str))) != OK)
        return rc;
    if ((rc = code_challenge(code_verifier_str, code_challenge_str, sizeof(code_challenge_str))) != OK)
        return rc;
    if ((rc = generate_state(state_str, sizeof(state_str))) != OK)
        return rc;

    char* auth_url = url_auth_build(client_id, redirect_uri, scopes, state_str, code_challenge_str);
    if (!auth_url)
        return NO_MEM_ERR;

    open_url_on_browser(auth_url);
    free(auth_url);

    char code[256];
    rc = wait_til_json_populated(code_file, code, sizeof(code));
    if (rc != OK){
        //logmsg("[spotify_pkce] Error waiting for code: %d", rc);
        return rc;
    }

    remove(code_file); // clean up the code file after reading, makeing sure it's single use for protection

    rc = code_to_tokens(code, client_id, redirect_uri, code_verifier_str, tokens_output);
    if (rc != OK){
        //log_msg("[spotify_pkce] Error exchanging code for tokens: %d", rc);
        return rc;
    }
    
    //log_msg("[spotify_pkce] successful authentication!");
    return OK;  
}


static int spotify_api_get(const char* access_token, const char* endpoint, cJSON** response_out) {
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

// **************** end of pkce implementation
// **************** start of integration test code, feel free to modify as needed for your testing purposes

// TESTS FOR GETTING TOP TRACKS AND ARTISTS, ALSO SHOWS HOW TO USE THE TOKENS TO MAKE API CALLS
// run gcc -Wall -Wextra -o integration integration.c     $(curl-config --libs)     -Ivendor/dotenv-c/include -Lvendor/dotenv-c/lib  -I/workspaces/CSE115-Stats-in-Term/include/ -I/workspaces/CSE115-Stats-in-Term/lib  -lssl -lcrypto -lcjson -ldotenv on codespaces


// static int get_top_items(const char* access_token, const char* type) {
//     // type is either "tracks" or "artists"
//     char endpoint[128];
//     snprintf(endpoint, sizeof(endpoint), "me/top/%s?limit=50&time_range=medium_term", type);

//     cJSON* response = NULL;
//     int rc = spotify_api_get(access_token, endpoint, &response);
//     if (rc != OK) return rc;

//     const cJSON* items = cJSON_GetObjectItemCaseSensitive(response, "items");
//     if (!cJSON_IsArray(items)) {
//         cJSON_Delete(response);
//         return JSON_ERR;
//     }

//     printf("\nTop %s:\n", type);
//     printf("----------------------------\n");

//     int rank = 1;
//     const cJSON* item = NULL;
//     cJSON_ArrayForEach(item, items) {

//         if (strcmp(type, "tracks") == 0) {
//             const cJSON* name       = cJSON_GetObjectItemCaseSensitive(item, "name");
//             const cJSON* popularity = cJSON_GetObjectItemCaseSensitive(item, "popularity");
//             const cJSON* duration   = cJSON_GetObjectItemCaseSensitive(item, "duration_ms");
//             const cJSON* artists    = cJSON_GetObjectItemCaseSensitive(item, "artists");
//             const cJSON* album      = cJSON_GetObjectItemCaseSensitive(item, "album");
//             const cJSON* album_name = cJSON_GetObjectItemCaseSensitive(album, "name");

//             // artists is an array — collect all artist names
//             char artist_names[512] = {0};
//             const cJSON* artist = NULL;
//             cJSON_ArrayForEach(artist, artists) {
//                 const cJSON* artist_name = cJSON_GetObjectItemCaseSensitive(artist, "name");
//                 if (cJSON_IsString(artist_name)) {
//                     if (artist_names[0]) strncat(artist_names, ", ", sizeof(artist_names) - strlen(artist_names) - 1);
//                     strncat(artist_names, artist_name->valuestring, sizeof(artist_names) - strlen(artist_names) - 1);
//                 }
//             }

//             int mins = cJSON_IsNumber(duration) ? (int)(duration->valuedouble / 60000) : 0;
//             int secs = cJSON_IsNumber(duration) ? (int)(duration->valuedouble / 1000) % 60 : 0;

//             printf("%2d. %-40s | %-30s | %-30s | %d:%02d | popularity: %d\n",
//                 rank,
//                 cJSON_IsString(name)       ? name->valuestring       : "unknown",
//                 artist_names[0]            ? artist_names            : "unknown",
//                 cJSON_IsString(album_name) ? album_name->valuestring : "unknown",
//                 mins, secs,
//                 cJSON_IsNumber(popularity) ? (int)popularity->valuedouble : 0);

//         } else if (strcmp(type, "artists") == 0) {
//             const cJSON* name       = cJSON_GetObjectItemCaseSensitive(item, "name");
//             const cJSON* popularity = cJSON_GetObjectItemCaseSensitive(item, "popularity");
//             const cJSON* followers  = cJSON_GetObjectItemCaseSensitive(item, "followers");
//             const cJSON* follower_count = cJSON_GetObjectItemCaseSensitive(followers, "total");
//             const cJSON* genres     = cJSON_GetObjectItemCaseSensitive(item, "genres");

//             // genres is an array — collect all genres
//             char genre_str[512] = {0};
//             const cJSON* genre = NULL;
//             cJSON_ArrayForEach(genre, genres) {
//                 if (cJSON_IsString(genre)) {
//                     if (genre_str[0]) strncat(genre_str, ", ", sizeof(genre_str) - strlen(genre_str) - 1);
//                     strncat(genre_str, genre->valuestring, sizeof(genre_str) - strlen(genre_str) - 1);
//                 }
//             }

//             printf("%2d. %-30s | followers: %-10d | popularity: %-3d | genres: %s\n",
//                 rank,
//                 cJSON_IsString(name)           ? name->valuestring                       : "unknown",
//                 cJSON_IsNumber(follower_count)  ? (int)follower_count->valuedouble        : 0,
//                 cJSON_IsNumber(popularity)      ? (int)popularity->valuedouble            : 0,
//                 genre_str[0]                   ? genre_str                               : "none");
//         }

//         rank++;
//     }

//     cJSON_Delete(response);
//     return OK;
// }

// int main(int argc, char *argv[]) {

//     //sample taken from https://github.com/Isty001/dotenv-c
//     env_load("/workspaces/CSE115-Stats-in-Term/.env", false); 
//     const char* client_id = getenv("SPOTIFY_CLIENT_ID");
//     const char* redirect_uri = getenv("SPOTIFY_REDIRECT_URI");


//     curl_global_init(CURL_GLOBAL_DEFAULT);

//     const char *code_file    = (argc >= 4) ? argv[3] : NULL;
//     const char *scopes       = (argc >= 5) ? argv[4] : NULL;

//     spotify_tokens tok = {0};
//     int rc = pkce_auth(client_id, redirect_uri, scopes, code_file, &tok);

//     switch (rc) {
//     case OK:
//         printf("Authenticated successfully!\n");

//         get_top_items(tok.access_token, "tracks");
//         get_top_items(tok.access_token, "artists");

//         free_tokens(&tok);
//         break;
//     case CRYPTO_ERR:  fprintf(stderr, "Error: PKCE crypto failed.\n");           break;
//     case CURL_ERR:    fprintf(stderr, "Error: libcurl request failed.\n");        break;
//     case AUTH_ERR:    fprintf(stderr, "Error: Spotify rejected the request.\n");  break;
//     case JSON_ERR:    fprintf(stderr, "Error: Could not parse token response.\n");break;
//     case TIMEOUT_ERR: fprintf(stderr, "Error: Timed out waiting for auth code.\n");break;
//     case NO_MEM_ERR:   fprintf(stderr, "Error: Out of memory.\n");                 break;
//     case IO_ERR:      fprintf(stderr, "Error: File I/O failure.\n");              break;
//     default:                  fprintf(stderr, "Error: Unknown (%d).\n", rc);              break;
//     }

//     curl_global_cleanup();
//     return (rc == OK) ? 0 : 1;
// }
