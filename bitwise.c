#include "bitwise.h"

#include "error.h"
#include <string.h>

#ifndef BW_BUF_SIZE
#define BW_BUF_SIZE BUFSIZ
#endif

#define bw_generic_byte(in, out, operator, operand) \
    byte buf[BW_BUF_SIZE]; \
    \
    size_t read; \
    while ((read = fread(buf, 1, BW_BUF_SIZE, in)) > 0) { \
        for (size_t i = 0; i < read; i++) { \
            buf[i] operator operand; \
        } \
        \
        fwrite(buf, 1, read, out); \
    }

/*
 * Attempts to read `count` bytes from `operand` to `buf`. The given eof_mode
 * will be used if EOF is reached before `count` bytes have been read.
 */
static size_t read_operand(byte *buf, size_t count, FILE *operand, eof_mode eof) {
    size_t read = fread(buf, 1, count, operand);

    if (read < count) {
        if (feof(operand)) {
            switch (eof) {
                case EOF_TRUNCATE:
                    // Return zero at end of function
                    break;
                case EOF_LOOP:
                    // Try to seek back to beginning
                    if (fseek(operand, 0, SEEK_SET) == -1) {
                        error(ERROR_OPERAND_NOT_SEEKABLE);
                    }

                    // Recursively read more bytes. May need to loop file
                    // multiple times.
                    read += read_operand(buf + read, count - read, operand, eof);

                    break;
                case EOF_ZERO:
                    // Set rest of buf to all zero-bits
                    memset(buf + read, 0, count - read);
                    read = count;
                    break;
                case EOF_ONE:
                    // Set rest of buf to all one-bits
                    memset(buf + read, ~0, count - read);
                    read = count;
                    break;
                case EOF_ERROR:
                    error(ERROR_OPERAND_UNDERFLOW);
                    break;
            }
        } else {
            error(ERROR_OPERAND_READ);
        }
    }

    return read;
}

// OR functions

void or_byte(FILE *in, FILE *out, byte operand) {
    bw_generic_byte(in, out, |=, operand);
}

void or_file(FILE *in, FILE *out, FILE *operand, eof_mode eof) {
    byte in_buf[BW_BUF_SIZE], op_buf[BW_BUF_SIZE];
    
    size_t read;
    // Read upto BW_BUF_SIZE bytes from in, then read up to `read` bytes from
    // operand. If eof is EOF_TRUNCATE, then this will cause the loop to only
    // output up to `read` bytes. If eof is anything else then read_operand will
    // always return `read`.
    while ((read = fread(in_buf, 1, BW_BUF_SIZE, in)) > 0 &&
            (read = read_operand(op_buf, read, operand, eof))) {
        for (size_t i = 0; i < read; i++) {
            in_buf[i] |= op_buf[i];
        }
        
        fwrite(in_buf, 1, read, out);
    }
}

// AND functions

void and_byte(FILE *in, FILE *out, byte operand) {
    bw_generic_byte(in, out, &=, operand);
}

void and_file(FILE *in, FILE *out, FILE *operand, eof_mode eof) {
    // TODO
}

// XOR functions

void xor_byte(FILE *in, FILE *out, byte operand) {
    bw_generic_byte(in, out, ^=, operand);
}

void xor_file(FILE *in, FILE *out, FILE *operand, eof_mode eof) {
    // TODO
}

// NOT function

void not(FILE *in, FILE *out) {
    // Should be optomised to ~
    bw_generic_byte(in, out, ^=, ~0);
}

// Shift functions

void lshift(FILE *in, FILE *out, shift amount) {
    // TODO
}

void rshift(FILE *in, FILE *out, shift amount) {
    // TODO
}
