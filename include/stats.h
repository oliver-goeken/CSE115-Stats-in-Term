#ifndef STATS_H
#define STATS_H

#include "display.h"
#include "parse_db_funcs.h"

static bool SIGINT_FLAG;

display_screen* LOADING_DATA_SCREEN;
display_screen* MAIN_SCREEN;
display_screen* QUIT_SCREEN;
display_screen* FULL_SCREEN;
display_window* LIST_WINDOW;

sqlite3* song_plays_database;

// initializes everything
void init();

//terminates everything
void terminate();


// handle interrupt signal
void handle_sigint();

// function handler for the yes button on the quit screen
void quit_yes_button_interact(display_content_node* content_node);

// function handler for the no button on the quit screen
void quit_no_button_interact(display_content_node* content_node);

// button to enter quit screen from main menu
void quit_button_interact(display_content_node* content_node);

void sql_get_top_albums(display_content_node* content_node);
void sql_get_top_artists(display_content_node* content_node);
void sql_get_listening_history(display_content_node* content_node);
void sql_get_top_songs(display_content_node* content_node);

void handle_album_click(display_content_node* content_node);

void go_to_quit_screen();

void draw_boognish();

#endif
