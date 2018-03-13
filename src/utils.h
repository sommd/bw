#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// Global application buf size
#ifndef BUF_SIZE
#define BUF_SIZE BUFSIZ
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Typedef for byte
#define BYTE_SIZE sizeof(byte)
#define BYTE_BIT (BYTE_SIZE * CHAR_BIT)
typedef unsigned char byte;

/* Typedef for bit-shift amount. */
typedef size_t shift;

/* Skip `count` bytes of `f`. Returns the amount of bytes skipped */
size_t fskip(FILE *f, size_t count);

/* Fill `count` bytes of `f` with zeroes. Returns the amount of bytes zeroed. */
size_t fzero(FILE *f, size_t count);

/* Shift all bits in `buf` by `shift` bits left up to 8. */
void memshiftl(byte *buf, size_t size, shift shift);

/* Shift all bits in `buf` by `shift` bits right up to 8. */
void memshiftr(byte *buf, size_t size, shift shift);

#endif
