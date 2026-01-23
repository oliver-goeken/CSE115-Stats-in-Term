#include "unity.h"
#include "display.h"

void setUp(){
}

void tearDown(){
}

/*
 * TEST display_create_window
 */

void test_display_create_window_only_accepts_valid_bounds(){
	TEST_ASSERT_TRUE(display_create_window(-1, -1, 1000000, 100000) == NULL);
}


int main(void){
	UNITY_BEGIN();

	RUN_TEST(test_display_create_window_only_accepts_valid_bounds);
	
	return UNITY_END();
}
