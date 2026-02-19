#include "stats.h"
#include "utils.h"
#include "display.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int SIGINT_FLAG = 0;
int SIGSEGV_FLAG = 0;

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
	display_window_box(info_window, '-', '|');

	display_window* help_window = display_create_window(MAIN, false, "0:h-2:w:2");
	display_set_content_node_alignment(display_window_add_content_node(help_window, UNKNOWN, "Use [arrow keys] or [hjkl] to navigate - Press [q] to quit!"), CENTER);

	display_window* quit_window = display_create_window(EXIT, false, "w1/3:h1/3:w1/3:3");
	display_set_content_node_alignment(display_window_add_content_node(quit_window, UNKNOWN, "Are you sure you want to quit? Press 'q' to confirm"), CENTER);
	display_window_box(quit_window, '-', '|');

	/*
		list_window->selected = false;
		info_window->selected = true;
	*/

	int DONE = 0;
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
			case KEY_ENTER:
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

void init(){
	display_init();
	
	signal(SIGINT, handle_interrupt);
}

void handle_interrupt(){
	SIGINT_FLAG = 1;
}
