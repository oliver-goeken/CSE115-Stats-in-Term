#pragma once

#include "display.h"
#include "sqlite3.h"

// Call once after you create LIST_WINDOW + INFO_WINDOW and after DB is ready.
void panel_init(sqlite3* database, display_window* list_window, display_window* info_window);

// Call whenever the selected row changes (you already added this in stats.c).
void panel_on_selection_changed(void);