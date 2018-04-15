#include <stdbool.h>
#include <check.h>

START_TEST(test_fskip_zero) {
    // TODO
} END_TEST

Suite *create_utils_suite() {
    Suite *s = suite_create("utils");
    
    {
        TCase *tc = tcase_create("fskip");
        
        tcase_add_test(tc, test_fskip_zero);
        
        suite_add_tcase(s, tc);
    }
    
    return s;
}
