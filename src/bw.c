#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <argp.h>
#include "bitwise.h"
#include "error.h"
#include "config.h"

#define DOC PACKAGE_NAME " - perform bitwise operations on files"
#define ARGS_DOC "OPERATOR [OPERAND]"

typedef enum operator {
    OP_OR,
    OP_AND,
    OP_XOR,
    OP_NOT,
    OP_LSHIFT,
    OP_RSHIFT,
} operator;

typedef struct operand {
    byte byte;
    shift shift;
    char *file;
} operand;

// Arguments struct
typedef struct arguments {
    // Input/output files
    char *input, *output;
    // Operator
    operator operator;
    // Operand value
    operand operand;
    // EOF Mode
    eof_mode eof;
} arguments;

// Argp options
const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;
error_t argp_err_exit_status = ERROR_INCORRECT_USAGE;

// Options definitions
static struct argp_option options[] = {
    {"input", 'i', "FILE", 0,
        "File to read input from, or '-' to use stdin (default)"},
    {"output", 'o', "FILE", 0,
        "File to write output to, or '-' to use stdout (default)"},
    {"eof-mode", 'e', "EOF_MODE", 0,
        "How to handle the operand file being shorter than input. One of: "
        "e[rror] (default), t[runcate], l[oop], z[ero], o[ne]"},
    {0}
};

/**
 * Check if the `option` starts with `arg`, case insensitively, and that
 * strlen(`arg`) > 0.
 */
static bool matches_option(char *arg, char *option) {
    for (size_t i = 0; arg[i] != '\0'; i++) {
        // False if we reached the end of option before end of arg, or if arg
        // and option don't match.
        if (option[i] == '\0' || tolower(arg[i]) != tolower(option[i])) {
            return false;
        }
    }
    
    // Only return true if arg wasn't 0 chars long
    return arg[0] != '\0';
}

static eof_mode parse_eof_mode(char *arg) {
    if (matches_option(arg, "error")) {
        return EOF_ERROR;
    } else if (matches_option(arg, "truncate")) {
        return EOF_TRUNCATE;
    } else if (matches_option(arg, "loop")) {
        return EOF_LOOP;
    } else if (matches_option(arg, "zero")) {
        return EOF_ZERO;
    } else if (matches_option(arg, "one")) {
        return EOF_ONE;
    } else {
        error(ERROR_ILLEGAL_ARGUMENT, "Unrecognised EOF mode", arg);
    }
}

static operator parse_operator(char *arg) {
    if (matches_option(arg, "|") || matches_option(arg, "or")) {
        return OP_OR;
    } else if (matches_option(arg, "&") || matches_option(arg, "and")) {
        return OP_AND;
    } else if (matches_option(arg, "^") || matches_option(arg, "xor")) {
        return OP_XOR;
    } else if (matches_option(arg, "~") || matches_option(arg, "not")) {
        return OP_NOT;
    } else if (matches_option(arg, "<<") || matches_option(arg, "lshift")) {
        return OP_LSHIFT;
    } else if (matches_option(arg, ">>") || matches_option(arg, "rshift")) {
        return OP_RSHIFT;
    } else {
        error(ERROR_ILLEGAL_ARGUMENT, "Unrecognised operator", arg);
    }
}

static void parse_operand(operator operator, operand *operand, char *arg) {
    switch (operator) {
        case OP_OR: case OP_AND: case OP_XOR:
            // Parse as byte if possible, otherwise assume file
            if (sscanf(arg, "%hhi", &operand->byte) != 1) {
                operand->file = arg;
            }
            break;
        case OP_LSHIFT: case OP_RSHIFT:
            if (sscanf(arg, "%zu", &operand->shift) != 1) {
                error(ERROR_ILLEGAL_ARGUMENT, "Invalid shift amount", arg);
            }
            break;
        default:
            error(ERROR_INCORRECT_USAGE, "Operator does not take an operand");
    }
}

// Argp parser
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    arguments *args = state->input;
    
    switch (key) {
        case 'i':
            args->input = arg;
            break;
        case 'o':
            args->output = arg;
            break;
        case 'e':
            args->eof = parse_eof_mode(arg);
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num == 0) {
                // Operator
                args->operator = parse_operator(arg);
            } else if (state->arg_num == 1) {
                // Operand
                parse_operand(args->operator, &args->operand, arg);
            }
            
            break;
        case ARGP_KEY_END:
            if (state->arg_num == 0) {
                argp_usage(state);
            } else if (state->arg_num == 1 && args->operator != OP_NOT) {
                error(ERROR_INCORRECT_USAGE, "Operator requires an operand");
            }
            
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    
    return 0;
}

// Argp struct
static const struct argp argp = { options, parse_opt, ARGS_DOC, DOC };

int main(int argc, char *argv[]) {
    // Default options
    arguments args = {
        .eof = EOF_ERROR,
    };
    
    argp_parse(&argp, argc, argv, 0, NULL, &args);
    
    FILE *input = stdin;
    if (args.input && strcmp(args.input, "-") != 0) {
        input = fopen(args.input, "r");
        
        if (!input) {
            error(ERROR_ILLEGAL_ARGUMENT, "Can't open input", args.input);
        }
    }
    
    FILE *output = stdout;
    if (args.output && strcmp(args.output, "-") != 0) {
        output = fopen(args.output, "w");
        
        if (!output) {
            error(ERROR_ILLEGAL_ARGUMENT, "Can't open output", args.output);
        }
    }
    
    FILE *operand = NULL;
    if (args.operand.file) {
        operand = fopen(args.operand.file, "r");
        
        if (!operand) {
            error(ERROR_ILLEGAL_ARGUMENT, "Can't open operand", args.operand.file);
        }
    }
    
    bw_error error = BW_ERROR_NONE;
    switch (args.operator) {
        case OP_OR:
            if (operand) {
                error = or_file(input, output, operand, args.eof);
            } else {
                error = or_byte(input, output, args.operand.byte);
            }
            break;
        case OP_AND:
            if (operand) {
                error = and_file(input, output, operand, args.eof);
            } else {
                error = and_byte(input, output, args.operand.byte);
            }
            break;
        case OP_XOR:
            if (operand) {
                error = xor_file(input, output, operand, args.eof);
            } else {
                error = xor_byte(input, output, args.operand.byte);
            }
            break;
        case OP_NOT:
            error = not(input, output);
            break;
        case OP_LSHIFT:
            error = lshift(input, output, args.operand.shift);
            break;
        case OP_RSHIFT:
            error = rshift(input, output, args.operand.shift);
            break;
    }
    
    // Close files
    if (input != stdin) {
        fclose(input);
    }
    if (output != stdout) {
        fclose(output);
    }
    if (operand) {
        fclose(operand);
    }
    
    // Handle errors
    if (error) {
        // TODO
        fprintf(stderr, "Error\n");
        return error;
    } else {
        return EXIT_SUCCESS;
    }
}
