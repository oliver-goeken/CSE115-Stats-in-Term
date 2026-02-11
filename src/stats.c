#include "stats.h"
#include "display.h"
#include <unistd.h>
#include <signal.h>

int SIGINT_FLAG = 0;

int main (){
	init();

	display_window* list_window = display_create_window(0, 0, LINES - 2, COLS / 2);
	display_window_add_content_node(list_window, UNKNOWN, "hello!");
	display_window_box(list_window, '-', '|');

	display_window* info_window = display_create_window(COLS / 2, 0, LINES - 2, COLS / 2);
	display_window_box(info_window, '-', '|');

	display_window* help_window = display_create_window(0, LINES - 2, 2, COLS);
	display_window_add_content_node(help_window, UNKNOWN, "press 'q' to quit!");
	display_window_add_content_node(help_window, QUIT_CONFIRM, "do you want to quit ('q' to confirm)");

	int DONE = 0;
	while(!DONE){
		if (SIGINT_FLAG != 0){
			break;
		}

		display_draw_all_windows();
		char ch = getch();

		switch(ch){
			case 'q':
				if (help_window->mode == QUIT_CONFIRM){
					DONE = 1;
				} else {
					help_window->mode = QUIT_CONFIRM;
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
