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

// OR functions

void or_byte(FILE *in, FILE *out, byte operand) {
    bw_generic_byte(in, out, |=, operand);
}

void or_file(FILE *in, FILE *out, FILE *operand, eof_mode eof) {
    // TODO
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
