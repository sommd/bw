#ifndef ERRORS_H
#define ERRORS_H

#define ERROR_INCORRECT_USAGE 1
#define ERROR_ILLEGAL_ARGUMENT 2

#define ERROR_OPERAND_READ 3
#define ERROR_OPERAND_UNDERFLOW 4
#define ERROR_OPERAND_NOT_SEEKABLE 5

void error(int error, ...);

#endif
