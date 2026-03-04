#include "cli.h"
#include <stdio.h>
#include <string.h>

static void print_help(FILE *out) {
    fprintf(out,
        "spotistats - Spotify listening stats in your terminal\n"
        "\n"
        "USAGE:\n"
        "  ./stats [OPTIONS]\n"
        "\n"
        "DEFAULT:\n"
        "  Run without options to launch the TUI.\n"
        "\n"
        "OPTIONS:\n"
        "  -h, --help\n"
        "      Show this help message and exit.\n"
        "\n"
        "CURRENT BEHAVIOR:\n"
        "  spotistats currently launches an ncurses TUI by default.\n"
        "  Help output does not start ncurses.\n"
        "\n"
        "DATA SOURCE:\n"
        "  The current build uses spotifyHistory.db and imports Spotify history\n"
        "  from the JSON path configured in the source code.\n"
        "\n"
        "TUI KEYS:\n"
        "  Arrow keys / h j k l    Move selection\n"
        "  Enter                  Select / interact\n"
        "  :                      Open command prompt\n"
        "  q / Esc                Quit or go back\n"
        "\n"
        "TUI COMMANDS:\n"
        "  :q                     Quit\n"
        "  :search                Reserved for future implementation\n"
        "\n"
        "EXIT STATUS:\n"
        "  0 on success\n"
        "  2 on invalid command line usage\n"
    );
}

int handle_args(int argc, char **argv) {
    // no args -> continue into TUI
    if (argc <= 1) return -1;

    // only implementing help right now
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help(stdout);
        return 0;
    }

    // unknown option -> usage error
    fprintf(stderr, "spotistats: unknown option '%s'\n", argv[1]);
    fprintf(stderr, "Try './stats --help'\n");
    return 2;
}
