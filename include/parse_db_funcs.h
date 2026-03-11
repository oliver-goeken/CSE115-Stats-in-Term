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

int json_import_directory(sqlite3* database, const char* path);

song_list get_all_songs_played_for_artist(sqlite3* database, char* artist_name) ;
int get_num_songs_played_for_song(sqlite3* database, char* song_name, char* artist_name) ;
song_list get_all_listens_from_album(sqlite3* database, char* artist_name, char* album_name) ;
void sql_change_timestamp_format(sqlite3* database) ;


// sort 
/*
artist_list get_top_artists			(sqlite3* database);
artist_list get_top_artists_limit	(sqlite3* database, int limit);
artist_list get_bottom_artists_limit(sqlite3* database, int limit);

album_list get_top_albums		  (sqlite3* database);
album_list get_top_albums_limit	  (sqlite3* database, int limit);
album_list get_bottom_albums_limit(sqlite3* database, int limit);

track_list get_top_tracks		  (sqlite3* database);
track_list get_top_tracks_limit   (sqlite3* database, int limit);
track_list get_bottom_tracks_limit(sqlite3* database, int limit);
*/
song_list get_listening_history		 (sqlite3* database);
song_list get_listening_history_limit(sqlite3* database, int limit);



// New Sorting
album_list  get_albums_sorted (sqlite3* db, const char* where_clause, const char* order_by, int limit);
artist_list get_artists_sorted(sqlite3* db, const char* where_clause, const char* order_by, int limit);
track_list  get_tracks_sorted (sqlite3* db, const char* where_clause, const char* order_by, int limit);

// Tracks
// Tracks — Number of Listens
track_list get_top_tracks		  (sqlite3* db);
track_list get_top_tracks_limit   (sqlite3* db, int limit);
track_list get_bottom_tracks_limit(sqlite3* db, int limit);
track_list tracks_by_album		  (sqlite3* db);

// Tracks — Date Listened
track_list get_recent_tracks		(sqlite3* db);
track_list get_recent_tracks_limit  (sqlite3* db, int limit);
track_list get_earliest_tracks_limit(sqlite3* db, int limit);

// Tracks — Alphabetical
track_list get_alpha_tracks			 (sqlite3* db);
track_list get_alpha_tracks_limit	 (sqlite3* db, int limit);
track_list get_rev_alpha_tracks_limit(sqlite3* db, int limit);


// Albums
// Albums — Number of Listens
album_list get_top_albums		  (sqlite3* db);
album_list get_top_albums_limit	  (sqlite3* db, int limit);
album_list get_bottom_albums_limit(sqlite3* db, int limit);

// Albums — Date Listened
album_list get_recent_albums		(sqlite3* db);
album_list get_recent_albums_limit  (sqlite3* db, int limit);
album_list get_earliest_albums_limit(sqlite3* db, int limit);

// Albums — Alphabetical
album_list get_alpha_albums			 (sqlite3* db);
album_list get_alpha_albums_limit	 (sqlite3* db, int limit);
album_list get_rev_alpha_albums_limit(sqlite3* db, int limit);


// Artists
// Artists — Number of Listens
artist_list get_top_artists			(sqlite3* db);
artist_list get_top_artists_limit	(sqlite3* db, int limit);
artist_list get_bottom_artists_limit(sqlite3* db, int limit);

// Artists — Date Listened
artist_list get_recent_artists		  (sqlite3* db);
artist_list get_recent_artists_limit  (sqlite3* db, int limit);
artist_list get_earliest_artists_limit(sqlite3* db, int limit);

// Artists — Alphabetical
artist_list get_alpha_artists		   (sqlite3* db);
artist_list get_alpha_artists_limit	   (sqlite3* db, int limit);
artist_list get_rev_alpha_artists_limit(sqlite3* db, int limit);


// search
// returns appropriate items from listening history
song_list search_track(sqlite3* database, char* track_name);

song_list search_album(sqlite3* database, char* album_name);

song_list search_artist(sqlite3* database, char* artist_name);

artist_list search_artists_by_name(sqlite3* database, const char* query, int limit);

album_list search_albums_by_name(sqlite3* database, const char* query, int limit);

track_list search_tracks_by_name(sqlite3* database, const char* query, int limit);

track_list get_top_tracks_for_artist_limit(sqlite3* database, const char* artist_name, int limit);

track_list get_top_tracks_for_album_limit(sqlite3* database, const char* album_name, const char* artist_name, int limit);

album_list get_top_albums_for_artist_limit(sqlite3* database, const char* artist_name, int limit);

char* get_track_uri_for_song(sqlite3* database, const char* track_name, const char* album_name, const char* artist_name);

#endif
