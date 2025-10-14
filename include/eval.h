

#ifndef __EVAL_H
#define __EVAL_H

#include "env.h"
#include "value.h"

typedef struct eval_context
{
  env_t *current_scope;

} eval_context_t;

void eval_init(eval_context_t *ctx);
void eval_free(eval_context_t *ctx);

value_t eval_program(eval_context_t *ctx, node_t *program);
value_t eval_node(eval_context_t *ctx, node_t *node);

#endif /* !__EVAL_H */