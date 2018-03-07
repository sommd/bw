#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <argp.h>
#include "bitwise.h"
#include "error.h"
#include "version.h"

#define DOC "bw - perform bitwise operations on files"
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
const char *argp_program_version = "bw " BW_VERSION;
const char *argp_program_bug_address = "<https://github.com/sommd/bw/issues/new>";
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
 * Check if the `operand` starts with `arg`, case insensitively, and that
 * strlen(`arg`) > 0.
 */
static bool matches_operand(char *arg, char *operand) {
    for (size_t i = 0; arg[i] != '\0'; i++) {
        // False if we reached the end of operand before end of arg, or if arg
        // and operand don't match.
        if (operand[i] == '\0' || tolower(arg[i]) != tolower(operand[i])) {
            return false;
        }
    }
    
    // Only return true if arg wasn't 0 chars long
    return arg[0] != '\0';
}

static operator parse_operator(char *arg) {
    if (matches_operand(arg, "|") || matches_operand(arg, "or")) {
        return OP_OR;
    } else if (matches_operand(arg, "&") || matches_operand(arg, "and")) {
        return OP_AND;
    } else if (matches_operand(arg, "^") || matches_operand(arg, "xor")) {
        return OP_XOR;
    } else if (matches_operand(arg, "~") || matches_operand(arg, "not")) {
        return OP_NOT;
    } else if (matches_operand(arg, "<<") || matches_operand(arg, "lshift")) {
        return OP_LSHIFT;
    } else if (matches_operand(arg, ">>") || matches_operand(arg, "rshift")) {
        return OP_RSHIFT;
    } else {
        error(ERROR_ILLEGAL_ARGUMENT, "Unrecognised operator", arg);
    }
}

static operand parse_operand(operator operator, char *arg) {
    operand operand;
    
    switch (operator) {
        case OP_OR: case OP_AND: case OP_XOR:
            // Parse as byte if possible, otherwise assume file
            if (sscanf(arg, "%hhi", &operand.byte) != 1) {
                operand.file = arg;
            }
            break;
        case OP_LSHIFT: case OP_RSHIFT:
            if (sscanf(arg, "%zu", &operand.shift) != 1) {
                error(ERROR_ILLEGAL_ARGUMENT, "Invalid shift amount", arg);
            }
            break;
        default:
            error(ERROR_INCORRECT_USAGE, "Operator does not take an operand");
    }
    
    return operand;
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
        case ARGP_KEY_ARG:
            if (state->arg_num == 0) {
                // Operator
                args->operator = parse_operator(arg);
            } else if (state->arg_num == 1) {
                // Operand
                args->operand = parse_operand(args->operator, arg);
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
    
    printf("Input=%s\n", args.input);
    printf("Output=%s\n", args.output);
    
    printf("Operator=");
    switch (args.operator) {
        case OP_OR: printf("|"); break;
        case OP_AND: printf("&"); break;
        case OP_XOR: printf("^"); break;
        case OP_NOT: printf("~"); break;
        case OP_LSHIFT: printf("<<"); break;
        case OP_RSHIFT: printf(">>"); break;
    }
    printf("\n");
    
    printf("Operand=");
    if (args.operand.file) {
        printf("%s (file)", args.operand.file);
    } else if (args.operator == OP_LSHIFT || args.operator == OP_RSHIFT) {
        printf("%zu (shift)", args.operand.shift);
    } else {
        printf("%hhu (byte)", args.operand.byte);
    }
    printf("\n");
    
    return 0;
}
