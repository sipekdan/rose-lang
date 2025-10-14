

#ifndef __SEMA_H
#define __SEMA_H

#include "location.h"
#include "node.h"
#include "env.h"
#include "lexer.h"

typedef struct sema
{
    node_t *program;
    size_t loop_depth;
    bool in_async_function;
    bool had_error;
} sema_t;

#define SEMA_ERROR(ctx, ...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        (ctx)->had_error = true; \
    } while (0)

void sema_init(sema_t *ctx, node_t *program);
void sema_free(sema_t *ctx);
void sema_analyze(sema_t *ctx);

#endif /* !__SEMA_H */