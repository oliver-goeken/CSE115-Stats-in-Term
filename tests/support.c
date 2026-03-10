#include "shared_defs.h"
#include "stats.h"

display_screen* LOADING_DATA_SCREEN = NULL;
display_screen* MAIN_SCREEN = NULL;
display_screen* QUIT_SCREEN = NULL;
display_screen* FULL_SCREEN = NULL;
display_window* LIST_WINDOW = NULL;

sqlite3* song_plays_database = NULL;

bool SIGINT_FLAG = false;

void draw_boognish(void)
{
}
