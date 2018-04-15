#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <check.h>

// fsize

static FILE *reg_file;
static FILE *char_file;
static FILE *dir_file;

void setup_fsize() {
    ck_assert(reg_file = tmpfile());
    ck_assert(char_file = fopen("/dev/null", "wb+"));
    ck_assert(dir_file = fopen("/", "rb"));
}

void teardown_fsize() {
    fclose(reg_file);
    fclose(char_file);
    fclose(dir_file);
}

START_TEST(test_fsize_empty) {
    ck_assert_int_eq(fsize(reg_file), 0);
} END_TEST

START_TEST(test_fsize_sized) {
    size_t size = _i * 10;
    unsigned char data[size]; // Doesn't matter what data we write
    
    fwrite(data, sizeof(*data), size, reg_file);
    fflush(reg_file);
    
    ck_assert_int_eq(fsize(reg_file), size);
} END_TEST

START_TEST(test_fsize_char_file) {
    ck_assert_int_eq(fsize(char_file), -1);
} END_TEST

START_TEST(test_fsize_dir_file) {
    ck_assert_int_eq(fsize(dir_file), -1);
} END_TEST

Suite *create_utils_suite() {
    Suite *s = suite_create("utils");
    
    {
        TCase *tc = tcase_create("fsize");
        tcase_add_checked_fixture(tc, setup_fsize, teardown_fsize);
        
        tcase_add_test(tc, test_fsize_empty);
        tcase_add_loop_test(tc, test_fsize_sized, 1, 10);
        tcase_add_test(tc, test_fsize_char_file);
        tcase_add_test(tc, test_fsize_dir_file);
        
        suite_add_tcase(s, tc);
    }
    
    return s;
}
