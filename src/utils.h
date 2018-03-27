#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>

// Global application buf size
#ifndef BUF_SIZE
#define BUF_SIZE BUFSIZ
#endif

// Min/max macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Typedef for byte
#define BYTE_BITS (sizeof(byte) * CHAR_BIT)
typedef unsigned char byte;

/* Typedef for bit-shift amount. */
typedef size_t shift;

/* Get the total size of `f` if `f` is a regular file, -1 otherwise. */
intmax_t fsize(FILE *f);

/* Skip `count` bytes of `f`. Returns the amount of bytes skipped */
size_t fskip(FILE *f, size_t count);

/* Fill `count` bytes of `f` with zeroes. Returns the amount of bytes zeroed. */
size_t fzero(FILE *f, size_t count);

/*
 * Read `f` into a dynamic buffer until EOF is reached. The number of bytes read
 * will be returned and `out` will be set to point to the buffer, which should
 * by freed by free().
 */
size_t freadall(FILE *f, byte **out);

/* Shift all bits in `buf` by `shift` bits left up to 8. */
void memshiftl(byte *buf, size_t size, shift shift);

/* Shift all bits in `buf` by `shift` bits right up to 8. */
void memshiftr(byte *buf, size_t size, shift shift);

#endif
