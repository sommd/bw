#include "test.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <check.h>
#include "utils.h"

void _test_check_error(const char *file, int line) {
    _ck_assert_failed(file, line, "Failed", strerror(errno), NULL);
}

void test_assert_file_bytes(FILE *f, size_t n, byte b) {
    byte buf[BUF_SIZE];
    
    // Read up to n bytes or eof
    size_t total = 0;
    while (total < n && !feof(f)) {
        size_t read = fread(buf, sizeof(byte), MIN(BUF_SIZE, n - total), f);
        
        // Check for read error
        test_check_error(!ferror(f));
        
        // Assert all bytes zero
        for (size_t i = 0; i < read; i++) {
            ck_assert_msg(buf[i] == b,
                          "Expected all bytes to be %u but byte %zu was %u",
                          b, total + i, buf[i]);
        }
        
        total += read;
    }
    
    ck_assert_msg(total == n, "Expected %zu bytes but only got %zu", n, total);
}
