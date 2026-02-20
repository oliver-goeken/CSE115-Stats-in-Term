#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		strlcpy(new_string, string, space + 1);
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
