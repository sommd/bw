#ifndef BITWISE_H
#define BITWISE_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// Types

/*
 * Used as an operand for all binary bitwise operations.
 */
typedef uint8_t byte;

/*
 * Used as an bit-shift amount, operand for all bitshift operations.
 */
typedef size_t shift;

/*
 * How to handle premature EOF of the operand file in '_file' functions.
 */
typedef enum eof_mode {
    /*
     * Close all files and exit with code ERROR_OPERAND_UNDERFLOW.
     */
    EOF_ERROR,
    /*
     * Ignore the rest of the input file and stop outputting, leaving the output
     * file the size of the operand file.
     */
    EOF_TRUNCATE,
    /*
     * Seek the operand file to the beginning and continue. Exit with code
     * ERROR_OPERAND_NOT_SEEKABLE if operand file cannot be seeked.
     */
    EOF_LOOP,
    /*
     * Continue the operation on the input file using zero-bits instead of the
     * input file.
     */
    EOF_ZERO,
    /*
     * Continue the operation on the input file using one-bits instead of the
     * input file.
     */
    EOF_ONE,
} eof_mode;

// OR functions

/*
 * Bitwise OR (|) each byte from `in` with `operand` and write to `out`.
 */
void or_byte(FILE *in, FILE *out, byte operand);

/*
 * Bitwise OR (|) each byte from `in` with each byte from `operand` and write to
 * `out`. If `operand` is smaller than `in`, then the specified eof_mode is used.
 */
void or_file(FILE *in, FILE *out, FILE *operand, eof_mode eof);

// AND functions

/*
 * Bitwise AND (&) each byte from `in` with `operand` and write to `out`.
 */
void and_byte(FILE *in, FILE *out, byte operand);

/*
 * Bitwise AND (&) each byte from `in` with each byte from `operand` and write
 * to `out`. If `operand` is smaller than `in`, then the specified eof_mode is
 * used.
 */
void and_file(FILE *in, FILE *out, FILE *operand, eof_mode eof);

// XOR functions

/*
 * Bitwise XOR (^) each byte from `in` with `operand` and write to `out`.
 */
void xor_byte(FILE *in, FILE *out, byte operand);

/*
 * Bitwise XOR (^) each byte from `in` with each byte from `operand` and write
 * to `out`. If `operand` is smaller than `in`, then the specified eof_mode is
 * used.
 */
void xor_file(FILE *in, FILE *out, FILE *operand, eof_mode eof);

// NOT function

/*
 * Bitwise NOT (~) each byte from `in` and write to `out`.
 */
void not(FILE *in, FILE *out);

// Shift functions

/*
 * Shift the bits from 'in' left by `amount` and write to `out`. Supports any
 * non-negative `amount`.
 */
void lshift(FILE *in, FILE *out, shift amount);

/*
 * Shift the bits from 'in' right by `amount` and write to `out`. Supports any
 * non-negative `amount`.
 */
void rshift(FILE *in, FILE *out, shift amount);

#endif
