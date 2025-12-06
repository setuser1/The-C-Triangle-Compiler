#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "compiler.h"

static void free_tokens(char **tokens, size_t count) {
    if (!tokens) return;
    for (size_t i = 0; i < count; ++i) {
        free(tokens[i]); /* free(NULL) is safe */
    }
    free(tokens);
}

/* lexsize: produce array of token texts. Caller retains ownership of 'source'
   and must free it after calling. On success returns non-NULL and *out_count
   is the number of tokens. On failure returns NULL and *out_count is 0. */
char **lexsize(char *source, size_t *out_count) {
    if (!source || !out_count) return NULL;

    char **arr = NULL;
    size_t capacity = 16, count = 0;

    Lexer lexer = {source, {TOK_UNKNOWN, NULL}};

    arr = malloc(capacity * sizeof *arr);
    if (!arr) {
        fprintf(stderr, "Failed to allocate token array\n");
        *out_count = 0;
        return NULL;
    }

    while (1) {
        const Token tok = lexer_next(&lexer);

        /* EOF cleanup */
        if (tok.type == TOK_EOF) {
            if (tok.text)
                free(tok.text);
            break;
        }

        /* Expand array dynamically, check for overflow */
        if (count >= capacity) {
            if (capacity > SIZE_MAX / 2) {
                fprintf(stderr, "Token array would overflow\n");
                /* free collected tokens */
                for (size_t i = 0; i < count; i++)
                    free(arr[i]);
                free(arr);
                if (tok.text) free(tok.text);
                *out_count = 0;
                return NULL;
            }
            size_t new_capacity = capacity * 2;
            char **new_arr = realloc(arr, new_capacity * sizeof *arr);
            if (!new_arr) {
                fprintf(stderr, "Failed to resize token array\n");
                for (size_t i = 0; i < count; i++)
                    free(arr[i]);
                free(arr);
                if (tok.text) free(tok.text);
                *out_count = 0;
                return NULL;
            }
            arr = new_arr;
            capacity = new_capacity;
        }

        /* take ownership of tok.text (may be NULL) */
        arr[count++] = tok.text;
    }

    *out_count = count;
    return arr;
}

int main(const int argc, char *argv[]) {
    char *source = NULL;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0] ? argv[0] : "program");
        return 1;
    }

    source = read_file(argv[1]);
    if (!source) {
        fprintf(stderr, "Failed to read file: %s\n", argv[1]);
        return 1;
    }

    size_t token_count = 0;
    char **tokens = lexsize(source, &token_count);

    /* Caller owns source; free it now regardless of lexsize outcome */
    free(source);
    source = NULL;

    if (!tokens) {
        /* lexsize sets token_count == 0 on failure */
        fprintf(stderr, "Lexing failed\n");
        return 1;
    }

    printf("Tokens:\n");
    for (size_t i = 0; i < token_count; i++)
        printf("%3zu : '%s'\n", i, tokens[i] ? tokens[i] : "(null)");

    free_tokens(tokens, token_count);
    tokens = NULL;

    return 0;
}

// made by the CT team
// U suck truly!