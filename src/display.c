#include "display.h"
#include <stdlib.h>
#include <string.h>

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

	display_init_window_contents(new_display_window);

	return new_display_window;
}

// TODO: delete old window contents from screen
int display_destroy_window(display_window* window){
	delwin(window->window);

	display_terminate_window_contents(window);

	free(window);

	return 0;
}

// TODO: delete old window contents from screen
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

int display_init_window_contents(display_window* window){
	window->content = malloc(sizeof(display_window_content_node));
	window->content->prev_node = NULL;
	window->content->next_node = NULL;

	window->content->data = NULL;
	
	return 0;
}

int display_terminate_window_contents(display_window* window){
	display_window_content_node* cur_node = window->content;

	while (cur_node != NULL){
		display_window_content_node* next_node = cur_node->next_node;

		if (cur_node->data != NULL){
			free(cur_node->data);
		}

		free(cur_node);

		cur_node = next_node;
	}

	return 0;
}

int display_window_add_content_node(display_window* window, char* data){
	display_window_content_node* cur_node = window->content;

	while(cur_node->data != NULL){
		if (cur_node->next_node != NULL){
			cur_node = cur_node->next_node;
		} else {
			cur_node->next_node->prev_node = cur_node;
			cur_node->next_node = malloc(sizeof(display_init_window_contents));
			cur_node = cur_node->next_node;
		}
	}

	cur_node->data = malloc(strlen(data) + 1);
	strcpy(cur_node->data, data);

	return 0;
}

int display_window_destroy_content_node(display_window* window, display_window_content_node* target_node){
	display_window_content_node* cur_node = window->content;

	while(cur_node != target_node){
		if(cur_node->next_node != NULL){
			cur_node = cur_node->next_node;
		} else {
			return -1;
		}
	}

	free(cur_node->data);
	cur_node->prev_node->next_node = cur_node->next_node; // make prev and next node reference each other
	cur_node->next_node->prev_node = cur_node->prev_node; // "      "

	free(cur_node);

	return 0;
}
