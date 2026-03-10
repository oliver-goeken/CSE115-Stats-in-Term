#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cli_options CLI_OPTIONS = {
	.db_path = "spotifyHistory.db",
	.json_path = "",
	.search_kind = NULL,
	.search_query = NULL,
	.recent_count = 0,
	.album_count = 0,
	.album_bottom_count = 0,
	.song_count = 0,
	.song_bottom_count = 0,
	.artist_count = 0,
	.artist_bottom_count = 0,
	.search_limit = 0
};

static void print_help(FILE *out) {
	fprintf(out,
			"spotistats - Spotify listening stats in your terminal\n"
			"\n"
			"USAGE:\n"
			"\t./stats [OPTIONS]\n"
			"\n"
			"GENERAL:\n"
			"\t--help\t\t\tShow this help message\n"
			"\t--db PATH\t\tUse a specific sqlite database file\n"
			"\t--json PATH\t\tImport Spotify history JSON from PATH\n"
			"\n"
			"SEARCH:\n"
			"\t--search songs N QUERY\tSearch closest artist match and print top N songs\n"
			"\t--search albums N QUERY\tSearch closest artist match and print top N albums\n"
			"\t--find TYPE N QUERY\tSame as --search\n"
			"\t-f s|a N QUERY\t\tShort search form, where s=songs and a=albums\n"
			"\n"
			"REPORTS:\n"
			"\t-h N, --history N\tPrint the N most recent listens and exit\n"
			"\t-r N, --artists N\tPrint the N top artists and exit\n"
			"\t-R N, --artists-bottom N\tPrint the N least played artists and exit\n"
			"\t-a N, --albums N\tPrint the N top albums and exit\n"
			"\t-A N, --albums-bottom N\tPrint the N least played albums and exit\n"
			"\t-s N, --songs N\t\tPrint the N top songs and exit\n"
			"\t-S N, --songs-bottom N\tPrint the N least played songs and exit\n"
			"\n"
			"DEFAULT:\n"
			"\tLaunches the TUI.\n"
	       );
}

int handle_args(int argc, char **argv) {

	for (int i = 1; i < argc; i++) {
		// help
		if (strcmp(argv[i], "--help") == 0) {
			print_help(stdout);
			return 0;
		}

		// custom sqlite db file path
		else if (strcmp(argv[i], "--db") == 0) {

			if (i + 1 >= argc) {
				fprintf(stderr, "--db requires a path\n");
				return 2;
			}

			CLI_OPTIONS.db_path = argv[++i];
		}
		
		// custom json path
		else if (strcmp(argv[i], "--json") == 0) {

			if (i + 1 >= argc) {
				fprintf(stderr, "--json requires a path\n");
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			CLI_OPTIONS.json_path = argv[++i];
		}
		else if (strcmp(argv[i], "--search") == 0 || strcmp(argv[i], "--find") == 0 || strcmp(argv[i], "-f") == 0) {
			if (i + 3 >= argc) {
				fprintf(stderr, "%s requires TYPE, count, and query\n", argv[i]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			const char *kind = argv[++i];
			if (strcmp(argv[i - 1], "-f") == 0) {
				if (strcmp(kind, "s") == 0) {
					kind = "songs";
				} else if (strcmp(kind, "a") == 0) {
					kind = "albums";
				}
			}

			if (strcmp(kind, "songs") != 0 && strcmp(kind, "albums") != 0) {
				fprintf(stderr, "%s type must be 'songs' or 'albums'\n", argv[i - 1]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			char *endptr = NULL;
			long val = strtol(argv[++i], &endptr, 10);
			if (endptr == argv[i] || *endptr != '\0' || val <= 0 || val > 1000000) {
				fprintf(stderr, "%s expects a positive integer count\n", argv[i - 2]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			CLI_OPTIONS.search_kind = kind;
			CLI_OPTIONS.search_limit = (int)val;
			CLI_OPTIONS.search_query = argv[++i];
		}
		// recent listens
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--history") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "%s requires a count\n", argv[i]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			char *endptr = NULL;
			long val = strtol(argv[++i], &endptr, 10);
			if (endptr == argv[i] || *endptr != '\0' || val <= 0 || val > 1000000) {
				fprintf(stderr, "%s expects a positive integer\n", argv[i - 1]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}
			CLI_OPTIONS.recent_count = (int)val;
		}
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--artists") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "%s requires a count\n", argv[i]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			char *endptr = NULL;
			long val = strtol(argv[++i], &endptr, 10);
			if (endptr == argv[i] || *endptr != '\0' || val <= 0 || val > 1000000) {
				fprintf(stderr, "%s expects a positive integer\n", argv[i - 1]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}
			CLI_OPTIONS.artist_count = (int)val;
		}
		else if (strcmp(argv[i], "-R") == 0 || strcmp(argv[i], "--artists-bottom") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "%s requires a count\n", argv[i]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			char *endptr = NULL;
			long val = strtol(argv[++i], &endptr, 10);
			if (endptr == argv[i] || *endptr != '\0' || val <= 0 || val > 1000000) {
				fprintf(stderr, "%s expects a positive integer\n", argv[i - 1]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}
			CLI_OPTIONS.artist_bottom_count = (int)val;
		}
		else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--albums") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "%s requires a count\n", argv[i]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			char *endptr = NULL;
			long val = strtol(argv[++i], &endptr, 10);
			if (endptr == argv[i] || *endptr != '\0' || val <= 0 || val > 1000000) {
				fprintf(stderr, "%s expects a positive integer\n", argv[i - 1]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}
			CLI_OPTIONS.album_count = (int)val;
		}
		else if (strcmp(argv[i], "-A") == 0 || strcmp(argv[i], "--albums-bottom") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "%s requires a count\n", argv[i]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			char *endptr = NULL;
			long val = strtol(argv[++i], &endptr, 10);
			if (endptr == argv[i] || *endptr != '\0' || val <= 0 || val > 1000000) {
				fprintf(stderr, "%s expects a positive integer\n", argv[i - 1]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}
			CLI_OPTIONS.album_bottom_count = (int)val;
		}
		else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--songs") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "%s requires a count\n", argv[i]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			char *endptr = NULL;
			long val = strtol(argv[++i], &endptr, 10);
			if (endptr == argv[i] || *endptr != '\0' || val <= 0 || val > 1000000) {
				fprintf(stderr, "%s expects a positive integer\n", argv[i - 1]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}
			CLI_OPTIONS.song_count = (int)val;
		}
		else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "--songs-bottom") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "%s requires a count\n", argv[i]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}

			char *endptr = NULL;
			long val = strtol(argv[++i], &endptr, 10);
			if (endptr == argv[i] || *endptr != '\0' || val <= 0 || val > 1000000) {
				fprintf(stderr, "%s expects a positive integer\n", argv[i - 1]);
				fprintf(stderr, "Try './stats --help'\n");
				return 2;
			}
			CLI_OPTIONS.song_bottom_count = (int)val;
		}

		// otherwise
		else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			fprintf(stderr, "Try './stats --help'\n");
			return 2;
		}
	}

	int selected_modes = 0;
	if (CLI_OPTIONS.search_kind != NULL) selected_modes++;
	if (CLI_OPTIONS.recent_count > 0) selected_modes++;
	if (CLI_OPTIONS.artist_count > 0) selected_modes++;
	if (CLI_OPTIONS.artist_bottom_count > 0) selected_modes++;
	if (CLI_OPTIONS.album_count > 0) selected_modes++;
	if (CLI_OPTIONS.album_bottom_count > 0) selected_modes++;
	if (CLI_OPTIONS.song_count > 0) selected_modes++;
	if (CLI_OPTIONS.song_bottom_count > 0) selected_modes++;

	if (selected_modes > 1) {
		fprintf(stderr, "Choose only one print mode at a time\n");
		fprintf(stderr, "Try './stats --help'\n");
		return 2;
	}

	return -1;
}
