#ifndef display_h
#define display_h

#include <ncurses.h>

typedef struct display_window_content_node {
	struct display_window_content_node* next_node;
	struct display_window_content_node* prev_node;

	char* data;
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

/*
 * @brief creates a display_window object with provided parameters
 *
 * @param start_x top left x coordinate
 * @param start_y top left y coordinate
 * @param height height of window
 * @param width witdth of window
 *
 * @return pointer to new display_window
 *
 * @details
 * creates a display_window struct, a wrapper for ncurses window which allows for further functionality such as resizing and moving
 */
display_window* display_create_window(int start_x, int start_y, int height, int width);

/*
 * @brief destroys a desplay_window
 *
 * @param window display_window to destroy
 *
 * @return 0 on success
 *
 * @details
 * calls delwin on the WINDOW* in display_window and frees the memory of display_window
 */
int display_destroy_window(display_window* window);

/*
 * @brief resizes a display_window
 *
 * @param window display_window to resize
 * @param new_width new width of display window
 * @param new_height new height of display window
 *
 * @return 0 on success
 *
 * @details
 * resizes the display_window by changing the parameters in the struct,
 * and then clearing the box of it's WINDOW and deleting that WINDOW,
 * before finally making a new WINDOW with the new parameters and
 * displaying it to the screen
 */
int display_resize_window(display_window* window, int new_width, int new_height);

/*
 * @brief moves a display_window
 *
 * @param window display_window to move
 * @param new_begin_x new top left x of display window
 * @param new_begin_y new top left y of display window
 *
 * @return 0 on success
 *
 * @details
 * moves the display_window by changing the parameters in the struct,
 * and then clearing the box of it's WINDOW and deleting that WINDOW,
 * before finally making a new WINDOW with the new parameters and
 * displaying it to the screen
 */
int display_move_window(display_window* winodw, int new_begin_x, int new_begin_y);

/*
 * @brief destroys an ncurses window
 *
 * @param window ncurses window to destroy
 *
 * @return 0 on success
 *
 * @details
 * destroys an ncurses window and does visual cleanup
 */
int display_destroy_ncurses_window(WINDOW* window);

/*
 * @brief changes the size and position of a display_window
 *
 * @param window display_window to change attributes of
 * @param new_start_x new top left corner x coordinate
 * @param new_start_y new top left corner y coordinate
 * @param new_width new width
 * @param new_new new height
 *
 * @return 0 on success
 *
 */
int display_window_change_attributes(display_window* window, int new_start_x, int new_start_y, int new_width, int new_height);

/*
 * @brief draws a display_window to the screen
 *
 * @param window display_window to draw
 *
 * @return 0 on success
 *
 * @details
 * calls display_draw_window_contents and then draws the window and refreshes
 */
int display_draw_window(display_window* window);

/*
 * @brief displays the contents of a window
 *
 * @param window display_window to draw the contents o
 *
 * @return 0 on succeess
 *
 * @details
 * not yet functioning fully, but hopefully will draw the contents of a 
 * window from the contents struct linked list
 */
int display_draw_window_contents(display_window* window);

/*
 * @brief initializes the data structure that holds the contents of the window
 *
 * @param window display_window's contents to initialize
 *
 * @return 0 on success
 *
 * @details
 * allocates memory and sets default values for the contents data structure
 */
int display_init_window_contents(display_window* window);

/*
 * @brief destorys a display_windows contents data structure
 *
 * @param window display_window whose contents to destroy
 *
 * @return 0 on success
 *
 * @details
 * frees all associated memory for every node in the contents linked list
 */
int display_terminate_window_contents(display_window* window);

/*
 * @brief adds a node to a display window's content struct
 *
 * @param window display_window to add content node to
 * @param data string data to put in new content node
 *
 * @return 0 on success
 *
 * @details
 * allocates memory, ensures references from previous node points correctly. also uses strcpy
 */
int display_window_add_content_node(display_window* window, char* data);

/*
 * @brief destorys a content node from a display window
 *
 * @param window window to destroy content node 
 * @param target_node content node to destroy
 *
 * @return 0 on success
 *
 * @details
 * frees memory associated with node, and moves previous and next node's pointers to correctly reflect lack of node
 */
int display_window_destroy_content_node(display_window* window, display_window_content_node* target_node);

#endif
