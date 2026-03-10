#include "unity.h"
#include "display.h"

void setUp(){
}

void tearDown(){
}

/*
 * TEST display_create_window
 */

void test_display_create_window_rejects_null_dimensions_format(){
	TEST_ASSERT_TRUE(display_create_window(NULL) == NULL);
}


int main(void){
	UNITY_BEGIN();

	RUN_TEST(test_display_create_window_rejects_null_dimensions_format);
	
	return UNITY_END();
}
