/*
 * Simple BASIC interpreter header
 */

#ifndef BASIC_H
#define BASIC_H

#include <stddef.h>
#include <stdint.h>

/* Token types */
typedef enum {
    TOKEN_NONE = 0,
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_STRING,
    TOKEN_PAREN_OPEN,
    TOKEN_PAREN_CLOSE,
    TOKEN_EXPRESSION
} TokenType;

/* Operator types */
typedef enum {
    OP_NONE = 0,
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_POWER,
    OP_FACTORIAL
} OperatorType;

/* Token structure */
typedef struct {
    TokenType type;
    char* value;
} Token;

/* Variable structure */
typedef struct {
    char name;
    int value;
} BasicVariable;

/* Error codes */
typedef enum {
    ERROR_NONE = 0,
    ERROR_INVALID_EXPRESSION,
    ERROR_SYNTAX_ERROR,
    ERROR_DIVISION_BY_ZERO,
    ERROR_UNKNOWN_VARIABLE,
    ERROR_INVALID_VARIABLE,
    ERROR_UNKNOWN_COMMAND,
    ERROR_MISSING_EXPRESSION,
    ERROR_MISSING_OPERATOR,
    ERROR_MISSING_VALUE,
    ERROR_INVALID_NUMBER
} BasicError;

/* Function prototypes */
int basic_tokenize(const char* input, Token* tokens, size_t* count);
int basic_parse(Token* tokens, size_t token_count, BasicError* error);
void basic_init();
void free_token_strings(Token* tokens, size_t count);

/* Error message function */
const char* basic_get_error_message(BasicError error);

#endif /* BASIC_H */
