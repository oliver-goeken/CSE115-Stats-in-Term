#include "stats.h"
#include "error.h"
#include "display.h"

int main (){
	display_init();

	printw("Hello World!");
	refresh();
	getch();

	display_kill();

	return 0;
}
