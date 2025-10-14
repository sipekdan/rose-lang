#include <string.h>

#include "env.h"
#include "utils.h"

env_t *env_create(env_t *parent)
{
    env_t *env = malloc(sizeof(env_t));
    if (!env) return NULL;

    env->parent = parent;
    env->size = 0;
    env->capacity = ENV_INITIAL_CAPACITY;
    env->vars = calloc(env->capacity, sizeof(variable_t));
    if (!env->vars) {
        free(env);
        return NULL;
    }

    return env;
}

void env_free(env_t *env)
{
    if (!env) return;

    for (size_t i = 0; i < env->size; i++)
    {
        free(env->vars[i].name);

        // if (env->vars[i].value.type == VALUE_STRING &&
        //     env->vars[i].value.string)
        // {
        //     free(env->vars[i].value.string);
        // }
        // todo: handle other dynamically allocated Value types if needed
    }

    free(env->vars);
    free(env);
}

// Look up a variable by name in the environment or parent scopes
variable_t *env_get(env_t *env, const char *name)
{
    while (env) {
        for (size_t i = 0; i < env->size; i++) {
            // printf("Comparing: %s vs. %s\n", env->vars[i].name, name);
            if (strcmp(env->vars[i].name, name) == 0) {
                return &env->vars[i];
            }
        }
        env = env->parent; // search in parent scope
    }
    return NULL; // not found
}

// Set a variable in the current environment (adds or updates)
void env_set(env_t *env, const char *name, value_t val)
{
    // Try to find an existing variable in the current env
    for (size_t i = 0; i < env->size; i++) {
        if (strcmp(env->vars[i].name, name) == 0) {
            env->vars[i].value = val;
            return;
        }
    }

    // Add a new variable if not found
    if (env->size >= env->capacity) {
        env->capacity *= 2;
        env->vars = realloc(env->vars, env->capacity * sizeof(variable_t));
        if (!env->vars) {
            fprintf(stderr, "Memory allocation failed in env_set\n");
            exit(1);
        }
    }

    env->vars[env->size].name = strdup(name);
    env->vars[env->size].value = val;
    env->size++;

    // printf("HERE!: %s\n", name);
}

env_t *env_enter_scope(env_t *current)
{
    env_t *child = env_create(current);  // parent = current
    return child;  // new scope
}

env_t *env_leave_scope(env_t *current)
{
    env_t *parent = current->parent;
    env_free(current);
    return parent;
}