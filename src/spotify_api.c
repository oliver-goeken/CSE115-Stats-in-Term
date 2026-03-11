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

extern char** environ;

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
