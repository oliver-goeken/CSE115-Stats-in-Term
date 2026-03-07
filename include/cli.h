#ifndef CLI_H
#define CLI_H

typedef struct {
	const char *db_path;
	const char *json_path;
	int recent_count;
	int album_count;
} cli_options;

extern cli_options CLI_OPTIONS;

// return values:
// -1 = continue program
//  0 = handled (help printed)
//  2 = usage error
int handle_args(int argc, char **argv);

#endif
