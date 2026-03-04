#include "cli.h"
#include <stdio.h>
#include <string.h>

int handle_args(int argc, char **argv) {
	if (argc > 1 && strcmp(argv[1], "-h") == 0) {
		printf("hello world\n");
		return 1;
	}

	return 0;
}
