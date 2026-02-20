#include "stats.h"
#include "utils.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int SIGINT_FLAG = 0;
int SIGSEGV_FLAG = 0;

int DONE = 0;
int main (){
	init();

	// 0:0:h1/1:l1/1
	display_window* list_window = display_create_window(MAIN, true, "0:0:w1/2:h-2");
	display_window_add_content_node(list_window, UNKNOWN, "test!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "2est!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "3est!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "4est!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "5est!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "6est!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_box(list_window, '-', '|');

	display_window* info_window = display_create_window(MAIN, true, "w1/2:0:w1/2:h-2");
	display_window_add_content_node(info_window, UNKNOWN, "hello!");
	display_window_add_content_node(info_window, UNKNOWN, "hello!");
	display_window_add_content_node(info_window, UNKNOWN, "hello!");
	display_window_box(info_window, '-', '|');

	display_window* help_window = display_create_window(MAIN, false, "0:h-2:w:2");
	display_set_content_node_alignment(display_window_add_content_node(help_window, UNKNOWN, "Use [arrow keys] or [hjkl] to navigate - Press [q] to quit!"), CENTER);


	display_window* quit_window = display_create_window(EXIT, false, "w1/3:h1/2-3:w1/3:6");
	display_set_content_node_alignment(display_window_add_content_node(quit_window, UNKNOWN, "Are you sure you want to quit"), CENTER);
	display_set_content_node_alignment(display_window_add_content_node(quit_window, UNKNOWN, "Select option or press 'q' to confirm."), CENTER);
	display_set_window_expansion(quit_window, true);
	display_window_box(quit_window, '-', '|');

	display_window* yes_window = display_create_window(EXIT, true, "w1/2-4:h1/2+1:3:1");
	display_window_content_node* yes_node = display_window_add_content_node(yes_window, UNKNOWN, "Yes");
	display_content_node_set_interaction(yes_node, handle_interact_quit_yes);
	display_set_content_node_alignment(yes_node, CENTER);

	display_window* no_window = display_create_window(EXIT, true, "w1/2+1:h1/2+1:2:1");
	display_window_content_node* no_node = display_window_add_content_node(no_window, UNKNOWN, "No");
	display_content_node_set_interaction(no_node, handle_interact_quit_no);
	display_set_content_node_alignment(no_node, CENTER);

	display_set_selected_window(no_window);

	while(!DONE){
		if (SIGINT_FLAG != 0){
			break;
		} 

		display_draw_all_windows();

		int ch = getch();

		switch(ch){
			case KEY_RESIZE:
				display_handle_winch();
				break;
			case 'j':
			case KEY_DOWN:
				display_window_select_next_node(display_get_current_window());
				break;
			case 'k':
			case KEY_UP:
				display_window_select_previous_node(display_get_current_window());
				break;
			case 'h':
			case KEY_LEFT:
				display_select_previous_window();
				break;
			case 'l':
			case KEY_RIGHT:
				display_select_next_window();
				break;
			case '\n':
			case KEY_ENTER:
			case 13:
				display_handle_interaction();
				break;
			default:
				switch (display_get_current_screen()){
					case MENU:
						break;
					case MAIN:
						switch(ch){
							case 'Q':
							case 'q':
								display_set_screen(EXIT);
								break;
						}
						break;
					case EXIT:
						switch(ch){
							case 'Q':
							case 'q':
								DONE = 1;
								break;
							default:
								display_set_screen(MAIN);
						}
						break;
				}
				break;
		}
	}

	display_terminate();

	return 0;
}

void handle_interact_quit_yes(display_window_content_node* node, display_window* window){
	DONE = 1;
}

void handle_interact_quit_no(display_window_content_node* node, display_window* window){
	display_set_screen(MAIN);
}

void init(){
	display_init();
	
	signal(SIGINT, handle_interrupt);
}

void handle_interrupt(){
	SIGINT_FLAG = 1;
}
