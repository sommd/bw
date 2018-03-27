#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

// Global application buf size
#ifndef BUF_SIZE
#define BUF_SIZE BUFSIZ
#endif

// Min/max macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Typedef for byte
#define BYTE_BIT CHAR_BIT
typedef unsigned char byte;
_Static_assert(sizeof(byte) == 1, "Byte size is not 1");

/* Typedef for bit-shift amount. */
typedef size_t shift;

/* Get the total size of `f` if `f` is a regular file, -1 otherwise. */
off_t fsize(FILE *f);

/* Skip `count` bytes of `f`. Returns the amount of bytes skipped */
size_t fskip(FILE *f, size_t count);

/* Fill `count` bytes of `f` with zeroes. Returns the amount of bytes zeroed. */
size_t fzero(FILE *f, size_t count);

/*
 * Read as many items of `size` bytes from `f` into a dynamically allocated
 * buffer as possible and return the buffer. The number of items read will be
 * put in `read`.
 * 
 * May return NULL if no bytes were read, or if the initial buffer could not be
 * allocated. If an error occurs, the items read up to that point will be
 * returned and ferror() will be set.
 */
void *freadall(size_t size, size_t *read, FILE *f);

/* Shift all bits in `buf` by `shift` bits left up to BYTE_BIT. */
void memshiftl(byte *buf, size_t size, shift shift);

/* Shift all bits in `buf` by `shift` bits right up to BYTE_BIT. */
void memshiftr(byte *buf, size_t size, shift shift);

#endif
