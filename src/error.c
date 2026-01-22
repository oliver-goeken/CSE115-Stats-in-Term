#include "error.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * @brief Throws an error with the provided information
 *
 * @param code: error code to exit with
 * @param msg: message to print to stderr on exiting
 * @param file: filename where error occured (pass the __FILE__ macro)
 * @param line: line number where error occured (pass the __LINE__ macro)
 *
 * @return nothing
 *
 * @details
 * Exits with passed code, printing the provided filename and line number along with a given message to stderr
 */
void throw_error(int code, char* msg, char* file, int line){
	fprintf(stderr, "%s:%d -- %s\n", file, line, msg);
	exit(code);
}
