#include "utils.h"

#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

intmax_t fsize(FILE *f) {
    struct stat st;
    if (fstat(fileno(f), &st) != -1 && st.st_mode & S_IFREG) {
        return st.st_size;
    } else {
        return -1;
    }
}

size_t fskip(FILE *f, size_t count) {
    // Try to seek if regular file
    intmax_t size = fsize(f);
    if (size != -1) {
        // Make sure we know our current position to not seek too far
        size_t pos = ftell(f);
        if (pos != -1) {
            size_t seek = MIN(count, ((size_t) size) - pos);
            
            if (fseek(f, seek, SEEK_CUR)) {
                return seek;
            }
        }
    }
    
    // Fallback to consuming bytes
    byte buf[BUF_SIZE];
    size_t total = 0, read;
    do {
        // Read up to BUF_SIZE or remaining bytes to skip
        read = fread(buf, sizeof(byte), MIN(BUF_SIZE, count - total), f);
        total += read;
    } while (total < count && read > 0);
    
    return total;
}

size_t fzero(FILE *f, size_t count) {
    // TODO: Try using fseek to extend file
    
    // Write zero bytes manually
    byte buf[BUF_SIZE] = {0};
    size_t total = 0, written;
    do {
        // Write up to BUF_SIZE or remaining bytes to zero
        written = fwrite(buf, sizeof(byte), MIN(BUF_SIZE, count - total), f);
        total += written;
    } while (total < count && written > 0);
    
    return total;
}

void *freadall(size_t item_size, size_t *total_items, FILE *f) {
    // Initalise `total_items` to 0
    *total_items = 0;
    
    // Check item_size is valid
    if (item_size == 0) {
        return NULL;
    }
    
    // Reading buffer
    size_t read_buf_items = MAX(BUF_SIZE / item_size, 1);
    byte read_buf[read_buf_items * item_size];
    
    // Output buffer
    size_t out_buf_items = 0;
    byte *out_buf = NULL;
    
    // Try to get size of `f` and pre-allocate out_buf
    intmax_t f_size = fsize(f);
    // If we can't read a whole item, we want to return NULL.
    if (f_size >= item_size) {
        out_buf_items = f_size / item_size;
        out_buf = malloc(out_buf_items * item_size);
        
        // Check malloc succeeded
        if (!out_buf) {
            return NULL;
        }
    }
    
    while (true) {
        // Read read_buf_items into read_buf
        size_t read_items = fread(read_buf, item_size, read_buf_items, f);
        if (read_items == 0) {
            break;
        }
        
        // Resize out_buf if needed
        if (*total_items + read_items > out_buf_items) {
            out_buf_items = *total_items + read_items;
            byte *new_out_buf = realloc(out_buf, out_buf_items * item_size);
            
            // Return what we have if realloc failed
            if (!new_out_buf) {
                break;
            }
            
            out_buf = new_out_buf;
        }
        
        // Copy items from read_buf to out_buf
        byte *out_buf_dest = out_buf + (*total_items * item_size);
        memcpy(out_buf_dest, read_buf, read_items * item_size);
        *total_items += read_items;
    }
    
    return out_buf;
}

void memshiftl(byte *buf, size_t size, shift shift) {
    // Check args
    assert(shift <= 8);
    if (size == 0) {
        return;
    }
    
    // Shift first size - 1 bytes
    for (size_t i = 0; i < size - 1; i++) {
        byte left = buf[i] << shift;
        byte right = buf[i + 1] >> (8 - shift);
        buf[i] = left | right;
    }
    
    // Shift last byte
    buf[size - 1] <<= shift;
}

void memshiftr(byte *buf, size_t size, shift shift) {
    // Check args
    assert(shift <= 8);
    if (size == 0) {
        return;
    }
    
    // Shift last size - 1 bytes
    for (size_t i = size - 1; i > 0; i--) {
        byte left = buf[i - 1] << (8 - shift);
        byte right = buf[i] >> shift;
        buf[i] = left | right;
    }
    
    // Shift first byte
    buf[0] >>= shift;
}
