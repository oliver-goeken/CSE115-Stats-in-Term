#ifndef display_h
#define display_h

#include <ncurses.h>

/*
 * @brief intializes ncurses
 *
 * @return 0 on success, -1 on error
 *
 * @details
 * should only be called at the very start of using ncurses
 */
int display_init();

/*
 * @brief terminates ncurses
 *
 * @return 0 on success, -1 on error
 *
 * @details
 * only call when done with ncurses
 */
int display_kill();

#endif
