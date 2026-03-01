#include "input.h"
#include "parse_db_funcs.h"
#include <string.h>

int get_input(display_window* window, int start_x, int start_y, char* input_buffer, int input_buffer_size){
	if (window == NULL){
		return -1;
	} if (start_x > window->width - (window->boxed ? 1 : 0) || start_x < 0 + (window->boxed ? 1 : 0)){
		return -2;
	} if (start_y > window->height || start_y < 0){
		return -3;
	} if (input_buffer_size <= 0){
		return -4;
	}

	memset(input_buffer, '\0', input_buffer_size);
	int input_buffer_pos = 0;

	bool cancel_input = false;
	bool input_finished = false;

	while (!input_finished){
		display_draw_window_and_update(window);

		int user_input = getch();

		switch (user_input){
			case KEY_RESIZE:
				display_handle_winch();
				break;
			case '\n':
			case KEY_ENTER:
			case 13:
				input_finished = true;
				break;
			case 27:
				input_finished = true;
				cancel_input = true;
				break;
			case KEY_BACKSPACE:
			case 127:
			case '\b':
				if (input_buffer_pos > 0){
					input_buffer[input_buffer_pos] = '\0';

					mvwprintw(window->ncurses_window, start_y, start_x + input_buffer_pos, " ");

					input_buffer_pos --;

					wattrset(window->ncurses_window, COLOR_PAIR(2));
					mvwprintw(window->ncurses_window, start_y, start_x + input_buffer_pos, " ");
					wattrset(window->ncurses_window, COLOR_PAIR(1));
				}
				break;
			default:
				if (input_buffer_pos < (input_buffer_size - 1) && 32 <= user_input && user_input <= 126){
					input_buffer[input_buffer_pos] = user_input;

					mvwprintw(window->ncurses_window, start_y, start_x + input_buffer_pos, "%c", user_input);

					input_buffer_pos ++;

					wattrset(window->ncurses_window, COLOR_PAIR(2));
					mvwprintw(window->ncurses_window, start_y, start_x + input_buffer_pos, " ");
					wattrset(window->ncurses_window, COLOR_PAIR(1));
				}
				break;
		}

		if (cancel_input){
			input_buffer[0] = '\0';
		} else {
			input_buffer[input_buffer_pos] = '\0';
		}
	}

	return 0;
}

int input_handle_command(display_window* window, int start_x, int start_y){
	if (window == NULL){
		return -1;
	} if (start_x > window->width - (window->boxed ? 1 : 0) || start_x < 0 + (window->boxed ? 1 : 0)){
		return -2;
	} if (start_y > window->height || start_y < 0){
		return -3;
	}

	wmove(window->ncurses_window, start_y, start_x);
	wprintw(window->ncurses_window, ":");
	wmove(window->ncurses_window, start_y, ++ start_x);
	wattrset(window->ncurses_window, COLOR_PAIR(2));
	wprintw(window->ncurses_window, " ");
	wattrset(window->ncurses_window, COLOR_PAIR(1));

	int max_input_size = 256;
	char in_buff[max_input_size];

	get_input(window, start_x, start_y, in_buff, max_input_size);
	input_command_remove_excess_space(in_buff, max_input_size);

	char command_buff[max_input_size];
	char args_buff[max_input_size];

	input_separate_command_and_args(in_buff, command_buff, args_buff, max_input_size);

	if (strcmp(command_buff, "q") == 0){
		return COMMAND_QUIT;
	} 
	
	return 0;
}

int input_separate_command_and_args(char* input_buffer, char* command_buffer, char* args_buffer, int buffer_size){
	if (buffer_size <= 0){
		return -1;
	}

	memset(command_buffer, '\0', buffer_size);
	memset(args_buffer, '\0', buffer_size);

	int command_buff_pos = 0;
	int args_buff_pos = 0;

	int* current_pos = &command_buff_pos;
	char** current_buffer = &command_buffer;

	for (int i = 0; i < buffer_size && input_buffer[i] != '\0'; i ++){
		if (current_buffer == &command_buffer && input_buffer[i] == ' '){
			current_pos = &args_buff_pos;
			current_buffer = &args_buffer;
			continue;
		}

		(*current_buffer)[(*current_pos)] = input_buffer[i];

		(*current_pos) ++;
	}

	return 0;
}

int input_command_remove_excess_space(char* input_buffer, int buffer_size){
	if (buffer_size <= 0){
		return -1;
	}

	int move_back = 0;

	//remove leading spaces
	for (int i = 0; i < buffer_size && input_buffer[i] == ' '; i ++){
		move_back ++;
	}
	
	//remove trailing spaces
	for (int i = buffer_size - 1; i >= 0 && (input_buffer[i] == ' ' || input_buffer[i] == '\0'); i --){
		input_buffer[i] = '\0';
	}

	// remove repeated spaces
	for (int i = move_back; i < buffer_size && input_buffer[i] != '\0'; i ++){
		if (input_buffer[i] != ' '){
			input_buffer[i - move_back] = input_buffer[i];

			if ((i <= (buffer_size - 1) && input_buffer[i + 1] == '\0') || (i == (buffer_size - 1))){
				input_buffer[i - move_back + 1] = '\0';
				break;	
			}
		} else {
			if (input_buffer[i - 1] == ' '){
				move_back ++;
			} else {
				input_buffer[i - move_back] = ' ';
			}
		}
	}

	return 0;
}
