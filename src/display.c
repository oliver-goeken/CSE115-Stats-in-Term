#include "display.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static display_window_list* window_list;

int display_init(){
	time_t rawtime;
	time(&rawtime);

	initscr();

	keypad(stdscr, TRUE);
	nonl();
	cbreak();
	noecho();
	curs_set(0);
	ESCDELAY = 0;

	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);

	refresh();

	window_list = malloc(sizeof(display_window_list));
	window_list->root = NULL;
	window_list->current_screen = MAIN;

	return 0;
}

int display_terminate(){
	if (window_list->root != NULL){
		if (window_list->root->next_node == NULL){
			display_destroy_window(window_list->root->display_window);
			free(window_list->root);
		} else {
			display_window_list_node* cur_node = window_list->root;
			display_window_list_node* prev_node = window_list->root;

			while (cur_node != NULL){
				cur_node = cur_node->next_node;

				display_destroy_window(prev_node->display_window);
				free(prev_node);

				prev_node = cur_node;
			}
		}

		window_list->root = NULL;
	}

	free(window_list);

	if (endwin() != OK){
		return -1;
	}

	fflush(stdout);

	return 0;
}

void display_handle_winch(){
	clear();
	refresh();

	display_window_list_node* cur_node = window_list->root;

	while(cur_node != NULL){
		display_parse_dimensions_format(cur_node->display_window);

		cur_node = cur_node->next_node;
	}

	display_draw_all_windows();
}

void display_parse_dimensions_format(display_window* window){
	/*
	 *
	 * parse a formatted string into window startx, starty, width, height
	 *
	 * format: "[startx]:[starty]:[width]:[height]"
	 * for each field, 'h' or 'w' can be used to represent percentage of screen width and height respectively
	 * '/' indicates the division symbol between a numerator and denominator
	 * '-' or '+' indicates an offset {ALWAYS PUT OFFSET AFTER DIMENSIONS RATIO}
	 * example: "h1/3:w1/3:h1/3:3" = start at 1/3 height, 1/3 width; width of 1/3 height and a height of 3
	 * example: "0:0:w1/2:h-2" = start at 0, 0; width of 1/2; height of window height minus 2
	 * 
	 *
	 * ADD SUPPORT FOR WINDOW EXPANSION IF TOO SMALL FOR TEXT */

	int lines = LINES;
	int cols = COLS;

	int position = 0;
	for(int i = 0; i < 4; i ++){
		char numerator[256] = {0};
		char denominator[256] = {0};
		int num_index = 0;
		int den_index = 0;

		numerator[0] = '1';
		denominator[0] = '1';

		char offset[256] = {0};
		int off_index = 0;

		offset[0] = '0';

		int mult = 1;

		int category = 0;

		for (; window->dimensions_format[position] != ':' && window->dimensions_format[position] != '\0'; position ++){
			if (window->dimensions_format[position] == 'h'){
				mult = lines;
			} else if (window->dimensions_format[position] == 'w'){
				mult = cols;
			}

			if (window->dimensions_format[position] == '/'){
				category = 1;
			} else if (window->dimensions_format[position] == '-'){
				offset[0] = '-';
				offset[1] = '0';
				off_index = 1;

				category = 2;
			} else if (window->dimensions_format[position] == '+'){
				category = 2;
			}

			if(window->dimensions_format[position] >= 48 && window->dimensions_format[position] <= 57){
				if (category == 0){
					numerator[num_index] = window->dimensions_format[position];
					num_index ++;
				} else if (category == 1){
					denominator[den_index] = window->dimensions_format[position];
					den_index ++;
				} else if (category == 2){
					offset[off_index] = window->dimensions_format[position];
 					off_index ++;
				}
			}
			
		}

		int* field;
		switch (i){
			case 0:
				field = &(window->start_x);
				break;
			case 1:
				field = &(window->start_y);
				break;
			case 2:
				field = &(window->width);
				break;
			case 3:
				field = &(window->height);
				break;
		}


		*field = ((atoi(numerator) * mult) / atoi(denominator)) + atoi(offset);

		display_destroy_ncurses_window(window->window);
		window->window = newwin(window->height, window->width, window->start_y, window->start_x);

		position ++;
	}
}

Screen display_get_current_screen(){
	return window_list->current_screen;
}

int display_set_screen(Screen screen){
	display_window_list_node* cur_node = window_list->root;

	while(cur_node != NULL){
		if (cur_node->display_window->associated_screen == window_list->current_screen){
			werase(cur_node->display_window->window);
			wrefresh(cur_node->display_window->window);
		}

		cur_node = cur_node->next_node;
	}

	window_list->current_screen = screen;

	return 0;
}

display_window* display_create_window(Screen screen, bool selectable, char* dimension_format_string){
	display_window* new_display_window = malloc(sizeof(display_window));

	new_display_window->dimensions_format = dimension_format_string;
	display_parse_dimensions_format(new_display_window);

	new_display_window->window = newwin(new_display_window->height, new_display_window->width, new_display_window->start_y, new_display_window->start_x);
	
	new_display_window->boxed = FALSE;
	new_display_window->horizontal_edges = '\0';
	new_display_window->vertical_edges = '\0';

	new_display_window->selectable = selectable;

	new_display_window->associated_screen = screen;

	new_display_window->mode = UNKNOWN;

	display_window_list_node* new_window_node = malloc(sizeof(display_window_list_node));

	new_window_node->display_window = new_display_window;
	new_window_node->next_node = NULL;

	if (window_list->root == NULL){
		window_list->root = new_window_node;
		new_window_node->display_window->selected = new_window_node->display_window->selectable ? true : false;
	} else {
		display_window_list_node* cur_node = window_list->root;

		while (cur_node->next_node != NULL){
			cur_node = cur_node->next_node;
		}

		cur_node->next_node = new_window_node;
		new_window_node->display_window->selected = false;
	}

	return new_display_window;
}

int display_window_box(display_window* window, char vertical_edges, char horizontal_edges){
	window->boxed = TRUE;
	window->vertical_edges = vertical_edges;
	window->horizontal_edges = horizontal_edges;

	box(window->window, horizontal_edges, vertical_edges);

	return 0;
}

int display_destroy_window(display_window* window){
	display_destroy_ncurses_window(window->window);
	display_terminate_window_contents(window);

	free(window);

	return 0;
}

int display_window_select_next_node(display_window* window){
	if (window == NULL){
		return -3;
	}

	display_window_content_node* selected_node;
	if ((selected_node = display_window_get_current_selection(window)) == NULL){
		return -1;
	}

	if (selected_node->next_node != NULL){
		selected_node->selected = false;
		selected_node->next_node->selected = true;
	} else {
		return -2;
	}

	return 0;
}

int display_window_select_previous_node(display_window* window){
	if (window == NULL){
		return -3;
	}

	display_window_content_node* selected_node;
	if ((selected_node = display_window_get_current_selection(window)) == NULL){
		return -1;
	}

	if (selected_node->prev_node != NULL){
		selected_node->selected = false;
		selected_node->prev_node->selected = true;
	} else {
		return -2;
	}

	return 0;
}

display_window_content_node* display_window_get_current_selection(display_window* window){
	display_window_content_node* cur_node = window->content;

	while (cur_node != NULL && cur_node->selected == false){
		cur_node = cur_node->next_node;
	}

	return cur_node;
}

display_window_list_node* display_get_current_window(){
	display_window_list_node* selected_window = window_list->root;

	while (selected_window != NULL && !(selected_window->display_window->associated_screen == window_list->current_screen && selected_window->display_window->selected == true)){
		selected_window = selected_window->next_node;
	}

	return selected_window;
}

int display_select_next_window(){
	display_window_list_node* cur_selection = display_get_current_window();

	if (cur_selection == NULL){
		return -1;
	}

	display_window_list_node* next_selection = window_list->root;

	// find which window is entirely to the right of current window
	while (next_selection != NULL){
		if (next_selection->display_window->associated_screen == window_list->current_screen && next_selection->display_window->selectable){
			if (next_selection->display_window->start_x > cur_selection->display_window->start_x){
				cur_selection->display_window->selected = false;
				next_selection->display_window->selected = true;
				return 0;
			}
		}
		next_selection = next_selection->next_node;
	}

	return -1;
}

int display_select_previous_window(){
	display_window_list_node* cur_selection = display_get_current_window();

	if (cur_selection == NULL){
		return -1;
	}

	display_window_list_node* next_selection = window_list->root;

	// find which window is entirely to the right of current window
	while (next_selection != NULL){
		if (next_selection->display_window->associated_screen == window_list->current_screen && next_selection->display_window->selectable){
			if (next_selection->display_window->start_x < cur_selection->display_window->start_x){
				cur_selection->display_window->selected = false;
				next_selection->display_window->selected = true;
				return 0;
			}
		}
		next_selection = next_selection->next_node;
	}

	return -1;
}

int display_move_window(display_window* window, int new_begin_x, int new_begin_y){
	window->start_x = new_begin_x;
	window->start_y = new_begin_y;

	display_window_change_attributes(window, new_begin_x, new_begin_y, window->width, window->height);

	return 0;
}

int display_resize_window(display_window* window, int new_width, int new_height){
	window->width = new_width;
	window->height = new_height;

	display_window_change_attributes(window, window->start_x, window->start_y, new_width, new_height);

	return 0;
}

int display_window_change_attributes(display_window* window, int new_start_x, int new_start_y, int new_width, int new_height){
	display_destroy_ncurses_window(window->window);

	window->window = newwin(new_height, new_width, new_start_y, new_start_x);

	display_draw_window(window);

	return 0;
}

int display_destroy_ncurses_window(WINDOW* window){
	werase(window);
	wrefresh(window);
	delwin(window);

	return 0;
}

int display_draw_window(display_window* window){
	if (window->boxed)
		display_window_box(window, window->vertical_edges, window->horizontal_edges);

	display_draw_window_contents(window);

	wrefresh(window->window);

	return 0;
}

int display_draw_all_windows(){
	display_window_list_node* cur_node = window_list->root;

	while (cur_node != NULL){
		if (cur_node->display_window->associated_screen == window_list->current_screen)
			display_draw_window(cur_node->display_window);

		cur_node = cur_node->next_node;
	}

	return 0;
}

int display_draw_window_contents(display_window* window){
	int startx = window->boxed ? 1 : 0;
	int starty = window->boxed ? 1 : 0;

	display_window_content_node* content_node = window->content;

	while(content_node != NULL){
		if (content_node->mode == window->mode){
			if (content_node->data != NULL){
				int newline_count = 0;
				for (int i = 0; content_node->data[i] != '\0'; i ++){
					if (content_node->data[i] == '\n')
						newline_count ++;
				}

				int window_available_width = window->width - (window->boxed ? 2 : 0);

				char trunc_string[window_available_width + 1];

				string_truncate_middle(content_node->data, window_available_width, trunc_string);

				int alignment_start_x = startx;
				if(content_node->alignment == CENTER){
					alignment_start_x = ((window_available_width - strlen(trunc_string)) / 2) + (window->boxed);

					if (alignment_start_x < 0){
						alignment_start_x = 0;
					}
				}

				if (content_node->associated_window->selectable && content_node->selected && content_node->associated_window->selected){
					wattron(window->window, COLOR_PAIR(2));
				} else {
					wattron(window->window, COLOR_PAIR(1));
				}
				mvwprintw(window->window, starty, alignment_start_x, trunc_string);
				wattron(window->window, COLOR_PAIR(1));
				starty += newline_count;
			}

			starty ++;
		}

		content_node = content_node->next_node;
	}

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

	window->content = NULL;

	return 0;
}

display_window_content_node* display_window_add_content_node(display_window* window, Mode mode, char* data){
	display_window_content_node* cur_node;

	if (window->content != NULL){
		cur_node = window->content;

		while (cur_node->next_node != NULL){
			cur_node = cur_node->next_node;
		}

		cur_node->next_node = malloc(sizeof(display_window_content_node));
		cur_node->next_node->prev_node = cur_node;

		cur_node = cur_node->next_node;
		cur_node->selected = false;
	} else {
		window->content = malloc(sizeof(display_window_content_node));
		window->content->prev_node = NULL;

		cur_node = window->content;
		cur_node->selected = window->selectable ? true : false;
	}

	cur_node->next_node = NULL;
	cur_node->mode = mode;
	cur_node->alignment = LEFT;
	cur_node->color_pair = 1;
	cur_node->associated_window = window;

	if (strlen(data) == 0){
		cur_node->data = NULL;
	} else {
		cur_node->data = malloc(strlen(data) + 1);
		strcpy(cur_node->data, data);
	}

	return cur_node;
}

int display_set_content_node_alignment(display_window_content_node* content_node, Alignment new_alignment){
	content_node->alignment = new_alignment;

	return 0;
}

int display_set_contend_node_color(display_window_content_node* content_node, int color_pair){
	content_node->color_pair = color_pair;

	return 0;
}

/*
 *
 * needs some maintanance!!!
 * i wrote this very poorly!
 *
 */
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

	// make prev and next node reference each other:
	//
	// if cur_node does not have a previous node, it is the root node
	if (cur_node->prev_node != NULL){
		cur_node->prev_node->next_node = cur_node->next_node;
	} else {
		window->content = cur_node->next_node;
	}
	// only change next_node->prev_node if next_node exists
	if (cur_node->next_node != NULL){
		cur_node->next_node->prev_node = cur_node->prev_node;
	}

	free(cur_node);

	return 0;
}
