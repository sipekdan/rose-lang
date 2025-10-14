

#include <string.h>

#include "utils.h"
#include "value.h"

void value_number_print(const number_t num, int decimals)
{
    char buf[2048];

    // convert number to string with fixed max decimals
    mpfr_sprintf(buf, "%.*Rf", decimals, num);

    // trim trailing zeros
    char *p = buf + strlen(buf) - 1;
    while (p > buf && *p == '0') p--;

    // remove decimal point if no fractional part left
    if (*p == '.') p--;

    *(p+1) = '\0';

    if (strcmp(buf, "-0") == 0) {
        strcpy(buf, "0");
    }

    printf("%s", buf);
}

void value_print(value_t value)
{
    switch (value.type)
    {
        case VALUE_NULL:
            printf("null");
            break;
        case VALUE_UNDEFINED:
            printf("undefined");
            break;
        case VALUE_BOOL:
            printf(value.boolean ? "true" : "false");
            break;
        case VALUE_STRING:
            printf("\"%s\"", value.string);
            break;
        case VALUE_NUMBER:
            // printf("%.0Lf", value.number);
            value_number_print(value.number, 16);
            // mpfr_printf("%.*Rf", 16, value.number);
            break;
        
        case VALUE_ARRAY: {
            printf("[");
            for (size_t i = 0; i < value.array.count; i++) {
                value_print(value.array.elements[i]);
                if (i + 1 < value.array.count)
                    printf(", ");
            }
            printf("]");
            break;
        }

        case VALUE_OBJECT: {
            printf("{");
            for (size_t i = 0; i < value.object.count; i++) {
                printf("\"%s\": ", value.object.keys[i]);
                value_print(value.object.values[i]);
                if (i + 1 < value.object.count)
                    printf(", ");
            }
            printf("}");
            break;
        }
        case VALUE_FUNCTION: {
            // TODO: print the function object
            printf("function");
            break;
        }

        default:
            UNREACHABLE;
    }
}

value_t value_undefined()
{
    value_t value = { 0 };
    value.type = VALUE_UNDEFINED;
    return value;
}

value_t value_number(number_t number)
{
    value_t value = { 0 };
    value.type = VALUE_NUMBER;

    // value.number = number;

    // mpfr_init2(value.number, MPFR_PRECISION);
    // mpfr_set(value.number, number, MPFR_RNDN);

    return value;
}

value_t value_string(char *string)
{
    value_t value = { 0 };
    value.type = VALUE_STRING;
    value.string = strdup(string);
    return value;
}

value_t value_bool(bool boolean)
{
    value_t value = { 0 };
    value.type = VALUE_BOOL;
    value.boolean = boolean;
    return value;
}

value_t value_array(array_t array)
{
    value_t value = { 0 };
    value.type = VALUE_ARRAY;

    // Make a shallow copy of the array structure
    value.array.elements = array.elements;
    value.array.count    = array.count;
    value.array.capacity = array.capacity;

    return value;
}

value_t value_object(object_t object)
{
    value_t value = { 0 };
    value.type = VALUE_OBJECT;

    // Make a shallow copy of the object structure
    value.object.keys     = object.keys;
    value.object.values   = object.values;
    value.object.count    = object.count;
    value.object.capacity = object.capacity;

    return value;
}

value_t value_object_create(void)
{
    object_t obj = {0};
    obj.count = 0;
    obj.capacity = 8;
    obj.keys = malloc(sizeof(char*) * obj.capacity);
    obj.values = malloc(sizeof(value_t) * obj.capacity);

    return value_object(obj);
}

void object_set(object_t *obj, const char *key, value_t val)
{
    // Check if key exists, replace if found
    for (size_t i = 0; i < obj->count; i++) {
        if (strcmp(obj->keys[i], key) == 0) {
            obj->values[i] = val;
            return;
        }
    }

    // Resize arrays if needed
    if (obj->count == obj->capacity) {
        obj->capacity *= 2;
        obj->keys = realloc(obj->keys, sizeof(char*) * obj->capacity);
        obj->values = realloc(obj->values, sizeof(value_t) * obj->capacity);
    }

    // Add new key/value
    obj->keys[obj->count] = strdup(key);  // copy the key
    obj->values[obj->count] = val;
    obj->count++;
}

value_t value_function(function_t *func)
{
    value_t value = {0};
    value.type = VALUE_FUNCTION;
    value.function = func;
    return value;
}

value_t value_null()
{
    value_t value = { 0 };
    value.type = VALUE_NULL;
    return value;
}

char *value_to_string(value_t *v) {
    switch (v->type) {
        case VALUE_NUMBER: {
            char *buf = malloc(64);
            // sprintf(buf, "%Lg", v->number);
            sprintf(buf, "<fix not working value.c>");
            return buf;
        }
        case VALUE_STRING:
            return strdup(v->string);
        default:
            return strdup("<unknown>");
    }
}
