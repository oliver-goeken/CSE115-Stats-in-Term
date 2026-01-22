#include "display.h"

int display_init(){
	WINDOW* w = initscr();
	if (w == NULL){
		return -1;
	}

	cbreak();
	noecho();
	keypad(stdscr, true);

	return 0;
}

int display_terminate(){
	if (endwin() != OK){
		return -1;
	}

	return 0;
}
