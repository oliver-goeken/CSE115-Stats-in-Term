#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_db_funcs.h"
// install cJSON : sudo apt install libcjson-dev
//install sqlite3 : sudo apt install sqlite3


// NOTE: TO LOAD THIS, YOU NEED TO LINK USING -lcjson -lsqlite3 flags

int create_db()
{

    // function to initialize db
    sqlite3 *database ; // database initialization
    int rem_con ; // to establish the remote connection

    // open db -- create if it doesn't exist
    rem_con = sqlite3_open("spotifyHistory.db", &database);
    
    if (rem_con != 0) { // if it can't open!
        fprintf(stderr, "Error: %s \n", sqlite_errmsg(&database)) ; 
        sqlite3_close(database) ; 
        return 1 ; 
    }

    const char *sql_cmd = "CREATE TABLE IF NOT EXISTS spotifyHistory ("
                        "artist TEXT, "
                        "track TEXT, "
                        "album TEXT, "
                        "ms_played INTEGER, " // milliseconds
                        "timestamp TEXT, "
                        "track_uri TEXT);" ;

    char* error_msg = 0 ; 
    rem_con = sqlite3_exec(database, sql_cmd, 0, 0, &error_msg) ; 

    // check connection again
    if (rem_con != 0)
    {
        fprintf(stderr, "Error in database: %s \n", sqlite_errmsg(&database)) ; 
        sqlite3_close(database) ; 
        return 1 ;
    }

    return 0 ; 
}

char* read_json (const char* file)
{
    FILE *f = fopen(file, "rb") ; 

    // 1) get len of file
    fseek(f, 0, SEEK_END) ; 
    long file_len = ftell(f) ; 

    // 2) set the pointer back to the start 
    fseek(f, 0, SEEK_SET) ; 

    // 3) save enough mem to go through full file & read the full file
    char* load_data = malloc (file_len + 1) ; 
    fread(load_data, 1, file_len, f) ; 

    // 4) close and exit properly
    fclose(f) ;
    load_data[file_len] = '\0' ; 

    // 5) return the data ! 
    return load_data ; 
}

void json_import_to_db(sqlite3* database, const char* json_file)
{
    cJSON* root = cJSON_Parse(json_file) ; if (!root) return ; 

    // 1) Set up the command to add stuff to the sql
    sqlite3_stmt* cmd ; 
    const char* sql_cmd = "INSERT INTO spotifyHistory (artist, track, album, ms_played, timestamp, track_uri) VALUES (?, ?, ?, ?, ?, ?);" ; 
    sqlite3_prepare_v2(database, sql_cmd, -1, &cmd, NULL) ; 
    
    // 2) start parsing!
    sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL) ; 
    cJSON* elem = NULL ; 
    cJSON_ArrayForEach (elem, root)
    {

        // NOTE: this starts off with the ms_played because that's the order it appears in the cJSON
        cJSON* ts = cJSON_GetObjectItemCaseSensitive(elem, "ts") ; 
        cJSON* ms_played = cJSON_GetObjectItemCaseSensitive(elem, "ms_played"); 
        cJSON* track = cJSON_GetObjectItemCaseSensitive(elem, "master_metadata_track_name") ; 
        cJSON* artist = cJSON_GetObjectItemCaseSensitive(elem, "master_metadata_album_artist_name") ; 
        cJSON* album = cJSON_GetObjectItemCaseSensitive(elem, "master_metadata_album_album_name") ; 
        cJSON* track_uri = cJSON_GetObjectItemCaseSensitive(elem, "spotify_track_uri"); 

        // bind each to the ?s
        sqlite3_bind_text(cmd, 1, (artist && artist->valuestring) ? artist->valuestring : "Unknown", -1, SQLITE_STATIC) ; 
        sqlite3_bind_text(cmd, 2, (track && track->valuestring) ? track->valuestring : "Unknown", -1, SQLITE_STATIC) ; 
        sqlite3_bind_text(cmd, 3, (album && album->valuestring) ? album->valuestring : "Unknown", -1, SQLITE_STATIC) ; 
        sqlite3_bind_int(cmd, 4, ms_played ? ms_played->valueint : 0) ;
        sqlite3_bind_text(cmd, 5, (ts && ts->valuestring) ? ts->valuestring : "", -1, SQLITE_STATIC) ;  
        sqlite3_bind_text(cmd, 6, (track_uri && track_uri->valuestring) ? track_uri->valuestring : "", -1, SQLITE_STATIC) ; 

        sqlite3_step(cmd) ; 
        sqlite3_reset(cmd) ; 

    }

    // close cleanly
    sqlite3_exec(database, "COMMIT;", NULL, NULL, NULL) ; 
    sqlite3_finalize(cmd) ; 
    cJSON_Delete(root) ; 

}

