#include "cli.h"
#include <stdio.h>
#include <string.h>

cli_options CLI_OPTIONS = {
	.db_path = "spotifyHistory.db",
	.json_path = ""
};

static void print_help(FILE *out) {
	fprintf(out,
			"spotistats - Spotify listening stats in your terminal\n"
			"\n"
			"USAGE:\n"
			"  ./stats [OPTIONS]\n"
			"\n"
			"OPTIONS:\n"
			"  -h, --help        Show this help message\n"
			"  --db PATH         Use a specific sqlite database file\n"
			"  --json PATH       Import Spotify history JSON from PATH\n"
			"\n"
			"DEFAULT:\n"
			"  Launches the TUI.\n"
	       );
}

int handle_args(int argc, char **argv) {

	for (int i = 1; i < argc; i++) {
		// help
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
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

		// otherwise
		else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			fprintf(stderr, "Try './stats --help'\n");
			return 2;
		}
	}

	return -1;
}
