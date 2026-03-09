#ifndef INTEGRATION_H
#define INTEGRATION_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <cJSON.h>
#include "dotenv.h"

#ifdef _WIN32
#  include <windows.h>
#  define sleep(s) Sleep((s) * 1000)
#else
#  include <unistd.h>
#endif


#define SPOTIFY_AUTH_URL   "https://accounts.spotify.com/authorize"
#define SPOTIFY_TOKEN_URL  "https://accounts.spotify.com/api/token"
#define DEFAULT_SCOPES     "user-top-read"
#define WAIT_FOR_CODE_TIME_MS     300  
#define POLL_INTERVAL_MS   500

#define OK            0
#define CRYPTO_ERR   -1
#define CURL_ERR     -2
#define AUTH_ERR     -3
#define JSON_ERR     -4
#define TIMEOUT_ERR  -5
#define NO_MEM_ERR   -6
#define IO_ERR       -7

typedef struct {
    char *access_token;
    char *refresh_token;
    char *token_type;
    int   expires_in;
} spotify_tokens;

typedef struct {
    char* data;
    size_t len;
} curl_buffer;

void free_tokens(spotify_tokens* token_list);

// PKCE conversion functions, these are static, so they are only used in the integration.c file
// URL encoding and authorization section, static
// open URL on user's browser, read codes, write to CUrl buffer, static


int write_code_file(const char* code_file, const char* code);

// pkce authentication
int pkce_auth(const char* client_id, const char* redirect_uri, const char* scopes, const char* code_file, spotify_tokens* tokens_output);

#endif