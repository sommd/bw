#ifndef TEST_H
#define TEST_H

#include "utils.h"

/* Abort with error message from errno if check is false. */
#define test_check_error(check) if (!(check)) _test_check_error(__FILE__, __LINE__)

/* Implementation of test_check_errno. */
void _test_check_error(const char *file, int line);

/* Assert that the next n bytes of f can be read and are all b. */
void test_assert_file_bytes(FILE *f, size_t n, byte b);

/* Write n bytes of junk data to f. */
void write_junk(FILE *f, size_t n);

#endif
