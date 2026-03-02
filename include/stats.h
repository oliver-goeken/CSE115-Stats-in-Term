#ifndef STATS_H
#define STATS_H

#include "display.h"

// initializes everything
void init();

//terminates everything
void terminate();

// function handler for the yes button on the quit screen
void quit_yes_button_interact(display_content_node* content_node);

// function handler for the no button on the quit screen
void quit_no_button_interact(display_content_node* content_node);

#endif
