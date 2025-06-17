#include <stdio.h>
#include <string.h>
#include "basic.h"

#define MAX_INPUT_LENGTH 256
#define MAX_TOKENS 64


int main() {
    char input[MAX_INPUT_LENGTH];
    Token tokens[MAX_TOKENS];
    size_t token_count;
    BasicError error;

    basic_init();

    printf("Simple BASIC Interpreter\n");
    printf("Type EXIT to quit.\n");

    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;  // EOF or error
        }

        // Remove trailing newline if any
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Tokenize input
        if (!basic_tokenize(input, tokens, &token_count)) {
            printf("Tokenization error\n");
            continue;
        }

        // Handle special commands
        if (token_count == 1 && 
            (strcmp(tokens[0].value, "QUIT") == 0 || 
             strcmp(tokens[0].value, "EXIT") == 0)) {
            free_token_strings(tokens, token_count);
            break;
        }

        // Parse and execute
        int res = basic_parse(tokens, token_count, &error);

        if (error != ERROR_NONE && error != ERROR_UNKNOWN_COMMAND) {
            printf("Error: %s\n", basic_get_error_message(error));
        }
        
        free_token_strings(tokens, token_count);
    }

    printf("Goodbye.\n");
    return 0;
}
