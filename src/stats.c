#include "stats.h"
#include "utils.h"
#include "display.h"
#include <unistd.h>
#include <signal.h>

int SIGINT_FLAG = 0;

int main (){
	init();

	// 0:0:h1/1:l1/1
	display_window* list_window = display_create_window(MAIN, "0:0:w1/2:h-2");
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_box(list_window, '-', '|');

	display_window* info_window = display_create_window(MAIN, "w1/2:0:w1/2:h-2");
	display_window_box(info_window, '-', '|');

	display_window* help_window = display_create_window(MAIN, "0:h-2:w:2");
	display_window_add_content_node(help_window, UNKNOWN, "press 'q' to quit!");

	display_window* quit_window = display_create_window(EXIT, "w1/3:h1/3:w1/3:3");
	display_set_content_node_alignment(display_window_add_content_node(quit_window, UNKNOWN, "Are you sure you want to quit? Press 'q' to confirm"), CENTER);
	display_window_box(quit_window, '-', '|');

	int DONE = 0;
	while(!DONE){
		if (SIGINT_FLAG != 0){
			break;
		}

		display_draw_all_windows();
		int ch = getch();

		if (ch == KEY_RESIZE){
			
		}

		switch (display_get_current_screen()){
			case MENU:
				break;
			case MAIN:
				switch(ch){
					case 'q':
						display_set_screen(EXIT);
						break;
				}
				break;
			case EXIT:
				switch(ch){
					case 'q':
						DONE = 1;
						break;
					default:
						display_set_screen(MAIN);
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
