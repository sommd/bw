#include "error.h"

#include <stdio.h>
#include <stdarg.h>

static const char *errors[] = {
    [ERROR_INCORRECT_USAGE] = "%s\n",
    [ERROR_ILLEGAL_ARGUMENT] = "%s: '%s'\n",
    [ERROR_OPERAND_READ] = "Error reading from operand file\n",
    [ERROR_OPERAND_UNDERFLOW] = "Unexpected end of operand file\n",
    [ERROR_OPERAND_NOT_SEEKABLE] = "Operand file is not seekable\n",
};

void _error(int error, ...) {
    va_list args;
    va_start(args, error);
    
    vfprintf(stderr, errors[error], args);
    
    va_end(args);
}
