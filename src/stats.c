#include "stats.h"
#include "input.h"
#include "log.h"
#include "parse_db_funcs.h"
#include <unistd.h>

sqlite3* song_plays_database;

display_screen* MENU_SCREEN;
display_screen* MAIN_SCREEN;
display_screen* QUIT_SCREEN;

bool IN_MAIN_LOOP = true;
int main(){
	init();

	MENU_SCREEN = display_create_new_screen("MENU");
	display_set_current_screen(MENU_SCREEN);
	
	display_window* MENU_TITLE_WINDOW = display_screen_add_new_window(MENU_SCREEN, "0:0:w:h1/4");
	display_window_set_boxed(MENU_TITLE_WINDOW, WINDOW_BOXED);
	display_window_set_selected(MENU_TITLE_WINDOW, WINDOW_UNSELECTABLE);

	
	MAIN_SCREEN = display_create_new_screen("MAIN");

	display_window* LIST_TITLE_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "0:0:w1/2:3");
	display_window_set_boxed(LIST_TITLE_WINDOW, WINDOW_BOXED);
	display_window_set_selected(LIST_TITLE_WINDOW, WINDOW_UNSELECTABLE);
	display_content_node* list_title_node = display_new_text_content_node(LIST_TITLE_WINDOW, "Listening History");
	display_set_content_node_alignment(list_title_node, CONTENT_NODE_ALIGN_CENTER);

	display_window* INFO_TITLE_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "w1/2:0:w1/2:3");
	display_window_set_boxed(INFO_TITLE_WINDOW, WINDOW_BOXED);
	display_window_set_selected(INFO_TITLE_WINDOW, WINDOW_UNSELECTABLE);
	display_content_node* info_title_node = display_new_text_content_node(INFO_TITLE_WINDOW, "Song Play Info");
	display_set_content_node_alignment(info_title_node, CONTENT_NODE_ALIGN_CENTER);

	display_window* LIST_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "0:2:w1/2:h-4");
	display_window_set_boxed(LIST_WINDOW,  WINDOW_BOXED);

	display_window* INFO_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "w1/2:2:w1/2:h-4");
	display_window_set_boxed(INFO_WINDOW , WINDOW_BOXED);
	INFO_WINDOW->selected = WINDOW_SELECTED;

	display_window* HELP_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "0:h-2:w:2");
	display_content_node* help_node = display_new_text_content_node(HELP_WINDOW, "[arrow keys] or [hjkl] to navigate - [:] to enter command - [q] to quit");
	display_window_set_selected(HELP_WINDOW, WINDOW_UNSELECTABLE);
	display_set_content_node_alignment(help_node, CONTENT_NODE_ALIGN_CENTER);

	display_window* COMMAND_WINDOW = display_screen_add_new_window(MAIN_SCREEN, "0:h-1:w:1");
	display_window_set_selected(COMMAND_WINDOW, WINDOW_UNSELECTABLE);


	QUIT_SCREEN = display_create_new_screen("QUIT");

	display_window* QUIT_WINDOW = display_screen_add_new_window(QUIT_SCREEN, "w1/3:h1/2-3:w1/3:6");
	display_window_set_boxed(QUIT_WINDOW , WINDOW_BOXED);
	display_window_set_expansion(QUIT_WINDOW, WINDOW_EXPAND_TO_FIT_TEXT);
	display_window_set_selected(QUIT_WINDOW, WINDOW_UNSELECTABLE);
	display_content_node* quit_node_1 = display_new_text_content_node(QUIT_WINDOW, "Are you sure you want to quit");
	display_set_content_node_alignment(quit_node_1, CONTENT_NODE_ALIGN_CENTER);
	display_content_node* quit_node_2 = display_new_text_content_node(QUIT_WINDOW, "Select option or press [q] to confirm.");
	display_set_content_node_alignment(quit_node_2, CONTENT_NODE_ALIGN_CENTER);

	display_window* QUIT_YES_WINDOW = display_screen_add_new_window(QUIT_SCREEN, "w1/2-4:h1/2+1:3:1");
	display_content_node* quit_yes_node = display_new_text_content_node(QUIT_YES_WINDOW, "Yes");
	display_content_node_set_interaction(quit_yes_node, quit_yes_button_interact);

	
	display_window* QUIT_NO_WINDOW = display_screen_add_new_window(QUIT_SCREEN, "w1/2+1:h1/2+1:2:1");
	display_content_node* quit_no_node = display_new_text_content_node(QUIT_NO_WINDOW, "No");
	display_content_node_set_interaction(quit_no_node, quit_no_button_interact);


	song_list sl = get_all_songs_played_for_artist(song_plays_database, "Des Rocs");

	for (int i = 0; i < sl.num_songs; i ++){
		display_new_text_content_node(LIST_WINDOW, sl.songs[i].track);
	}

	while (IN_MAIN_LOOP){
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
							display_set_screen(MAIN_SCREEN);
							break;
					}
				}
			}
		}
	}

	terminate();
	return 0;
}

void init(){
	log_init_file("stats.log");

	display_init();

	create_db(song_plays_database);
	json_import_to_db(song_plays_database, "~/Downloads/Spotify Extended Streaming History/Streaming_History_Audio_2019-2020_0.json");
}

void terminate(){
	log_msg("closing...");

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
	display_set_screen(MAIN_SCREEN);
}

