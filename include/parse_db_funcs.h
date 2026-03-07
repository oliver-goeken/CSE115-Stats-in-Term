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

typedef struct artist {
	char* name;
	int num_plays;
} artist;

typedef struct artist_list {
	artist* root;
	int len;
} artist_list ;


typedef struct album {
	char* name;
	char* artist; // changed to string for easier parsing
	int num_plays;
} album;

typedef struct album_list {
	album* root;
	int len;
} album_list ;


typedef struct{
	char* name;
	char* album;
	char* artist;
	char* track_uri;
	int num_plays;
} track;

typedef struct track_list {
	track* root;
	int len;
} track_list ;


typedef struct {
	char* artist;
    char* track ;
    char* album ;
    int ms_played ;
    char* timestamp ;
} song_info ;

typedef struct {
    song_info* songs ;
    int num_songs ;
} song_list ; 


void free_song_list(song_list* list) ;
void free_album_list(album_list list) ;
void free_artist_list(artist_list list) ;
void free_track_list(track_list list) ;

int create_db(sqlite3 *database) ; 

char* read_json (char* file) ; 

int json_import_to_db(sqlite3* database, char* file) ; 

int json_import_directory(sqlite3* database, char* path);

song_list get_all_songs_played_for_artist(sqlite3* database, char* artist_name) ;
int get_num_songs_played_for_song(sqlite3* database, char* song_name, char* artist_name) ;
song_list get_all_listens_from_album(sqlite3* database, char* artist_name, char* album_name) ;
void sql_change_timestamp_format(sqlite3* database) ;


// sort 
artist_list get_top_artists(sqlite3* database);
artist_list get_top_artists_limit(sqlite3* database, int limit);
artist_list get_bottom_artists_limit(sqlite3* database, int limit);

album_list get_top_albums(sqlite3* database);
album_list get_top_albums_limit(sqlite3* database, int limit);
album_list get_bottom_albums_limit(sqlite3* database, int limit);

track_list get_top_tracks(sqlite3* database);
track_list get_top_tracks_limit(sqlite3* database, int limit);
track_list get_bottom_tracks_limit(sqlite3* database, int limit);

song_list get_listening_history(sqlite3* database);
song_list get_listening_history_limit(sqlite3* database, int limit);


// search
// returns appropriate items from listening history
song_list search_track(sqlite3* database, char* track_name);

song_list search_album(sqlite3* database, char* album_name);

song_list search_artist(sqlite3* database, char* artist_name);

#endif
