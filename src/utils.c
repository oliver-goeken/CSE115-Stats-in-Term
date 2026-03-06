#include "utils.h"
#include "log.h"
#include "input.h"
#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void print_error(char* file, int line){
	fprintf(stderr, "%s[%d]\n", file, line);
}

void count_newlines(char* string, int* newlines){
	for (int i = 0; string[i] != '\0'; i ++){
		if (string[i] == '\n')
			(*newlines) ++;
	}
}

void string_truncate_middle(char* string, int space, char* new_string){
	int string_len = strlen(string);
	int start_str_len = 0;
	int end_str_len = 0;

	if (string_len <= space){
		strncpy(new_string, string, space + 1);
		return;
	} else {
		start_str_len = (space - 3) / 2;
		end_str_len = (space - 3) - start_str_len;

		int pos = 0;
		for(int i = 0; i < start_str_len; i ++){
			new_string[i] = string[i];
			pos ++;
		}

		for(int i = 0; i < 3; i ++){
			new_string[pos] = '.';
			pos ++;
		}

		for(int i = 0; i < end_str_len; i ++){
			new_string[pos] = string[string_len - end_str_len + i];
			pos ++;
		}

		new_string[pos] = '\0';
	}
}

void throw_error(int code, char* msg, char* file, int line){
	fprintf(stderr, "%s:%d -- %s\n", file, line, msg);
	exit(code);
}

void remove_non_printable_chars(char* string){
	int move_back = 0;

	int i;
	for (i = 0; string[i] != '\0'; i ++){
		if (string[i] < 32 || string[i] > 126){
			move_back ++;
			continue;
		}

		if (move_back != 0){
			string[i - move_back] = string[i];
		}
	}

	string[i - move_back] = '\0';
}

bool is_directory(char* path){
	struct stat stat_buffer;
	int ret_val = stat(path, &stat_buffer);

	if (ret_val != 0){
		log_err_f("error with stat at %s", path);
		return false;
	}

	if (S_ISDIR(stat_buffer.st_mode)){
		log_msg_f("%s is a directory", path);
		return true;
	}

	return false;
}

bool is_file(char* path){
	struct stat stat_buffer;
	stat(path, &stat_buffer);

	return S_ISREG(stat_buffer.st_mode);
}

bool string_ends_with(char* cmp, char* end){
	int end_len = strlen(end);

	return strncmp(cmp + strlen(cmp) - end_len, end, end_len) == 0;
}

void get_dir_path(char* dir_path, char* path, int dir_path_size){
	strncpy(dir_path, path, dir_path_size);
	input_command_remove_excess_space(dir_path, dir_path_size);

	int final_char = strlen(dir_path) - 1;

	if (path[final_char] != '/'){
		if ((final_char + 2) < dir_path_size){
			path[final_char + 1] = '/';
			path[final_char + 2] = '\0';
		}
	}
}

void clear_screen(display_screen* screen){
	if (screen != NULL){
		display_destroy_window_list(screen->window_list);
		display_create_window_list(screen);
		display_screen_draw_windows(screen);
	} else {
		log_err("screen does not exist");
	}
}
