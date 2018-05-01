#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <check.h>
#include "utils.h"

void _check_error(const char *file, int line) {
    _ck_assert_failed(file, line, "Failed", strerror(errno), NULL);
}

/* Implements assert_file_bytes/mem. Uses b if mem is NULL. */
static void assert_file_eq(FILE *f, size_t n, byte *mem, byte b) {
    byte buf[BUF_SIZE];
    
    // Read up to n bytes or eof
    size_t total = 0;
    while (total < n && !feof(f)) {
        size_t read = fread(buf, sizeof(byte), MIN(BUF_SIZE, n - total), f);
        
        // Check for read error
        check_error(!ferror(f));
        
        // Assert all bytes zero
        for (size_t i = 0; i < read; i++) {
            // Byte index since start of reading
            size_t j = total + i;
            
            byte expected = mem ? mem[j] : b;
            ck_assert_msg(buf[i] == expected,
                          "Expected byte %zu to be %u but %u",
                          j, expected, buf[j]);
        }
        
        total += read;
    }
    
    ck_assert_msg(total == n, "Expected %zu bytes but only got %zu", n, total);
}

void assert_file_bytes(FILE *f, size_t n, byte b) {
    assert_file_eq(f, n, NULL, b);
}

void assert_file_mem(FILE *f, size_t n, void *mem) {
    assert_file_eq(f, n, mem, 0);
}

void create_junk(void *buf, size_t n) {
    byte *junk = buf;
    for (size_t i = 0; i < n; i++) {
        junk[i] = rand() % (UCHAR_MAX + 1);
    }
}

void write_junk(FILE *f, size_t n) {
    byte junk[BUF_SIZE];
    create_junk(junk, BUF_SIZE);
    
    while (n > 0) {
        size_t to_write = MIN(BUF_SIZE, n);
        size_t written = fwrite(junk, sizeof(byte), to_write, f);
        check_error(written == to_write);
        
        n -= written;
    }
}
