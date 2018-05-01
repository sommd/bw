#ifndef TEST_H
#define TEST_H

#include "utils.h"

/* Abort with error message from errno if check is false. */
#define check_error(check) if (!(check)) _check_error(__FILE__, __LINE__)

/* Implementation of test_check_errno. */
void _check_error(const char *file, int line);

/* Assert that the next n bytes of f can be read and are all b. */
void assert_file_bytes(FILE *f, size_t n, byte b);

/* Assert that the next n bytes of f can be read and are equal to mem. */
void assert_file_mem(FILE *f, size_t n, void *mem);

/* Fill buf with n bytes of junk. */
void create_junk(void *buf, size_t n);

/* Write n bytes of junk data to f. */
void write_junk(FILE *f, size_t n);

#endif
