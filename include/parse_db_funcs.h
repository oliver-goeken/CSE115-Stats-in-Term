#ifndef PARSE_DB_FUNCS_H
#define PARSE_DB_FUNCS_H
#define _POSIX_C_SOURCE 200809L 
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cJSON.h"
#include <sqlite3.h>
#include "sqlite3.h"
// note : you have to install the cJSON package: sudo apt install libcjson-dev
//install sqlite3 : sudo apt install sqlite3

typedef struct {
    char* artist ;
    char* track ;
    char* album ;
    int ms_played ;
    char* timestamp ;
    char* track_uri ;
} song_info ;

typedef struct {
    song_info* songs ;
    int num_songs ;
} song_list ; 

void free_song_list(song_list* list) ;

int create_db(); 

char* read_json (char* file) ; 

int json_import_to_db(sqlite3* database, char* file) ; 

song_list get_total_played_per_artist(sqlite3* database, char* artist_name) ;
#endif
