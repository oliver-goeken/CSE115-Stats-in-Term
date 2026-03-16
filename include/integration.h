#ifndef INTEGRATION_H
#define INTEGRATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "parse_db_funcs.h"

#ifdef _WIN32
#  include <windows.h>
#  define sleep(s) Sleep((s) * 1000)
#else
#  include <unistd.h>
#endif

#define SPOTIFY_TOKEN_URL  "https://accounts.spotify.com/api/token"
#define DEFAULT_SCOPES     "user-top-read"

#define OK            0
#define CURL_ERR     -2
#define AUTH_ERR     -3
#define JSON_ERR     -4
#define NO_MEM_ERR   -6
#define IO_ERR       -7

typedef struct {
    char *access_token;
    char *refresh_token;
    char *token_type;
    int   expires_in;
} spotify_tokens;

typedef struct {
    char*  data;
    size_t len;
} curl_buffer;

typedef struct{
	char* name;
	char* album;
	char* artist;
	char* track_uri;
} api_track;

typedef struct api_track_list {
	api_track* root;
	int len;
} api_track_list ;

void free_tokens(spotify_tokens* token_list);
int  load_tokens(const char* path, spotify_tokens* tokens_output);
int  spotify_api_get(const char* access_token, const char* endpoint, cJSON** response_out);
api_track_list api_get_top_tracks(const char* access_token);
void api_free_track_list(api_track_list list);
api_track_list open_auth();
#endif