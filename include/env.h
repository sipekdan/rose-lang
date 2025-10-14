

#ifndef __ENV_H
#define __ENV_H

#include <stddef.h>
#include "value.h"

#define ENV_INITIAL_CAPACITY 8

typedef struct variable
{
    char *name;
    value_t value;
} variable_t;

typedef struct env
{
    struct env *parent;
    variable_t *vars;
    size_t size;        /**< current number of variables */
    size_t capacity;    /**< allocated slots in the array */
} env_t;

env_t *env_create(env_t *parent);
void env_free(env_t *env);
variable_t *env_get(env_t *env, const char *name);
void env_set(env_t *env, const char *name, value_t val);
env_t *env_enter_scope(env_t *env);
env_t *env_leave_scope(env_t *env);

#endif /* !__ENV_H */