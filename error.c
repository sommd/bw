#include "error.h"

#include <stdio.h>
#include <stdarg.h>

static const char *errors[] = {
    [ERROR_INCORRECT_USAGE] = NULL, // Handled by main
    [ERROR_ILLEGAL_ARGUMENT] = "Illegal argument: '%s'",
    [ERROR_OPERAND_READ] = "Error reading from operand file",
    [ERROR_OPERAND_UNDERFLOW] = "Unexpected end of operand file",
    [ERROR_OPERAND_NOT_SEEKABLE] = "Operand file is not seekable",
};

void error(int error, ...) {
    va_list args;
    va_start(args, error);
    
    vfprintf(stderr, errors[error], args);
    
    va_end(args);
    
    exit(error);
}
