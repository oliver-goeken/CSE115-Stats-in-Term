#ifndef DISPLAY_H
#define DISPLAY_H

#include <curses.h>
#include <stdbool.h>
#include <time.h>

#define WINDOW_VISIBLE true
#define WINDOW_HIDDEN false

#define WINDOW_BOXED true
#define WINDOW_NOT_BOXED false

#define WINDOW_EXPAND_TO_FIT_TEXT true
#define WINDOW_SET_SIZE false

#define CONTENT_NODE_UNSELECTABLE -1
#define CONTENT_NODE_NOT_SELECTED 0
#define CONTENT_NODE_SELECTED 1

#define WINDOW_UNSELECTABLE -1
#define WINDOW_NOT_SELECTED 0
#define WINDOW_SELECTED 1

#define NEXT_WINDOW 1
#define PREV_WINDOW -1

#define WINDOW_HORIZONTAL 1
#define WINDOW_VERTICAL -1

#define COLOR_PAIR_DEFAULT 1
#define COLOR_PAIR_SELECTED 2
#define COLOR_PAIR_ERROR 3

// text alignment for content nodes
typedef enum content_node_alignment{
	CONTENT_NODE_ALIGN_LEFT,
	CONTENT_NODE_ALIGN_CENTER
} content_node_alignment;


// text data for content node
typedef struct display_content_node_data{
	char* text_data;

	void* other_data;

	int color_pair_num;
} display_content_node_data;

// content nodes for a display window
// alignment: text alignment
// selected: whether or not a node is selected
// time_created: the time a node was created, for nodes that can disappear over time
// timeout: how many seconds after a node is created should it be destroyed
// handle_interact: the function to run if this node is selected and the user presses enter
// data: the struct containing text data for this node
typedef struct display_content_node {
	struct display_content_node* next_node;

	content_node_alignment alignment;
	int selected;

	time_t time_created;
	time_t timeout;

	void (*handle_interact)(struct display_content_node* content_node);

	display_content_node_data* data;
} display_content_node;

// linked list of content nodes for a display window
// drawn to the screen in the order they are added (FIFO)
typedef struct display_window_contents {
	display_content_node* root;
} display_window_contents;


// display window, a sort of wrapper for an ncurses window
// dimension_format: a string which represents the dimensions a window can take up (format below), including percentages of screen so a 
// start_x, start_y, width, height: the starting positions and dimensions of a display window (will be altered when display size is changed)
// visible: whether or not a window will currently be displayed onscreen
// box_sides: a bit flag for which sides of a window box are shown
// boxed: whether are not a window's border should be shown
// expand: whether a window taking up a certain percentage of the screep should expand horizontally to fit text if there is room in the window
// selected: whether or not this window is currently selected
// ncurses_window: associated ncurses window
// content_offset: how far into content nodes the window should currently start at at the top of the screen
// contents: a struct containing the text contents of the window
typedef struct display_window {

	/*
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
	char* dimensions_format;

	int start_x;
	int start_y;
	int width;
	int height;

	bool visible;

	/*
	 * bit flag for which sides of a windows box are shown
	 * bits:
	 * 1-top left corner
	 * 2-top right corner
	 * 3-bottom left corner
	 * 4-bottom right corner
	 * 5-top side
	 * 6-left side
	 * 7-right side
	 * 8-bottom side
	 */
	uint8_t box_sides;

	bool boxed;
	bool expand;

	int selected;

	WINDOW* ncurses_window;

	int content_offset;
	display_window_contents* contents;
} display_window;

// a node in the list of display windows
typedef struct display_window_list_node {
	struct display_window_list_node* next_node;

	display_window* display_window;
} display_window_list_node;

// a list of display windows, one list per screen
typedef struct display_window_list {
	display_window_list_node* root;
	
}	display_window_list;


// a "screen" of the display
// each set of windows is associated with a screen
// each screen will only displays its windows when it is active
// there can only be one active screen at a time, and there must be at least one screen to display windows
// ex.
// title screen, main screen, exit screen
typedef struct display_screen {
	char* name;

	display_window_list* window_list;
} display_screen;

// a node in a list of screens
typedef struct display_screen_node{
	struct display_screen_node* next_node;

	display_screen* display_screen;
} display_screen_node;

// a list of display screens
typedef struct display_screen_list {
	display_screen_node* root;
} display_screen_list;


// the struct which holds the list of screens and stores which screen is currently displayed
typedef struct display_info {
	display_screen_list* screen_list;
	display_screen* current_screen;
} display_info;


// initializes display - call at begining of use
int display_init();

// initializes ncurses-specific stuff, called by display_init()
int display_ncurses_init();

// termiantes display - call at end of use
int display_terminate();

// terminates ncurses-specific stuff, called by display_terminate()
int display_ncurses_terminate();


// inits internal display struct, called by display_init
int display_intitialize_display_info();

// terminates internal display struct, called by display_terminate
int display_terminate_display_info();


// initializes internal screen list, called internally
display_screen_list* display_initialize_screen_list();

// terminates screen list, called internally
int display_terminate_screen_list(display_screen_list* screen_list);


// creates a node in the internal screen list
display_screen_node* display_create_screen_node();

// destroys a node in the internal screen list
int display_destroy_screen_node(display_screen_node* screen_node);


// gets the currently selected content node
display_content_node* display_get_selected_content_node();


// create a new screen with name screen_name
display_screen* display_create_new_screen(char* screen_name);

// destroys a screen
int display_destroy_screen(display_screen* screen);

// draws all windows in a screen
int display_screen_draw_windows(display_screen* screen);

// sets the currently displayed screen 
int display_set_screen(display_screen* screen);

// sets which window is slected within a screen, unselects other windows in that screen
int display_screen_set_selected_window(display_screen* screen, display_window* window);


// selects the next window in a screen (horizontally)
int display_screen_select_next_window(display_screen* screen);

// selects the previous window in a screen (horizontally)
int display_screen_select_previous_window(display_screen* screen);

// select a window in the given direction (horizontally)
int display_screen_select_window_directional(display_screen* screen, int direction, int dimension);

// get the currently selected content node in a screen
display_content_node* display_screen_get_selected_content_node();

// draw a window without updating screen
int display_draw_window(display_window* window);

// draws a window and updates screen
int display_draw_window_and_update(display_window* window);

// boxes a ncurses window with appropriate sides shown or hidden
int display_box_window(display_window* window);


// set which screen is currently displayed
int display_set_current_screen(display_screen* screen);

// get which screen is currently being displayed
display_screen* display_get_current_screen();

// get which window node in a screen is selected
display_window_list_node* display_screen_get_selected_window_node(display_screen* screen);

// add a new window to a screen with the given dimension format string
display_window* display_screen_add_new_window(display_screen* screen, char* dimensions_format);

// destroy a window in a screen
int display_screen_destroy_window(display_screen* screen, display_window* window);

// gets the window from a window node
display_window* display_window_node_get_window(display_window_list_node* window_node);

// set a window's visibility
int display_window_set_visibility(display_window* window, bool visible);

// set if a window is boxed
int display_window_set_boxed(display_window* window, bool boxed);

// set which sides of a window's box are displayed
// uses a 8 bit flag system where:
// 1st bit: top left corner
// 2nd bit: top right corner
// 3rd bit: bottom left corner
// 4th bit: bottom right corner
// 5th bit: top side
// 6th bit: left side
// 7th bit: right side
// 8th bit: bottom side
int display_window_set_box_sides(display_window* window, uint8_t sides);

// set if a window expands to fit text
int display_window_set_expansion(display_window* window, bool expand);

// set if a window is selected or not
int display_window_set_selected(display_window* window, int selected);

// get the currently selected content node of a window
display_content_node* display_window_get_selected_node(display_window* window);

// add a content node to a window
int display_window_add_content_node(display_window* window, display_content_node* content_node);

// draw the content nodes of a window
int display_window_draw_contents(display_window* window);

// get which screen a window is associated with
display_screen* display_window_get_screen(display_window* window);

// get which node is selected in a window
display_content_node* display_window_get_selected_node(display_window* window);

// select the next node of a window
int display_window_select_next_node(display_window* window);

// select the previous node of a window
int display_window_select_prev_node(display_window* window);

// figures out which window is currently selected and gets the next content node of that window
int display_generic_select_next_node();

// figures out which window is currently selected and gets the previous content node of that window
int display_generic_select_prev_node();

// create a new window list for a screen, used internally
display_window_list* display_create_window_list(display_screen* screen);

// destroy a window list, used internally
int display_destroy_window_list(display_window_list* window_list);

// handle SIGWINCh
int display_handle_winch();

// redraw the ncurses window for a display window using that display window's parameters
int display_reset_ncurses_window(display_window* window);

// destroy a ncurses window and clears its space
int display_destroy_ncurses_window(WINDOW* window);

// parse a dimension format string for a window, used internally
int display_parse_dimensions_format(display_window* window);

// create a new window with a given format string
display_window* display_create_window(char* dimensions_format);

// initialiaze the content struct for a screen, used internally
int display_window_init_contents(display_window* window);

// destroys a display window
int display_destroy_window(display_window* window);

// destroys a chain of content nodes beginning at first in window
int display_window_destroy_content_nodes(display_window* window);

// destroy the content nodes of a display window
int display_terminate_window_contents(display_window* window);

// create a new content node, used mostly internally
display_content_node* display_create_content_node();

// sets the timeout for a content node
int display_content_node_set_timeout(display_content_node* content_node, long time_after_creation);

// sets which color pair a content node is displayed with
int display_content_node_set_color_pair(display_content_node* content_node, int color_pair_num);

// set the interaction function for a content node
int display_content_node_set_interaction(display_content_node* content_node, void (*handle_interact)(display_content_node* content_node));

// handle a display interaction function (call it) for a content node, will do nothing if there is no function assigned
int display_handle_interact(display_content_node* content_node);

// destroy a content node
int display_destroy_content_node(display_content_node* content_node);

// set a content node's alignment
int display_set_content_node_alignment(display_content_node* content_node, content_node_alignment alignment);

// sets the data struct for a content node
int display_content_node_set_data(display_content_node* content_node, display_content_node_data* data);

// clears the data struct for a content node and destroys it
int display_content_node_clear_data(display_content_node* content_node);

// set if a content node is selected
int display_content_node_set_selected(display_content_node* content_node, int selected);

// get which window is associated with a content node
display_window* display_content_node_get_window(display_content_node* content_node);

// create a new content node with the given text, meant to save time when created content nodes with hard coded string data
display_content_node* display_new_text_content_node(display_window* window, char* text);


// set the string text of a content node's data struct
int display_content_node_data_set_text(display_content_node_data* content_data, char* data_text);

// draws a content node to the string in a window, called internally
int display_draw_content_node(display_window* window, int start_x, int start_y, display_content_node* content_node);


// initializes the data struct for a content node, called internally
int display_content_node_init_data(display_content_node* content_node);

// terminate the data struct for a content node, called internally
int display_content_node_terminate_data(display_content_node* content_node);


#endif
