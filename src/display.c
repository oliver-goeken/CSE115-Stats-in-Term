#include "display.h"
#include "utils.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

// bit shifting from https://www.chiefdelphi.com/t/extracting-individual-bits-in-c/48028
#define get_bit(bitmap, pos)  (( bitmap & (1 << (pos - 1)) ? 1 : 0 ))

static display_info* display_info_struct;

int display_init(){
	if (display_ncurses_init() != 0){
		log_err("error with ncurses intialization");
		return -1;
	}

	if (display_intitialize_display_info() != 0){
		log_err("error terminating display info");
		return -2;
	}

	return 0;
}

int display_ncurses_init(){
	if (initscr() == NULL){
		log_err("initscr failed");
		return -1;
	}

	if (LINES < 10 || COLS < 50){
		log_err("terminal too small to run");

		fprintf(stderr, "Minimum suppported terminal Size is 50 wide and 10 high.\n");
		endwin();
		fflush(stdout);

		exit(1);
	}

	keypad(stdscr, TRUE);
	nonl();
	cbreak();
	noecho();
	curs_set(0);
	ESCDELAY = 0;
	nodelay(stdscr, TRUE);

	timeout(33);

	start_color();
	init_pair(COLOR_PAIR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_PAIR_SELECTED, COLOR_BLACK, COLOR_WHITE);
	init_pair(COLOR_PAIR_ERROR, COLOR_WHITE, COLOR_RED);

	refresh();

	return 0;
}

int display_terminate(){
	if (display_ncurses_terminate() != 0){
		log_err("error terminating ncurses display");
		return -1;
	}

	if (display_terminate_display_info() != 0){
		log_err("error terminating display info");
		return -2;
	}

	return 0;
}

int display_ncurses_terminate(){
	if (endwin() != OK){
		log_err("endwin failed");
		return -1;
	}

	return 0;
}


int display_intitialize_display_info(){
	if (display_info_struct != NULL){
		log_err("display info struct already exists");
		return -1;
	}

	display_info_struct = malloc(sizeof(display_info));

	display_info_struct->current_screen = NULL;
	display_info_struct->screen_list = display_initialize_screen_list();

	return 0;
}

int display_terminate_display_info(){
	if (display_info_struct == NULL){
		log_err("display info struct does not exist");
		return -1;
	}

	display_terminate_screen_list(display_info_struct->screen_list);
	free(display_info_struct);

	return 0;
}


display_screen_list* display_initialize_screen_list(){
	display_screen_list* new_screen_list = malloc(sizeof(display_screen_list));
	new_screen_list->root = NULL;

	return new_screen_list;
}

int display_terminate_screen_list(display_screen_list* screen_list){
	if (screen_list == NULL){
		log_err("screen list does not exist");
		return -1;
	}

	display_screen_node* screen_node = screen_list->root;
	display_screen_node* next_screen_node = screen_list->root;

	while (screen_node != NULL){
		next_screen_node = screen_node->next_node;

		display_destroy_screen_node(screen_node);

		screen_node = next_screen_node;
	}

	free(screen_list);

	return 0;
}


display_screen_node* display_create_screen_node(){
	display_screen_node* new_screen_node = malloc(sizeof(display_screen_node));
	new_screen_node->next_node = NULL;
	new_screen_node->display_screen = NULL;

	return new_screen_node;
}

int display_destroy_screen_node(display_screen_node* screen_node){
	if (screen_node == NULL){
		log_err("screen node does not exist");
		return -1;
	}

	display_destroy_screen(screen_node->display_screen);

	free(screen_node);

	return 0;
}

display_content_node* display_get_selected_content_node(){
	if (display_get_current_screen() == NULL){
		log_err("no currently selected screen");
		return NULL;
	}

	display_window_list_node* current_window = display_screen_get_selected_window_node(display_get_current_screen());

	if (current_window == NULL || current_window->display_window == NULL || current_window->display_window->contents == NULL){
		log_err("window to select content node of doesn't exist, or its contents are unitialized");
		return NULL;
	}

	display_content_node* cur_node = current_window->display_window->contents->root;

	while (cur_node != NULL){
		if (cur_node->selected == CONTENT_NODE_SELECTED){
			return cur_node;
		}

		cur_node = cur_node->next_node;
	}

	log_err("cant find a selected content node");
	return NULL;
}

display_screen* display_create_new_screen(char* screen_name){
	if (display_info_struct == NULL){
		log_err("display info struct doesn't exist");
		return NULL;
	}

	if (display_info_struct->screen_list == NULL){
		log_err("display info struct does not have a screen list");
		return NULL;
	}

	display_screen_node* new_screen_node = display_create_screen_node();

	display_screen* new_screen = malloc(sizeof(display_screen));
	new_screen->name = screen_name;
	display_create_window_list(new_screen);

	new_screen_node->display_screen = new_screen;

	if (display_info_struct->screen_list->root == NULL){
		display_info_struct->screen_list->root = new_screen_node;
	} else {
		display_screen_node* cur_node = display_info_struct->screen_list->root;
		display_screen_node* prev_node = cur_node;

		while (cur_node->next_node != NULL){
			if (strcmp(cur_node->display_screen->name, screen_name) == 0){
				free(new_screen);
				log_err_f("a screen with name '%s' already exists", screen_name);
				return NULL;
			}

			prev_node = cur_node;
			cur_node = cur_node->next_node;
		}

		prev_node->next_node = new_screen_node;
	}

	return new_screen;
}

int display_destroy_screen(display_screen* screen){
	if (screen == NULL){
		log_err("screen to destroy doesn't exist");
		return -1;
	} 

	log_msg_f("destroying screen %s", screen->name);

	if (screen->window_list != NULL){
		display_destroy_window_list(screen->window_list);
	}

	free(screen);

	return 0;
}

int display_handle_winch(){
	if (display_info_struct == NULL){
		log_err("display info struct doesn't exist");
		return -1;
	}  if (display_info_struct->current_screen == NULL){
		log_err("display info struct doesn't have a current screen");
		return -2;
	} if (display_info_struct->current_screen->window_list == NULL){
		log_err("display info struct's current screen doesn't have a window list");
		return -3;
	}

	display_window_list_node* window_node = display_info_struct->current_screen->window_list->root;

	while (window_node != NULL){
		display_parse_dimensions_format(window_node->display_window);

		window_node = window_node->next_node;
	}

	return 0;
}

int display_screen_draw_windows(display_screen* screen){
	if (screen == NULL){
		log_err("screen to draw windows of does not exist");
		return -1;
	} if (screen->window_list == NULL){
		log_err("screen's window list does not exist");
		return -2;
	}

	display_window_list_node* cur_node = screen->window_list->root;

	while (cur_node != NULL){
		if (cur_node->display_window != NULL && cur_node->display_window->visible == WINDOW_VISIBLE){
			werase(cur_node->display_window->ncurses_window);
			display_draw_window(cur_node->display_window);
		}

		cur_node = cur_node->next_node;
	}

	doupdate();

	return 0;
}

int display_set_screen(display_screen* screen){
	if (screen == NULL){
		log_err("screen to set as current screen does not exist");
		return -1;
	} if (display_info_struct == NULL){
		log_err("display info struct does not exist");
		return -1;
	}

	if (display_info_struct->current_screen != screen){
		erase();
		display_info_struct->current_screen = screen;
	}

	return 0;
}

int display_screen_set_selected_window(display_screen* screen, display_window* window){
	if (screen == NULL){
		log_err("screen does not exist");
		return -1;
	} if (window == NULL){
		log_err("window does not exist");
		return -2;
	} if (screen->window_list == NULL){
		log_err("screen does not have window list");
		return -3;
	} if (window->selected == WINDOW_UNSELECTABLE){
		log_err("window is not selectable");
		return -4;
	}

	display_window_list_node* window_node = screen->window_list->root;

	while (window_node != NULL){
		if (window_node->display_window != window){
			if (window_node->display_window->selected != WINDOW_UNSELECTABLE){
				display_window_set_selected(window, WINDOW_NOT_SELECTED);
			}
		} else {
			display_window_set_selected(window, WINDOW_SELECTED);
		}

		window_node = window_node->next_node;
	}

	return 0;
}

int display_screen_select_window_directional(display_screen* screen, int direction){
	if (screen == NULL){
		log_err("screen does not exist");
		return -1;
	} if (screen->window_list == NULL){
		log_err("screen's window list does not exist");
		return -2;
	} if (screen->window_list->root == NULL){
		log_err("screen's window list does not have a root");
		return -3;
	} if ((direction != NEXT_WINDOW) && (direction != PREV_WINDOW)){
		log_err("invalid direction");
		return -4;
	}
	
	display_window_list_node* currently_selected = display_screen_get_selected_window_node(screen);
	display_window_list_node* cur_node = screen->window_list->root;

	int* comp_greater;
	int* comp_lesser;

	if (currently_selected != NULL){
		while (cur_node != NULL){
			if (direction == NEXT_WINDOW){
				comp_greater = &(cur_node->display_window->start_x);
				comp_lesser = &(currently_selected->display_window->start_x);
			} else {
				comp_greater = &(currently_selected->display_window->start_x);
				comp_lesser = &(cur_node->display_window->start_x);
			}

			if (cur_node != currently_selected && (*comp_greater > *comp_lesser) && cur_node->display_window->selected != WINDOW_UNSELECTABLE){
				display_window_set_selected(currently_selected->display_window, WINDOW_NOT_SELECTED);
				display_window_set_selected(cur_node->display_window, WINDOW_SELECTED);

				return 0;
			}

			cur_node = cur_node->next_node;
		}
	}

	return 0;
}

int display_screen_select_next_window(display_screen* screen){
	if (display_screen_select_window_directional(screen, NEXT_WINDOW) != 0){
		return -1;
	}
	
	return 0;
}

int display_screen_select_previous_window(display_screen* screen){
	if (display_screen_select_window_directional(screen, PREV_WINDOW) != 0){
		return -1;
	}

	return 0;
}

int display_set_current_screen(display_screen* screen){
	if (screen == NULL){
		log_err("screen does not exist");
		return -1;
	} if (display_info_struct == NULL){
		log_err("display info struct does not exist");
		return -2;
	}
	
	display_info_struct->current_screen = screen;

	return 0;
}

display_screen* display_get_current_screen(){
	if (display_info_struct == NULL){
		log_err("display info struct does not exist");
		return NULL;
	}

	return display_info_struct->current_screen;
}

int display_draw_window(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (window->ncurses_window == NULL){
		log_err("ncurses window of window does not exist");
		return -2;
	}

	if (window->visible == WINDOW_VISIBLE){
		if (window->expand == WINDOW_EXPAND_TO_FIT_TEXT){
			if (window->contents != NULL && window->contents->root != NULL){
				display_content_node* content_node = window->contents->root;

				while (content_node != NULL){
					if (content_node->data == NULL || content_node->data->text_data == NULL){
						continue;
					}

					int size_diff;
					int new_start_x;
					int new_width;

					size_diff = window->width - (window->boxed ? 2 : 0) - strlen(content_node->data->text_data);

					if (size_diff % 2 != 0){
						size_diff --;
					}

					if (size_diff < 0){
						new_start_x = window->start_x + (size_diff / 2);
						new_width = window->width - size_diff;

						if (new_start_x < 0){
							new_start_x = 0;
						}

						if (new_start_x + new_width >= COLS){
							new_width = COLS - new_start_x;
						}

						window->start_x = new_start_x;
						window->width = new_width;
					}

					content_node = content_node->next_node;
				}
			}

			display_reset_ncurses_window(window);
		}

		if (window->boxed){
			//box(window->ncurses_window, '|', '-');
			display_box_window(window);
		}

		display_window_draw_contents(window);
	} else {
		werase(window->ncurses_window);
	}

	wnoutrefresh(window->ncurses_window);

	return 0;
}

int display_draw_window_and_update(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	}

	display_draw_window(window);

	doupdate();

	return 0;
}

int display_box_window(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (window->ncurses_window == NULL){
		log_err("window's ncurses window does not exist");
		return -2;
	}

	uint8_t box_sides = window->box_sides;

	wborder(window->ncurses_window, get_bit(box_sides, 6) ? ACS_VLINE : ' ', 
									get_bit(box_sides, 7) ? ACS_VLINE : ' ', 
									get_bit(box_sides, 5) ? ACS_HLINE : ' ', 
									get_bit(box_sides, 8) ? ACS_HLINE : ' ',
									get_bit(box_sides, 1) ? ACS_ULCORNER : ' ', 
									get_bit(box_sides, 2) ? ACS_URCORNER : ' ', 
									get_bit(box_sides, 3) ? ACS_LLCORNER : ' ', 
									get_bit(box_sides, 4) ? ACS_LRCORNER : ' ');

	return 0;
}

display_window_list_node* display_screen_get_selected_window_node(display_screen* screen){
	if (screen == NULL){
		log_err("screen does not exist");
		return NULL;
	} if (screen->window_list == NULL){
		log_err("screen's window list does not exist");
		return NULL;
	}

	display_window_list_node* cur_node = screen->window_list->root;

	while (cur_node != NULL){
		if (cur_node->display_window->selected == WINDOW_SELECTED){
			return cur_node;
		}

		cur_node = cur_node->next_node;
	}

	log_err("could not find a selected window node");
	return NULL;
}

display_window* display_screen_add_new_window(display_screen* screen, char* dimensions_format){
	if (screen == NULL){
		log_err("screen does not exist");
		return NULL;
	} if (screen->window_list == NULL) {
		log_err("screen's window list does not exist");
		return NULL;
	} if (dimensions_format == NULL){
		log_err("dimension format is empty");
		return NULL;
	}

	display_window* new_window;
	
	if ((new_window = display_create_window(dimensions_format)) == NULL){
		return NULL;
	}

	display_window_list_node* new_node = malloc(sizeof(display_window_list_node));
	new_node->display_window = new_window;
	new_node->next_node = NULL;

	if (screen->window_list->root == NULL){
		screen->window_list->root = new_node;
	} else {
		display_window_list_node* cur_node = screen->window_list->root;	
		display_window_list_node* prev_node = cur_node;

		while (cur_node != NULL){
			prev_node = cur_node;
			cur_node = cur_node->next_node;
		}

		prev_node->next_node = new_node;
	}

	return new_window;
}

int display_screen_destroy_window(display_screen* screen, display_window* window){
	if (screen == NULL){
		log_err("screen does not exist");
		return -1;
	} if (screen->window_list == NULL){
		log_err("screen's window list does not exist");
		return -2;
	} if (screen->window_list->root == NULL) {
		log_err("screen's window list does not have root");
		return -3;
	}

	display_window_list_node* cur_node = screen->window_list->root;
	display_window_list_node* prev_node = cur_node;

	while (cur_node != NULL){
		prev_node = cur_node;

		if (cur_node->display_window == window){
			prev_node->next_node = cur_node->next_node;
			display_destroy_window(window);

			free(cur_node);

			return 0;
		}

		cur_node = cur_node->next_node;
	}

	return 0;
}

display_window* display_window_node_get_window(display_window_list_node* window_node){
	if (window_node == NULL){
		log_err("window node does not exist");
		return NULL;
	} if (window_node->display_window == NULL){
		log_err("window node's display window does not exist");
		return NULL;
	}

	return window_node->display_window;
}

int display_window_set_visibility(display_window* window, bool visible){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if ((visible != WINDOW_VISIBLE) && (visible != WINDOW_HIDDEN)){
		log_err("invalid visibility");
		return -2;
	}

	window->visible = visible;

	return 0;
}

int display_window_set_boxed(display_window* window, bool boxed){
	if (window == NULL){
		log_err("window does not exist")
		return -1;
	} if ((boxed != WINDOW_VISIBLE) && (boxed != WINDOW_HIDDEN)){
		log_err("invalid window visiblity");
		return -2;
	}

	window->boxed = boxed;

	return 0;
}

int display_window_set_box_sides(display_window* window, uint8_t sides){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	}

	window->box_sides = sides;

	return 0;
}

int display_window_set_expansion(display_window* window, bool expand){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if ((expand != WINDOW_EXPAND_TO_FIT_TEXT) && (expand != WINDOW_SET_SIZE)){
		log_err("invalid window expansion parameter");
		return -2;
	}

	window->expand = expand;

	return 0;
}

int display_window_set_selected(display_window* window, int selected){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if ((selected != WINDOW_SELECTED) && (selected != WINDOW_NOT_SELECTED) && (selected != WINDOW_UNSELECTABLE)){
		log_err("invalid window selection parameter");
		return -2;
	}

	window->selected = selected;

	return 0;
}

display_content_node* display_window_get_selected_node(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return NULL;
	} if (window->contents == NULL){
		log_err("window contents does not exist");
		return NULL;
	}

	display_content_node* cur_node = window->contents->root;

	while (cur_node != NULL){
		if (cur_node->selected == CONTENT_NODE_SELECTED){
			return cur_node;
		}

		cur_node = cur_node->next_node;
	}

	log_err("could not find a selected node");
	return NULL;
}

int display_window_add_content_node(display_window* window, display_content_node* content_node){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (content_node == NULL){
		log_err("content node does not exist");
		return -2;
	} if (window->contents == NULL){
		log_err("window contents does not exist");
		return -3;
	}

	if (window->contents->root == NULL){
		window->contents->root = content_node;
		content_node->selected = CONTENT_NODE_SELECTED;
	} else {
		display_content_node* cur_node = window->contents->root;

		while (cur_node->next_node != NULL){
			cur_node = cur_node->next_node;
		}

		cur_node->next_node = content_node;
	}

	return 0;
}

int display_window_draw_contents(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (window->contents == NULL){
		log_err("window contents does not exist");
		return -1;
	}

	// window expand to fit text

	int start_x = window->boxed ? 1 : 0;
	int start_y = window->boxed ? 1 : 0;

	display_content_node* cur_node = window->contents->root;

	for (int i = 0; i < window->content_offset && cur_node != NULL; i ++){
		cur_node = cur_node->next_node;
	}

	display_content_node* prev_node = cur_node;

	while (cur_node != NULL && start_y <= window->height - (window->boxed ? 0 : 1)){
		if (cur_node->timeout != 0){
			time_t current_time;
			time(&current_time);

			if (cur_node->timeout < current_time){
				display_content_node* next_node = cur_node->next_node;

				if (window->contents->root == prev_node){
					window->contents->root = next_node;
				} else {
					prev_node->next_node = next_node;
				}

				display_destroy_content_node(cur_node);

				cur_node = next_node;

				continue;
			}
		}
		
		display_draw_content_node(window, start_x, start_y, cur_node);

		start_y ++;

		prev_node = cur_node;
		cur_node = cur_node->next_node;
	}

	return 0;
}

display_screen* display_window_get_screen(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return NULL;
	} if (display_info_struct == NULL){
		log_err("display info struct does not exist");
		return NULL;
	} if (display_info_struct->screen_list == NULL){
		log_err("display info struct screen list does not exist");
		return NULL;
	}

	display_screen_node* cur_screen = display_info_struct->screen_list->root;

	while (cur_screen != NULL){
		if (cur_screen->display_screen != NULL && cur_screen->display_screen->window_list != NULL){
			display_window_list_node* cur_window = cur_screen->display_screen->window_list->root;

			while (cur_window != NULL){
				if (cur_window->display_window == window){
					return cur_screen->display_screen;
				}

				cur_window = cur_window->next_node;
			}
		}

		cur_screen = cur_screen->next_node;
	}

	log_err("could not find screen for this window");
	return NULL;
}

int display_window_select_next_node(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (window->contents == NULL){
		log_err("window contents does not exist");
		return -2;
	} if (window->contents->root == NULL){
		log_err("window contents does not have root node");
		return -3;
	}

	display_content_node* currently_selected = display_window_get_selected_node(window);
	display_content_node* new_selection = currently_selected->next_node;

	if (new_selection == NULL){
		return 0;
	} 

	if (currently_selected != NULL){
		display_content_node_set_selected(currently_selected, CONTENT_NODE_NOT_SELECTED);
		display_content_node_set_selected(new_selection, CONTENT_NODE_SELECTED);

		// loop if not null to see if selection (position - offset) > window height, if so update offset
		int node_window_pos_x = 1;

		display_content_node* cur_node = window->contents->root;
		while (cur_node != NULL && cur_node != new_selection){
			node_window_pos_x ++;

			cur_node = cur_node->next_node;
		}

		node_window_pos_x -= window->content_offset;

		if (node_window_pos_x >= window->height - (window->boxed ? 1 : 0)){
			window->content_offset ++;
		}
	}


	return 0;
}

int display_window_select_prev_node(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (window->contents == NULL){
		log_err("window contents does not exist");
		return -2;
	} if (window->contents->root == NULL){
		log_err("window contents does not have root node");
		return -3;
	}

	display_content_node* currently_selected = display_window_get_selected_node(window);
	display_content_node* new_selection = NULL;

	if (currently_selected != NULL){
		display_content_node* cur_node = window->contents->root;
		while (cur_node != NULL && cur_node != currently_selected){
			if (cur_node->next_node == currently_selected){
				new_selection = cur_node;
				break;
			}

			cur_node = cur_node->next_node;
		}

		display_content_node_set_selected(currently_selected, CONTENT_NODE_NOT_SELECTED);
		display_content_node_set_selected(new_selection, CONTENT_NODE_SELECTED);
		
		// loop if not null to see if selection (position - offset) > window height, if so update offset
		int node_window_pos_x = 1;

		cur_node = window->contents->root;
		while (cur_node != NULL && cur_node != new_selection){
			node_window_pos_x ++;

			cur_node = cur_node->next_node;
		}

		if (node_window_pos_x <= window->content_offset){
			window->content_offset --;
		}
	}

	return 0;
}

int display_generic_select_next_node(){
	return display_window_select_next_node(display_window_node_get_window(display_screen_get_selected_window_node(display_get_current_screen())));
}

int display_generic_select_prev_node(){
	return display_window_select_prev_node(display_window_node_get_window(display_screen_get_selected_window_node(display_get_current_screen())));
}

display_window_list* display_create_window_list(display_screen* screen){
	if (screen == NULL){
		log_err("screen does not exist");
		return NULL;
	}

	screen->window_list = malloc(sizeof(display_window_list));

	return screen->window_list;
}

int display_destroy_window_list(display_window_list* window_list){
	if (window_list == NULL){
		log_err("window list does not exist");
		return -1;
	} 

	if (window_list->root != NULL){
		display_window_list_node* window_node = window_list->root;
		display_window_list_node* next_node = window_node->next_node;

		while (window_node != NULL){
			next_node = window_node->next_node;

			display_destroy_window(window_node->display_window);
			free(window_node);

			window_node = next_node;
		}
	}

	free(window_list);

	return 0;
}

int display_reset_ncurses_window(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} 

	if (window->ncurses_window != NULL){
		display_destroy_ncurses_window(window->ncurses_window);
	}

	window->ncurses_window = newwin(window->height, window->width, window->start_y, window->start_x);

	return 0;
}

int display_destroy_ncurses_window(WINDOW* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	}

	werase(window);
	delwin(window);

	return 0;
}

int display_parse_dimensions_format(display_window* window){
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

	if (window == NULL) {
		log_err("window does not exist");
		return -1;
	}

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

		display_reset_ncurses_window(window);

		position ++;
	}

	return 0;
}

display_window* display_create_window(char* dimensions_format){
	if (dimensions_format == NULL){
		log_err("dimensions format is empty");
		return NULL;
	}

	display_window* new_window = malloc(sizeof(display_window));
	
	new_window->dimensions_format = dimensions_format;
	display_parse_dimensions_format(new_window);

	new_window->visible = WINDOW_VISIBLE;
	new_window->expand = WINDOW_SET_SIZE;
	new_window->selected = WINDOW_NOT_SELECTED;

	new_window->boxed = WINDOW_NOT_BOXED;
	new_window->box_sides = 0b11111111;

	new_window->content_offset = 0;

	display_window_init_contents(new_window);

	return new_window;
}

int display_window_init_contents(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (window->contents != NULL){
		log_err("window contents does not exist");
		return -2;
	}

	window->contents = malloc(sizeof(display_window_contents));

	return 0;
}

int display_destroy_window(display_window* window){
	if (window->ncurses_window != NULL){
		display_destroy_ncurses_window(window->ncurses_window);
	}

	if (window->contents != NULL){
		display_terminate_window_contents(window);
	}

	free(window);

	return 0;
}

int display_window_destroy_content_nodes(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (window->contents == NULL){
		log_err("window contents does not exist");
		return -2;
	} if (window->contents->root == NULL){
		log_err("window contents does not have root");
		return -3;
	}

	display_content_node* cur_node = window->contents->root;
	display_content_node* next_node;

	while (cur_node != NULL){
		next_node = cur_node->next_node;

		display_destroy_content_node(cur_node);

		cur_node = next_node;
	}

	window->contents->root = NULL;

	return 0;
}

int display_terminate_window_contents(display_window* window){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (window->contents == NULL){
		log_err("window does not have contents");
		return -2;
	} 

	display_window_destroy_content_nodes(window);

	free(window->contents);

	window->content_offset = 0;

	return 0;
}

display_content_node* display_create_content_node(){
	display_content_node* new_content_node = malloc(sizeof(display_content_node));

	new_content_node->alignment = CONTENT_NODE_ALIGN_LEFT;
	new_content_node->next_node = NULL;
	new_content_node->handle_interact = NULL;
	new_content_node->selected = 0;

	new_content_node->timeout = 0;
	time(&(new_content_node->time_created));

	display_content_node_init_data(new_content_node);

	return new_content_node;
}

int display_content_node_set_timeout(display_content_node* content_node, long time_after_creation){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	} if (time_after_creation < 0){
		log_err("timeout is before creation time");
		return -2;
	}

	content_node->timeout = content_node->time_created + time_after_creation;

	return 0;
}

int display_content_node_set_color_pair(display_content_node* content_node, int color_pair_num){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	}

	content_node->data->color_pair_num = color_pair_num;

	return 0;
}

int display_content_node_set_interaction(display_content_node* content_node, void (*handle_interact)(display_content_node* content_node)){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	} else if (handle_interact == NULL){
		log_err("handle interact not passed");
		return -2;
	}

	content_node->handle_interact = handle_interact;

	return 0;
}

int display_handle_interact(display_content_node* content_node){
	if (content_node->handle_interact != NULL){
		(*(content_node->handle_interact))(content_node);
	}
	
	return 0;
}

int display_destroy_content_node(display_content_node* content_node){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	}

	if (content_node->data != NULL){
		free(content_node->data);
	}

	free(content_node);

	return 0;
}

int display_set_content_node_alignment(display_content_node* content_node, content_node_alignment alignment){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	} 

	content_node->alignment = alignment;

	return 0;
}

int display_content_node_set_data(display_content_node* content_node, display_content_node_data* data){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	} if (data == NULL){
		log_err("data node does not exist");
		return -2;
	}

	if (content_node->data != NULL){
		display_content_node_terminate_data(content_node);
	}

	content_node->data = data;

	return 0;
}

int display_content_node_clear_data(display_content_node* content_node){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	} if (content_node->data == NULL){
		log_err("content node's data struct does not exist");
		return -1;
	}

	free(content_node->data);

	return 0;
}

int display_content_node_set_selected(display_content_node* content_node, int selected){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	} if ((selected != CONTENT_NODE_SELECTED) && (selected != CONTENT_NODE_NOT_SELECTED) && (selected != CONTENT_NODE_UNSELECTABLE)){
		log_err("invalid selection parameter");
		return -2;
	}

	content_node->selected = selected;

	return 0;
}

display_window* display_content_node_get_window(display_content_node* content_node){
	if (content_node == NULL){
		log_err("content node does not exist");
		return NULL;
	} if (display_info_struct == NULL){
		log_err("display info struct does not exist");
		return NULL;
	} if (display_info_struct->screen_list == NULL){
		log_err("display info struct does not exist");
		return NULL;
	}

	display_screen_node* screen_node = display_info_struct->screen_list->root;

	while (screen_node != NULL){
		if (screen_node->display_screen != NULL && screen_node->display_screen->window_list != NULL && screen_node->display_screen->window_list->root != NULL){
			display_window_list_node* window_node = screen_node->display_screen->window_list->root;

			while (window_node != NULL){
				if (window_node->display_window != NULL && window_node->display_window->contents != NULL && window_node->display_window->contents->root != NULL){
					display_content_node* test_content_node = window_node->display_window->contents->root;

					while (test_content_node != NULL){
						if (test_content_node == content_node){
							return window_node->display_window;
						}

						test_content_node = test_content_node->next_node;
					}
				}
				
				window_node = window_node->next_node;
			}
		}

		screen_node = screen_node->next_node;
	}

	log_err("could not find content node");
	return NULL;
}

display_content_node* display_new_text_content_node(display_window* window, char* text){
	if (window == NULL){
		log_err("window does not exist");
		return NULL;
	} if (window->contents == NULL){
		log_err("window's contents does not exist");
		return NULL;
	} 

	display_content_node* content_node = display_create_content_node();

	display_content_node_data_set_text(content_node->data, text);

	display_window_add_content_node(window, content_node);

	return content_node;
}

int display_content_node_data_set_text(display_content_node_data* content_data, char* text_data){
	if (content_data == NULL){
		log_err("content_data struct does not exist");
		return -1;
	} 

	content_data->text_data = text_data;

	return 0;
}

int display_draw_content_node(display_window* window, int start_x, int start_y, display_content_node* content_node){
	if (window == NULL){
		log_err("window does not exist");
		return -1;
	} if (content_node == NULL){
		log_err("content node does not exist");
		return -2;
	} if (content_node->data == NULL){
		log_err("content node's data struct does not exist");
		return -3;
	} if (start_y >= window->height - (window->boxed ? 1 : 0) || start_x >= window->width - (window->boxed ? 1 : 0)){
		log_err("invalid start positions within window");
		return -4;
	}

	int window_available_width = window->width - (window->boxed ? 2 : 0);

	char trunc_string[window_available_width + 1];

	string_truncate_middle(content_node->data->text_data, window_available_width, trunc_string);

	int alignment_start_x = start_x;
	if(content_node->alignment == CONTENT_NODE_ALIGN_CENTER){
		alignment_start_x = ((window_available_width - strlen(trunc_string)) / 2) + (window->boxed);
	}

	if ((window->selected == WINDOW_SELECTED) && (content_node->selected == CONTENT_NODE_SELECTED)){
		wattron(window->ncurses_window, COLOR_PAIR(COLOR_PAIR_SELECTED));
	} else {
		wattron(window->ncurses_window, COLOR_PAIR(content_node->data->color_pair_num));
	}
	mvwprintw(window->ncurses_window, start_y, alignment_start_x, trunc_string);
	wattron(window->ncurses_window, COLOR_PAIR(COLOR_PAIR_DEFAULT));

	return 0;
}

int display_content_node_init_data(display_content_node* content_node){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	} if (content_node->data != NULL){
		log_err("content node's data struct does not exist");
		return -2;
	}

	display_content_node_data* new_data = malloc(sizeof(display_content_node_data));

	new_data->text_data = NULL;
	new_data->color_pair_num = COLOR_PAIR_DEFAULT;

	content_node->data=new_data;

	return 0;
}

int display_content_node_terminate_data(display_content_node* content_node){
	if (content_node == NULL){
		log_err("content node does not exist");
		return -1;
	} if (content_node->data == NULL){
		log_err("content node's data struct does not exist");
		return -2;
	}

	free(content_node->data);
	content_node->data = NULL;

	return 0;
}
