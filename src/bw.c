#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <argp.h>
#include <error.h>
#include "bitwise.h"
#include "config.h"

/* Exit code when called with incorrent usage. */
#define EXIT_INCORRECT_USAGE 1
/* Exit code when called with illegal argument. */
#define EXIT_ILLEGAL_ARGUMENT 2
/* Exit code when a file cannot be opened. */
#define EXIT_CANNOT_OPEN 3
/* Exit code when a file cannot be closed. */
#define EXIT_CANNOT_CLOSE 4
/* Exit code when an unknown error occurred. */
#define EXIT_UNKNOWN_ERROR -1
/* Exit code from bw_error. */
#define EXIT_BW_ERROR(e) (EXIT_CANNOT_CLOSE + (e).type)

#define DOC "Perform bitwise operations on files."
#define ARGS_DOC "OPERATOR [OPERAND]"

typedef enum operator {
    OP_OR,
    OP_AND,
    OP_XOR,
    OP_NOT,
    OP_LSHIFT,
    OP_RSHIFT,
} operator;

// Arguments struct
typedef struct arguments {
    // Input/output files
    char *input, *output;
    // Operator
    operator operator;
    // Operand
    struct {
        // Type
        enum {
            OPERAND_BYTE,
            OPERAND_SHIFT,
            OPERAND_FILE,
        } type;
        // Value
        union {
            byte byte;
            shift shift;
            char *file;
        };
    } operand;
    // EOF Mode
    eof_mode eof;
} arguments;

// Argp options
const char *argp_program_version = PROJECT_NAME " " PROJECT_VERSION;
const char *argp_program_bug_address = PROJECT_BUGREPORT;
error_t argp_err_exit_status = EXIT_INCORRECT_USAGE;

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
    }
    
    error(EXIT_ILLEGAL_ARGUMENT, 0, "Unrecognised EOF mode '%s'", arg);
    return -1;
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
    }
    
    error(EXIT_ILLEGAL_ARGUMENT, 0, "Unrecognised operator '%s'", arg);
    return -1;
}

static int parse_byte(char *str, byte *b) {
    // Handle binary input
    char buf[strlen(str)];
    if (sscanf(str, "0b%[01]", buf) == 1) {
        // Convert from binary
        *b = 0;
        for (char *c = buf; *c != '\0'; c++) {
            *b = (*b << 1) | (*c == '0' ? 0 : 1);
        }
        
        return 1;
    }
    
    // Handle octal preceded by "0o"
    if (sscanf(str, "0o%hho", b) == 1) {
        return 1;
    }
    
    // Let sscanf handle decimal, octal preceded by "0" and hex
    return sscanf(str, "%hhi", b);
}

static void parse_operand(operator operator, arguments *args, char *arg) {
    switch (operator) {
        case OP_OR: case OP_AND: case OP_XOR:
            // Parse as byte if possible, otherwise assume file
            args->operand.type = OPERAND_BYTE;
            if (!parse_byte(arg, &args->operand.byte)) {
                args->operand.type = OPERAND_FILE;
                args->operand.file = arg;
            }
            break;
        case OP_LSHIFT: case OP_RSHIFT:
            args->operand.type = OPERAND_SHIFT;
            if (sscanf(arg, "%zu", &args->operand.shift) != 1) {
                error(EXIT_ILLEGAL_ARGUMENT, 0, "Invalid shift amount '%s'", arg);
            }
            break;
        default:
            error(EXIT_INCORRECT_USAGE, 0, "Operator does not take an operand");
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
                parse_operand(args->operator, args, arg);
            }
            
            break;
        case ARGP_KEY_END:
            if (state->arg_num == 0) {
                argp_usage(state);
            } else if (state->arg_num == 1 && args->operator != OP_NOT) {
                error(EXIT_INCORRECT_USAGE, 0, "Operator requires an operand");
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
        input = fopen(args.input, "rb");
        
        if (!input) {
            error(EXIT_CANNOT_OPEN, errno, "%s", args.input);
        }
    }
    
    FILE *output = stdout;
    if (args.output && strcmp(args.output, "-") != 0) {
        output = fopen(args.output, "wb");
        
        if (!output) {
            error(EXIT_CANNOT_OPEN, errno, "%s", args.output);
        }
    }
    
    FILE *operand = NULL;
    if (args.operand.type == OPERAND_FILE) {
        operand = fopen(args.operand.file, "rb");
        
        if (!operand) {
            error(EXIT_CANNOT_OPEN, errno, "%s", args.operand.file);
        }
    }
    
    bw_error e = no_error;
    switch (args.operator) {
        case OP_OR:
            if (operand) {
                e = or_file(input, output, operand, args.eof);
            } else {
                e = or_byte(input, output, args.operand.byte);
            }
            break;
        case OP_AND:
            if (operand) {
                e = and_file(input, output, operand, args.eof);
            } else {
                e = and_byte(input, output, args.operand.byte);
            }
            break;
        case OP_XOR:
            if (operand) {
                e = xor_file(input, output, operand, args.eof);
            } else {
                e = xor_byte(input, output, args.operand.byte);
            }
            break;
        case OP_NOT:
            e = not(input, output);
            break;
        case OP_LSHIFT:
            e = lshift(input, output, args.operand.shift);
            break;
        case OP_RSHIFT:
            e = rshift(input, output, args.operand.shift);
            break;
    }
    
    // Close files
    if (input != stdin && fclose(input)) {
        error(EXIT_CANNOT_CLOSE, errno, "%s", args.input);
    }
    if (output != stdout && fclose(output)) {
        error(EXIT_CANNOT_CLOSE, errno, "%s", args.output);
    }
    if (operand && fclose(operand)) {
        error(EXIT_CANNOT_CLOSE, errno, "%s", args.operand.file);
    }
    
    // Handle errors
    if (e.type) {
        char *file;
        switch (e.type) {
            case BW_ERR_INPUT_READ:
                file = args.input;
                break;
            case BW_ERR_OUTPUT_WRITE:
                file = args.output;
                break;
            case BW_ERR_OPERAND_READ:
            case BW_ERR_OPERAND_SEEK:
                file = args.operand.file;
                break;
            // Special cases
            case BW_ERR_OPERAND_EOF:
                error(EXIT_BW_ERROR(e), 0, "%s: Operand file too short", args.operand.file);
            default:
                error(EXIT_UNKNOWN_ERROR, 0, "Unknown error");
        }
        
        error(EXIT_BW_ERROR(e), e.error_number, "%s", file);
    }
    
    return EXIT_SUCCESS;
}
