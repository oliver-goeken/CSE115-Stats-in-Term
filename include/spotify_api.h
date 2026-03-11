#ifndef SPOTIFY_API_H
#define SPOTIFY_API_H

#include <stdbool.h>

typedef struct {
    char* track_id;
    char* track_name;
    char* album_name;
    char* artist_name;
    char* track_uri;
    char* preview_url;
    int duration_ms;
} spotify_track_metadata;

enum {
    SPOTIFY_API_OK = 0,
    SPOTIFY_API_ERR_INVALID_ARG = -1,
    SPOTIFY_API_ERR_INVALID_URI = -2,
    SPOTIFY_API_ERR_NO_TOKEN = -3,
    SPOTIFY_API_ERR_HTTP = -4,
    SPOTIFY_API_ERR_PARSE = -5
};

bool spotify_track_uri_get_id(const char* track_uri, char* out_track_id, int out_size);

const char* spotify_api_get_access_token(void);

int spotify_api_fetch_track_metadata(const char* track_uri, spotify_track_metadata* out_metadata);

int spotify_api_fetch_track_metadata_with_token(
    const char* access_token,
    const char* track_uri,
    spotify_track_metadata* out_metadata
);

void spotify_track_metadata_free(spotify_track_metadata* metadata);

//catlin
void spotify_api_build_auth_url(char* out_url, int out_size);
int spotify_api_exchange_code_for_token(const char* auth_code, char* out_error, int out_error_size);
bool spotify_api_has_auth_env(void);
#endif
