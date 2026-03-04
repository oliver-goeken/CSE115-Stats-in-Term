#include "stats.h"
#include "input.h"
#include "log.h"
#include "parse_db_funcs.h"
#include <unistd.h>
#include <signal.h>

sqlite3* song_plays_database;

display_screen* MAIN_SCREEN;
display_screen* QUIT_SCREEN;

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
	int num_contents;
	content_setup contents[256];
} window_setup;

typedef struct {
	int num_windows;
	window_setup windows[256];
} screen_setup;

bool IN_MAIN_LOOP = true;
int main(){

	init();

	MAIN_SCREEN = display_create_new_screen("MENU");
	
	screen_setup menu_setup = {
		8, {
			{"0:0:w:4", WINDOW_UNSELECTABLE, WINDOW_BOXED, 2, {
				//{"", NULL, CONTENT_NODE_ALIGN_CENTER},  // MAKE A OPTION TO SHRINK WINDOW HEIGHT AS MUCH AS POSSIBLE TO FIT CONTENT, as well as option to center nodes vertically
													// this title will be 6 high (two spaces) unless terminal too small
				{"Listening History and Stats", NULL, CONTENT_NODE_ALIGN_CENTER},
				{"Right In Your Terminal!", NULL, CONTENT_NODE_ALIGN_CENTER}
				 }},
			{"w1/7:4:w1/7:3", WINDOW_SELECTED, WINDOW_BOXED, 1, {
				{"Listening History", NULL, CONTENT_NODE_ALIGN_CENTER}
									}},
			{"w2/7:4:w1/7:3", WINDOW_NOT_SELECTED, WINDOW_BOXED, 1, {
				{"Top Artists", NULL, CONTENT_NODE_ALIGN_CENTER}
										}},
			{"w3/7:4:w1/7:3", WINDOW_NOT_SELECTED, WINDOW_BOXED, 1, {
				{"Top Albums", NULL, CONTENT_NODE_ALIGN_CENTER}
										 }},
			{"w4/7:4:w1/7:3", WINDOW_NOT_SELECTED, WINDOW_BOXED, 1, {
				{"Top Songs", NULL, CONTENT_NODE_ALIGN_CENTER}
										 }},
			{"w5/7:4:w1/7:3", WINDOW_NOT_SELECTED, WINDOW_BOXED, 1, {
				{"Quit", quit_button_interact, CONTENT_NODE_ALIGN_CENTER}
										 }},
			{"0:7:w:h-9", WINDOW_NOT_SELECTED, WINDOW_BOXED, 0, {
										 }},
			{"0:h-2:w:2", WINDOW_UNSELECTABLE, WINDOW_NOT_BOXED, 1, {
				{"[arrow keys] or [hjkl] to navigate - [:] to enter command - [q] to quit", NULL, CONTENT_NODE_ALIGN_CENTER}
										 }}
		}
	};

	for (int i = 0; i < menu_setup.num_windows; i ++){
		display_window* NEW_WINDOW = display_screen_add_new_window(MAIN_SCREEN, (menu_setup.windows)[i].fmt_string);
		display_window_set_boxed(NEW_WINDOW, menu_setup.windows[i].window_boxed);
		display_window_set_selected(NEW_WINDOW, menu_setup.windows[i].selected);

		for (int j = 0; j < menu_setup.windows[i].num_contents; j ++){
			display_content_node* new_node = display_new_text_content_node(NEW_WINDOW, menu_setup.windows[i].contents[j].text);
			display_set_content_node_alignment(new_node, menu_setup.windows[i].contents[j].alignment);
			display_content_node_set_interaction(new_node, menu_setup.windows[i].contents[j].interact);
		}
	}

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

	
	display_window* QUIT_NO_WINDOW = display_screen_add_new_window(QUIT_SCREEN, "w1/2+2:h1/2+1:2:1");
	display_content_node* quit_no_node = display_new_text_content_node(QUIT_NO_WINDOW, "No");
	display_content_node_set_interaction(quit_no_node, quit_no_button_interact);


	//song_list sl = get_all_songs_played_for_artist(song_plays_database, "Des Rocs");

	/*
		for (int i = 0; i < sl.num_songs; i ++){
			display_new_text_content_node(LIST_WINDOW, sl.songs[i].track);
		}
	*/


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
						case 27:
							SCREEN_RETURN = MAIN_SCREEN;
							display_set_screen(QUIT_SCREEN);
							display_screen_set_selected_window(QUIT_SCREEN, QUIT_NO_WINDOW);
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

	remove("spotifyHistory.db");

	create_db(song_plays_database);
	sqlite3_open("spotifyHistory.db", &song_plays_database);
	json_import_to_db(song_plays_database, "/Users/oliverdgoeken/Downloads/Spotify Extended Streaming History/Streaming_History_Audio_2019-2020_0.json");
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
// pragma because of uneccesary parameter necessary to function pointer
void quit_no_button_interact(display_content_node* content_node){
	if (SCREEN_RETURN != NULL){
		display_set_screen(SCREEN_RETURN);
	} else {
		display_set_screen(MAIN_SCREEN);
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void quit_button_interact(display_content_node* content_node){
	SCREEN_RETURN = MAIN_SCREEN;
	display_set_screen(QUIT_SCREEN);
}
