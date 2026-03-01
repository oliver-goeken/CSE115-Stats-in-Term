#ifndef STATS_H
#define STATS_H

#include "display.h"

void init();
void terminate();

void quit_yes_button_interact(display_content_node* content_node);
void quit_no_button_interact(display_content_node* content_node);

#endif
