#include "stats.h"
#include "input.h"
#include "log.h"
#include "cli.h"
#include "utils.h"
#include <unistd.h>
#include <signal.h>

display_window* LOADING_DATA_WINDOW;

display_window* LIST_WINDOW;
display_window* QUIT_NO_WINDOW;

display_screen* SCREEN_RETURN = NULL;

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



int main(int argc, char **argv) {
	int rc = handle_args(argc, argv);
	if (rc >= 0) return rc;

	init();

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
				{"Listening History", sql_get_listening_history, CONTENT_NODE_ALIGN_CENTER}
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
			{"0:h-2:w:2", WINDOW_UNSELECTABLE, WINDOW_NOT_BOXED, NULL, 1, {
				{"[arrow keys] or [hjkl] to navigate - [enter] to select - [esc] to return to options - [h] for help - [:] to enter command - [q] to quit", NULL, CONTENT_NODE_ALIGN_CENTER}
										 }}
		}
	};

	for (int i = 0; i < menu_setup.num_windows; i ++){
		display_window* NEW_WINDOW = display_screen_add_new_window(MAIN_SCREEN, (menu_setup.windows)[i].fmt_string);
		display_window_set_boxed(NEW_WINDOW, menu_setup.windows[i].window_boxed);
		display_add_window_to_group(NEW_WINDOW, options_group);
		display_window_set_selected(NEW_WINDOW, menu_setup.windows[i].selected);

		for (int j = 0; j < menu_setup.windows[i].num_contents; j ++){
			display_content_node* new_node = display_new_text_content_node(NEW_WINDOW, menu_setup.windows[i].contents[j].text);
			display_set_content_node_alignment(new_node, menu_setup.windows[i].contents[j].alignment);
			display_content_node_set_interaction(new_node, menu_setup.windows[i].contents[j].interact);
		}
	}

	LIST_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "0:7:w:h-9");
	display_window_set_boxed(LIST_WINDOW, WINDOW_BOXED);

	


	display_window* COMMAND_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "0:h-1:w:1");
	display_window_set_selected(COMMAND_WINDOW, WINDOW_UNSELECTABLE);

	display_set_current_screen(MAIN_SCREEN);

	QUIT_SCREEN = display_create_new_screen("QUIT");

	display_window* QUIT_WINDOW = display_screen_add_new_window(QUIT_SCREEN, "w1/3:h1/2-3:w1/3:6");
	display_window_set_boxed(QUIT_WINDOW , WINDOW_BOXED);
	display_window_set_expansion(QUIT_WINDOW, WINDOW_EXPAND_TO_FIT_TEXT);
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
				break;
			case 'k':
			case KEY_UP:
				display_generic_select_prev_node();
				break;
			case 'h':
			case KEY_LEFT:
				display_screen_select_previous_window(display_get_current_screen());
				break;
			case 'l':
			case KEY_RIGHT:
				display_screen_select_next_window(display_get_current_screen());
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
						case ':': {
							int command_return_val = input_handle_command(COMMAND_WINDOW, 0, 0);

							if (command_return_val == COMMAND_QUIT){
								IN_MAIN_LOOP = false;
							} else if (command_return_val == COMMAND_NOT_RECOGNIZED){
								input_display_command_error(COMMAND_WINDOW, "Command not recognized");
							}
							break;

							return 0;
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
	display_window_set_expansion(LOADING_DATA_WINDOW, WINDOW_EXPAND_TO_FIT_TEXT);
	display_window_set_selected(LOADING_DATA_WINDOW, WINDOW_UNSELECTABLE);
	display_content_node* loading_node_1 = display_new_text_content_node(LOADING_DATA_WINDOW, "Loading json file...");
	display_set_content_node_alignment(loading_node_1, CONTENT_NODE_ALIGN_CENTER);

	display_screen_draw_windows(LOADING_DATA_SCREEN);

	create_db(song_plays_database);
	sqlite3_open(CLI_OPTIONS.db_path, &song_plays_database);
	
	// move to its own function taking string
	// function called by command or by cli option
	// defaults to nothing
	json_import_directory(song_plays_database, CLI_OPTIONS.json_path);
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

		strptime(listening_history.songs[i].timestamp, "%Y-%m-%dT%H:%M:%S", &time_struct);
		strftime(time_formatted, str_data_size, "%D %R", &time_struct);

		snprintf(listen_str_data, str_data_size, "[%s] %s - %s - %s", time_formatted, listening_history.songs[i].track, listening_history.songs[i].album, listening_history.songs[i].artist);

		display_content_node* new_node = display_new_text_content_node(LIST_WINDOW, listen_str_data);
		display_content_node_set_interaction(new_node, handle_album_click);
	}

	log_msg("done showing listening history");

	display_screen_set_selected_window(MAIN_SCREEN, LIST_WINDOW);
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
}

void handle_album_click(display_content_node* content_node){
	log_msg_f("%s", content_node->data->text_data);
}

void go_to_quit_screen(){
	display_set_screen(QUIT_SCREEN);
	display_screen_set_selected_window(QUIT_SCREEN, QUIT_NO_WINDOW);
}
