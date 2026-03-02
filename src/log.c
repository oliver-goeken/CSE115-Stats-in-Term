#include "log.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

#define TIME_STR_LEN 128


FILE* LOG_FILE;

int log_init(){
	LOG_FILE = fopen("stats.log", "a+");

	if (LOG_FILE == NULL){
		return -1;
	}

	time_t raw_time;
	time(&raw_time);
	struct tm* time_struct = localtime(&raw_time);

	char time_formatted[TIME_STR_LEN];

	strftime(time_formatted, TIME_STR_LEN, "%A, %B %d %Y -- %r", time_struct);

	fprintf(LOG_FILE, "\n[%s]\n", time_formatted);
	
	return 0;
}

int log_terminate(){
	if (LOG_FILE == NULL){
		return -1;
	}

	return fclose(LOG_FILE);
}

int log_msg_detailed(char* file, int line, char* msg){
	if (LOG_FILE == NULL){
		return -1;
	}

	fprintf(LOG_FILE, "%s:%d - %s", file, line, msg);

	return 0;
}

int log_msg_f_detailed(int line, char* file, char* format, ...){
	va_list args;
	va_start(args, format);

	fprintf(LOG_FILE, "%s:%d - ", file, line);
	vfprintf(LOG_FILE, format, args);
	fprintf(LOG_FILE, "\n");

	va_end(args);

	return 0;
}
