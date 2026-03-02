#include "log.h"
#include <stdio.h>
#include <time.h>

#define TIME_STR_LEN 128


FILE* log_file;

int log_init(){
	log_file = fopen("stats.log", "a+");

	if (log_file == NULL){
		return -1;
	}

	time_t raw_time;
	time(&raw_time);
	struct tm* time_struct = localtime(&raw_time);

	char time_formatted[TIME_STR_LEN];

	strftime(time_formatted, TIME_STR_LEN, "%A, %B %d %Y -- %r", time_struct);

	fprintf(log_file, "\n[%s]\n", time_formatted);
	
	return 0;
}

int log_terminate(){
	if (log_file == NULL){
		return -1;
	}

	return fclose(log_file);
}

int log_msg_detailed(char* file, int line, char* msg){
	if (log_file == NULL){
		return -1;
	}

	fprintf(log_file, "%s:%d - %s", file, line, msg);

	return 0;
}
