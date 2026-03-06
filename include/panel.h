#pragma once
#include "display.h"
#include "sqlite3.h"
void panel_init(sqlite3* database, display_window* list_window, display_window* info_window);
void panel_on_selection_changed(void);