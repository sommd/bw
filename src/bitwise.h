#ifndef BITWISE_H
#define BITWISE_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils.h"

// Types

/* Used as an bit-shift amount, operand for all bitshift operations. */
typedef size_t shift;

/* Details on type of error (if any) in bitwise functions. */
typedef struct bw_error {
    enum {
        /* No error occured. */
        BW_ERR_NONE,
        /* Error reading from input file. */
        BW_ERR_INPUT_READ,
        /* Error writing to output file. */
        BW_ERR_OUTPUT_WRITE,
        /* Error reading from operand file. */
        BW_ERR_OPERAND_READ,
        /*
         * EOF mode is EOF_ERROR and operand file reached EOF, or EOF
         * mode is EOF_LOOP and operand file is 0 bytes long.
         */
        BW_ERR_OPERAND_EOF,
        /* EOF mode is EOF_LOOP and operand file cannot be seeked. */
        BW_ERR_OPERAND_SEEK,
    } type;
    /* The errno of the error that occurred. */
    int error_number;
} bw_error;

extern const bw_error no_error;

/* How to handle premature EOF of the operand file in '_file' functions. */
typedef enum eof_mode {
    /* Close all files and exit with code ERROR_OPERAND_UNDERFLOW. */
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

// Byte functions

/* Bitwise OR each byte from `input` with `operand` and write to `output`. */
bw_error or_byte(FILE *input, FILE *output, byte operand);

/* Bitwise AND each byte from `input` with `operand` and write to `output`. */
bw_error and_byte(FILE *input, FILE *output, byte operand);

/* Bitwise XOR each byte from `input` with `operand` and write to `output`. */
bw_error xor_byte(FILE *input, FILE *output, byte operand);

// File functions

/*
 * Bitwise OR each byte from `input` with each byte from `operand` and write to
 * `output`. If `operand` is smaller than `input`, then the specified eof_mode
 * is used.
 */
bw_error or_file(FILE *input, FILE *output, FILE *operand, eof_mode eof);

/*
 * Bitwise AND each byte from `input` with each byte from `operand` and write to
 * `output`. If `operand` is smaller than `input`, then the specified eof_mode
 * is used.
 */
bw_error and_file(FILE *input, FILE *output, FILE *operand, eof_mode eof);

/*
 * Bitwise XOR each byte from `input` with each byte from `operand` and write to
 * `output`. If `operand` is smaller than `input`, then the specified eof_mode
 * is used.
 */
bw_error xor_file(FILE *input, FILE *output, FILE *operand, eof_mode eof);

// NOT function

/* Bitwise NOT each byte from `input` and write to `output`. */
bw_error not(FILE *input, FILE *output);

// Shift functions

/* Shift the bits from 'in' left by `amount` and write to `output`. */
bw_error lshift(FILE *input, FILE *output, shift amount);

/* Shift the bits from 'in' right by `amount` and write to `output`. */
bw_error rshift(FILE *input, FILE *output, shift amount);

#endif
