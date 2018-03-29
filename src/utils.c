#include "utils.h"

#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

off_t fsize(FILE *f) {
    struct stat st;
    int res = fstat(fileno(f), &st);
    
    // Check `f` is a regular file and it's size is a sane value
    if (res != -1 && (st.st_mode & S_IFREG) && st.st_size >= 0) {
        return st.st_size;
    } else {
        return -1;
    }
}

size_t fskip(FILE *f, size_t count) {
    // Try to seek if regular file
    off_t size = fsize(f);
    if (size != -1) {
        // Make sure we know our current position to not seek too far
        off_t pos = ftello(f);
        if (pos != -1) {
            off_t seek = MIN(count, size - pos);
            
            if (fseeko(f, seek, SEEK_CUR)) {
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
    off_t f_size = fsize(f);
    // If we can't read a whole item, we want to return NULL.
    if (f_size > 0 && f_size >= item_size) {
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

void memshiftl(byte *buf, size_t size, shift amount) {
    // Check args
    assert(amount < size * BYTE_BIT);
    if (size == 0 || amount == 0) {
        return;
    }
    
    // Calculate byte and bit offset
    size_t byte_offset = MIN(amount / BYTE_BIT, size);
    shift bit_offset = amount % BYTE_BIT;
    
    // Shift left bytes by byte_offset bytes and bit_offset bits
    for (size_t i = 0; i < size - byte_offset - 1; i++) {
        // Lower bits of buf[i + byte_offset]
        byte left = buf[i + byte_offset] << bit_offset;
        // Upper bits of buf[i + byte_offset + 1]
        byte right = buf[i + byte_offset + 1] >> (BYTE_BIT - bit_offset);
        
        buf[i] = left | right;
    }
    
    // Shift last byte by byte_offset bytes and bit_offset bits
    buf[size - byte_offset - 1] = buf[size - 1] << bit_offset;
    
    // Zero fill remaining bytes (if there are any)
    if (byte_offset > 0) {
        memset(&buf[size - byte_offset], 0, byte_offset);
    }
}

void memshiftr(byte *buf, size_t size, shift amount) {
    // Check args
    assert(amount < size * BYTE_BIT);
    if (size == 0 || amount == 0) {
        return;
    }
    
    // Calculate byte and bit offset
    size_t byte_offset = MIN(amount / BYTE_BIT, size);
    shift bit_offset = amount % BYTE_BIT;
    
    // Shift right bytes by byte_offset bytes and bit_offset bits
    for (size_t i = size - 1; i > byte_offset; i--) {
        // Lower bits of buf[i - byte_offset - 1]
        byte left = buf[i - byte_offset - 1] << (BYTE_BIT - amount);
        // Upper bits of buf[i - byte_offset]
        byte right = buf[i - byte_offset] >> amount;
        
        buf[i] = left | right;
    }
    
    // Shift last byte by byte_offset bytes and bit_offset bits
    buf[byte_offset] = buf[0] << bit_offset;
    
    // Zero fill remaining bytes (if there are any)
    if (byte_offset > 0) {
        memset(buf, 0, byte_offset);
    }
}
