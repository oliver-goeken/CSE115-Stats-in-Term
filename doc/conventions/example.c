#include "example.h"

int example_function(int* first_arg, int second_arg){ //int* ptr; (not: int *ptr;)
	first_arg ++; //unrelated to scope change
	
	if (1 == 1){ //scope change
		
	}

	second_arg --; //related to scope change
	if (second_arg == *first_arg){ //scope change

	}

	return 0; //should have return code on every function for error checking purposes
}
