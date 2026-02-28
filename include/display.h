#ifndef DISPLAY_H
#define DISPLAY_H

#include <curses.h>
#include <stdbool.h>

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

typedef enum content_node_alignment{
	CONTENT_NODE_ALIGN_LEFT,
	CONTENT_NODE_ALIGN_CENTER
} content_node_alignment;


typedef struct display_content_node_data{
	char* text_data;
} display_content_node_data;

typedef struct display_content_node {
	struct display_content_node* next_node;

	content_node_alignment alignment;
	int selected;

	void (*handle_interact)(struct display_content_node* content_node);

	display_content_node_data* data;
} display_content_node;

typedef struct display_window_contents {
	display_content_node* root;
} display_window_contents;


typedef struct display_window {
	char* dimensions_format;

	int start_x;
	int start_y;
	int width;
	int height;

	bool visible;
	bool boxed;
	bool expand;

	int selected;

	WINDOW* ncurses_window;

	int content_offset;
	display_window_contents* contents;
} display_window;

typedef struct display_window_list_node {
	struct display_window_list_node* next_node;

	display_window* display_window;
} display_window_list_node;

typedef struct display_window_list {
	display_window_list_node* root;
	
}	display_window_list;


typedef struct display_screen {
	char* name;

	display_window_list* window_list;
} display_screen;

typedef struct display_screen_node{
	struct display_screen_node* next_node;

	display_screen* display_screen;
} display_screen_node;

typedef struct display_screen_list {
	display_screen_node* root;
} display_screen_list;


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
int display_screen_select_window_directional(display_screen* screen, int direction);

// get the currently selected content node in a screen
display_content_node* display_screen_get_selected_content_node();

// draw a window
int display_draw_window(display_window* window);


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

// set a window's visibility
int display_window_set_visibility(display_window* window, bool visible);

// set if a window is boxed
int display_window_set_boxed(display_window* window, bool boxed);

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

// destroy the content nodes of a display window
int display_window_destroy_content_nodes(display_window* window);

// create a new content node, used mostly internally
display_content_node* display_create_content_node();

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
