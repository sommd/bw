#include "bitwise.h"

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

// Utils

const bw_error no_error = {
    .type = BW_ERR_NONE,
    .error_number = 0
};

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

// OR

#define OP_NAME or
#define OP(a, b) a |= b
#include "bitwise_template.inc"

// AND

#define OP_NAME and
#define OP(a, b) a &= b
#include "bitwise_template.inc"

// XOR

#define OP_NAME xor
#define OP(a, b) a ^= b
#include "bitwise_template.inc"

// NOT

// Define not_byte function which ignores operand argument
#define OP_NAME not
#define OP(a, b) a = ~(a)
#define QUALIFIERS static inline
#define NO_FILE_FUNCTION
#include "bitwise_template.inc"

bw_error not(FILE * input, FILE *output) {
    return not_byte(input, output, 0);
}

// Shift functions

typedef void (*bw_shifter)(byte *buffer, size_t size, shift amount);

/* Generic function for shifting. Actual shifting done by `shifter` function. */
static bw_error bw_shift(FILE *input, FILE *output, shift amount, bw_shifter shifter) {
    // Keep track of most recent error to return at end
    bw_error error = no_error;
    
    // Read entire `input`
    size_t size;
    byte *buffer = freadall(sizeof(byte), &size, input);
    // Record error if there was one, but continue with what we did read
    if (ferror(input)) {
        error = create_error(BW_ERR_INPUT_READ);
    }
    
    // Do shift
    shifter(buffer, size, amount);
    
    // Write to output
    size_t written = fwrite(buffer, sizeof(byte), size, output);
    if (written != size) {
        error = create_error(BW_ERR_OUTPUT_WRITE);
    }
    
    return error;
}

bw_error lshift(FILE *input, FILE *output, shift amount) {
    return bw_shift(input, output, amount, memshiftl);
}

bw_error rshift(FILE *input, FILE *output, shift amount) {
    return bw_shift(input, output, amount, memshiftr);
}
