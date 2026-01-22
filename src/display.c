#include "display.h"
#include <stdlib.h>

int display_init(){
	initscr();

	keypad(stdscr, TRUE);
	nonl();
	cbreak();
	noecho();
	curs_set(0);

	return 0;
}

int display_terminate(){
	if (endwin() != OK){
		return -1;
	}

	return 0;
}

display_window* display_create_window(int start_x, int start_y, int height, int width){
	display_window* new_display_window = malloc(sizeof(display_window));

	new_display_window->window = newwin(height, width, start_y, start_x);

	new_display_window->start_x = start_x;
	new_display_window->start_y = start_y;
	new_display_window->height = height;
	new_display_window->width = width;

	new_display_window->content = NULL;

	return new_display_window;
}

int display_destroy_window(display_window* window){
	delwin(window->window);
	free(window);

	return 0;
}

int display_move_window(display_window* window, int new_begin_x, int new_begin_y){
	window->start_x = new_begin_x;
	window->start_y = new_begin_y;

	wborder(window->window, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(window->window);
	delwin(window->window);

	window->window = newwin(window->height, window->width, window->start_y, window->start_x);

	display_draw_window(window);

	return 0;
}

int display_resize_window(display_window* window, int new_width, int new_height){
	window->width = new_width;
	window->height = new_height;

	wborder(window->window, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(window->window);
	delwin(window->window);

	window->window = newwin(window->height, window->width, window->start_y, window->start_x);

	display_draw_window(window);

	return 0;
}

int display_draw_window(display_window* window){
	display_draw_window_contents(window);

	wrefresh(window->window);

	return 0;
}

int display_draw_window_contents(display_window* window){
	mvwprintw(window->window, 1, 1, "Hello, World!");

	return 0;
}
