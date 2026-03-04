#ifndef CLI_H
#define CLI_H

// Returns:
//   -1 = no CLI action; caller should continue into TUI
//    0 = handled successfully; caller should exit(0)
//    2 = usage error; caller should exit(2)
int handle_args(int argc, char **argv);

#endif
