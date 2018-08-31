/*
 * Macro-like template file to be included in bitwise.c to implement XX_byte and
 * XX_file functions.
 * 
 * This file is used by defining some or all of the following macros and then
 * including this file.
 * 
 * OP_NAME: The name operation name. (Required)
 * OP: The operator macro. (Required)
 * QUALIFIERS: Any qualifiers for the functions defined. (Optional)
 * NO_BYTE_FUNCTION: Don't define an XX_byte function. (Optional)
 * NO_FILE_FUNCTION: Don't define an XX_file function. (Optional)
 * 
 * All of these macros will be undefined after including for convenience.
 * 
 * E.g:
 * 
 * #define OP_NAME or
 * #define OP(a, b) a |= b
 * #define QUALIFIERS static inline
 * #define NO_FILE_FUNCTION
 * #include "bitwise_template.inc"
 */

#if !defined(OP_NAME) || !defined(OP)
#error Must define OP_NAME and OP before including
#endif

#ifndef QUALIFIERS
#define QUALIFIERS
#endif

#ifndef NO_BYTE_FUNCTION

QUALIFIERS bw_error CONCAT(OP_NAME, _byte)(FILE *input, FILE *output, byte operand) {
    byte buf[BUF_SIZE];
    
    while (true) {
        // Read from input
        size_t read = fread(buf, 1, BUF_SIZE, input);
        // Check error if nothing read, or return if reached EOF
        if (!read) {
            if (ferror(input)) {
                return create_error(BW_ERR_INPUT_READ);
            } else {
                return no_error;
            }
        }
        
        // Perform operation on each byte of buf
        for (size_t i = 0; i < read; i++) {
            OP(buf[i], operand);
        }
        
        // Write to output
        size_t written = fwrite(buf, 1, read, output);
        // Error if not enough written
        if (written != read) {
            return create_error(BW_ERR_OUTPUT_WRITE);
        }
    }
}

#endif

#ifndef NO_FILE_FUNCTION

QUALIFIERS bw_error CONCAT(OP_NAME, _file)(FILE *input, FILE *output, FILE *operand, eof_mode eof) {
    byte in_buf[BUF_SIZE], op_buf[BUF_SIZE];
    
    while (true) {
        // Read from input
        size_t in_read = fread(in_buf, 1, BUF_SIZE, input);
        // Check error if nothing read, or return if reached EOF
        if (!in_read) {
            if (ferror(input)) {
                return create_error(BW_ERR_INPUT_READ);
            } else {
                return no_error;
            }
        }
        
        // Read from operand
        size_t op_read = fread(op_buf, 1, in_read, operand);
        // Check error if not enough read, or use EOF mode if EOF reached
        bw_error op_error = no_error;
        if (op_read < in_read) {
            if (feof(operand)) {
                // Reach EOF
                op_error = handle_eof(operand, eof, in_read, op_buf, &op_read);
            } else {
                // Must be a read error, write as much as we can before
                // returning the error at the end of this iteration
                op_error = create_error(BW_ERR_OPERAND_READ);
            }
        }
        
        // Perform operation on each byte of in_buf
        for (size_t i = 0; i < op_read; i++) {
            OP(in_buf[i], op_buf[i]);
        }
        
        // Write to output
        size_t written = fwrite(in_buf, 1, op_read, output);
        // Error if not enough written
        if (written != op_read) {
            return create_error(BW_ERR_OUTPUT_WRITE);
        }
        
        // Return operand error if there was one, or if EOF reached but no error
        if (op_error.type || op_read < in_read) {
            return op_error;
        }
    }
}

#endif

// Undefine for convenience
#undef OP_NAME
#undef OP
#undef QUALIFIERS
#undef NO_BYTE_FUNCTION
#undef NO_FILE_FUNCTION
