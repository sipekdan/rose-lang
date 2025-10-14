#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils.h"
#include "node.h"

static void node_indent(int level)
{
    for (int i = 0; i < level; i++) printf("  ");
}

static void node_print_internal(node_t *node, int level);

void node_print(node_t *node)
{
    node_print_internal(node, 0);
}

static void node_print_internal(node_t *node, int level)
{
    static_assert(NODE_COUNT == 37, "Fix NODE_COUNT in 'node_print_internal'");

    node_indent(level);

    if (!node)
    {
        printf("(null)\n");
        return;
    }

    switch (node->type)
    {
        case NODE_PROGRAM:
            printf("Program (%zu statements)\n", node->program.count);
            for (size_t i = 0; i < node->program.count; i++)
                node_print_internal(node->program.statements[i], level + 1);
            break;

        case NODE_BLOCK:
            printf("Block (%zu statements)\n", node->block.count);
            for (size_t i = 0; i < node->block.count; i++)
                node_print_internal(node->block.statements[i], level + 1);
            break;

        case NODE_DECLARATION:
            printf("Declaration (%s):\n", node->declaration.kind.value);
            for (size_t i = 0; i < node->declaration.count; i++) {
                node_indent(level + 1);
                printf("%s", node->declaration.names[i]->identifier);
                if (node->declaration.values[i]) {
                    printf(" =\n");
                    node_print_internal(node->declaration.values[i], level + 2);
                } else {
                    printf("\n");
                }
            }
            break;

        case NODE_SWITCH:
            printf("Switch");
            printf("\n");

            node_indent(level + 1);
            printf("Expression:\n");
            node_print_internal(node->switch_stmt.expr, level + 2);

            for (size_t i = 0; i < node->switch_stmt.cases_count; i++) {
                if (node->switch_stmt.cases[i].is_default) {
                    node_indent(level + 1);
                    printf("Default:\n");
                } else {
                    node_indent(level + 1);
                    printf("Case %zu:\n", i);

                    if (node->switch_stmt.cases[i].labels_count > 0) {
                        node_indent(level + 2);
                        printf("Labels (%zu):\n", node->switch_stmt.cases[i].labels_count);
                        for (size_t j = 0; j < node->switch_stmt.cases[i].labels_count; j++) {
                            node_print_internal(node->switch_stmt.cases[i].labels[j], level + 3);
                        }
                    }
                }
                // Print body
                node_indent(level + 2);
                printf("Body:\n");
                node_print_internal(node->switch_stmt.cases[i].body, level + 3);
            }
            break;
        
        case NODE_LABEL:
            printf("Label: '%s'\n", node->label.name);
            node_print_internal(node->label.statement, level + 1);
            break;

        case NODE_AWAIT:
            printf("Await\n");
            node_print_internal(node->await_expr.argument, level + 1);
            break;    
                
        case NODE_BINARY:
            printf("Binary: '%s'\n", node->binary.op.value);
            node_print_internal(node->binary.left, level + 1);
            node_print_internal(node->binary.right, level + 1);
            break;
        
        case NODE_UNARY:
            printf("Unary '%s'\n", node->unary.op.value);
            node_print_internal(node->unary.right, level + 1);
            break;
        
        case NODE_ASSIGNMENT:
            printf("Assignment '%s'\n", node->assignment.op.value);
            node_print_internal(node->assignment.target, level + 1);
            node_print_internal(node->assignment.value, level + 1);
            break;

        case NODE_TERNARY:
            printf("Ternary:\n");
            node_indent(level + 1);
            printf("Condition:\n");
            node_print_internal(node->ternary.condition, level + 2);

            node_indent(level + 1);
            printf("True expr:\n");
            node_print_internal(node->ternary.true_expr, level + 2);

            node_indent(level + 1);
            printf("False expr:\n");
            node_print_internal(node->ternary.false_expr, level + 2);
            break;

        case NODE_IF:
            printf("If\n");
            node_indent(level + 1); printf("Condition:\n");
            node_print_internal(node->if_stmt.condition, level + 2);
            node_indent(level + 1); printf("Then:\n");
            node_print_internal(node->if_stmt.then_branch, level + 2);
            if (node->if_stmt.else_branch) {
                node_indent(level + 1); printf("Else:\n");
                node_print_internal(node->if_stmt.else_branch, level + 2);
            }
            break;

        case NODE_WHILE:
            printf("while");
            printf("\n");

            node_indent(level + 1); printf("Condition:\n");
            node_print_internal(node->while_stmt.condition, level + 2);
            node_indent(level + 1); printf("Body:\n");
            node_print_internal(node->while_stmt.body, level + 2);
            break;

        case NODE_DO_WHILE:
            printf("do-while\n");
            node_indent(level + 1); printf("Body:\n");
            node_print_internal(node->while_stmt.body, level + 2);
            node_indent(level + 1); printf("Condition:\n");
            node_print_internal(node->while_stmt.condition, level + 2);
            break;

        
        case NODE_FOR:
            printf("for");
            printf("\n");

            if (node->for_stmt.init) {
                node_indent(level + 1); printf("Init:\n");
                node_print_internal(node->for_stmt.init, level + 2);
            }

            if (node->for_stmt.condition) {
                node_indent(level + 1); printf("Condition:\n");
                node_print_internal(node->for_stmt.condition, level + 2);
            }

            if (node->for_stmt.increment) {
                node_indent(level + 1); printf("Increment:\n");
                node_print_internal(node->for_stmt.increment, level + 2);
            }

            node_indent(level + 1); printf("Body:\n");
            node_print_internal(node->for_stmt.body, level + 2);
            break;

        case NODE_CALL:
            printf("Call\n");
            node_print_internal(node->call.callee, level + 1);
            node_indent(level + 1); printf("Args\n");
            
            if (node->call.arg_count == 0) {
                node_indent(level + 2); printf("No parameters\n");
            }

            for (size_t i = 0; i < node->call.arg_count; ++i)
                node_print_internal(node->call.args[i], level + 2);
            break;

        case NODE_POSTFIX:
            printf("Postfix '%s'\n", node->postfix.op.value);
    		node_print_internal(node->postfix.left, level + 1);
            break;
        
        case NODE_INDEX:
            printf("Index access\n");
            node_indent(level + 1); printf("Array:\n");
            node_print_internal(node->index.array, level + 2);
            node_indent(level + 1); printf("Index:\n");
            node_print_internal(node->index.index, level + 2);
            break;

        case NODE_MEMBER:
            printf("Member access:\n");
            node_indent(level + 1); printf("Object:\n");
            node_print_internal(node->member.object, level + 2);
            node_indent(level + 1); printf("Property:\n");
            node_print_internal(node->member.property, level + 2);
            break;

        case NODE_FUNCTION:
            printf("Function: (async: %s) %s\n", node->function.is_async ? "true" : "false", node->function.name ? node->function.name : "(anonymous)");
            
            node_indent(level + 1);
            printf("Parameters (%zu):\n", node->function.param_count);
            for (size_t i = 0; i < node->function.param_count; i++) {
                node_indent(level + 2);
                printf("Param (spread: %s): %s\n", node->function.params[i].is_rest ? "true" : "false", node->function.params[i].name);
                if (node->function.params[i].default_value) {
                    node_indent(level + 3);
                    printf("Default value:\n");
                    node_print_internal(node->function.params[i].default_value, level + 4);
                }
            }

            node_indent(level + 1);
            printf("Body:\n");
            node_print_internal(node->function.body, level + 2);
            break;

        case NODE_ARRAY:
            printf("Array (%zu elements)\n", node->array.count);
            for (size_t i = 0; i < node->array.count; i++)
                node_print_internal(node->array.elements[i], level + 1);
            break;

        case NODE_OBJECT:
            printf("Object (%zu properties)\n", node->object.count);
            for (size_t i = 0; i < node->object.count; i++) {
                node_indent(level + 1);
                printf("Key: \"%s\"\n", node->object.keys[i]);
                node_print_internal(node->object.values[i], level + 2);
            }
            break;

        case NODE_SPREAD:
            printf("Spread\n");
            node_print_internal(node->spread.argument, level + 1);
            break;

        case NODE_BREAK:
            printf("Break");
            if (node->break_stmt.label)
                printf(" (label: %s)", node->break_stmt.label);
            printf("\n");
            break;

        case NODE_CONTINUE:
            printf("Continue");
            if (node->continue_stmt.label)
                printf(" (label: %s)", node->continue_stmt.label);
            printf("\n");
            break;

        case NODE_THROW:
            printf("Throw\n");
            node_print_internal(node->throw_stmt.value, level + 1);
            break;

        case NODE_TRY:
            printf("Try\n");
            node_indent(level + 1); printf("Try Block:\n");
            node_print_internal(node->try_stmt.try_block, level + 2);
            if (node->try_stmt.catch_block) {
                node_indent(level + 1); printf("Catch (%s):\n", node->try_stmt.catch_param ? node->try_stmt.catch_param : "(anonymous)");
                node_print_internal(node->try_stmt.catch_block, level + 2);
            }
            if (node->try_stmt.finally_block) {
                node_indent(level + 1); printf("Finally:\n");
                node_print_internal(node->try_stmt.finally_block, level + 2);
            }
            break;

        case NODE_RETURN:
            printf("Return\n");
            if (node->return_stmt.value)
                node_print_internal(node->return_stmt.value, level + 1);
            break;

        case NODE_IMPORT:
            printf("Import:\n");

            if (node->import_stmt.default_name) {
                node_indent(level + 1);
                printf("Default: %s\n", node->import_stmt.default_name);
            }

            if (node->import_stmt.named_count > 0) {
                node_indent(level + 1);
                printf("Named (%zu):\n", node->import_stmt.named_count);
                for (size_t i = 0; i < node->import_stmt.named_count; i++) {
                    node_indent(level + 2);
                    printf("%s\n", node->import_stmt.imported[i]);
                }
            }

            node_indent(level + 1);
            printf("From: \"%s\"\n", node->import_stmt.module);
            break;


        case NODE_EXPORT:
            printf("Export (default: %s)\n", node->export_stmt.is_default ? "true" : "false");

            if (node->export_stmt.declaration) {
                node_print_internal(node->export_stmt.declaration, level + 1);
            }
            else {
                if (node->export_stmt.named_count == 0) {
                    node_indent(level + 1);
                    printf("(no exports)\n");
                }
                else {
                    for (size_t i = 0; i < node->export_stmt.named_count; i++) {
                        node_indent(level + 1);
                        printf("Named export: %s\n", node->export_stmt.exported[i]);
                    }
                }
            }
            break;


        case NODE_UNDEFINED:
            printf("Undefined\n");
            break;

        case NODE_NULL:
            printf("Null\n");
            break;

        case NODE_NUMBER:
            // printf("Number: %Lf\n", node->number);
            printf("Number: %Lf\n", node->number);
            // mpfr_printf("%.16Rg\n", node->number);
            break;

        case NODE_STRING:
            printf("String: \"%s\"\n", node->string);
            break;
        
        case NODE_BOOL:
            printf("Bool: %s\n", node->boolean ? "true" : "false");
            break;
        
        case NODE_IDENTIFIER:
            printf("Identifier: %s\n", node->identifier);
            break;
        case NODE_THIS:
            printf("This\n");
            break;

        case NODE_EMPTY:
            /* nothing to print */
            printf("Empty\n");
            break;

        default:
            printf("Unreachable! For type: %d\n", node->type);
            break;
    }
}

void node_free(node_t *node)
{
    static_assert(NODE_COUNT == 37, "Fix NODE_COUNT in 'node_free'");

    if (!node) return;

    switch (node->type)
    {
        case NODE_PROGRAM:
            for (size_t i = 0; i < node->program.count; i++)
                node_free(node->program.statements[i]);
            free(node->program.statements);
            break;
        
        case NODE_BLOCK:
            for (size_t i = 0; i < node->block.count; i++)
                node_free(node->block.statements[i]);
            free(node->block.statements);
            break;
        
        case NODE_BINARY:
            node_free(node->binary.left);
            node_free(node->binary.right);
            break;
        
        case NODE_UNARY:
            node_free(node->unary.right);
            break;

        case NODE_ASSIGNMENT:
            node_free(node->assignment.target);
            node_free(node->assignment.value);
            break;

        case NODE_TERNARY:
            node_free(node->ternary.condition);
            node_free(node->ternary.true_expr);
            node_free(node->ternary.false_expr);
            break;

        case NODE_IF:
            node_free(node->if_stmt.condition);
            node_free(node->if_stmt.then_branch);
            node_free(node->if_stmt.else_branch);
            break;

        case NODE_WHILE:
            node_free(node->while_stmt.condition);
            node_free(node->while_stmt.body);
            break;

        case NODE_DO_WHILE:
            node_free(node->while_stmt.body);
            node_free(node->while_stmt.condition);
            break;


        case NODE_FOR:
            node_free(node->for_stmt.body);
            node_free(node->for_stmt.init);
            node_free(node->for_stmt.condition);
            node_free(node->for_stmt.increment);
            break;

        case NODE_CALL:
            node_free(node->call.callee);
            for (size_t i = 0; i < node->call.arg_count; ++i)
                node_free(node->call.args[i]);
            free(node->call.args);
            break;

        case NODE_POSTFIX:
            node_free(node->postfix.left);
            break;
        
        case NODE_INDEX:
            node_free(node->index.array);
            node_free(node->index.index);
            break;

        case NODE_MEMBER:
            node_free(node->member.object);
            node_free(node->member.property);
            break;

        case NODE_FUNCTION:
            free(node->function.name);
            for (size_t i = 0; i < node->function.param_count; i++) {
                free(node->function.params[i].name);
                node_free(node->function.params[i].default_value);
            }
            free(node->function.params);
            node_free(node->function.body);
            break;

        case NODE_ARRAY:
            for (size_t i = 0; i < node->array.count; i++)
                node_free(node->array.elements[i]);
            free(node->array.elements);
            break;

        case NODE_OBJECT:
            for (size_t i = 0; i < node->object.count; i++) {
                free(node->object.keys[i]);
                node_free(node->object.values[i]);
            }
            free(node->object.keys);
            free(node->object.values);
            break;

        case NODE_SPREAD:
            node_free(node->spread.argument);
            break;

        case NODE_DECLARATION:
            for (size_t i = 0; i < node->declaration.count; i++) {
                node_free(node->declaration.names[i]);
                node_free(node->declaration.values[i]);
            }
            free(node->declaration.names);
            free(node->declaration.values);
            break;

        case NODE_SWITCH:
            node_free(node->switch_stmt.expr);

            for (size_t i = 0; i < node->switch_stmt.cases_count; i++) {
                for (size_t j = 0; j < node->switch_stmt.cases[i].labels_count; j++) {
                    node_free(node->switch_stmt.cases[i].labels[j]);
                }
                free(node->switch_stmt.cases[i].labels);
                node_free(node->switch_stmt.cases[i].body);
            }
            free(node->switch_stmt.cases);
            break;

        case NODE_LABEL:
            free(node->label.name);
            node_free(node->label.statement);
            break;
        
        case NODE_AWAIT:
            node_free(node->await_expr.argument);
            break;

        case NODE_BREAK:
            free(node->break_stmt.label);
            break;

        case NODE_CONTINUE:
            free(node->continue_stmt.label);
            break;

        case NODE_THROW:
            node_free(node->throw_stmt.value);
            break;

        case NODE_TRY:
            node_free(node->try_stmt.try_block);
            free(node->try_stmt.catch_param);
            node_free(node->try_stmt.catch_block);
            node_free(node->try_stmt.finally_block);
            break;

        case NODE_RETURN:
            node_free(node->return_stmt.value);
            break;

        case NODE_IMPORT:
            free(node->import_stmt.module);

            if (node->import_stmt.default_name) {
                free(node->import_stmt.default_name);
            }

            for (size_t i = 0; i < node->import_stmt.named_count; i++) {
                free(node->import_stmt.imported[i]);
            }
            free(node->import_stmt.imported);
            break;

        case NODE_EXPORT:
            node_free(node->export_stmt.declaration);
            for (size_t i = 0; i < node->export_stmt.named_count; i++) {
                free(node->export_stmt.exported[i]);
            }
            free(node->export_stmt.exported);
            break;

        case NODE_UNDEFINED:
            /* nothing to free */
            break;

        case NODE_NULL:
            /* nothing to free */
            break;
        
        case NODE_NUMBER:
            // mpfr_clear(node->number);
            break;
        
        case NODE_STRING:
            free(node->string);
            break;
        
        case NODE_BOOL:
            /* nothing to free */
            break;

        case NODE_IDENTIFIER:
            free(node->identifier);
            break;

        case NODE_THIS:
            /* nothing to free */
            break;

        case NODE_EMPTY:
            /* nothing to free */
            break;
    
        default:
            ERROR("Unreachable! For type: (%s)%d\n", node_type_to_string(node->type), node->type);
            break;
    }

    free(node);
}

static void node_build_internal(node_t *node);

void node_build(node_t *node)
{
    if (!node) return;
    node_build_internal(node);
    printf("\n");
}

static void node_build_internal(node_t *node)
{
    if (!node) return;

    switch (node->type)
    {
        case NODE_PROGRAM:
            for (size_t i = 0; i < node->program.count; i++) {
                node_build_internal(node->program.statements[i]);
                printf(";");
            }
            break;

        case NODE_BLOCK:
            printf("{");
            for (size_t i = 0; i < node->block.count; i++) {
                node_build_internal(node->block.statements[i]);
                printf(";");
            }
            printf("}");
            break;

        case NODE_DECLARATION:
            printf("%s ", node->declaration.kind.value);
            for (size_t i = 0; i < node->declaration.count; i++) {
                printf("%s", node->declaration.names[i]->identifier);
                if (node->declaration.values[i]) {
                    printf(" = ");
                    node_build_internal(node->declaration.values[i]);
                }
                if (i < node->declaration.count - 1) printf(", ");
            }
            break;

        case NODE_SWITCH:
            printf("switch (");
            node_build_internal(node->switch_stmt.expr);
            printf(") {");
            for (size_t i = 0; i < node->switch_stmt.cases_count; i++) {
                if (node->switch_stmt.cases[i].is_default) {
                    printf("default:");
                } else {
                    for (size_t j = 0; j < node->switch_stmt.cases[i].labels_count; j++) {
                        printf("case ");
                        node_build_internal(node->switch_stmt.cases[i].labels[j]);
                        printf(":");
                    }
                }
                node_build_internal(node->switch_stmt.cases[i].body);
                // printf("\n");
            }
            printf("}");
            break;

        case NODE_LABEL:
            printf("%s:", node->label.name);
            node_build_internal(node->label.statement);
            break;

        case NODE_AWAIT:
            printf("await ");
            node_build_internal(node->await_expr.argument);
            break;

        case NODE_BINARY:
            printf("(");
            node_build_internal(node->binary.left);
            printf(" %s ", node->binary.op.value);
            node_build_internal(node->binary.right);
            printf(")");
            break;

        case NODE_UNARY:
            printf("%s", node->unary.op.value);
            node_build_internal(node->unary.right);
            break;

        case NODE_ASSIGNMENT:
            node_build_internal(node->assignment.target);
            printf(" %s ", node->assignment.op.value);
            node_build_internal(node->assignment.value);
            break;

        case NODE_TERNARY:
            node_build_internal(node->ternary.condition);
            printf(" ? ");
            node_build_internal(node->ternary.true_expr);
            printf(" : ");
            node_build_internal(node->ternary.false_expr);
            break;

        case NODE_IF:
            printf("if (");
            node_build_internal(node->if_stmt.condition);
            printf(") ");
            node_build_internal(node->if_stmt.then_branch);
            if (node->if_stmt.else_branch) {
                printf(" else ");
                node_build_internal(node->if_stmt.else_branch);
            }
            break;

        case NODE_WHILE:
            printf("while (");
            node_build_internal(node->while_stmt.condition);
            printf(") ");
            node_build_internal(node->while_stmt.body);
            break;

        case NODE_DO_WHILE:
            printf("do ");
            node_build_internal(node->while_stmt.body);
            printf(" while (");
            node_build_internal(node->while_stmt.condition);
            printf(")");
            break;

        case NODE_FOR:
            printf("for (");
            if (node->for_stmt.init) node_build_internal(node->for_stmt.init);
            printf("; ");
            if (node->for_stmt.condition) node_build_internal(node->for_stmt.condition);
            printf("; ");
            if (node->for_stmt.increment) node_build_internal(node->for_stmt.increment);
            printf(") ");
            node_build_internal(node->for_stmt.body);
            break;

        case NODE_CALL:
            node_build_internal(node->call.callee);
            printf("(");
            for (size_t i = 0; i < node->call.arg_count; i++) {
                node_build_internal(node->call.args[i]);
                if (i < node->call.arg_count - 1) printf(", ");
            }
            printf(")");
            break;

        case NODE_POSTFIX:
            node_build_internal(node->postfix.left);
            printf("%s", node->postfix.op.value);
            break;

        case NODE_INDEX:
            node_build_internal(node->index.array);
            printf("[");
            node_build_internal(node->index.index);
            printf("]");
            break;

        case NODE_MEMBER:
            node_build_internal(node->member.object);
            printf(".");
            node_build_internal(node->member.property);
            break;

        case NODE_FUNCTION:
            if (node->function.is_async) printf("async ");
            printf("function ");
            if (node->function.name) printf("%s", node->function.name);
            printf("(");
            for (size_t i = 0; i < node->function.param_count; i++) {
                if (node->function.params[i].is_rest) printf("...");
                printf("%s", node->function.params[i].name);
                if (node->function.params[i].default_value) {
                    printf(" = ");
                    node_build_internal(node->function.params[i].default_value);
                }
                if (i < node->function.param_count - 1) printf(", ");
            }
            printf(") ");
            node_build_internal(node->function.body);
            break;

        case NODE_ARRAY:
            printf("[");
            for (size_t i = 0; i < node->array.count; i++) {
                node_build_internal(node->array.elements[i]);
                if (i < node->array.count - 1) printf(", ");
            }
            printf("]");
            break;

        case NODE_OBJECT:
            printf("{");
            for (size_t i = 0; i < node->object.count; i++) {
                printf("%s: ", node->object.keys[i]);
                node_build_internal(node->object.values[i]);
                if (i < node->object.count - 1) printf(", ");
            }
            printf("}");
            break;

        case NODE_SPREAD:
            printf("...");
            node_build_internal(node->spread.argument);
            break;

        case NODE_BREAK:
            printf("break");
            if (node->break_stmt.label) printf(" %s", node->break_stmt.label);
            break;

        case NODE_CONTINUE:
            printf("continue");
            if (node->continue_stmt.label) printf(" %s", node->continue_stmt.label);
            break;

        case NODE_THROW:
            printf("throw ");
            node_build_internal(node->throw_stmt.value);
            break;

        case NODE_TRY:
            printf("try ");
            node_build_internal(node->try_stmt.try_block);
            if (node->try_stmt.catch_block) {
                printf(" catch");
                if (node->try_stmt.catch_param) printf("(%s)", node->try_stmt.catch_param);
                printf(" ");
                node_build_internal(node->try_stmt.catch_block);
            }
            if (node->try_stmt.finally_block) {
                printf(" finally ");
                node_build_internal(node->try_stmt.finally_block);
            }
            break;

        case NODE_RETURN:
            printf("return");
            if (node->return_stmt.value) {
                printf(" ");
                node_build_internal(node->return_stmt.value);
            }
            break;

        case NODE_IMPORT:
            printf("import ");
            if (node->import_stmt.default_name) printf("%s", node->import_stmt.default_name);
            if (node->import_stmt.named_count > 0) {
                if (node->import_stmt.default_name) printf(", ");
                printf("{ ");
                for (size_t i = 0; i < node->import_stmt.named_count; i++) {
                    printf("%s", node->import_stmt.imported[i]);
                    if (i < node->import_stmt.named_count - 1) printf(", ");
                }
                printf(" }");
            }
            printf(" from \"%s\"", node->import_stmt.module);
            break;

        case NODE_EXPORT:
            printf("export");
            if (node->export_stmt.is_default) printf(" default");
            printf(" ");
            if (node->export_stmt.declaration) node_build_internal(node->export_stmt.declaration);
            else if (node->export_stmt.named_count > 0) {
                printf("{ ");
                for (size_t i = 0; i < node->export_stmt.named_count; i++) {
                    printf("%s", node->export_stmt.exported[i]);
                    if (i < node->export_stmt.named_count - 1) printf(", ");
                }
                printf(" }");
            }
            break;

        case NODE_NUMBER:
            printf("%Lf", node->number);
            break;

        case NODE_STRING:
            printf("\"%s\"", node->string);
            break;

        case NODE_BOOL:
            printf("%s", node->boolean ? "true" : "false");
            break;

        case NODE_IDENTIFIER:
            printf("%s", node->identifier);
            break;

        case NODE_NULL:
            printf("null");
            break;

        case NODE_UNDEFINED:
            printf("undefined");
            break;

        case NODE_THIS:
            printf("this");
            break;

        case NODE_EMPTY:
            /* nothing */
            break;

        default:
            printf("/* unsupported node: (%s)%d */", node_type_to_string(node->type), node->type);
            break;
    }
}

static node_t *node_new(node_type_t type, location_t loc)
{
    node_t *node = calloc(1, sizeof(node_t));
    if (!node) {
        fprintf(stderr, "node allocation failed!\n");
        exit(1);
    }
    node->type = type;
    node->loc = loc;
    return node;
}

// simple literals
node_t *node_create_number(number_t value, location_t loc)  {
    node_t *node = node_new(NODE_NUMBER, loc);
    node->number = value;
    return node;
}

node_t *node_create_string(string_t value, location_t loc) {
    node_t *node = node_new(NODE_STRING, loc);
    node->string = value;
    return node;
}

node_t *node_create_bool(bool value, location_t loc) {
    node_t *node = node_new(NODE_BOOL, loc);
    node->boolean = value;
    return node;
}

node_t *node_create_identifier(const char *name, location_t loc) {
    node_t *node = node_new(NODE_IDENTIFIER, loc);
    node->identifier = strdup(name);
    return node;
}

node_t *node_create_null(location_t loc) {
    return node_new(NODE_NULL, loc);
}

node_t *node_create_undefined(location_t loc) {
    return node_new(NODE_UNDEFINED, loc);
}

node_t *node_create_this(location_t loc) {
    return node_new(NODE_THIS, loc);
}

/* Unary / Binary / Ternary */
node_t *node_create_unary(token_t op, node_t *right, location_t loc) {
    node_t *node = node_new(NODE_UNARY, loc);
    node->unary.op = op;
    node->unary.right = right;
    return node;
}

node_t *node_create_postfix(token_t op, node_t *left, location_t loc) {
    node_t *node = node_new(NODE_POSTFIX, loc);
    node->postfix.op = op;
    node->postfix.left = left;
    return node;
}

node_t *node_create_binary(node_t *left, token_t op, node_t *right, location_t loc) {
    node_t *node = node_new(NODE_BINARY, loc);
    node->binary.left = left;
    node->binary.op = op;
    node->binary.right = right;
    return node;
}

node_t *node_create_assignment(node_t *target, token_t op, node_t *value, location_t loc) {
    node_t *node = node_new(NODE_ASSIGNMENT, loc);
    node->assignment.target = target;
    node->assignment.op = op;
    node->assignment.value = value;
    return node;
}

node_t *node_create_ternary(node_t *condition, node_t *true_expr, node_t *false_expr, location_t loc) {
    node_t *node = node_new(NODE_TERNARY, loc);
    node->ternary.condition = condition;
    node->ternary.true_expr = true_expr;
    node->ternary.false_expr = false_expr;
    return node;
}

/* Control flow */
node_t *node_create_if(node_t *condition, node_t *then_branch, node_t *else_branch, location_t loc) {
    node_t *node = node_new(NODE_IF, loc);
    node->if_stmt.condition = condition;
    node->if_stmt.then_branch = then_branch;
    node->if_stmt.else_branch = else_branch;
    return node;
}

node_t *node_create_while(node_t *condition, node_t *body, location_t loc) {
    node_t *node = node_new(NODE_WHILE, loc);
    node->while_stmt.condition = condition;
    node->while_stmt.body = body;
    return node;
}

node_t *node_create_do_while(node_t *body, node_t *condition, location_t loc) {
    node_t *node = node_new(NODE_DO_WHILE, loc);
    node->do_while_stmt.body = body;
    node->do_while_stmt.condition = condition;
    return node;
}

node_t *node_create_for(node_t *init, node_t *condition, node_t *increment, node_t *body, location_t loc) {
    node_t *node = node_new(NODE_FOR, loc);
    node->for_stmt.init = init;
    node->for_stmt.condition = condition;
    node->for_stmt.increment = increment;
    node->for_stmt.body = body;
    return node;
}

node_t *node_create_break(const char *label, location_t loc) {
    node_t *node = node_new(NODE_BREAK, loc);
    if (label) node->break_stmt.label = strdup(label);
    return node;
}

node_t *node_create_continue(const char *label, location_t loc) {
    node_t *node = node_new(NODE_CONTINUE, loc);
    if (label) node->continue_stmt.label = strdup(label);
    return node;
}

node_t *node_create_return(node_t *value, location_t loc) {
    node_t *node = node_new(NODE_RETURN, loc);
    node->return_stmt.value = value;
    return node;
}

node_t *node_create_throw(node_t *value, location_t loc) {
    node_t *node = node_new(NODE_THROW, loc);
    node->throw_stmt.value = value;
    return node;
}

node_t *node_create_try(node_t *try_block, const char *catch_param, node_t *catch_block, node_t *finally_block, location_t loc) {
    node_t *node = node_new(NODE_TRY, loc);
    node->try_stmt.try_block = try_block;
    node->try_stmt.catch_block = catch_block;
    node->try_stmt.finally_block = finally_block;
    if (catch_param) node->try_stmt.catch_param = strdup(catch_param);
    return node;
}

node_t *node_create_label(const char *name, node_t *statement, location_t loc) {
    node_t *node = node_new(NODE_LABEL, loc);
    node->label.statement = statement;
    if (name) node->label.name = strdup(name);
    return node;
}

/* Blocks and programs */
node_t *node_create_block(node_t **statements, size_t count, location_t loc) {
    node_t *node = node_new(NODE_BLOCK, loc);
    node->block.statements = statements;
    node->block.count = count;
    return node;
}

node_t *node_create_program(node_t **statements, size_t count, location_t loc) {
    node_t *node = node_new(NODE_PROGRAM, loc);
    node->program.statements = statements;
    node->program.count = count;
    return node;
}

/* Functions and calls */
node_t *node_create_function(const char *name, bool is_async, size_t param_count, location_t loc) {
    node_t *node = node_new(NODE_FUNCTION, loc);
    node->function.name = name ? strdup(name) : NULL;
    node->function.is_async = is_async;
    node->function.param_count = param_count;
    node->function.params = NULL; // allocate separately if needed
    node->function.body = NULL;
    return node;
}

node_t *node_create_call(node_t *callee, node_t **args, size_t arg_count, location_t loc) {
    node_t *node = node_new(NODE_CALL, loc);
    node->call.callee = callee;
    node->call.args = args;
    node->call.arg_count = arg_count;
    return node;
}

node_t *node_create_index(node_t *array, node_t *index, location_t loc) {
    node_t *node = node_new(NODE_INDEX, loc);
    node->index.array = array;
    node->index.index = index;
    return node;
}

node_t *node_create_member(node_t *object, node_t *property, location_t loc) {
    node_t *node = node_new(NODE_MEMBER, loc);
    node->member.object = object;
    node->member.property = property;
    return node;
}

/* Arrays / Objects / Spread */
node_t *node_create_array(node_t **elements, size_t count, location_t loc) {
    node_t *node = node_new(NODE_ARRAY, loc);
    node->array.elements = elements;
    node->array.count = count;
    return node;
}

node_t *node_create_object(char **keys, node_t **values, size_t count, location_t loc) {
    node_t *node = node_new(NODE_OBJECT, loc);
    node->object.keys = keys;
    node->object.values = values;
    node->object.count = count;
    return node;
}

node_t *node_create_spread(node_t *argument, location_t loc) {
    node_t *node = node_new(NODE_SPREAD, loc);
    node->spread.argument = argument;
    return node;
}

/* Imports / Exports / Declarations */
node_t *node_create_import(const char *module, const char *default_name, size_t named_count, char **imported, location_t loc) {
    node_t *node = node_new(NODE_IMPORT, loc);
    node->import_stmt.module = strdup(module);
    node->import_stmt.default_name = default_name ? strdup(default_name) : NULL;
    node->import_stmt.named_count = named_count;
    node->import_stmt.imported = imported;
    return node;
}

node_t *node_create_export(node_t *declaration, bool is_default, size_t named_count, char **exported, location_t loc) {
    node_t *node = node_new(NODE_EXPORT, loc);
    node->export_stmt.declaration = declaration;
    node->export_stmt.is_default = is_default;
    node->export_stmt.named_count = named_count;
    node->export_stmt.exported = exported;
    return node;
}

node_t *node_create_declaration(token_t kind, node_t **names, node_t **values, size_t count, location_t loc) {
    node_t *node = node_new(NODE_DECLARATION, loc);
    node->declaration.kind = kind;
    node->declaration.names = names;
    node->declaration.values = values;
    node->declaration.count = count;
    return node;
}

/* Await */
node_t *node_create_await(node_t *argument, location_t loc) {
    node_t *node = node_new(NODE_AWAIT, loc);
    node->await_expr.argument = argument;
    return node;
}

/* Empty node */
node_t *node_create_empty(location_t loc) {
    return node_new(NODE_EMPTY, loc);
}

const char *node_type_to_string(node_type_t type)
{
    static_assert(NODE_COUNT == 37, "Fix NODE_COUNT in 'node_type_to_string'");

    switch (type)
    {
        case NODE_PROGRAM: return "PROGRAM";
        case NODE_BLOCK: return "BLOCK";
        case NODE_DECLARATION: return "DECLARATION";
        case NODE_SWITCH: return "SWITCH";
        case NODE_LABEL: return "LABEL";
        case NODE_AWAIT: return "AWAIT";
        case NODE_BINARY: return "BINARY";
        case NODE_UNARY: return "UNARY";
        case NODE_ASSIGNMENT: return "ASSIGNMENT";
        case NODE_TERNARY: return "TERNARY";
        case NODE_IF: return "IF";
        case NODE_WHILE: return "WHILE";
        case NODE_DO_WHILE: return "DO_WHILE";
        case NODE_FOR: return "FOR";
        case NODE_CALL: return "CALL";
        case NODE_POSTFIX: return "POSTFIX";
        case NODE_INDEX: return "INDEX";
        case NODE_MEMBER: return "MEMBER";
        case NODE_FUNCTION: return "FUNCTION";
        case NODE_ARRAY: return "ARRAY";
        case NODE_OBJECT: return "OBJECT";
        case NODE_SPREAD: return "SPREAD";
        case NODE_BREAK: return "BREAK";
        case NODE_CONTINUE: return "CONTINUE";
        case NODE_THROW: return "THROW";
        case NODE_TRY: return "TRY";
        case NODE_RETURN: return "RETURN";
        case NODE_IMPORT: return "IMPORT";
        case NODE_EXPORT: return "EXPORT";
        case NODE_UNDEFINED: return "UNDEFINED";
        case NODE_NULL: return "NULL";
        case NODE_NUMBER: return "NUMBER";
        case NODE_STRING: return "STRING";
        case NODE_BOOL: return "BOOL";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_THIS: return "THIS";
        case NODE_EMPTY: return "EMPTY";
        default: return "UNKNOWN";
    }
}

