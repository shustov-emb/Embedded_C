#include "../src/homework_3.h"
#include "../unity/unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_first_char_of_substring(void) {
    TEST_ASSERT_EQUAL_INT('w', *(task_4_find_substring("hello world", "world")));
    TEST_ASSERT_EQUAL_INT(NULL, task_4_find_substring("hello world", "b"));
    TEST_ASSERT_EQUAL_INT('e', *(task_4_find_substring("hello world", "ell")));
    TEST_ASSERT_EQUAL_INT(NULL, (task_4_find_substring("hello world", "ldo")));
    TEST_ASSERT_EQUAL_INT('7', *(task_4_find_substring("89076123", "7")));
}

int main(void) {
    UNITY_BEGIN(); 

    RUN_TEST(test_first_char_of_substring);

    return UNITY_END(); 
}