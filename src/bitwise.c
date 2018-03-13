#include "bitwise.h"

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#ifndef BW_BUF_SIZE
#define BW_BUF_SIZE BUFSIZ
#endif

// Error handling

static bw_error create_error(int type) {
    int e = errno;
    if (type == BW_ERR_NONE || type == BW_ERR_OPERAND_EOF) {
        e = 0;
    }
    
    return (bw_error){
        .type = type,
        .error_number = e
    };
}

const bw_error no_error = {
    .type = BW_ERR_NONE,
    .error_number = 0
};

// Byte functions

typedef byte (*bw_operation)(byte a, byte b);

/* Generic version of _byte functions. Performs given operation on each byte. */
static inline bw_error bw_byte(FILE *input, FILE *output, byte operand, bw_operation operation) {
    byte buf[BW_BUF_SIZE];
    
    while (true) {
        // Read from input
        size_t read = fread(buf, 1, BW_BUF_SIZE, input);
        // Check error if nothing read, or return if reached EOF
        if (!read) {
            if (ferror(input)) {
                return create_error(BW_ERR_INPUT_READ);
            } else {
                return no_error;
            }
        }
        
        // Perform operation on each byte of buf
        for (size_t i = 0; i < read; i++) {
            buf[i] = operation(buf[i], operand);
        }
        
        // Write to output
        size_t written = fwrite(buf, 1, read, output);
        // Error if not enough written
        if (written != read) {
            return create_error(BW_ERR_OUTPUT_WRITE);
        }
    }
}

// OR

static inline byte bw_or(byte a, byte b) {
    return a | b;
}

bw_error or_byte(FILE *input, FILE *output, byte operand) {
    return bw_byte(input, output, operand, bw_or);
}

// AND

static inline byte bw_and(byte a, byte b) {
    return a & b;
}

bw_error and_byte(FILE *input, FILE *output, byte operand) {
    return bw_byte(input, output, operand, bw_and);
}

// XOR

static inline byte bw_xor(byte a, byte b) {
    return a ^ b;
}

bw_error xor_byte(FILE *input, FILE *output, byte operand) {
    return bw_byte(input, output, operand, bw_xor);
}

// File functions

/*
 * Handle operand file reaching EOF. Returns error to be returned after writing
 * remaining data. If `eof` is EOF_TRUNCATE, will return no error but `op_read`
 * won't be changed, indicating that writing should stop with no error.
 */
static inline bw_error handle_eof(FILE *operand, eof_mode eof, size_t in_read, byte *op_buf, size_t *op_read) {
    switch (eof) {
        case EOF_ERROR:
            return create_error(BW_ERR_OPERAND_EOF);
        case EOF_LOOP:
            // Check that operand file isn't 0 bytes long
            if (ftell(operand) == 0) {
                return create_error(BW_ERR_OPERAND_EOF);
            }
            
            // Seek to beginning of operand file and read until we have enough
            while (*op_read < in_read) {
                // Seek operand file to start, or return error
                if (fseek(operand, 0, SEEK_SET) != 0) {
                    return create_error(BW_ERR_OPERAND_SEEK);
                }
                
                byte *op_buf_rem = op_buf + *op_read;
                size_t in_read_rem = in_read - *op_read;
                
                *op_read += fread(op_buf_rem, 1, in_read_rem, operand);
            }
            
            break;
        case EOF_ONE: case EOF_ZERO:
            // Fill rest of op_buf with one or zero bits
            memset(op_buf + *op_read, eof == EOF_ONE ? ~0 : 0, in_read - *op_read);
            *op_read = in_read;
            // Fall through
        case EOF_TRUNCATE:
            // Do nothing
            break;
        default:
            assert("This shouldn't happen" && false);
    }
    
    return no_error;
}

/* Generic version of _file functions. Performs given operation on each byte. */
static inline bw_error bw_file(FILE *input, FILE *output, FILE *operand, eof_mode eof, bw_operation operation) {
    byte in_buf[BW_BUF_SIZE], op_buf[BW_BUF_SIZE];
    
    while (true) {
        // Read from input
        size_t in_read = fread(in_buf, 1, BW_BUF_SIZE, input);
        // Check error if nothing read, or return if reached EOF
        if (!in_read) {
            if (ferror(input)) {
                return create_error(BW_ERR_INPUT_READ);
            } else {
                return no_error;
            }
        }
        
        // Read from operand
        size_t op_read = fread(in_buf, 1, in_read, operand);
        // Check error if not enough read, or use EOF mode if EOF reached
        bw_error op_error = no_error;
        if (op_read < in_read) {
            if (feof(operand)) {
                // Reach EOF
                op_error = handle_eof(operand, eof, in_read, op_buf, &op_read);
            } else {
                // Must be a read error, write as much as we can before
                // returning the error at the end of this iteration
                op_error = create_error(BW_ERR_OPERAND_READ);
            }
        }
        
        // Perform operation on each byte of in_buf
        for (size_t i = 0; i < op_read; i++) {
            in_buf[i] = operation(in_buf[i], op_buf[i]);
        }
        
        // Write to output
        size_t written = fwrite(in_buf, 1, op_read, output);
        // Error if not enough written
        if (written != op_read) {
            return create_error(BW_ERR_OUTPUT_WRITE);
        }
        
        // Return operand error if there was one, or if EOF reached but no error
        if (op_error.type || op_read < in_read) {
            return op_error;
        }
    }
}

bw_error or_file(FILE *input, FILE *output, FILE *operand, eof_mode eof) {
    return bw_file(input, output, operand, eof, bw_or);
}

bw_error and_file(FILE *input, FILE *output, FILE *operand, eof_mode eof) {
    return bw_file(input, output, operand, eof, bw_and);
}

bw_error xor_file(FILE *input, FILE *output, FILE *operand, eof_mode eof) {
    return bw_file(input, output, operand, eof, bw_xor);
}

// NOT function

static inline byte bw_not(byte a, byte _) {
    return ~a;
}

bw_error not(FILE * input, FILE *output) {
    return bw_byte(input, output, 0, bw_not);
}

// Shift functions

bw_error lshift(FILE *input, FILE *output, shift amount) {
    // Calculate byte and bit offset
    size_t byte_offset = amount / 8;
    shift bit_offset = amount % 8;
    
    // Skip first byte_offset bytes from input
    if (fskip(input, byte_offset) < byte_offset && ferror(input)) {
        return create_error(BW_ERR_INPUT_READ);
    }
    
    // Write bytes from input to output, lshifted by bit_offset
    byte buf[BW_BUF_SIZE] = {0};
    while (true) {
        // Read from input, allowing space to lshift buffer
        size_t read = fread(buf, 1, BW_BUF_SIZE - 1, input);
        // If no data read, check for error or break if EOF
        if (!read) {
            if (ferror(input)) {
                return create_error(BW_ERR_INPUT_READ);
            } else {
                break;
            }
        }
        
        // LShift buffer
        for (size_t i = 0; i < read; i++) {
            byte left = buf[i] << bit_offset;
            byte right = buf[i + 1] >> (8 - bit_offset);
            buf[i] = left | right;
        }
        
        // Write lshifted buffer
        if (fwrite(buf, 1, read, output) != read) {
            return create_error(BW_ERR_OUTPUT_WRITE);
        }
    }
    
    // Output byte_offset (or the size of input if smaller) zero bytes
    if (fzero(output, byte_offset) < byte_offset) {
        return create_error(BW_ERR_OUTPUT_WRITE);
    } else {
        return no_error;
    }
}

bw_error rshift(FILE *input, FILE *output, shift amount) {
    // Calculate byte and bit offset
    size_t byte_offset = amount / 8;
    shift bit_offset = amount % 8;
    
    // Zero-fill first byte_offset bytes of output
    if (fzero(output, byte_offset) < byte_offset) {
        return create_error(BW_ERR_OUTPUT_WRITE);
    }
    
    // Write bytes from input to output, rshifted by bit_offset
    byte buf[BW_BUF_SIZE] = {0};
    while (true) {
        // Read from input, allowing space to rshift buffer
        size_t read = fread(buf + 1, 1, BW_BUF_SIZE - 1, input);
        // If no data read, check for error or break if EOF
        if (!read) {
            if (ferror(input)) {
                return create_error(BW_ERR_INPUT_READ);
            } else {
                break;
            }
        }
        
        // RShift buffer
        for (size_t i = read; i >= 1; i--) {
            size_t left = buf[i - 1] << (8 - bit_offset);
            size_t right = buf[i] >> bit_offset;
            buf[i] = left | right;
        }
        
        // Write rshifted buffer
        if (fwrite(buf + 1, 1, read, output) != read) {
            return create_error(BW_ERR_OUTPUT_WRITE);
        }
    }
    
    return no_error;
}
