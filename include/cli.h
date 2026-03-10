#ifndef CLI_H
#define CLI_H

typedef struct {
	const char *db_path;
	const char *json_path;
	const char *search_kind;
	const char *search_query;
	int recent_count;
	int album_count;
	int album_bottom_count;
	int song_count;
	int song_bottom_count;
	int artist_count;
	int artist_bottom_count;
	int search_limit;
} cli_options;

extern cli_options CLI_OPTIONS;

// return values:
// -1 = continue program
//  0 = handled (help printed)
//  2 = usage error
int handle_args(int argc, char **argv);

#endif
