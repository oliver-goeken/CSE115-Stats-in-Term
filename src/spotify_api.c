#include "spotify_api.h"

#include "cJSON.h"
#include "log.h"

#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <ctype.h> //catlinh

extern char** environ;

static int spotify_api_read_command_output(char* const argv[], char** out_buffer);

static void spotify_track_metadata_init(spotify_track_metadata* metadata)
{
    if (metadata == NULL) {
        return;
    }

    memset(metadata, 0, sizeof(*metadata));
}

void spotify_track_metadata_free(spotify_track_metadata* metadata)
{
    if (metadata == NULL) {
        return;
    }

    free(metadata->track_id);
    free(metadata->track_name);
    free(metadata->album_name);
    free(metadata->artist_name);
    free(metadata->track_uri);
    free(metadata->preview_url);

    spotify_track_metadata_init(metadata);
}

static char* spotify_strdup_json_string(cJSON* object, const char* field_name)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(object, field_name);
    if (!cJSON_IsString(item) || item->valuestring == NULL) {
        return NULL;
    }

    return strdup(item->valuestring);
}

bool spotify_track_uri_get_id(const char* track_uri, char* out_track_id, int out_size)
{
    const char* prefix = "spotify:track:";
    size_t prefix_len = strlen(prefix);

    if (track_uri == NULL || out_track_id == NULL || out_size <= 1) {
        return false;
    }

    if (strncmp(track_uri, prefix, prefix_len) != 0) {
        return false;
    }

    const char* track_id = track_uri + prefix_len;
    size_t track_id_len = strlen(track_id);

    if (track_id_len == 0 || track_id_len >= (size_t)out_size) {
        return false;
    }

    for (size_t i = 0; i < track_id_len; i++) {
        char ch = track_id[i];
        bool valid =
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9');

        if (!valid) {
            return false;
        }
    }

    memcpy(out_track_id, track_id, track_id_len + 1);
    return true;
}

const char* spotify_api_get_access_token(void)
{
    return getenv("SPOTIFY_ACCESS_TOKEN");
}

static void spotify_api_safe_copy(char* dst, int dst_size, const char* src)
{
    if (dst == NULL || dst_size <= 0) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    snprintf(dst, (size_t)dst_size, "%s", src);
}

static void spotify_api_url_encode(const char* src, char* dst, int dst_size)
{
    static const char hex[] = "0123456789ABCDEF";

    if (dst == NULL || dst_size <= 0) {
        return;
    }

    dst[0] = '\0';

    if (src == NULL) {
        return;
    }

    int j = 0;
    for (int i = 0; src[i] != '\0' && j < dst_size - 1; i++) {
        unsigned char ch = (unsigned char)src[i];
        bool unreserved =
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_' || ch == '.' || ch == '~';

        if (unreserved) {
            dst[j++] = (char)ch;
        } else {
            if (j + 3 >= dst_size) {
                break;
            }
            dst[j++] = '%';
            dst[j++] = hex[(ch >> 4) & 0xF];
            dst[j++] = hex[ch & 0xF];
        }
    }

    dst[j] = '\0';
}

bool spotify_api_has_auth_env(void)
{
    const char* client_id = getenv("SPOTIFY_CLIENT_ID");
    const char* client_secret = getenv("SPOTIFY_CLIENT_SECRET");
    const char* redirect_uri = getenv("SPOTIFY_REDIRECT_URI");

    return
        client_id != NULL && client_id[0] != '\0' &&
        client_secret != NULL && client_secret[0] != '\0' &&
        redirect_uri != NULL && redirect_uri[0] != '\0';
}

void spotify_api_build_auth_url(char* out_url, int out_size)
{
    const char* client_id = getenv("SPOTIFY_CLIENT_ID");
    const char* redirect_uri = getenv("SPOTIFY_REDIRECT_URI");

    const char* scope =
        "user-read-private "
        "user-read-email "
        "user-top-read "
        "user-read-recently-played";

    char encoded_redirect[1024];
    char encoded_scope[1024];

    if (out_url == NULL || out_size <= 0) {
        return;
    }

    out_url[0] = '\0';

    if (client_id == NULL || client_id[0] == '\0' ||
        redirect_uri == NULL || redirect_uri[0] == '\0') {
        return;
    }

    spotify_api_url_encode(redirect_uri, encoded_redirect, (int)sizeof(encoded_redirect));
    spotify_api_url_encode(scope, encoded_scope, (int)sizeof(encoded_scope));

    snprintf(
        out_url,
        (size_t)out_size,
        "https://accounts.spotify.com/authorize"
        "?response_type=code"
        "&client_id=%s"
        "&redirect_uri=%s"
        "&scope=%s",
        client_id,
        encoded_redirect,
        encoded_scope
    );
}

int spotify_api_exchange_code_for_token(const char* auth_code, char* out_error, int out_error_size)
{
    const char* client_id = getenv("SPOTIFY_CLIENT_ID");
    const char* client_secret = getenv("SPOTIFY_CLIENT_SECRET");
    const char* redirect_uri = getenv("SPOTIFY_REDIRECT_URI");

    char credentials[1024];
    char code_arg[2048];
    char redirect_arg[2048];
    char* response_json = NULL;

    if (out_error != NULL && out_error_size > 0) {
        out_error[0] = '\0';
    }

    if (auth_code == NULL || auth_code[0] == '\0') {
        spotify_api_safe_copy(out_error, out_error_size, "Missing authorization code.");
        return SPOTIFY_API_ERR_INVALID_ARG;
    }

    if (client_id == NULL || client_id[0] == '\0' ||
        client_secret == NULL || client_secret[0] == '\0' ||
        redirect_uri == NULL || redirect_uri[0] == '\0') {
        spotify_api_safe_copy(
            out_error,
            out_error_size,
            "Missing SPOTIFY_CLIENT_ID, SPOTIFY_CLIENT_SECRET, or SPOTIFY_REDIRECT_URI."
        );
        return SPOTIFY_API_ERR_INVALID_ARG;
    }

    snprintf(credentials, sizeof(credentials), "%s:%s", client_id, client_secret);
    snprintf(code_arg, sizeof(code_arg), "code=%s", auth_code);
    snprintf(redirect_arg, sizeof(redirect_arg), "redirect_uri=%s", redirect_uri);

    char* curl_argv[] = {
        "curl",
        "-sS",
        "-u",
        credentials,
        "-X",
        "POST",
        "https://accounts.spotify.com/api/token",
        "-d",
        "grant_type=authorization_code",
        "--data-urlencode",
        code_arg,
        "--data-urlencode",
        redirect_arg,
        NULL
    };

    int rc = spotify_api_read_command_output(curl_argv, &response_json);
    if (rc != SPOTIFY_API_OK || response_json == NULL) {
        spotify_api_safe_copy(out_error, out_error_size, "Failed to contact Spotify token endpoint.");
        free(response_json);
        return SPOTIFY_API_ERR_HTTP;
    }

    cJSON* root = cJSON_Parse(response_json);
    if (root == NULL) {
        spotify_api_safe_copy(out_error, out_error_size, "Spotify returned invalid JSON.");
        free(response_json);
        return SPOTIFY_API_ERR_PARSE;
    }

    cJSON* access_token = cJSON_GetObjectItemCaseSensitive(root, "access_token");
    cJSON* error_item = cJSON_GetObjectItemCaseSensitive(root, "error");
    cJSON* error_desc = cJSON_GetObjectItemCaseSensitive(root, "error_description");

    if (!cJSON_IsString(access_token) || access_token->valuestring == NULL) {
        if (cJSON_IsString(error_desc) && error_desc->valuestring != NULL) {
            spotify_api_safe_copy(out_error, out_error_size, error_desc->valuestring);
        } else if (cJSON_IsString(error_item) && error_item->valuestring != NULL) {
            spotify_api_safe_copy(out_error, out_error_size, error_item->valuestring);
        } else {
            spotify_api_safe_copy(out_error, out_error_size, "Spotify did not return an access token.");
        }

        cJSON_Delete(root);
        free(response_json);
        return SPOTIFY_API_ERR_PARSE;
    }

    setenv("SPOTIFY_ACCESS_TOKEN", access_token->valuestring, 1);

    cJSON_Delete(root);
    free(response_json);
    return SPOTIFY_API_OK;
}

static int spotify_api_read_command_output(char* const argv[], char** out_buffer)
{
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return SPOTIFY_API_ERR_HTTP;
    }

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addclose(&actions, pipefd[0]);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipefd[1]);

    pid_t pid = 0;
    int spawn_rc = posix_spawnp(&pid, argv[0], &actions, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&actions);
    close(pipefd[1]);

    if (spawn_rc != 0) {
        close(pipefd[0]);
        return SPOTIFY_API_ERR_HTTP;
    }

    size_t capacity = 1024;
    size_t length = 0;
    char* buffer = malloc(capacity);
    if (buffer == NULL) {
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        return SPOTIFY_API_ERR_HTTP;
    }

    while (true) {
        if (length + 512 + 1 > capacity) {
            capacity *= 2;
            char* new_buffer = realloc(buffer, capacity);
            if (new_buffer == NULL) {
                free(buffer);
                close(pipefd[0]);
                waitpid(pid, NULL, 0);
                return SPOTIFY_API_ERR_HTTP;
            }
            buffer = new_buffer;
        }

        ssize_t bytes_read = read(pipefd[0], buffer + length, capacity - length - 1);
        if (bytes_read < 0) {
            free(buffer);
            close(pipefd[0]);
            waitpid(pid, NULL, 0);
            return SPOTIFY_API_ERR_HTTP;
        }
        if (bytes_read == 0) {
            break;
        }
        length += (size_t)bytes_read;
    }

    close(pipefd[0]);
    buffer[length] = '\0';

    int status = 0;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        free(buffer);
        return SPOTIFY_API_ERR_HTTP;
    }

    *out_buffer = buffer;
    return SPOTIFY_API_OK;
}

static int spotify_api_parse_track_metadata(
    const char* response_json,
    const char* track_id,
    spotify_track_metadata* out_metadata
)
{
    cJSON* root = cJSON_Parse(response_json);
    if (root == NULL) {
        return SPOTIFY_API_ERR_PARSE;
    }

    cJSON* album = cJSON_GetObjectItemCaseSensitive(root, "album");
    cJSON* artists = cJSON_GetObjectItemCaseSensitive(root, "artists");
    cJSON* first_artist = cJSON_IsArray(artists) ? cJSON_GetArrayItem(artists, 0) : NULL;

    spotify_track_metadata_init(out_metadata);

    out_metadata->track_id = strdup(track_id);
    out_metadata->track_name = spotify_strdup_json_string(root, "name");
    out_metadata->track_uri = spotify_strdup_json_string(root, "uri");
    out_metadata->preview_url = spotify_strdup_json_string(root, "preview_url");
    out_metadata->album_name = cJSON_IsObject(album) ? spotify_strdup_json_string(album, "name") : NULL;
    out_metadata->artist_name = cJSON_IsObject(first_artist) ? spotify_strdup_json_string(first_artist, "name") : NULL;

    cJSON* duration = cJSON_GetObjectItemCaseSensitive(root, "duration_ms");
    out_metadata->duration_ms = cJSON_IsNumber(duration) ? duration->valueint : 0;

    cJSON_Delete(root);

    if (out_metadata->track_id == NULL || out_metadata->track_name == NULL) {
        spotify_track_metadata_free(out_metadata);
        return SPOTIFY_API_ERR_PARSE;
    }

    return SPOTIFY_API_OK;
}

int spotify_api_fetch_track_metadata_with_token(
    const char* access_token,
    const char* track_uri,
    spotify_track_metadata* out_metadata
)
{
    char track_id[128];
    char auth_header[512];
    char url[256];
    char* response_json = NULL;

    if (access_token == NULL || track_uri == NULL || out_metadata == NULL) {
        return SPOTIFY_API_ERR_INVALID_ARG;
    }

    if (!spotify_track_uri_get_id(track_uri, track_id, (int)sizeof(track_id))) {
        return SPOTIFY_API_ERR_INVALID_URI;
    }

    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", access_token);
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/tracks/%s", track_id);

    char* curl_argv[] = {
        "curl",
        "-sS",
        "-H",
        auth_header,
        url,
        NULL
    };

    int rc = spotify_api_read_command_output(curl_argv, &response_json);
    if (rc != SPOTIFY_API_OK) {
        log_err("failed to fetch Spotify track metadata");
        return rc;
    }

    rc = spotify_api_parse_track_metadata(response_json, track_id, out_metadata);
    free(response_json);
    return rc;
}

int spotify_api_fetch_track_metadata(const char* track_uri, spotify_track_metadata* out_metadata)
{
    const char* access_token = spotify_api_get_access_token();
    if (access_token == NULL || access_token[0] == '\0') {
        return SPOTIFY_API_ERR_NO_TOKEN;
    }

    return spotify_api_fetch_track_metadata_with_token(access_token, track_uri, out_metadata);
}

//helper func for int panel
/*void spotify_print_auth_url(char* buffer, int size) {

    snprintf(buffer, size,
        "https://accounts.spotify.com/authorize?"
        "client_id=%s"
        "&response_type=code"
        "&redirect_uri=%s"
        "&scope=user-read-recently-played",
        SPOTIFY_CLIENT_ID,
        SPOTIFY_REDIRECT_URI
    );
}*/