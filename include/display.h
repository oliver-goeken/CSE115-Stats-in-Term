#ifndef display_h
#define display_h

#include <ncurses.h>

typedef struct display_window_content_node {
	struct display_window_content_node* NEXT;
	struct display_window_content_node* PREV;
} display_window_content_node;

typedef struct display_window {
	int start_x;
	int start_y;
	int width;
	int height;

	WINDOW* window;

	display_window_content_node* content;
	int content_offset;
} display_window;

/*
 * @brief intializes ncurses
 *
 * @return 0 on success, -1 on error
 *
 * @details
 * should only be called at the very start of using ncurses
 */
int display_init();

/*
 * @brief terminates ncurses
 *
 * @return 0 on success, -1 on error
 *
 * @details
 * only call when done with ncurses
 */
int display_terminate();

display_window* display_create_window(int start_x, int start_y, int height, int width);
int display_destroy_window(display_window* window);

int display_resize_window(display_window* window, int new_width, int new_height);
int display_move_window(display_window* winodw, int new_begin_x, int new_begin_y);

int display_draw_window(display_window* window);

int display_draw_window_contents(display_window* window);

#endif
