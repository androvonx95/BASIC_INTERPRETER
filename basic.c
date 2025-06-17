/*
* Simple BASIC interpreter implementation
*/

#include "basic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* Function declarations */
int get_variable_value(char var_name);
void free_token_strings(Token* tokens, size_t count);

/* Global variables */
BasicVariable variables[26];  // A-Z variables

/* Initialize BASIC interpreter */
void basic_init() {
    // Initialize variables
    for (int i = 0; i < 26; i++) {
        variables[i].name = 'A' + i;
        variables[i].value = 0;
    }
}

/* Helper function to skip whitespace */
static const char* skip_whitespace(const char* str) {
    while (*str && (*str == ' ' || *str == '\t')) {
        str++;
    }
    return str;
}

/* Helper function to check if character is digit */
static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

/* Helper function to check if character is letter */
static int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/* Helper function to get operator type */
static OperatorType get_operator_type(const char* op) {
    switch (*op) {
        case '+': return OP_PLUS;
        case '-': return OP_MINUS;
        case '*': return OP_MULTIPLY;
        case '/': return OP_DIVIDE;
        case '%': return OP_MODULO;
        case '^': return OP_POWER;
        case '!': return OP_FACTORIAL;
        default: return OP_NONE;
    }
}

/* Helper function to evaluate expression */
static int validate_number(const char* num_str, BasicError* error) {
    if (!num_str || !*num_str) {
        *error = ERROR_INVALID_NUMBER;
        return 0;
    }
    
    // Check for leading sign
    if (*num_str == '-' || *num_str == '+') {
        num_str++;
    }
    
    // Must have at least one digit
    if (!isdigit(*num_str)) {
        *error = ERROR_INVALID_NUMBER;
        return 0;
    }
    
    // Check rest of string
    while (*num_str) {
        if (!isdigit(*num_str)) {
            *error = ERROR_INVALID_NUMBER;
            return 0;
        }
        num_str++;
    }
    
    *error = ERROR_NONE;
    return 1;
}

static int validate_variable(const char* var_str, BasicError* error) {
    if (!var_str || !*var_str) {
        *error = ERROR_INVALID_VARIABLE;
        return 0;
    }
    
    if (*var_str < 'A' || *var_str > 'Z') {
        *error = ERROR_INVALID_VARIABLE;
        return 0;
    }
    
    *error = ERROR_NONE;
    return 1;
}

/* Helper function to find matching parenthesis */
static size_t find_matching_paren(Token* tokens, size_t start, size_t end, BasicError* error) {
    int depth = 1;
    for (size_t i = start; i <= end; i++) {
        if (tokens[i].type == TOKEN_PAREN_OPEN) {
            depth++;
        }
        else if (tokens[i].type == TOKEN_PAREN_CLOSE) {
            depth--;
            if (depth == 0) {
                return i;
            }
        }
    }
    
    *error = ERROR_SYNTAX_ERROR;
    return 0;
}

/* Helper function to calculate factorial */
static int calculate_factorial(int n, BasicError* error) {
    if (n < 0) {
        *error = ERROR_INVALID_EXPRESSION;
        return 0;
    }
    
    if (n > 12) {  // Prevent overflow (12! fits in 32-bit int)
        *error = ERROR_INVALID_EXPRESSION;
        return 0;
    }
    
    int result = 1;
    while (n > 0) {
        result *= n;
        n--;
    }
    
    return result;
}

/* Helper function to evaluate a sub-expression */
static int evaluate_subexpression(Token* tokens, size_t start, size_t end, BasicError* error) {
    int result = 0;
    int current = 0;
    OperatorType operation = OP_PLUS;
    
    // Handle parentheses first
    for (size_t i = start; i <= end; i++) {
        if (tokens[i].type == TOKEN_PAREN_OPEN) {
            size_t close = find_matching_paren(tokens, i, end, error);
            if (*error != ERROR_NONE) {
                return 0;
            }
            
            int result = evaluate_subexpression(tokens, i + 1, close - 1, error);
            if (*error != ERROR_NONE) {
                return 0;
            }
            
            tokens[i].type = TOKEN_NUMBER;
            sprintf(tokens[i].value, "%d", result);
            
            // Shift tokens left to remove the evaluated parentheses
            size_t shift_amount = close - i;
            for (size_t j = i + 1; j + shift_amount <= end; j++) {
                tokens[j] = tokens[j + shift_amount];
            }
            
            end -= shift_amount;
            i = start - 1; // reset loop to start after shifting
        }
        else if (tokens[i].type == TOKEN_NUMBER &&
                i + 1 <= end &&
                tokens[i + 1].type == TOKEN_OPERATOR &&
                get_operator_type(tokens[i + 1].value) == OP_FACTORIAL) {
            int value = atoi(tokens[i].value);
            int factorial = calculate_factorial(value, error);
            if (*error != ERROR_NONE) {
                return 0;
            }
            
            tokens[i].type = TOKEN_NUMBER;
            sprintf(tokens[i].value, "%d", factorial);
            
            // Shift tokens left to remove the factorial operator
            for (size_t j = i + 1; j < end; j++) {
                tokens[j] = tokens[j + 1];
            }
            
            end--;
            i = start - 1; // reset loop to start after shifting
        }
    }
    
    // Evaluate expression without parentheses or factorials
    if (start > end) {
        *error = ERROR_INVALID_EXPRESSION;
        return 0;
    }
    
    // Initialize with first value
    if (tokens[start].type == TOKEN_NUMBER) {
        result = atoi(tokens[start].value);
    }
    else if (tokens[start].type == TOKEN_VARIABLE) {
        result = get_variable_value(*tokens[start].value);
    }
    else {
        *error = ERROR_INVALID_EXPRESSION;
        return 0;
    }
    
    // Start from second token
    for (size_t i = start + 1; i <= end; i++) {
        if (tokens[i].type == TOKEN_NUMBER) {
            current = atoi(tokens[i].value);
        }
        else if (tokens[i].type == TOKEN_VARIABLE) {
            current = get_variable_value(*tokens[i].value);
        }
        else if (tokens[i].type == TOKEN_OPERATOR) {
            OperatorType op = get_operator_type(tokens[i].value);
            operation = op;
            continue; // Skip operator, we'll use it in next iteration
        }
        else {
            *error = ERROR_INVALID_EXPRESSION;
            return 0;
        }
        
        // Apply the operation to the current value
        switch (operation) {
            case OP_PLUS:
                result += current;
                break;
            case OP_MINUS:
                result -= current;
                break;
            case OP_MULTIPLY:
                if (result > (INT_MAX / current)) {
                    *error = ERROR_INVALID_EXPRESSION;
                    return 0;
                }
                result *= current;
                break;
            case OP_DIVIDE:
                if (current == 0) {
                    *error = ERROR_DIVISION_BY_ZERO;
                    return 0;
                }
                result = result / current;  // Use integer division
                break;
            case OP_MODULO:
                if (current == 0) {
                    *error = ERROR_DIVISION_BY_ZERO;
                    return 0;
                }
                result = result % current;  // Use integer modulo
                break;
            case OP_POWER:
                if (current > 31) {  // Prevent overflow (2^31 is max for int)
                    *error = ERROR_INVALID_EXPRESSION;
                    return 0;
                }
                int base = result;
                result = 1;
                while (current > 0) {
                    result *= base;
                    current--;
                }
                break;
            default:
                *error = ERROR_INVALID_EXPRESSION;
                return 0;
        }
    }
    
    *error = ERROR_NONE;
    return result;
}

/* Main evaluation function */
static int evaluate_expression(Token* tokens, size_t count, BasicError* error) {
    if (count == 0) {
        *error = ERROR_INVALID_EXPRESSION;
        return 0;
    }
    
    // Validate parentheses balance
    int depth = 0;
    for (size_t i = 0; i < count; i++) {
        if (tokens[i].type == TOKEN_PAREN_OPEN) {
            depth++;
        }
        else if (tokens[i].type == TOKEN_PAREN_CLOSE) {
            depth--;
            if (depth < 0) {
                *error = ERROR_SYNTAX_ERROR;
                return 0;
            }
        }
    }
    if (depth != 0) {
        *error = ERROR_SYNTAX_ERROR;
        return 0;
    }
    
    return evaluate_subexpression(tokens, 0, count - 1, error);
}

/* Error message function */
const char* basic_get_error_message(BasicError error) {
    switch (error) {
        case ERROR_NONE:
            return "No error";
        case ERROR_SYNTAX_ERROR:
            return "Syntax error";
        case ERROR_UNKNOWN_COMMAND:
            return "Unknown command";
        case ERROR_INVALID_NUMBER:
            return "Invalid number";
        case ERROR_INVALID_VARIABLE:
            return "Invalid variable";
        case ERROR_DIVISION_BY_ZERO:
            return "Division by zero";
        case ERROR_INVALID_EXPRESSION:
            return "Invalid expression";
        default:
            return "Unknown error";
    }
}

/* Get variable value */
int get_variable_value(char var_name) {
    if (var_name >= 'A' && var_name <= 'Z') {
        return variables[var_name - 'A'].value;
    }
    return 0;
}

/* Set variable value */
void set_variable_value(char var_name, int value) {
    if (var_name >= 'A' && var_name <= 'Z') {
        variables[var_name - 'A'].value = value;
    }
}

/* Tokenizer */
int basic_tokenize(const char* input, Token* tokens, size_t* count) {
    size_t token_count = 0;
    const char* ptr = input;
    
    while (*ptr != '\0') {
        ptr = skip_whitespace(ptr);
        if (*ptr == '\0') break;
        
        Token current = {0};
        
        // Check for keywords (LET, PRINT, EXIT)
        if (isalpha(*ptr)) {
            const char* start = ptr;
            while (isalpha(*ptr)) ptr++;
            size_t len = ptr - start;
            
            char* token_str = malloc(len + 1);
            if (!token_str) return 0;
            memcpy(token_str, start, len);
            token_str[len] = '\0';
            
            // Check if keyword
            if (strcmp(token_str, "LET") == 0 ||
                strcmp(token_str, "PRINT") == 0 ||
                strcmp(token_str, "EXIT") == 0) {
                current.type = TOKEN_KEYWORD;
                current.value = token_str;
            }
            else if (len == 1 && token_str[0] >= 'A' && token_str[0] <= 'Z') {
                current.type = TOKEN_VARIABLE;
                current.value = token_str;
            }
            else {
                // Unknown identifier
                free(token_str);
                return 0;
            }
            
            tokens[token_count++] = current;
            continue;
        }
        
        // Number
        if (isdigit(*ptr) || ((*ptr == '-' || *ptr == '+') && isdigit(*(ptr + 1)))) {
            const char* start = ptr;
            if (*ptr == '-' || *ptr == '+') ptr++;
            while (isdigit(*ptr)) ptr++;
            
            size_t len = ptr - start;
            char* token_str = malloc(len + 1);
            if (!token_str) return 0;
            memcpy(token_str, start, len);
            token_str[len] = '\0';
            
            current.type = TOKEN_NUMBER;
            current.value = token_str;
            
            tokens[token_count++] = current;
            continue;
        }
        
        // Operators
        if (strchr("+-*/%^=!", *ptr)) {
            current.type = TOKEN_OPERATOR;
            char* op_str = malloc(2);
            if (!op_str) return 0;
            op_str[0] = *ptr;
            op_str[1] = '\0';
            current.value = op_str;
            ptr++;
            
            tokens[token_count++] = current;
            continue;
        }
        
        // Parentheses
        if (*ptr == '(') {
            current.type = TOKEN_PAREN_OPEN;
            char* pstr = malloc(2);
            if (!pstr) return 0;
            pstr[0] = '(';
            pstr[1] = '\0';
            current.value = pstr;
            ptr++;
            
            tokens[token_count++] = current;
            continue;
        }
        if (*ptr == ')') {
            current.type = TOKEN_PAREN_CLOSE;
            char* pstr = malloc(2);
            if (!pstr) return 0;
            pstr[0] = ')';
            pstr[1] = '\0';
            current.value = pstr;
            ptr++;
            
            tokens[token_count++] = current;
            continue;
        }
        
        // Unknown character
        return 0;
    }
    
    *count = token_count;
    return 1;
}

/* Free allocated token strings */
void free_token_strings(Token* tokens, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (tokens[i].value) {
            free(tokens[i].value);
            tokens[i].value = NULL;
        }
    }
}

/* Basic parse and execute */
int basic_parse(Token* tokens, size_t count, BasicError* error) {
    if (count == 0) {
        *error = ERROR_NONE;
        return 0;
    }
    
    if (tokens[0].type != TOKEN_KEYWORD) {
        *error = ERROR_SYNTAX_ERROR;
        return 0;
    }
    
    if (strcmp(tokens[0].value, "EXIT") == 0) {
        *error = ERROR_NONE;
        return -1; // Signal exit
    }
    else if (strcmp(tokens[0].value, "PRINT") == 0) {
        // PRINT <expr>
        if (count < 2) {
            *error = ERROR_SYNTAX_ERROR;
            return 0;
        }
        int result = evaluate_expression(&tokens[1], count - 1, error);
        if (*error == ERROR_NONE) {
            printf("%d\n", result);
        }
        return 1;
    }
    else if (strcmp(tokens[0].value, "LET") == 0) {
        // LET <var> = <expr>
        if (count < 4) {
            *error = ERROR_SYNTAX_ERROR;
            return 0;
        }
        
        if (tokens[1].type != TOKEN_VARIABLE) {
            *error = ERROR_INVALID_VARIABLE;
            return 0;
        }
        
        if (tokens[2].type != TOKEN_OPERATOR || strcmp(tokens[2].value, "=") != 0) {
            *error = ERROR_SYNTAX_ERROR;
            return 0;
        }
        
        int result = evaluate_expression(&tokens[3], count - 3, error);
        if (*error == ERROR_NONE) {
            set_variable_value(*tokens[1].value, result);
        }
        return 1;
    }
    
    *error = ERROR_UNKNOWN_COMMAND;
    return 0;
}
