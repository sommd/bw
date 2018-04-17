#include "utils.h"

#include <unistd.h>
#include <check.h>
#include "test.h"

#define MAX_SIZE 100000
#define NSIZES (sizeof(sizes) / sizeof(*sizes))

static const size_t sizes[] = {
    0,
    1,
    2,
    10,
    50,
    1000,
    5000,
    10000,
    50000,
    MAX_SIZE,
};

static FILE *reg_file;
static FILE *char_file;
static FILE *dir_file;

// fsize

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

/* Test fsize with an empty file. */
START_TEST(test_fsize_empty) {
    ck_assert_int_eq(fsize(reg_file), 0);
} END_TEST

/* Test fsize with variously sized files. */
START_TEST(test_fsize_sized) {
    size_t size = sizes[_i];
    
    test_check_error(fzero(reg_file, size) == size);
    test_check_error(fflush(reg_file) == 0);
    
    ck_assert_int_eq(fsize(reg_file), size);
} END_TEST

/* Test fsize with a character device. */
START_TEST(test_fsize_char_file) {
    ck_assert_int_eq(fsize(char_file), -1);
} END_TEST

/* Test fsize with a directory. */
START_TEST(test_fsize_dir_file) {
    ck_assert_int_eq(fsize(dir_file), -1);
} END_TEST

// fskip

void setup_fskip() {
    // Create regular file of MAX_SIZE
    test_check_error(reg_file = tmpfile());
    test_check_error(fzero(reg_file, MAX_SIZE) == MAX_SIZE);
    // Return to start of file
    test_check_error(fseek(reg_file, 0, SEEK_SET) == 0);
    
    ck_assert(char_file = fopen("/dev/zero", "rb"));
}

void teardown_fskip() {
    fclose(reg_file);
    fclose(char_file);
}

/* Test fskip with various counts. */
START_TEST(test_fskip) {
    size_t n = sizes[_i];
    ck_assert_int_eq(fskip(reg_file, n), n);
    ck_assert_int_eq(ftell(reg_file), n);
} END_TEST

/* Test fskip with various counts on a character device. */
START_TEST(test_fskip_char) {
    size_t n = sizes[_i];
    // No real way to check position so we can't properly test this
    ck_assert_int_eq(fskip(char_file, n), n);
} END_TEST

/* Test fskip with various counts larger than the size of the file. */
START_TEST(test_fskip_eof) {
    size_t n = sizes[_i];
    ck_assert_int_eq(fskip(reg_file, MAX_SIZE + n), MAX_SIZE);
    ck_assert_int_eq(ftell(reg_file), MAX_SIZE);
} END_TEST

// fzero

void setup_fzero() {
    test_check_error(reg_file = tmpfile());
}

void teardown_fzero() {
    fclose(reg_file);
}

/* Test fzero with various counts. */
START_TEST(test_fzero) {
    size_t n = sizes[_i];
    
    ck_assert_int_eq(fzero(reg_file, n), n);
    
    // Check the whole file is all zero
    test_check_error(fseek(reg_file, 0, SEEK_SET) == 0);
    test_assert_file_bytes(reg_file, n, 0);
} END_TEST

/* Test fzero with various counts start at end of file. */
START_TEST(test_fzero_end) {
    size_t n = sizes[_i];
    
    // Just use write junk data
    byte data[n];
    test_check_error(fwrite(data, sizeof(byte), n, reg_file) == n);
    
    ck_assert_int_eq(fzero(reg_file, n), n);
    
    // Check the file is all zero after our starting position
    test_check_error(fseek(reg_file, n, SEEK_SET) == 0);
    test_assert_file_bytes(reg_file, n, 0);
} END_TEST

/* Test fzero with various counts start at the middle of the file. */
START_TEST(test_fzero_middle) {
    size_t n = sizes[_i];
    
    // Just use write junk data
    byte data[n];
    test_check_error(fwrite(data, sizeof(byte), n, reg_file) == n);
    // Seek to middle of file
    test_check_error(fseek(reg_file, n / 2, SEEK_SET) == 0);
    
    ck_assert_int_eq(fzero(reg_file, n), n);
    
    // Check the file is all zero after our starting position
    test_check_error(fseek(reg_file, n / 2, SEEK_SET) == 0);
    test_assert_file_bytes(reg_file, n, 0);
} END_TEST

// Suite

Suite *create_utils_suite() {
    Suite *s = suite_create("utils");
    
    {
        TCase *tc = tcase_create("fsize");
        tcase_add_checked_fixture(tc, setup_fsize, teardown_fsize);
        
        tcase_add_test(tc, test_fsize_empty);
        tcase_add_loop_test(tc, test_fsize_sized, 0, NSIZES);
        tcase_add_test(tc, test_fsize_char_file);
        tcase_add_test(tc, test_fsize_dir_file);
        
        suite_add_tcase(s, tc);
    }
    
    {
        TCase *tc = tcase_create("fskip");
        tcase_add_checked_fixture(tc, setup_fskip, teardown_fskip);
        
        tcase_add_loop_test(tc, test_fskip, 0, NSIZES);
        tcase_add_loop_test(tc, test_fskip_char, 0, NSIZES);
        tcase_add_loop_test(tc, test_fskip_eof, 0, NSIZES);
        
        suite_add_tcase(s, tc);
    }
    
    {
        TCase *tc = tcase_create("fzero");
        tcase_add_checked_fixture(tc, setup_fzero, teardown_fzero);
        
        tcase_add_loop_test(tc, test_fzero, 0, NSIZES);
        tcase_add_loop_test(tc, test_fzero_end, 0, NSIZES);
        tcase_add_loop_test(tc, test_fzero_middle, 0, NSIZES);
        
        suite_add_tcase(s, tc);
    }
    
    return s;
}
