#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void print_error(char* file, int line){
	fprintf(stderr, "%s[%d]\n", file, line);
}

void throw_error(int code, char* msg, char* file, int line){
	fprintf(stderr, "%s:%d -- %s\n", file, line, msg);
	exit(code);
}
