#include "parse_db_funcs.h"
// install cJSON : sudo apt install libcjson-dev
//install sqlite3 : sudo apt install sqlite3


// NOTE: TO LOAD THIS, YOU NEED TO LINK USING -lcjson -lsqlite3 flags

void free_song_list(song_list* list) {
    for (int i = 0; i < list->num_songs; i++) {
        free(list->songs[i].artist);
        free(list->songs[i].track);
        free(list->songs[i].album);
        free(list->songs[i].timestamp);
        free(list->songs[i].track_uri);
    }
    free(list->songs);
    list->songs = NULL;
    list->num_songs = 0;
}

int create_db()
{

    // function to initialize db
    sqlite3 *database ; // database initialization
    int rem_con ; // to establish the remote connection

    // open db -- create if it doesn't exist
    rem_con = sqlite3_open("spotifyHistory.db", &database);
    
    if (rem_con != 0) { // if it can't open!
        fprintf(stderr, "Error: %s \n", sqlite3_errmsg(database)) ; 
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
        fprintf(stderr, "Error in database: %s \n", sqlite3_errmsg(database)) ; 
        database = NULL ;
        sqlite3_close(database) ; 
        return 1 ;
    }

    return 0 ; 
}

char* read_json (char* file)
{
    if (!file) return NULL ;
    FILE *f = fopen(file, "rb") ;
    if (!f) return NULL ;

    // 1) get len of file
    if (fseek(f, 0, SEEK_END) != 0) {fclose(f) ; return NULL ;}
    
    long file_len = ftell(f) ; 
    if (file_len < 0)  {fclose(f) ; return NULL ;}

    // 2) set the pointer back to the start 
    if (fseek(f, 0, SEEK_SET) != 0) {fclose(f) ; return NULL ;}

    // 3) save enough mem to go through full file & read the full file
    char* load_data = malloc (file_len + 1) ; 
    if (!load_data) {fclose(f) ; return NULL ;}
    if (fread(load_data, 1, (size_t) file_len, f) != (size_t) file_len) {free(load_data) ; fclose(f) ; return NULL ;}

    // 4) close and exit properly
    fclose(f) ;
    load_data[file_len] = '\0' ; 

    // 5) return the data ! 
    return load_data ; 
}

int json_import_to_db(sqlite3* database, char* file_name)
{
    if (!database || !file_name) return 1 ;

    char* json_data = read_json(file_name) ;
    if (!json_data) return 1; 

    cJSON* root = cJSON_Parse(json_data) ; 
    if (!root) 
    {
        free(json_data) ; 
        return 1; 
    }
    // 1) Set up the command to add stuff to the sql
    sqlite3_stmt* cmd = NULL; 
    const char* sql_cmd = "INSERT INTO spotifyHistory (artist, track, album, ms_played, timestamp, track_uri) VALUES (?, ?, ?, ?, ?, ?);" ; 
    int rem_con = sqlite3_prepare_v2(database, sql_cmd, -1, &cmd, NULL) ; 
    
    if (rem_con != 0) { // if it can't open!
        cJSON_Delete(root) ;
        free(json_data) ;
        return 1; 
    }
        
    // 2) start parsing!
    rem_con = sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL) ; 
    if (rem_con != 0) { // if it can't open!
        sqlite3_finalize(cmd) ;
        cJSON_Delete(root) ;
        free(json_data) ;
        return 1; 
    }
    bool success = true ; 
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

        rem_con = sqlite3_step(cmd) ; 
        if (rem_con != SQLITE_DONE) {
            sqlite3_exec(database, "ROLLBACK;", NULL, NULL, NULL) ; // reset everything if failed!!!
            success = false ; 
            break ; 
        }
        sqlite3_reset(cmd) ; 

    }

    // close cleanly
    if (success) sqlite3_exec(database, "COMMIT;", NULL, NULL, NULL) ; 
    sqlite3_finalize(cmd) ; 
    cJSON_Delete(root) ; 
    free(json_data) ; 

    return 0 ; 

}


song_list get_all_songs_played_for_artist(sqlite3* database, char* artist_name)
{
    // goal: this function is used to get the unique artist names and then count the amount of songs the user has listened to for that specific artist
    sqlite3_stmt* cmd = NULL ;
    const char* get_unique_artist_cmd = "SELECT * FROM spotifyHistory WHERE artist = ? COLLATE NOCASE;" ; 

    // initialize song list and song info struct to store the songs that match the query
    song_list list = { .songs = NULL, .num_songs = 0 } ;

    if (sqlite3_prepare_v2(database, get_unique_artist_cmd, -1, &cmd, NULL) != 0) return list;

    sqlite3_bind_text(cmd, 1, artist_name, -1, SQLITE_TRANSIENT); 

    while (sqlite3_step(cmd) == SQLITE_ROW)
    {
        int count = list.num_songs + 1 ; 
        song_info* temp = list.songs = realloc(list.songs, count * sizeof(song_info)) ;
        if (!temp) break ; 
        list.songs = temp ;
        list.num_songs = count ;

        song_info* info = &list.songs[list.num_songs - 1] ;
        info->artist = strdup((char*) sqlite3_column_text(cmd, 0)) ;
        info->track = strdup((char*) sqlite3_column_text(cmd, 1)) ;
        info->album = strdup((char*) sqlite3_column_text(cmd, 2)) ;
        info->ms_played = sqlite3_column_int(cmd, 3) ;
        info->timestamp = strdup((char*) sqlite3_column_text(cmd, 4)) ;
        info->track_uri = strdup((char*) sqlite3_column_text(cmd, 5)) ;
            
    }
        
    sqlite3_finalize(cmd) ;
    return list ;

}
