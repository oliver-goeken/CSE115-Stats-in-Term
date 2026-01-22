#include "stats.h"
#include "error.h"
#include "display.h"
#include <string.h>
#include <signal.h>
#include <stdlib.h>

int main (){
	init();

	display_terminate();

	return 0;
}

void init(){
	display_init();
	
	signal(SIGINT, handle_interrupt);
	signal(SIGWINCH, handle_winch);
}

void handle_interrupt(){
	endwin();

	exit(0);
}

void handle_winch(){
	endwin();

	refresh();
}
