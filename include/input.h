#ifndef INPUT_H
#define INPUT_H

#include "display.h"

#define COMMAND_QUIT -100
#define COMMAND_NOT_RECOGNIZED -200
#define COMMAND_CANCEL 100

// gets input from the user
int get_input(display_window* window, int start_x, int start_y, char* input_buffer, int input_buffer_size);

// handles a command from the user
int input_handle_command(display_window* window, int start_x, int start_y);

// separates command and args into two buffers, assumes the size of all three input buffers are the same
int input_separate_command_and_args(char* input_buffer, char* command_buffer, char* args_buffer, int buffer_size);

// removes excess spaces from command
// this means:
// - leading spaces
// - trailing spaces
// - any instances of multiple spaces in a row
int input_command_remove_excess_space(char* input_buffer, int buffer_size);

// display an error message if command goes wrong or isn't recognized
int input_display_command_error(display_window* window, char* msg);

#endif
