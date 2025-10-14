

#ifndef __LEXER_H
#define __LEXER_H

#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>

#include "location.h"
#include "token.h"

typedef struct lexer
{
    token_t *tokens;
    size_t index;
    size_t capacity;
    size_t count;

    char *source;
    size_t length;
    location_t loc;
    size_t pos;

    bool had_error;
} lexer_t;

#define LEXER_ERROR(lexer, msg, ...) \
    do { \
        fprintf(stderr, "[ERROR] [%s:%zu:%zu]: " msg, \
            (lexer)->loc.filename, (lexer)->loc.line, (lexer)->loc.column, ##__VA_ARGS__\
        ); \
        (lexer)->had_error = true; \
    } while (0)

void lexer_init(lexer_t *lexer, char *filename, char *source);
void lexer_free(lexer_t *lexer);

char *lexer_readfile(char *filename);

token_t *lexer_next(lexer_t *lexer);


#endif /* !__LEXER_H */