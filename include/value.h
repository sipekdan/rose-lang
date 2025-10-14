

#ifndef __VALUE_H
#define __VALUE_H

#include <stdbool.h>

#include "types.h"
#include "node.h"



/* forward declarations */
typedef struct value value_t;
typedef struct env env_t;
typedef struct eval_context eval_context_t;

typedef struct array
{
    value_t *elements;
    size_t count;
    size_t capacity;
} array_t;

typedef struct object
{
    char **keys;
    value_t *values;
    size_t count;
    size_t capacity;
} object_t;

typedef struct function
{
    bool is_native;
    union
    {
        struct
        {
            size_t param_count;
            char **param_names;
            node_t *body;
            env_t *closure;
        } user;

        value_t (*native_ptr)(eval_context_t *ctx, size_t argc, value_t *argv);
    };
} function_t;

typedef enum value_type
{
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_BOOL,
    VALUE_FUNCTION,
    VALUE_ARRAY,
    VALUE_OBJECT,
    VALUE_NULL,
    VALUE_UNDEFINED
} value_type_t;

typedef struct value
{
    value_type_t type;
    union
    {
        number_t number;
        char *string;
        function_t *function;
        bool boolean;
        array_t array;
        object_t object;
    };
} value_t;

void value_print(value_t value);

value_t value_number(number_t number);
value_t value_string(char *string);
value_t value_bool(bool boolean);
value_t value_array(array_t array);
value_t value_function(function_t *func);

value_t value_object(object_t object);
value_t value_object_create(void);
void object_set(object_t *obj, const char *key, value_t val);

value_t value_null();
value_t value_undefined();

char *value_to_string(value_t *v);

#endif // __VALUE_H