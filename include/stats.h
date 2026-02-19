#ifndef stats_h
#define stats_h

#include "display.h"

void init();

void handle_interrupt();
void handle_winch();
void handle_segfault();

void handle_interact_quit_yes(display_window_content_node* node, display_window* window);
void handle_interact_quit_no(display_window_content_node* node, display_window* window);

#endif
