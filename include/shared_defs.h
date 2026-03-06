#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H

#include "display.h"
#include "sqlite3.h"

extern display_screen* LOADING_DATA_SCREEN;
extern display_screen* MAIN_SCREEN;
extern display_screen* QUIT_SCREEN;
extern display_screen* FULL_SCREEN;
extern display_window* LIST_WINDOW;

extern sqlite3* song_plays_database;

#endif
