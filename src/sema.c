#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "sema.h"

void sema_init(sema_t *sema, node_t *program)
{
    sema->program = program;
    sema->loop_depth = 0;
    sema->in_async_function = false;
    sema->had_error = false;
}

void sema_free(sema_t *sema)
{
    free(sema);
}

static void sema_visit(sema_t *sema, node_t *node);

static void sema_visit_program(sema_t *sema, node_t *node)
{
    for (size_t i = 0; i < node->block.count; i++)
        sema_visit(sema, node->block.statements[i]);
}

static void sema_visit_block(sema_t *sema, node_t *node)
{
    for (size_t i = 0; i < node->block.count; i++)
        sema_visit(sema, node->block.statements[i]);
} 

static void sema_visit_binary(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->binary.left);
    sema_visit(sema, node->binary.right);
}

static void sema_visit_unary(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->unary.right);
}

static void sema_visit_assignment(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->assignment.target);
    sema_visit(sema, node->assignment.value);
}

static void sema_visit_ternary(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->ternary.condition);
    sema_visit(sema, node->ternary.true_expr);
    sema_visit(sema, node->ternary.false_expr);
}

static void sema_visit_if(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->if_stmt.condition);
    sema_visit(sema, node->if_stmt.then_branch);
    sema_visit(sema, node->if_stmt.else_branch);
}

static void sema_visit_while(sema_t *sema, node_t *node)
{
    sema->loop_depth++;
    sema_visit(sema, node->while_stmt.condition);
    sema_visit(sema, node->while_stmt.body);
    sema->loop_depth--;
}

static void sema_visit_do_while(sema_t *sema, node_t *node)
{
    sema->loop_depth++;
    sema_visit(sema, node->do_while_stmt.body);
    sema_visit(sema, node->do_while_stmt.condition);
    sema->loop_depth--;
}

static void sema_visit_for(sema_t *sema, node_t *node)
{
    sema->loop_depth++;
    sema_visit(sema, node->for_stmt.init);
    sema_visit(sema, node->for_stmt.condition);
    sema_visit(sema, node->for_stmt.increment);
    sema_visit(sema, node->for_stmt.body);
    sema->loop_depth--;
}

static void sema_visit_call(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->call.callee);
    for (size_t i = 0; i < node->call.arg_count; i++)
        sema_visit(sema, node->call.args[i]);
}

static void sema_visit_index(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->index.array);
    sema_visit(sema, node->index.index);
}

static void sema_visit_member(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->member.object);
}

static void sema_visit_postfix(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->postfix.left);
}

static void sema_visit_function(sema_t *sema, node_t *node)
{
    bool prev_async = sema->in_async_function;
    sema->in_async_function = node->function.is_async;

    for (size_t i = 0; i < node->function.param_count; i++)
        ; /* no symbol table logic for now */

    sema_visit(sema, node->function.body);
    sema->in_async_function = prev_async;
}

static void sema_visit_declaration(sema_t *sema, node_t *node)
{
    for (size_t i = 0; i < node->declaration.count; i++)
        sema_visit(sema, node->declaration.values ? node->declaration.values[i] : NULL);
}

static void sema_visit_await(sema_t *sema, node_t *node)
{
    if (!sema->in_async_function)
        SEMA_ERROR(sema,
            "[ERROR] [%s:%zu:%zu]: 'await' can only be used inside async functions\n",
            node->loc.filename, node->loc.line, node->loc.column);
    sema_visit(sema, node->await_expr.argument);
}

static void sema_visit_break(sema_t *sema, node_t *node)
{
    if (sema->loop_depth == 0)
        SEMA_ERROR(sema,
            "[ERROR] [%s:%zu:%zu]: break not in loop\n",
            node->loc.filename, node->loc.line, node->loc.column);
}

static void sema_visit_continue(sema_t *sema, node_t *node)
{
    if (sema->loop_depth == 0)
        SEMA_ERROR(sema,
            "[ERROR] [%s:%zu:%zu]: continue not in loop\n",
            node->loc.filename, node->loc.line, node->loc.column);
}

static void sema_visit_throw(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->throw_stmt.value);
}

static void sema_visit_try(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->try_stmt.try_block);
    sema_visit(sema, node->try_stmt.catch_block);
    sema_visit(sema, node->try_stmt.finally_block);
}

static void sema_visit_return(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->return_stmt.value);
}

static void sema_visit_identifier(sema_t *sema, node_t *node)
{
    (void)sema; (void)node;
}

static void sema_visit_array(sema_t *sema, node_t *node)
{
    for (size_t i = 0; i < node->array.count; i++)
        sema_visit(sema, node->array.elements[i]);
}

static void sema_visit_object(sema_t *sema, node_t *node)
{
    for (size_t i = 0; i < node->object.count; i++)
        sema_visit(sema, node->object.values[i]);
}

static void sema_visit_spread(sema_t *sema, node_t *node)
{
    sema_visit(sema, node->spread.argument);
}

static void sema_visit(sema_t *sema, node_t *node)
{
    static_assert(NODE_COUNT == 37, "Fix NODE_COUNT in 'sema_visit'");

    if (!node) return;

    switch (node->type)
    {
        case NODE_NUMBER:
        case NODE_STRING:
        case NODE_BOOL:
        case NODE_UNDEFINED:
        case NODE_NULL:
        case NODE_THIS: break;

        case NODE_IDENTIFIER: sema_visit_identifier(sema, node); break;
        case NODE_ARRAY: sema_visit_array(sema, node); break;
        case NODE_OBJECT: sema_visit_object(sema, node); break;
        case NODE_SPREAD: sema_visit_spread(sema, node); break;
        case NODE_PROGRAM: sema_visit_program(sema, node); break;
        case NODE_BLOCK: sema_visit_block(sema, node); break;
        case NODE_BINARY: sema_visit_binary(sema, node); break;
        case NODE_UNARY: sema_visit_unary(sema, node); break;
        case NODE_ASSIGNMENT: sema_visit_assignment(sema, node); break;
        case NODE_TERNARY: sema_visit_ternary(sema, node); break;
        case NODE_IF: sema_visit_if(sema, node); break;
        case NODE_WHILE: sema_visit_while(sema, node); break;
        case NODE_DO_WHILE: sema_visit_do_while(sema, node); break;
        case NODE_FOR: sema_visit_for(sema, node); break;
        case NODE_CALL: sema_visit_call(sema, node); break;
        case NODE_INDEX: sema_visit_index(sema, node); break;
        case NODE_MEMBER: sema_visit_member(sema, node); break;
        case NODE_POSTFIX: sema_visit_postfix(sema, node); break;
        case NODE_FUNCTION: sema_visit_function(sema, node); break;
        case NODE_DECLARATION: sema_visit_declaration(sema, node); break;
        case NODE_BREAK: sema_visit_break(sema, node); break;
        case NODE_CONTINUE: sema_visit_continue(sema, node); break;
        case NODE_THROW: sema_visit_throw(sema, node); break;
        case NODE_TRY: sema_visit_try(sema, node); break;
        case NODE_RETURN: sema_visit_return(sema, node); break;
        case NODE_AWAIT: sema_visit_await(sema, node); break;

        case NODE_EMPTY: break;
        default: break;
    }
}

void sema_analyze(sema_t *sema)
{
    sema_visit(sema, sema->program);
}
