#ifndef LOG_H
#define LOG_H

#define log_msg(msg);  log_msg_detailed("", __FILE__, __LINE__, msg);
#define log_err(msg);  log_msg_detailed("ERROR: ", __FILE__, __LINE__, msg);

#define log_msg_f(format_msg, ...); log_msg_f_detailed("", __FILE__, __LINE__, format_msg, __VA_ARGS__);
#define log_err_f(format_msg, ...); log_msg_f_detailed("ERROR: ", __FILE__, __LINE__, format_msg, __VA_ARGS__);

#define log_init(); log_init_file(".log");

// initialize logging
int log_init_file(char* filename);

// terminates logging
int log_terminate();

// log an error
int log_msg_detailed(char* err, char* file, int line, char* msg);

// log a formatted message printf style
int log_msg_f_detailed(char* err, char* file, int line, char* format, ...);

#endif
