#ifndef display_h
#define display_h

#include <ncurses.h>

#define WINDOW_SELECTABLE true
#define WINDOW_NOT_SELECTABLE false
#define WINDOW_EXPAND_TO_FIT_TEXT true

#ifdef __cplusplus
extern "C"{
#else
  #define EXPORT_C
#endif

/*
 *
 * enum for mode types
 *
 */
typedef enum Mode {
	UNKNOWN,
	HELP_LINE,
	QUIT_CONFIRM
} Mode;

typedef enum Screen {
	MENU,
	MAIN,
	EXIT,
	HIDDEN
} Screen;

typedef enum Alignment {
	LEFT,
	CENTER
} Alignment;

struct display_window;

typedef struct display_window_content_node {
	struct display_window_content_node* next_node;
	struct display_window_content_node* prev_node;

	struct display_window* associated_window;

	Alignment alignment;
	int color_pair;

	bool selected;
	
	/*
	 * pointer to "interaction" function should go next
	 * this function will contain the code to run if enter is pressed while a content node is selected
	 * not sure how to integrate exactly
	 */

	void (*handle_interact)(struct display_window_content_node*, struct display_window*);

	Mode mode;
	char* data;
} display_window_content_node;

typedef struct display_window {
	int start_x;
	int start_y;
	int width;
	int height;

	char* dimensions_format;

	Screen associated_screen;

	bool boxed;
	char horizontal_edges;
	char vertical_edges;

	bool selectable;
	bool selected;

	bool expand_to_fit_text;

	Mode mode;

	WINDOW* window;

	display_window_content_node* content;
	int content_offset;
} display_window;

typedef struct display_window_list_node {
	struct display_window_list_node* next_node;

	display_window* display_window;
} display_window_list_node;

typedef struct display_window_list {
	display_window_list_node* root;

	Screen current_screen;
} display_window_list;

/*
 * @brief intializes ncurses and display_window
 *
 * @return 0 on success, -1 on error
 *
 * @details
 * should only be called at the very start of using ncurses
 * intializes a list of display_windows
 */
int display_init();

/*
 * @brief terminates ncurses and display_window
 *
 * @return 0 on success, -1 on error
 *
 * @details
 * only call when done with ncurses
 * terminates each display_window in display_window_list
 */
int display_terminate();

Screen display_get_current_screen();
void display_parse_dimensions_format(display_window* window);
int display_set_screen(Screen screen);

/*
 * @brief creates a display_window object with provided parameters
 *
 * @param start_x top left x coordinate (must be within bounds [0, COLS])
 * @param start_y top left y coordinate (must be within bounds [0, LINES])
 * @param height height of window
 * @param width witdth of window
 *
 * @return pointer to new display_window on success, NULL on error
 *
 * @details
 * creates a display_window struct, a wrapper for ncurses window which allows for further functionality such as resizing and moving
 */
display_window* display_create_window(Screen screen, bool selectable, char* dimension_format_string);

int display_window_box(display_window* window, char vertical_edges, char horizontal_edges);

int display_window_select_next_node(display_window_list_node* window_node);
int display_window_select_previous_node(display_window_list_node* window_node);

display_window_content_node* display_window_get_current_selection(display_window_list_node* window_node);

int display_set_selected_window(display_window* window);

int display_content_node_set_interaction(display_window_content_node* content_node, void (*interact_function)(display_window_content_node*, display_window*));
void display_handle_interaction();

display_window_list_node* display_get_current_window();

int display_select_next_window();
int display_select_previous_window();

int display_set_window_screen(display_window* window, Screen screen);

int display_handle_command(int* SIGINT_FLAG, display_window* command_window);

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

int display_draw_all_windows();

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
 * @param mode mode for content to be displayed on
 * @param data string data to put in new content node
 *
 * @return 0 on success
 *
 * @details
 * allocates memory, ensures references from previous node points correctly. also uses strcpy
 */
display_window_content_node* display_window_add_content_node(display_window* window, char* data);

int display_set_content_node_alignment(display_window_content_node* content_node, Alignment new_alignment);
int display_set_contend_node_color(display_window_content_node* content_node, int color_pair);

int display_set_window_expansion(display_window* window, bool expand_to_fit_text);

void display_handle_winch();

/*
 * @brief destorys a content node from a display window
 *
 * @param window window to destroy content node 
 * @param target_node content node to destroy
 *
 * @return 0 on success, -1 if node is not in display window's linked list
 *
 * @details
 * frees memory associated with node, and moves previous and next node's pointers to correctly reflect lack of node
 */
int display_window_destroy_content_node(display_window* window, display_window_content_node* target_node);
#ifdef __cplusplus
}
#endif

#endif
