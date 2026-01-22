#include "error.h"
#include <stdio.h>
#include <stdlib.h>

// Exits with passed code, printing the provided filename and line number along with a given message to stderr
// Takes:
// int code: error code
// char* msg: message to print to stderr
// char* file: filename of error, provided by __FILE__ macro when calling
// int line: line number of error, provided by __LINE__ macro when calling

void throw_error(int code, char* msg, char* file, int line){
	fprintf(stderr, "%s:%d -- %s\n", file, line, msg);
	exit(code);
}
