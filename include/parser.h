

#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "node.h"

typedef struct parser
{
    lexer_t *lexer;
    token_t *current;
    token_t *previous;
} parser_t;

#define PARSER_ERROR(parser, ...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
    } while (0)

void parser_init(parser_t *parser, lexer_t *lexer);
void parser_free(parser_t *parser);

node_t *parse_program(parser_t *parser);

#endif /* !__PARSER_H */