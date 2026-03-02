#include "log.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

#define TIME_STR_LEN 128


FILE* LOG_FILE;
time_t raw_time;

int log_init_file(char* filename){
	LOG_FILE = fopen(filename, "a+");

	if (LOG_FILE == NULL){
		return -1;
	}

	time(&raw_time);
	struct tm* time_struct = localtime(&raw_time);

	char time_formatted[TIME_STR_LEN];
	strftime(time_formatted, TIME_STR_LEN, "%A, %B %d %Y -- %r", time_struct);

	fprintf(LOG_FILE, "\n[%s]\n", time_formatted);

	log_msg("initializing...");
	
	return 0;
}

int log_terminate(){
	if (LOG_FILE == NULL){
		return -1;
	}

	log_msg("done!");

	return fclose(LOG_FILE);
}

int log_msg_detailed(char* err, char* file, int line, char* msg){
	if (LOG_FILE == NULL){
		return -1;
	}

	time(&raw_time);
	struct tm* time_struct = localtime(&raw_time);
	char time_formatted[TIME_STR_LEN];
	strftime(time_formatted, TIME_STR_LEN, "%T", time_struct);

	fprintf(LOG_FILE, "[%s] ", time_formatted);
	fprintf(LOG_FILE, "%s:%d -> %s%s\n", file, line, err, msg);

	return 0;
}

int log_msg_f_detailed(char* err, char* file, int line, char* format, ...){
	if (LOG_FILE == NULL){
		return -1;
	}

	va_list args;
	va_start(args, format);

	time(&raw_time);
	struct tm* time_struct = localtime(&raw_time);
	char time_formatted[TIME_STR_LEN];
	strftime(time_formatted, TIME_STR_LEN, "%T", time_struct);

	fprintf(LOG_FILE, "[%s] %s:%d -> %s", time_formatted, file, line, err);
	vfprintf(LOG_FILE, format, args);
	fprintf(LOG_FILE, "\n");

	va_end(args);

	return 0;
}
