#include "utils.h"

#include <stdlib.h>
#include <unistd.h>
#include <check.h>
#include "test.h"

#define MAX_COUNT 100000
#define NCOUNTS (sizeof(counts) / sizeof(*counts))

static const size_t counts[] = {
    0,
    1,
    2,
    10,
    50,
    1000,
    5000,
    10000,
    50000,
    MAX_COUNT,
};

static FILE *reg_file;
static FILE *char_file;
static FILE *dir_file;

// setup/teardown

/* Setup empty reg_file. */
void setup_reg_empty() {
    check_error(reg_file = tmpfile());
}

/* Setup reg_file filled with MAX_COUNT bytes. */
void setup_reg_filled() {
    // Create regular file of MAX_COUNT
    check_error(reg_file = tmpfile());
    write_junk(reg_file, MAX_COUNT);
    
    // Return to start of file
    check_error(fseek(reg_file, 0, SEEK_SET) == 0);
}

/* Setup char_file and dir_file. */
void setup_special() {
    ck_assert(char_file = fopen("/dev/urandom", "wb+"));
    ck_assert(dir_file = fopen("/", "rb"));
}

void teardown_reg() {
    fclose(reg_file);
}

void teardown_special() {
    fclose(char_file);
    fclose(dir_file);
}

// fsize

/* Test fsize with an empty file. */
START_TEST(test_fsize_empty) {
    ck_assert_int_eq(fsize(reg_file), 0);
} END_TEST

/* Test fsize with variously sized files. */
START_TEST(test_fsize_sized) {
    size_t size = counts[_i];
    
    write_junk(reg_file, size);
    check_error(fflush(reg_file) == 0);
    
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

/* Test fskip with various counts. */
START_TEST(test_fskip) {
    size_t n = counts[_i];
    ck_assert_int_eq(fskip(reg_file, n), n);
    ck_assert_int_eq(ftell(reg_file), n);
} END_TEST

/* Test fskip with various counts on a character device. */
START_TEST(test_fskip_char) {
    size_t n = counts[_i];
    // No real way to check position so we can't properly test this
    ck_assert_int_eq(fskip(char_file, n), n);
} END_TEST

/* Test fskip with various counts larger than the size of the file. */
START_TEST(test_fskip_eof) {
    size_t n = counts[_i];
    ck_assert_int_eq(fskip(reg_file, MAX_COUNT + n), MAX_COUNT);
    ck_assert_int_eq(ftell(reg_file), MAX_COUNT);
} END_TEST

// fzero

/* Test fzero with various counts. */
START_TEST(test_fzero) {
    size_t n = counts[_i];
    
    ck_assert_int_eq(fzero(reg_file, n), n);
    
    // Check the whole file is all zero
    check_error(fseek(reg_file, 0, SEEK_SET) == 0);
    assert_file_bytes(reg_file, n, 0);
} END_TEST

/* Test fzero with various counts start at end of file. */
START_TEST(test_fzero_end) {
    size_t n = counts[_i];
    
    // Write some data to the file
    write_junk(reg_file, n);
    
    ck_assert_int_eq(fzero(reg_file, n), n);
    
    // Check the file is all zero after our starting position
    check_error(fseek(reg_file, n, SEEK_SET) == 0);
    assert_file_bytes(reg_file, n, 0);
} END_TEST

/* Test fzero with various counts start at the middle of the file. */
START_TEST(test_fzero_middle) {
    size_t n = counts[_i];
    
    // Write some data to the file
    write_junk(reg_file, n);
    
    // Seek to middle of file
    check_error(fseek(reg_file, n / 2, SEEK_SET) == 0);
    
    ck_assert_int_eq(fzero(reg_file, n), n);
    
    // Check the file is all zero after our starting position
    check_error(fseek(reg_file, n / 2, SEEK_SET) == 0);
    assert_file_bytes(reg_file, n, 0);
} END_TEST

// freadall

/* Test freadall with various counts of single-byte items. */
START_TEST(test_freadall_bytes) {
    size_t n = counts[_i];
    
    // Create junk test data
    byte *junk = malloc(n);
    check_error(junk);
    create_junk(junk, n);
    
    // Write junk to file
    check_error(fwrite(junk, sizeof(byte), n, reg_file) == n);
    // Return to start of file
    check_error(fseek(reg_file, 0, SEEK_SET) == 0);
    
    // Read data back
    size_t read;
    byte *data = freadall(sizeof(byte), &read, reg_file);
    if (read != 0) {
        ck_assert_ptr_nonnull(data);
    }
    
    // Check read n bytes
    ck_assert_uint_eq(read, n);
    // Check data
    ck_assert_mem_eq(junk, data, read);
    
    free(data);
    free(junk);
} END_TEST

// Suite

Suite *create_utils_suite() {
    Suite *s = suite_create("utils");
    
    {
        TCase *tc = tcase_create("fsize");
        tcase_add_checked_fixture(tc, setup_reg_empty, teardown_reg);
        tcase_add_checked_fixture(tc, setup_special, teardown_special);
        
        tcase_add_test(tc, test_fsize_empty);
        tcase_add_loop_test(tc, test_fsize_sized, 0, NCOUNTS);
        tcase_add_test(tc, test_fsize_char_file);
        tcase_add_test(tc, test_fsize_dir_file);
        
        suite_add_tcase(s, tc);
    }
    
    {
        TCase *tc = tcase_create("fskip");
        tcase_add_checked_fixture(tc, setup_reg_filled, teardown_reg);
        tcase_add_checked_fixture(tc, setup_special, teardown_special);
        
        tcase_add_loop_test(tc, test_fskip, 0, NCOUNTS);
        tcase_add_loop_test(tc, test_fskip_char, 0, NCOUNTS);
        tcase_add_loop_test(tc, test_fskip_eof, 0, NCOUNTS);
        
        suite_add_tcase(s, tc);
    }
    
    {
        TCase *tc = tcase_create("fzero");
        tcase_add_checked_fixture(tc, setup_reg_empty, teardown_reg);
        
        tcase_add_loop_test(tc, test_fzero, 0, NCOUNTS);
        tcase_add_loop_test(tc, test_fzero_end, 0, NCOUNTS);
        tcase_add_loop_test(tc, test_fzero_middle, 0, NCOUNTS);
        
        suite_add_tcase(s, tc);
    }
    
    {
        TCase *tc = tcase_create("freadall");
        tcase_add_checked_fixture(tc, setup_reg_empty, teardown_reg);
        
        tcase_add_loop_test(tc, test_freadall_bytes, 0, NCOUNTS);
        
        suite_add_tcase(s, tc);
    }
    
    return s;
}
