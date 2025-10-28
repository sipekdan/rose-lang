

#ifndef __NODE_H
#define __NODE_H

#include <stdbool.h>
#include <stddef.h>

#include "types.h"
#include "token.h"
#include "location.h"

typedef enum node_type
{
    NODE_NUMBER,
    NODE_STRING,
    NODE_BOOL,
    NODE_IDENTIFIER,
    NODE_ARRAY,
    NODE_OBJECT,
    NODE_SPREAD,
    NODE_UNDEFINED,
    NODE_NULL,

    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_BINARY,
    NODE_UNARY,
    NODE_ASSIGNMENT,
    NODE_TERNARY,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_FOR,
    NODE_CALL,
    NODE_INDEX,
    NODE_MEMBER,
    NODE_POSTFIX,
    NODE_FUNCTION,
    NODE_DECLARATION,
    NODE_SWITCH,
    NODE_LABEL,
    NODE_AWAIT,
    NODE_NEW,
    NODE_THIS,
    NODE_DEBUGGER,
    
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_THROW,
    NODE_TRY,
    NODE_RETURN,
    NODE_IMPORT,
    NODE_EXPORT,

    NODE_EMPTY,

    /* the number of nodes */
    NODE_COUNT,
} node_type_t;

typedef struct node
{
    node_type_t type;
    location_t loc;

    union
    {
        /* NODE_NUMBER */
        number_t number;

        /* NODE_STRING */
        string_t string;

        /* NODE_BOOL */
        bool boolean;

        /* NODE_IDENTIFIER */
        char *identifier;

        /* NODE_ARRAY */
        struct
        {
            struct node **elements;
            size_t count;
        } array;

        /* NODE_OBJECT */
        struct
        {
            char **keys;
            struct node **values;
            size_t count;
        } object;

        /* NODE_SPREAD */
        struct
        {
            struct node *argument;
        } spread;

        /* NODE_UNDEFINED */
        /* no data needed */

        /* NODE_NULL */
        /* no data needed */

        /* NODE_BLOCK */
        struct
        {
            /* array of statements in block*/
            struct node **statements;

            /* count of statements in block */
            size_t count;
        } block;

        struct
        {
            /* array of statements in program*/
            struct node **statements;

            /* count of statements in program */
            size_t count;
        } program;

        /* NODE_BINARY */
        struct
        {
            /* operand for binary node */
            struct token op;
            
            /* lhs for binary node */
            struct node *left;

            /* rhs for binary node */
            struct node *right;
        } binary;

        /* NODE_UNARY */
        struct
        {
            /* operand for unary node */
            struct token op;

            /* rhs for unary node */
            struct node *right;
        } unary;

        /* NODE_ASSIGNMENT */
        struct
        {
            /* assign operator (=, +=, -=, ...) */
            struct token op;

            struct node *target;
            struct node *value;
        } assignment;
        
        /* NODE_TERNARY */
        struct
        {
            struct node *condition;
            struct node *true_expr;
            struct node *false_expr;
        } ternary;

        /* NODE_IF */
        struct
        {
            struct node *condition;
            struct node *then_branch;
            struct node *else_branch;
        } if_stmt;

        /* NODE_WHILE */
        struct
        {
            struct node *condition;
            struct node *body;
        } while_stmt;

        /* NODE_DO_WHILE */
        struct
        {
            struct node *body;
            struct node *condition;
        } do_while_stmt;

        /* NODE_FOR */
        struct
        {
            struct node *body;

            struct node *init;
            struct node *condition;
            struct node *increment;
        } for_stmt;

        /* NODE_CALL */
        struct
        {
            struct node *callee;
            struct node **args;
            size_t arg_count;
        } call;
        
        /* NODE_INDEX */
        struct
        {
            struct node *array;
            struct node *index;
        } index;

        /* NODE_MEMBER */
        struct
        {
            struct node *object;
            struct node *property;
            // bool optional;
        } member;

        /* NODE_POSTFIX */
        struct
        {
            struct token op;
            struct node *left;
        } postfix;

        /* NODE_FUNCTION */
        struct
        {
            bool is_async;
            char *name;
            struct
            {
                char *name;
                struct node *default_value;
                bool is_rest;
            } *params;
            size_t param_count;
            
            struct node *body;
        } function;

        /* NODE_BREAK */
        struct
        {
            char *label;
        } break_stmt;

        /* NODE_CONTINUE */
        struct
        {
            char *label;
        } continue_stmt;

        /* NODE_THROW */
        struct
        {
            struct node *value;
        } throw_stmt;

        /* NODE_TRY */
        struct
        {
            struct node *try_block;
            char *catch_param;
            struct node *catch_block;
            struct node *finally_block;
        } try_stmt;

        /* NODE_RETURN */
        struct
        {
            struct node *value;
        } return_stmt;

        /* NODE_IMPORT */
        struct
        {
            char *module;
            /* optional: default import */
            char *default_name;
            size_t named_count;

            /* named imports */
            char **imported;
        } import_stmt;

        /* NODE_EXPORT */
        struct
        {
            /* default export */
            bool is_default;

            /* declaration for export */
            struct node *declaration;
            
            /* named export (e.g. export { a, b }) */
            size_t named_count;
            char **exported;
        } export_stmt;

        /* NODE_DECLARATION */
        struct
        {
            struct token kind;
            struct node **names;
            /* optional initializers */
            struct node **values;
            size_t count;
        } declaration;

        /* NODE_SWITCH */
        struct
        {
            struct node *expr;
            struct
            {
                struct node **labels;
                size_t labels_count;
                struct node *body;
                bool is_default;
            } *cases;
            size_t cases_count;
        } switch_stmt;

        /* NODE_LABEL */
        struct
        {
            char *name;
            struct node *statement;
        } label;

        /* NODE_AWAIT */
        struct
        {
            struct node *argument;
        } await_expr;

        /* NODE_NEW */
        struct
        {
            struct node *argument;
        } new_expr;

        /* NODE_THIS */
        /* no data needed */
        
        /* NODE_DEBUGGER */
        /* no data needed */
    };
} node_t;

void node_print(node_t *node);
void node_free(node_t *node);
void node_build(node_t *node);

// simple literals
node_t *node_create_number(number_t value, location_t loc);
node_t *node_create_string(string_t value, location_t loc);
node_t *node_create_bool(bool value, location_t loc);
node_t *node_create_identifier(const char *name, location_t loc);
node_t *node_create_null(location_t loc);
node_t *node_create_undefined(location_t loc);
node_t *node_create_this(location_t loc);

// unary / binary / ternary
node_t *node_create_unary(token_t op, node_t *right, location_t loc);
// node_t *node_create_prefix(token_t op, node_t *right, location_t loc); // optional for ++i, --i
node_t *node_create_postfix(token_t op, node_t *left, location_t loc);
node_t *node_create_binary(node_t *left, token_t op, node_t *right, location_t loc);
node_t *node_create_assignment(node_t *target, token_t op, node_t *value, location_t loc);
node_t *node_create_ternary(node_t *condition, node_t *true_expr, node_t *false_expr, location_t loc);

// control flow
node_t *node_create_if(node_t *condition, node_t *then_branch, node_t *else_branch, location_t loc);
node_t *node_create_while(node_t *condition, node_t *body, location_t loc);
node_t *node_create_do_while(node_t *body, node_t *condition, location_t loc);
node_t *node_create_for(node_t *init, node_t *condition, node_t *increment, node_t *body, location_t loc);
node_t *node_create_break(const char *label, location_t loc);
node_t *node_create_continue(const char *label, location_t loc);
node_t *node_create_return(node_t *value, location_t loc);
node_t *node_create_throw(node_t *value, location_t loc);
node_t *node_create_try(node_t *try_block, const char *catch_param, node_t *catch_block, node_t *finally_block, location_t loc);
node_t *node_create_switch(node_t *expr, size_t cases_count, location_t loc); // cases handled separately
node_t *node_create_label(const char *name, node_t *statement, location_t loc);

// blocks and programs
node_t *node_create_block(node_t **statements, size_t count, location_t loc);
node_t *node_create_program(node_t **statements, size_t count, location_t loc);

// functions / calls
node_t *node_create_function(const char *name, bool is_async, size_t param_count, location_t loc);
node_t *node_create_call(node_t *callee, node_t **args, size_t arg_count, location_t loc);
node_t *node_create_index(node_t *array, node_t *index, location_t loc);
node_t *node_create_member(node_t *object, node_t *property, location_t loc);

// arrays / objects / spread
node_t *node_create_array(node_t **elements, size_t count, location_t loc);
node_t *node_create_object(char **keys, node_t **values, size_t count, location_t loc);
node_t *node_create_spread(node_t *argument, location_t loc);

// imports / exports / declarations
node_t *node_create_import(const char *module, const char *default_name, size_t named_count, char **imported, location_t loc);
node_t *node_create_export(node_t *declaration, bool is_default, size_t named_count, char **exported, location_t loc);
node_t *node_create_declaration(token_t kind, node_t **names, node_t **values, size_t count, location_t loc);

node_t *node_create_await(node_t *argument, location_t loc);
node_t *node_create_empty(location_t loc);

const char *node_type_to_string(node_type_t type);

#endif /* !__NODE_H */