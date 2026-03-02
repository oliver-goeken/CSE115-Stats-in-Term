#ifndef LOG_H
#define LOG_H

#define log_msg(msg);  log_error_detailed(__FILE__, __LINE__, msg);


// initialize logging
int log_init();

// terminates logging
int log_terminate();

// log an error
int log_msg_detailed(char* file, int line, char* msg);

#endif
