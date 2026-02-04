#include "stats.h"
#include "display.h"
#include <unistd.h>
#include <signal.h>

int SIGINT_FLAG = 0;

int main (){
	init();

	display_window* list_window = display_create_window(0, 0, LINES, COLS / 2);
	box(list_window->window, '|', '-');

	display_window* info_window = display_create_window(COLS / 2, 0, LINES, COLS / 2);
	box(info_window->window, '|', '-');

	int DONE = 0;
	while(!DONE){
		if (SIGINT_FLAG != 0){
			break;
		}

		display_draw_all_windows();
		char ch = getch();

		switch(ch){
			case 'q':
				DONE = 1;
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
