#include "stats.h"
#include "error.h"

int main (){
	throw_error(-1, "testing", __FILE__, __LINE__);

	return 0;
}
