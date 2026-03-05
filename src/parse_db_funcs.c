#include "parse_db_funcs.h"
#include "log.h"
#include "utils.h"
#include <time.h>
#include <sys/errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
// install cJSON : sudo apt install libcjson-dev
//install sqlite3 : sudo apt install sqlite3


// NOTE: TO LOAD THIS, YOU NEED TO LINK USING -lcjson -lsqlite3 flags

void free_song_list(song_list* list) {
    for (int i = 0; i < list->num_songs; i++) {
        free(list->songs[i].artist);
        free(list->songs[i].track);
        free(list->songs[i].album);
        free(list->songs[i].timestamp);
        free(list->songs[i].track);
    }
    free(list->songs);
    list->songs = NULL;
    list->num_songs = 0;
}
void free_album_list(album_list list) {
 for (int i = 0; i < list.len; i++) {
        free(list.root[i].name);
        free(list.root[i].artist);
    }
    free(list.root);
}

void free_artist_list(artist_list list)
{
    for (int i = 0; i < list.len; i++) {
        free(list.root[i].name);
    }
    free(list.root);
}

 
int create_db(sqlite3 *database)
{

    // function to initialize db
    // database initialization
    int rem_con ; // to establish the remote connection

    // open db -- create if it doesn't exist
    rem_con = sqlite3_open("spotifyHistory.db", &database);
    
    if (rem_con != 0) { // if it can't open!
        log_msg_detailed("Error opening database: ", __FILE__, __LINE__, sqlite3_errmsg(database)) ;
        database = NULL ;
        sqlite3_close(database) ; 
        return 1 ; 
    }

    const char *sql_cmd = "CREATE TABLE IF NOT EXISTS spotifyHistory ("
                        "artist VARCHAR, "
                        "track VARCHAR, "
                        "album VARCHAR, "
                        "ms_played INTEGER, " // milliseconds
                        "timestamp TEXT, "
                        "track_uri TEXT);" ;

    char* error_msg = 0 ; 
    rem_con = sqlite3_exec(database, sql_cmd, 0, 0, &error_msg) ;  // memory leak here maybe
    free(error_msg) ; 
    // check connection again
    if (rem_con != 0)
    {
        log_msg_detailed("Error in database: ", __FILE__, __LINE__, sqlite3_errmsg(database)) ;
        database = NULL ;
        sqlite3_close(database) ; 
        return 1 ;
    }

    return 0 ; 
}

char* read_json (char* file)
{
    if (!file) 
    {
        log_msg_detailed("Error: file is NULL", __FILE__, __LINE__, NULL) ;
        return NULL ;
    }
    FILE *f = fopen(file, "rb") ;
    if (!f) {
        //log_msg_detailed("Error: could not open file", __FILE__, __LINE__, NULL) ;
		log_err_f("could not open file: %s", strerror(errno));
        return NULL ;
    }

    // 1) get len of file
    if (fseek(f, 0, SEEK_END) != 0) 
    {
        log_msg_detailed("Error: could not seek to end of file", __FILE__, __LINE__, NULL) ;
        fclose(f) ; 
        return NULL ;
    }
    
    long file_len = ftell(f) ; 
    if (file_len < 0)
    {
        log_msg_detailed("Error: file length is negative", __FILE__, __LINE__, NULL) ;
        fclose(f) ; 
        return NULL ;
    }

    // 2) set the pointer back to the start 
    if (fseek(f, 0, SEEK_SET) != 0) 
    {
        log_msg_detailed("Error: start of file could not be set", __FILE__, __LINE__, NULL) ;
        fclose(f) ; 
        return NULL ;
    }

    // 3) save enough mem to go through full file & read the full file
    char* load_data = malloc (file_len + 1) ; 
    if (!load_data) 
    {
        log_msg_detailed("Error: memory for file could not be allocated", __FILE__, __LINE__, NULL) ;
        return NULL ;
    }
    if (fread(load_data, 1, (size_t) file_len, f) != (size_t) file_len) 
    {
        free(load_data) ; 
        log_msg_detailed("Error: data could not be read", __FILE__, __LINE__, NULL) ;
        fclose(f) ; 
        return NULL ;
    }

    // 4) close and exit properly
    fclose(f) ;
    load_data[file_len] = '\0' ; 

    // 5) return the data ! 
    return load_data ; 
}

int json_import_to_db(sqlite3* database, char* file_name)
{
	log_msg_f("attempting to load json file %s", file_name);

    if (!database || !file_name) 
    {
        log_msg_detailed("Error: database or file name is NULL", __FILE__, __LINE__, NULL) ;
        return 1 ; 
    }

    char* json_data = read_json(file_name) ;
    if (!json_data) 
    {
        log_msg_detailed("Error: JSON could not be read", __FILE__, __LINE__, NULL) ;
        free(json_data) ; 
        return 1 ; 
    }

    cJSON* root = cJSON_Parse(json_data) ; 
    if (!root) 
    {
        log_msg_detailed("Error: JSON could not be parsed", __FILE__, __LINE__, NULL) ;
        free(json_data) ;
        cJSON_Delete(root) ; 
        return 1; 
    }


    // 1) Set up the command to add stuff to the sql
    sqlite3_stmt* cmd = NULL; 
    const char* sql_cmd = "INSERT INTO spotifyHistory (artist, track, album, ms_played, timestamp, track_uri) VALUES (?, ?, ?, ?, ?, ?);" ; 
    int rem_con = sqlite3_prepare_v2(database, sql_cmd, -1, &cmd, NULL) ; 
    
    if (rem_con != 0) { // if it can't open!
        log_msg_detailed("Error: could not insert data into database", __FILE__, __LINE__, sqlite3_errmsg(database)) ;
        cJSON_Delete(root) ;
        free(json_data) ;
        return 1; 
    }
        
    // 2) start parsing!
    rem_con = sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL) ; 
    if (rem_con != 0) { // if it can't open!
        log_msg_detailed("Error: insert could not be executed", __FILE__, __LINE__, NULL) ;
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
            log_msg_detailed("Error: execute failed. Resetting table.", __FILE__, __LINE__, sqlite3_errmsg(database)) ;
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

    sql_change_timestamp_format(database) ;

    return 0 ; 

}

int json_import_directory(sqlite3* database, char* path){
	if (database == NULL){
		log_err("database does not exist");
		return -1;
	} if (path == NULL){
		log_err("path is empty");
		return -2;
	}

	if (!is_directory(path)){
		json_import_to_db(database, path);
		return -0;
	}

	log_msg_f("opening directory %s", path);

	DIR* json_dir = opendir(path);

	if (json_dir == NULL){
		log_err_f("error opening directory at %s", path);
		return -3;
	}

	int path_buff_size = 1012;
	char path_buffer[path_buff_size];

	struct dirent* dir_ent;
	while ((dir_ent = readdir(json_dir)) != NULL){
		memset(path_buffer, 0, path_buff_size);

		get_dir_path(path_buffer, path, path_buff_size);
		strncat(path_buffer, dir_ent->d_name, path_buff_size);

		if (is_file(path_buffer) && string_ends_with(path_buffer, ".json")){
			json_import_to_db(database, path_buffer);
		} else {
			log_msg_f("%s is not a file or is not a json file", dir_ent->d_name);
		}
	}

	closedir(json_dir);

	return 0;
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
        song_info* temp = realloc(list.songs, count * sizeof(song_info)) ;
        if (!temp) break ; 
        list.songs = temp ;
        list.num_songs = count ;

        song_info* info = &list.songs[list.num_songs - 1] ;
        info->artist = strdup((char*) sqlite3_column_text(cmd, 0)) ;
        info->track = strdup((char*) sqlite3_column_text(cmd, 1)) ;
        info->album = strdup((char*) sqlite3_column_text(cmd, 2)) ;
        info->ms_played = sqlite3_column_int(cmd, 3) ;
        info->timestamp = strdup((char*) sqlite3_column_text(cmd, 4)) ;
        info->track = strdup((char*) sqlite3_column_text(cmd, 5)) ;
            
    }
        
    sqlite3_finalize(cmd) ;
    return list ;

}

int song_total(sqlite3* database, char* song_name, char* artist_name)
{
    // goal: get total number of listens for a song
    sqlite3_stmt* cmd = NULL ;
    const char* get_song_count = "SELECT * FROM spotifyHistory WHERE track = ? COLLATE NOCASE AND artist = ? COLLATE NOCASE;" ;

    if (sqlite3_prepare_v2(database, get_song_count, -1, &cmd, NULL) != 0) 
    { 
        log_msg_detailed("Error preparing statement for song total query.", __FILE__, __LINE__, NULL) ;
        return -1;
    }

    sqlite3_bind_text(cmd, 1, song_name, -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(cmd, 2, artist_name, -1, SQLITE_TRANSIENT); 
    int count = 0 ;
    while (sqlite3_step(cmd) == SQLITE_ROW)
    {
        count++ ;
    }
    sqlite3_finalize(cmd) ;
    return count ;

}

song_list get_all_listens_from_album(sqlite3* database, char* artist_name, char* album_name)
{
    sqlite3_stmt* cmd = NULL ;
    const char* get_album_rows = "SELECT * FROM spotifyHistory WHERE album = ? COLLATE NOCASE AND artist = ? COLLATE NOCASE;" ; 

    // initialize song list and song info struct to store the songs that match the query
    song_list list = { .songs = NULL, .num_songs = 0 } ;

    if (sqlite3_prepare_v2(database, get_album_rows, -1, &cmd, NULL) != 0) return list;

    sqlite3_bind_text(cmd, 1, album_name, -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(cmd, 2, artist_name, -1, SQLITE_TRANSIENT) ; 

    while (sqlite3_step(cmd) == SQLITE_ROW)
    {
        int count = list.num_songs + 1 ; 
        song_info* temp = realloc(list.songs, count * sizeof(song_info)) ;
        if (!temp) break ; 
        list.songs = temp ;
        list.num_songs = count ;

        song_info* info = &list.songs[list.num_songs - 1] ;
        info->artist = strdup((char*) sqlite3_column_text(cmd, 0)) ;
        info->track = strdup((char*) sqlite3_column_text(cmd, 1)) ;
        info->album = strdup((char*) sqlite3_column_text(cmd, 2)) ;
        info->ms_played = sqlite3_column_int(cmd, 3) ;
        info->timestamp = strdup((char*) sqlite3_column_text(cmd, 4)) ;
        info->track = strdup((char*) sqlite3_column_text(cmd, 5)) ;
            
    }
        
    sqlite3_finalize(cmd) ;
    return list ;

}


void sql_change_timestamp_format(sqlite3* database)
{
    // goal: change the timestamp format
    const char* convert_time = "UPDATE spotifyHistory SET timestamp = strftime('%m-%d-%Y %H:%M:%S', timestamp);" ; 
    char* err_msg = NULL ;
    int rc = sqlite3_exec(database, convert_time, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        log_msg_detailed("Error updating timestamps", __FILE__, __LINE__, err_msg);
        sqlite3_free(err_msg);
    }
}


artist_list get_top_artist(sqlite3* database)
{
    // goal: get the top artist
    sqlite3_stmt* cmd = NULL ;
    const char* get_artist_listen_count = "SELECT artist, COUNT(*) as play_count FROM spotifyHistory GROUP BY artist ORDER BY play_count DESC;" ; 

    artist_list list = { .root = NULL, .len = 0 } ;

    if (sqlite3_prepare_v2(database, get_artist_listen_count, -1, &cmd, NULL) != 0) return list;

    while (sqlite3_step(cmd) == SQLITE_ROW)
    {
        int count = list.len + 1 ; 
        artist* temp = realloc(list.root, count * sizeof(artist)) ;
        if (!temp) break ; 
        list.root = temp ;
        list.len = count ;

        artist* info = &list.root[list.len - 1] ;
        info->name = strdup((char*) sqlite3_column_text(cmd, 0)) ;
        info->num_plays = sqlite3_column_int(cmd, 1) ;
            
    }

    // NOTE: to get the first few artists, you just have to get the first few elements from the list that's returned when this function is called     
    sqlite3_finalize(cmd) ;
    return list ;

}

album_list get_top_albums(sqlite3* database)
{
    sqlite3_stmt* cmd = NULL ;
    const char* get_album_listen_count = "SELECT album, artist, COUNT(*) as play_count FROM spotifyHistory GROUP BY album, artist ORDER BY play_count DESC;" ; 

    album_list list = { .root = NULL, .len = 0 } ;

    if (sqlite3_prepare_v2(database, get_album_listen_count, -1, &cmd, NULL) != 0) return list;

    while (sqlite3_step(cmd) == SQLITE_ROW)
    {
        int count = list.len + 1 ; 
        album* temp = realloc(list.root, count * sizeof(album)) ;
        if (!temp) break ; 
        list.root = temp ;
        list.len = count ;

        album* info = &list.root[list.len - 1] ;
        info->name = strdup((char*) sqlite3_column_text(cmd, 0)) ;
        info->artist = strdup((char*) sqlite3_column_text(cmd, 1)) ;
        info->num_plays = sqlite3_column_int(cmd, 2) ;
    }
        
    sqlite3_finalize(cmd) ;
    return list ;
}



    
// function to clear the table
// function to delete table
// function to merge tables
