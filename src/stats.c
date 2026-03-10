
#define _XOPEN_SOURCE 700
// definition necessary for strptime on linux
#define _XOPEN_SOURCE 700
#include "stats.h"
#include "input.h"
#include "boognish.h"
#include "utils.h"
#include "panel.h"
#include <time.h>
#include "log.h"
#include "cli.h"
#include "shared_defs.h"
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>

sqlite3* song_plays_database;
display_window* LOADING_DATA_WINDOW = NULL;

display_window* QUIT_NO_WINDOW = NULL;

display_screen* SCREEN_RETURN = NULL;

display_screen* LOADING_DATA_SCREEN = NULL;
display_screen* MAIN_SCREEN = NULL;
display_screen* QUIT_SCREEN = NULL;
display_screen* FULL_SCREEN = NULL;
display_window* LIST_WINDOW = NULL;

sqlite3* song_plays_database = NULL;

bool SIGINT_FLAG = NULL;

typedef struct {
	char* text;
	void (*interact)(display_content_node*);
	int alignment;
} content_setup;

typedef struct {
	char* fmt_string;
	int selected;
	bool window_boxed;
	display_window_group* group;
	int num_contents;
	content_setup contents[256];
} window_setup;

typedef struct {
	int num_windows;
	window_setup windows[256];
} screen_setup;

bool IN_MAIN_LOOP = true;

static int init_db_only(void){
	signal(SIGINT, handle_sigint);
	log_init_file("stats.log");

	if (sqlite3_open(CLI_OPTIONS.db_path, &song_plays_database) != SQLITE_OK){
		log_msg_detailed("Error opening database: ", __FILE__, __LINE__, sqlite3_errmsg(song_plays_database));
		log_terminate();
		return 1;
	}

	if (create_db(song_plays_database) != 0){
		sqlite3_close(song_plays_database);
		log_terminate();
		return 1;
	}

	if (CLI_OPTIONS.json_path && CLI_OPTIONS.json_path[0] != '\0'){
		json_import_directory(song_plays_database, (char*)CLI_OPTIONS.json_path);
	}

	return 0;
}

static void terminate_db_only(void){
	sqlite3_close(song_plays_database);
	log_terminate();
}

static int print_recent_history(int limit){
	song_list listening_history = get_listening_history_limit(song_plays_database, limit);

	if (listening_history.songs == NULL){
		log_err("no songs in list");
		return 1;
	}

	int str_data_size = 256;
	for (int i = 0; i < listening_history.num_songs; i ++){
		char listen_str_data[str_data_size];
		memset(listen_str_data, 0, str_data_size);

		char time_formatted[str_data_size];
		struct tm time_struct;
		memset(&time_struct, 0, sizeof(time_struct));

		if (strptime(listening_history.songs[i].timestamp, "%Y-%m-%dT%H:%M:%S", &time_struct) == NULL){
			strncpy(time_formatted, listening_history.songs[i].timestamp, str_data_size - 1);
		} else {
			strftime(time_formatted, str_data_size, "%D %R", &time_struct);
		}

		snprintf(listen_str_data, str_data_size, "[%s] %s - %s - %s",
			time_formatted,
			listening_history.songs[i].track,
			listening_history.songs[i].album,
			listening_history.songs[i].artist);

		printf("%s\n", listen_str_data);
	}

	free_song_list(&listening_history);
	return 0;
}

static int print_top_albums(int limit){
	album_list top_albums = get_top_albums_limit(song_plays_database, limit);

	if (top_albums.root == NULL){
		log_err("top albums list is empty");
		return 1;
	}

	for (int i = 0; i < top_albums.len; i++){
		printf("%d. %s - %s [%d plays]\n",
			i + 1,
			top_albums.root[i].name,
			top_albums.root[i].artist,
			top_albums.root[i].num_plays);
	}

	free_album_list(top_albums);
	return 0;
}

static int print_bottom_albums(int limit){
	album_list bottom_albums = get_bottom_albums_limit(song_plays_database, limit);

	if (bottom_albums.root == NULL){
		log_err("bottom albums list is empty");
		return 1;
	}

	for (int i = 0; i < bottom_albums.len; i++){
		printf("%d. %s - %s [%d plays]\n",
			i + 1,
			bottom_albums.root[i].name,
			bottom_albums.root[i].artist,
			bottom_albums.root[i].num_plays);
	}

	free_album_list(bottom_albums);
	return 0;
}

static int print_top_artists(int limit){
	artist_list top_artists = get_top_artists_limit(song_plays_database, limit);

	if (top_artists.root == NULL){
		log_err("top artists list is empty");
		return 1;
	}

	for (int i = 0; i < top_artists.len; i++){
		printf("%d. %s [%d plays]\n",
			i + 1,
			top_artists.root[i].name,
			top_artists.root[i].num_plays);
	}

	free_artist_list(top_artists);
	return 0;
}

static int print_bottom_artists(int limit){
	artist_list bottom_artists = get_bottom_artists_limit(song_plays_database, limit);

	if (bottom_artists.root == NULL){
		log_err("bottom artists list is empty");
		return 1;
	}

	for (int i = 0; i < bottom_artists.len; i++){
		printf("%d. %s [%d plays]\n",
			i + 1,
			bottom_artists.root[i].name,
			bottom_artists.root[i].num_plays);
	}

	free_artist_list(bottom_artists);
	return 0;
}

static int print_top_songs(int limit){
	track_list top_songs = get_top_tracks_limit(song_plays_database, limit);

	if (top_songs.root == NULL){
		log_err("top songs list is empty");
		return 1;
	}

	for (int i = 0; i < top_songs.len; i++){
		printf("%d. %s - %s - %s [%d plays]\n",
			i + 1,
			top_songs.root[i].name,
			top_songs.root[i].album,
			top_songs.root[i].artist,
			top_songs.root[i].num_plays);
	}

	free_track_list(top_songs);
	return 0;
}

static int print_bottom_songs(int limit){
	track_list bottom_songs = get_bottom_tracks_limit(song_plays_database, limit);

	if (bottom_songs.root == NULL){
		log_err("bottom songs list is empty");
		return 1;
	}

	for (int i = 0; i < bottom_songs.len; i++){
		printf("%d. %s - %s - %s [%d plays]\n",
			i + 1,
			bottom_songs.root[i].name,
			bottom_songs.root[i].album,
			bottom_songs.root[i].artist,
			bottom_songs.root[i].num_plays);
	}

	free_track_list(bottom_songs);
	return 0;
}

static int print_search_results(const char* kind, int limit, const char* query){
	artist_list matches = search_artists_by_name(song_plays_database, query, 1);

	if (matches.root == NULL || matches.len == 0){
		fprintf(stderr, "No artist found matching \"%s\"\n", query);
		return 1;
	}

	const char* artist_name = matches.root[0].name;
	printf("Best match: %s\n", artist_name);

	if (strcmp(kind, "songs") == 0){
		track_list top_songs = get_top_tracks_for_artist_limit(song_plays_database, artist_name, limit);

		if (top_songs.root == NULL){
			fprintf(stderr, "No songs found for \"%s\"\n", artist_name);
			free_artist_list(matches);
			return 1;
		}

		for (int i = 0; i < top_songs.len; i++){
			printf("%d. %s - %s [%d plays]\n",
				i + 1,
				top_songs.root[i].name,
				top_songs.root[i].album,
				top_songs.root[i].num_plays);
		}

		free_track_list(top_songs);
	} else {
		album_list top_albums = get_top_albums_for_artist_limit(song_plays_database, artist_name, limit);

		if (top_albums.root == NULL){
			fprintf(stderr, "No albums found for \"%s\"\n", artist_name);
			free_artist_list(matches);
			return 1;
		}

		for (int i = 0; i < top_albums.len; i++){
			printf("%d. %s [%d plays]\n",
				i + 1,
				top_albums.root[i].name,
				top_albums.root[i].num_plays);
		}

		free_album_list(top_albums);
	}

	free_artist_list(matches);
	return 0;
}


int main(int argc, char **argv) {
	int rc = handle_args(argc, argv);
	if (rc >= 0) return rc;

	if (CLI_OPTIONS.search_kind != NULL){
		if (init_db_only() != 0) return 1;
		int prc = print_search_results(CLI_OPTIONS.search_kind, CLI_OPTIONS.search_limit, CLI_OPTIONS.search_query);
		terminate_db_only();
		return prc;
	}

	if (CLI_OPTIONS.recent_count > 0){
		if (init_db_only() != 0) return 1;
		int prc = print_recent_history(CLI_OPTIONS.recent_count);
		terminate_db_only();
		return prc;
	}

	if (CLI_OPTIONS.artist_count > 0){
		if (init_db_only() != 0) return 1;
		int prc = print_top_artists(CLI_OPTIONS.artist_count);
		terminate_db_only();
		return prc;
	}

	if (CLI_OPTIONS.artist_bottom_count > 0){
		if (init_db_only() != 0) return 1;
		int prc = print_bottom_artists(CLI_OPTIONS.artist_bottom_count);
		terminate_db_only();
		return prc;
	}

	if (CLI_OPTIONS.album_count > 0){
		if (init_db_only() != 0) return 1;
		int prc = print_top_albums(CLI_OPTIONS.album_count);
		terminate_db_only();
		return prc;
	}

	if (CLI_OPTIONS.album_bottom_count > 0){
		if (init_db_only() != 0) return 1;
		int prc = print_bottom_albums(CLI_OPTIONS.album_bottom_count);
		terminate_db_only();
		return prc;
	}

	if (CLI_OPTIONS.song_count > 0){
		if (init_db_only() != 0) return 1;
		int prc = print_top_songs(CLI_OPTIONS.song_count);
		terminate_db_only();
		return prc;
	}

	if (CLI_OPTIONS.song_bottom_count > 0){
		if (init_db_only() != 0) return 1;
		int prc = print_bottom_songs(CLI_OPTIONS.song_bottom_count);
		terminate_db_only();
		return prc;
	}

	init();

	FULL_SCREEN = display_create_new_screen("FULL");

	display_screen* HELP_SCREEN = display_create_new_screen("HELP");

	screen_setup help_setup = {
		7, {
			{"0:0:w:3", WINDOW_UNSELECTABLE, WINDOW_BOXED, NULL, 1, {
				{"Help Menu", NULL, CONTENT_NODE_ALIGN_CENTER}
																	}},
			{"0:3:w1/3:3", WINDOW_UNSELECTABLE, WINDOW_BOXED, NULL, 1, {
				{"Hotkeys", NULL, CONTENT_NODE_ALIGN_CENTER}
																	}},
			{"w1/3:3:w1/3:3", WINDOW_UNSELECTABLE, WINDOW_BOXED, NULL, 1, {
				{"Commands", NULL, CONTENT_NODE_ALIGN_CENTER}
																	}},
			{"w2/3:3:w1/3:3", WINDOW_UNSELECTABLE, WINDOW_BOXED, NULL, 1, {
				{"Info", NULL, CONTENT_NODE_ALIGN_CENTER}
																	}},
			{"0:5:w1/3:h-5", WINDOW_SELECTED, WINDOW_BOXED, NULL, 7, {
				{"[esc] - return to main screen from here", NULL, CONTENT_NODE_ALIGN_CENTER},
				{"[esc] - on main screen, select new option", NULL, CONTENT_NODE_ALIGN_CENTER},
				{"[arrows keys]/[hjkl] - navigate", NULL, CONTENT_NODE_ALIGN_CENTER},
				{"[enter] interact", NULL, CONTENT_NODE_ALIGN_CENTER},
				{"[H] on main screen, show this help screen", NULL, CONTENT_NODE_ALIGN_CENTER},
				{"[:] on main screen, enter command", NULL, CONTENT_NODE_ALIGN_CENTER},
				{"[q] quit", NULL, CONTENT_NODE_ALIGN_CENTER},
																	 }},
			{"w1/3:5:w1/3:h-5", WINDOW_NOT_SELECTED, WINDOW_BOXED, NULL, 1, {
				{"", NULL, CONTENT_NODE_ALIGN_CENTER}
																			}},
			{"w2/3:5:w1/3:h-5", WINDOW_NOT_SELECTED, WINDOW_BOXED, NULL, 1, {
				{"", NULL, CONTENT_NODE_ALIGN_CENTER}
																			}}
		}
	};

	/*
	 *
	 * 3 column help screen
	 * hotkeys
	 * commands
	 * general info
	 *
	 *
	 */

	for (int i = 0; i < help_setup.num_windows; i ++){
		display_window* NEW_WINDOW = display_screen_add_new_window(HELP_SCREEN, (help_setup.windows)[i].fmt_string);
		display_window_set_boxed(NEW_WINDOW, help_setup.windows[i].window_boxed);
		display_add_window_to_group(NEW_WINDOW, help_setup.windows[i].group);
		display_window_set_selected(NEW_WINDOW, help_setup.windows[i].selected);

		for (int j = 0; j < help_setup.windows[i].num_contents; j ++){
			display_content_node* new_node = display_new_text_content_node(NEW_WINDOW, help_setup.windows[i].contents[j].text);
			display_set_content_node_alignment(new_node, help_setup.windows[i].contents[j].alignment);
			display_content_node_set_interaction(new_node, help_setup.windows[i].contents[j].interact);
		}
	}


	MAIN_SCREEN = display_create_new_screen("MAIN");
	display_set_screen(MAIN_SCREEN);

	display_window_group* options_group = display_create_window_group();
	
	screen_setup menu_setup = {
		7, {
			{"0:0:w:4", WINDOW_UNSELECTABLE, WINDOW_BOXED, NULL, 2, {
				//{"", NULL, CONTENT_NODE_ALIGN_CENTER},  // MAKE A OPTION TO SHRINK WINDOW HEIGHT AS MUCH AS POSSIBLE TO FIT CONTENT, as well as option to center nodes vertically
													// this title will be 6 high (two spaces) unless terminal too small
				{"Listening History and Stats", NULL, CONTENT_NODE_ALIGN_CENTER},
				{"Right In Your Terminal!", NULL, CONTENT_NODE_ALIGN_CENTER}
				 }},
			{"w1/7:4:w1/7:3", WINDOW_SELECTED, WINDOW_BOXED, options_group, 1, {
				{"History", sql_get_listening_history, CONTENT_NODE_ALIGN_CENTER}
									}},
			{"w2/7:4:w1/7:3", WINDOW_NOT_SELECTED, WINDOW_BOXED, options_group, 1, {
				{"Top Artists", sql_get_top_artists, CONTENT_NODE_ALIGN_CENTER}
										}},
			{"w3/7:4:w1/7:3", WINDOW_NOT_SELECTED, WINDOW_BOXED, options_group, 1, {
				{"Top Albums", sql_get_top_albums, CONTENT_NODE_ALIGN_CENTER}
										 }},
			{"w4/7:4:w1/7:3", WINDOW_NOT_SELECTED, WINDOW_BOXED, options_group, 1, {
				{"Top Songs", sql_get_top_songs, CONTENT_NODE_ALIGN_CENTER}
										 }},
			{"w5/7:4:w1/7:3", WINDOW_NOT_SELECTED, WINDOW_BOXED, options_group, 1, {
				{"Quit", quit_button_interact, CONTENT_NODE_ALIGN_CENTER}
										 }},
			{"0:h-2:w:1", WINDOW_UNSELECTABLE, WINDOW_NOT_BOXED, NULL, 1, {
				{"[arrow keys] or [hjkl] to navigate - [enter] to select - [esc] to return to options - [H] for help - [:] to enter command - [q] to quit", NULL, CONTENT_NODE_ALIGN_CENTER}
										 }}
		}
	};

	for (int i = 0; i < menu_setup.num_windows; i ++){
		display_window* NEW_WINDOW = display_screen_add_new_window(MAIN_SCREEN, (menu_setup.windows)[i].fmt_string);
		display_window_set_boxed(NEW_WINDOW, menu_setup.windows[i].window_boxed);
		display_add_window_to_group(NEW_WINDOW, menu_setup.windows[i].group);
		display_window_set_selected(NEW_WINDOW, menu_setup.windows[i].selected);

		for (int j = 0; j < menu_setup.windows[i].num_contents; j ++){
			display_content_node* new_node = display_new_text_content_node(NEW_WINDOW, menu_setup.windows[i].contents[j].text);
			display_set_content_node_alignment(new_node, menu_setup.windows[i].contents[j].alignment);
			display_content_node_set_interaction(new_node, menu_setup.windows[i].contents[j].interact);
		}
	}

	LIST_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "0:7:w:h-9");
	display_window_set_boxed(LIST_WINDOW, WINDOW_BOXED);

	//panel
	display_window* INFO_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "w2/3:0:w1/3:1");
	display_window_set_boxed(INFO_WINDOW, WINDOW_BOXED);
	display_window_set_constraint_window(INFO_WINDOW, LIST_WINDOW);
	panel_init(song_plays_database, LIST_WINDOW, INFO_WINDOW);
	display_window_set_visibility(INFO_WINDOW, WINDOW_HIDDEN);
	display_window_set_vertical_expansion(INFO_WINDOW, WINDOW_EXPAND_TO_FIT_TEXT);


	display_window* COMMAND_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "0:h-1:w:1");
	display_window_set_selected(COMMAND_WINDOW, WINDOW_UNSELECTABLE);

	display_set_current_screen(MAIN_SCREEN);

	QUIT_SCREEN = display_create_new_screen("QUIT");

	display_window* QUIT_WINDOW = display_screen_add_new_window(QUIT_SCREEN, "w1/3:h1/2-3:w1/3:6");
	display_window_set_boxed(QUIT_WINDOW , WINDOW_BOXED);
	display_window_set_horizontal_expansion(QUIT_WINDOW, WINDOW_EXPAND_TO_FIT_TEXT);
	display_window_set_selected(QUIT_WINDOW, WINDOW_UNSELECTABLE);
	display_content_node* quit_node_1 = display_new_text_content_node(QUIT_WINDOW, "Are you sure you want to quit?");
	display_set_content_node_alignment(quit_node_1, CONTENT_NODE_ALIGN_CENTER);
	display_content_node* quit_node_2 = display_new_text_content_node(QUIT_WINDOW, "Select option or press [q] to confirm.");
	display_set_content_node_alignment(quit_node_2, CONTENT_NODE_ALIGN_CENTER);

	display_window* QUIT_YES_WINDOW = display_screen_add_new_window(QUIT_SCREEN, "w1/2-4:h1/2+1:3:1");
	display_content_node* quit_yes_node = display_new_text_content_node(QUIT_YES_WINDOW, "Yes");
	display_content_node_set_interaction(quit_yes_node, quit_yes_button_interact);

	
	QUIT_NO_WINDOW = display_screen_add_new_window(QUIT_SCREEN, "w1/2+2:h1/2+1:2:1");
	display_content_node* quit_no_node = display_new_text_content_node(QUIT_NO_WINDOW, "No");
	display_content_node_set_interaction(quit_no_node, quit_no_button_interact);


	while (IN_MAIN_LOOP){
		if (SIGINT_FLAG){
			log_msg("recieved SIGINT");
			break;
		}

		display_screen_draw_windows(display_get_current_screen());

		int user_in = getch();

		if (user_in == ERR){
			continue;
		}

		switch (user_in){
			case KEY_RESIZE:
				display_handle_winch();
				break;
			case 'j':
			case KEY_DOWN:
				display_generic_select_next_node();
				panel_on_selection_changed(); //CHANGES
				break;
			case 'k':
			case KEY_UP:
				display_generic_select_prev_node();
				panel_on_selection_changed(); //CHANGES
				break;
			case 'h':
			case KEY_LEFT:
				display_screen_select_previous_window(display_get_current_screen());
				break;
			case 'l':
			case KEY_RIGHT:
				display_screen_select_next_window(display_get_current_screen());
				break;
			case 'g':
				display_generic_select_first_node();
				panel_on_selection_changed();
				break;

			case 'G':
				display_generic_select_last_node();
				panel_on_selection_changed();
				break;
			case '\n':
			case KEY_ENTER:
			case 13:
				display_handle_interact(display_get_selected_content_node());
				break;
			default: {
				display_screen* current_screen = display_get_current_screen();

				if (current_screen == MAIN_SCREEN){
					switch (user_in) {
						case 'Q':
						case 'q':
							SCREEN_RETURN = MAIN_SCREEN;
							go_to_quit_screen();
							break;
						case 27:
							display_screen_set_selected_window(MAIN_SCREEN, options_group->selected_window);
							break;
						case 'H':
							display_set_screen(HELP_SCREEN);
							break;
						case ':': {
							int command_return_val = input_handle_command(COMMAND_WINDOW, 0, 0);

							if (command_return_val == COMMAND_QUIT){
								IN_MAIN_LOOP = false;
							} else if (command_return_val == COMMAND_NOT_RECOGNIZED){
								input_display_command_error(COMMAND_WINDOW, "Command not recognized");
							} else if (command_return_val == COMMAND_HELP){
								display_set_screen(HELP_SCREEN);
							} else if (command_return_val == COMMAND_FILE_NOT_FOUND){
								input_display_command_error(COMMAND_WINDOW, "Error loading file");
							}
							break;
						}
				break;
					}
				} else if (current_screen == QUIT_SCREEN){
					switch(user_in){
						case 'Q':
						case 'q':
							IN_MAIN_LOOP = false;
							break;
						case 27:
							if (SCREEN_RETURN == NULL){
								display_set_screen(MAIN_SCREEN);
							} else {
								display_set_screen(SCREEN_RETURN);
							}
							break;
					}
				} else if (current_screen == HELP_SCREEN){
					switch(user_in){
						case 27:
						case 'H':
							display_set_screen(MAIN_SCREEN);
							break;
						case 'Q':
						case 'q':
							SCREEN_RETURN = HELP_SCREEN;
							go_to_quit_screen();
							break;
					}
				}
			}
		}
	}

	display_destroy_window_group(options_group);

	terminate();
	return 0;
}

void handle_sigint(){
	SIGINT_FLAG = true;
}

void init(){
	signal(SIGINT, handle_sigint);

	log_init_file("stats.log");

	display_init();

	LOADING_DATA_SCREEN = display_create_new_screen("loading");

	LOADING_DATA_WINDOW = display_screen_add_new_window(LOADING_DATA_SCREEN, "w3/8:h1/2-1:w1/4:3");
	display_window_set_boxed(LOADING_DATA_WINDOW , WINDOW_BOXED);
	display_window_set_horizontal_expansion(LOADING_DATA_WINDOW, WINDOW_EXPAND_TO_FIT_TEXT);
	display_window_set_selected(LOADING_DATA_WINDOW, WINDOW_UNSELECTABLE);
	display_content_node* loading_node_1 = display_new_text_content_node(LOADING_DATA_WINDOW, "Loading json file(s)...");
	display_set_content_node_alignment(loading_node_1, CONTENT_NODE_ALIGN_CENTER);

	display_screen_draw_windows(LOADING_DATA_SCREEN);

	sqlite3_open(CLI_OPTIONS.db_path, &song_plays_database);
	create_db(song_plays_database);
	
	// move to its own function taking string
	// function called by command or by cli option
	// defaults to nothing
	if (CLI_OPTIONS.json_path && CLI_OPTIONS.json_path[0] != '\0'){
		json_import_directory(song_plays_database, CLI_OPTIONS.json_path);
	}
}

void terminate(){
	log_msg("closing...");

	sqlite3_close(song_plays_database);

	display_terminate();

	log_terminate();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
// pragma because of uneccesary parameter necessary to function pointer
void quit_yes_button_interact(display_content_node* content_node){
	IN_MAIN_LOOP = false;
}

void quit_no_button_interact(display_content_node* content_node){
	if (SCREEN_RETURN != NULL){
		display_set_screen(SCREEN_RETURN);
	} else {
		display_set_screen(MAIN_SCREEN);
	}
}

void quit_button_interact(display_content_node* content_node){
	SCREEN_RETURN = MAIN_SCREEN;
	go_to_quit_screen();
}

void sql_get_top_albums(display_content_node* content_node){
	log_msg("getting top albums");

	display_window_destroy_content_nodes(LIST_WINDOW);

	album_list top_albums_list;

	top_albums_list = get_top_albums(song_plays_database);

	if (top_albums_list.root == NULL){
		log_err("top albums list is empty");
		return;
	}

	// ERROR
	// printing quit screen message sometimes

	int str_data_size = 256;
	for (int i = 0; i < top_albums_list.len; i ++){
		char album_str_data[str_data_size];
		memset(album_str_data, 0, str_data_size);

		snprintf(album_str_data, str_data_size, "%d. %s - %s - [%d plays]", i + 1, top_albums_list.root[i].name, top_albums_list.root[i].artist, top_albums_list.root[i].num_plays);

		display_content_node* new_node = display_new_text_content_node(LIST_WINDOW, album_str_data);
		display_content_node_set_interaction(new_node, handle_album_click);
	}

	display_screen_set_selected_window(MAIN_SCREEN, LIST_WINDOW);
	panel_on_selection_changed(); //CHANGES
}

void sql_get_top_artists(display_content_node* content_node){
	log_msg("getting top artists");

	display_window_destroy_content_nodes(LIST_WINDOW);

	artist_list top_artists_list = get_top_artists(song_plays_database);

	if (top_artists_list.root == NULL){
		log_err("no artists in list");
		return;
	}

	int str_data_size = 256;
	for (int i = 0; i < top_artists_list.len; i ++){
		char artist_str_data[str_data_size];
		memset(artist_str_data, 0, str_data_size);

		snprintf(artist_str_data, str_data_size, "%d. %s - [%d plays]", i + 1, top_artists_list.root[i].name, top_artists_list.root[i].num_plays);

		display_content_node* new_node = display_new_text_content_node(LIST_WINDOW, artist_str_data);
		display_content_node_set_interaction(new_node, handle_album_click);
	}

	display_screen_set_selected_window(MAIN_SCREEN, LIST_WINDOW);
	panel_on_selection_changed(); //CHANGES
}

void sql_get_listening_history(display_content_node* content_node){
	log_msg("getting listening history");

	display_window_destroy_content_nodes(LIST_WINDOW);

	song_list listening_history = get_listening_history(song_plays_database);

	if (listening_history.songs == NULL){
		log_err("no songs in list");
		return;
	}

	int str_data_size = 256;
	for (int i = 0; i < listening_history.num_songs; i ++){
		char listen_str_data[str_data_size];
		memset(listen_str_data, 0, str_data_size);

		char time_formatted[str_data_size];
		struct tm time_struct;

		strptime(listening_history.songs[i].timestamp, "%Y-%m-%dT%H:%M:%S%z", &time_struct);
		strftime(time_formatted, str_data_size, "%D %R", &time_struct);

		snprintf(listen_str_data, str_data_size, "[%s] %s - %s - %s", time_formatted, listening_history.songs[i].track, listening_history.songs[i].album, listening_history.songs[i].artist);

		display_content_node* new_node = display_new_text_content_node(LIST_WINDOW, listen_str_data);
		display_content_node_set_interaction(new_node, handle_album_click);
	}

	log_msg("done showing listening history");

	display_screen_set_selected_window(MAIN_SCREEN, LIST_WINDOW);
	panel_on_selection_changed(); //CHANGES
}

void sql_get_top_songs(display_content_node* content_node){
	log_msg("getting top songs");

	display_window_destroy_content_nodes(LIST_WINDOW);

	track_list top_songs_list = get_top_tracks(song_plays_database);

	if (top_songs_list.root == NULL){
		log_err("no songs in list");
		return;
	}

	int str_data_size = 256;
	for (int i = 0; i < top_songs_list.len; i ++){
		char songs_str_data[str_data_size];
		memset(songs_str_data, 0, str_data_size);

		snprintf(songs_str_data, str_data_size, "%d. %s - %s - %s - [%d plays]", i + 1, top_songs_list.root[i].name, top_songs_list.root[i].album, top_songs_list.root[i].artist, top_songs_list.root[i].num_plays);

		display_content_node* new_node = display_new_text_content_node(LIST_WINDOW, songs_str_data);
		display_content_node_set_interaction(new_node, handle_album_click);
	}

	display_screen_set_selected_window(MAIN_SCREEN, LIST_WINDOW);
	panel_on_selection_changed(); //CHANGES
}

void handle_album_click(display_content_node* content_node){
	log_msg_f("%s", content_node->data->text_data);
}

void go_to_quit_screen(){
	display_set_screen(QUIT_SCREEN);
	display_screen_set_selected_window(QUIT_SCREEN, QUIT_NO_WINDOW);
}

void draw_boognish(){
	clear_screen(FULL_SCREEN);

	display_set_screen(FULL_SCREEN);

	display_window* win = display_screen_add_new_window(FULL_SCREEN, "0:0:w:h");
	display_window_set_boxed(win, WINDOW_BOXED);
	display_window_set_selected(win, WINDOW_UNSELECTABLE);

	char* buff = malloc(1);

	ssize_t lines_read = 0;
	size_t len = 1;

	FILE* boog = fmemopen(lib_boognish_txt, lib_boognish_txt_len, "r");

	while ((lines_read = getline(&buff, &len, boog)) != -1){
		buff[100] = '\0';
		display_content_node* content_node = display_new_text_content_node(win, buff);
		display_set_content_node_alignment(content_node, CONTENT_NODE_ALIGN_CENTER);
	}

	display_new_text_content_node(win, "");
	display_new_text_content_node(win, "");
	display_new_text_content_node(win, "");
	display_set_content_node_alignment(display_new_text_content_node(win, "press any key to continue, mang"), CONTENT_NODE_ALIGN_CENTER);

	display_screen_draw_windows(FULL_SCREEN);

	wrefresh(win->ncurses_window);

	fclose(boog);

	getchar();

	display_set_screen(MAIN_SCREEN);
}
