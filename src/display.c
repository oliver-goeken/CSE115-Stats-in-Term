#include "display.h"

/*
 * @brief intializes ncurses
 *
 * @return 0 on success, -1 on error
 *
 * @details
 * should only be called at the very start of using ncurses
 */
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

/*
 * @brief terminates ncurses
 *
 * @return 0 on success, -1 on error
 *
 * @details
 * only call when done with ncurses
 */
int display_kill(){
	if (endwin() != OK){
		return -1;
	}

	return 0;
}
