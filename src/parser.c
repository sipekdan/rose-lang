
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "utils.h"
#include "parser.h"

void parser_init(parser_t *parser, lexer_t *lexer)
{
    parser->lexer = lexer;
    parser->current = lexer_next(lexer);
    parser->previous = NULL;
}

void parser_free(parser_t *parser)
{
    (void) parser;
    free(parser);
}

static void parser_advance(parser_t *parser)
{
    parser->previous = parser->current;
    parser->current = lexer_next(parser->lexer);
}

static token_t *parser_peek(parser_t *parser)
{
    return &parser->lexer->tokens[parser->lexer->index];
}

static bool parser_check(parser_t *parser, token_type_t type)
{
    return parser->current->type == type;
}

static bool parser_match(parser_t *parser, token_type_t type)
{
    if (parser_check(parser, type))
    {
        parser_advance(parser);
        return true;
    }
    return false;
}

/* declaration of all functions during parsing */
static node_t *parse_statement(parser_t *parser);
static node_t *parse_block(parser_t *parser);
static node_t *parse_label(parser_t *parser);
static node_t *parse_if(parser_t *parser);
static node_t *parse_switch(parser_t *parser);
static node_t *parse_while(parser_t *parser);
static node_t *parse_do_while(parser_t *parser);
static node_t *parse_for(parser_t *parser);
static node_t *parse_function(parser_t *parser);
static node_t *parse_declaration(parser_t *parser);
static node_t *parse_import(parser_t *parser);
static node_t *parse_export(parser_t *parser);
static node_t *parse_return(parser_t *parser);
static node_t *parse_break(parser_t *parser);
static node_t *parse_continue(parser_t *parser);
static node_t *parse_throw(parser_t *parser);
static node_t *parse_try(parser_t *parser);

static node_t *parse_comma(parser_t *parser);
static node_t *parse_expression(parser_t *parser);
static node_t *parse_assignment(parser_t *parser);      // =, +=, -=, ...
static node_t *parse_ternary(parser_t *parser);         // ?:
static node_t *parse_logical_or(parser_t *parser);      // ||
static node_t *parse_logical_and(parser_t *parser);     // &&
static node_t *parse_bitwise_or(parser_t *parser);      // |
static node_t *parse_bitwise_xor(parser_t *parser);     // ^
static node_t *parse_bitwise_and(parser_t *parser);     // &
static node_t *parse_equality(parser_t *parser);        // ==, !=
static node_t *parse_relational(parser_t *parser);      // >, >=, <, <=
static node_t *parse_shift(parser_t *parser);           // <<, >>
static node_t *parse_term(parser_t *parser);            // +, -
static node_t *parse_factor(parser_t *parser);          // *, /
static node_t *parse_exponent(parser_t *parser);        // **
static node_t *parse_unary(parser_t *parser);           // await, typeof, +, -, !, ~, ++, --
static node_t *parse_postfix(parser_t *parser);         // ++, --, member, index
static node_t *parse_primary(parser_t *parser);

static node_t *parse_spread(parser_t *parser);
static node_t *parse_number(parser_t *parser);
static node_t *parse_string(parser_t *parser);
static node_t *parse_bool(parser_t *parser);
static node_t *parse_null(parser_t *parser);
static node_t *parse_undefined(parser_t *parser);
static node_t *parse_identifier(parser_t *parser);
static node_t *parse_array(parser_t *parser);
static node_t *parse_object(parser_t *parser);

node_t *parse_program(parser_t *parser)
{
    node_t *program = calloc(1, sizeof(node_t));
    if (!program) ERROR("Malloc failed!\n");

    program->type = NODE_PROGRAM;
    program->program.count = 0;

    size_t capacity = 4;
    program->program.statements = malloc(sizeof(node_t*) * capacity);

    while (parser->current->type != TOKEN_EOF)
    {
        node_t *stmt = parse_statement(parser);
        if (!stmt) {
            for (size_t i = 0; i < program->program.count; i++) {
                node_free(program->program.statements[i]);
            }
            free(program->program.statements);
            free(program);
            return NULL;
        }

        if (program->program.count >= capacity)
        {
            capacity *= 2;
            program->program.statements =
                realloc(program->program.statements, sizeof(node_t*) * capacity);
        }

        program->program.statements[program->program.count++] = stmt;
    }

    return program;
}

static node_t *parse_statement(parser_t *parser)
{
    if (parser_match(parser, TOKEN_SEMICOLON))
    {
        /* empty statement */
        node_t *empty = calloc(1, sizeof(node_t));
        if (!empty) ERROR("Calloc failed!\n");

        empty->type = NODE_EMPTY;

        /* location of semicolon */
        empty->loc = parser->previous->loc;
        return empty;
    }
    
    /* label check */
    if (parser->current->type == TOKEN_IDENTIFIER && parser_peek(parser)->type == TOKEN_COLON)
        return parse_label(parser);

    if (parser->current->type == TOKEN_LEFT_BRACE)
        return parse_block(parser);
    if (parser->current->type == TOKEN_WHILE)
        return parse_while(parser);
    if (parser->current->type == TOKEN_DO)
        return parse_do_while(parser);
    if (parser->current->type == TOKEN_FOR)
        return parse_for(parser);
    if (parser->current->type == TOKEN_SWITCH)
        return parse_switch(parser);
    if (parser->current->type == TOKEN_IF)
        return parse_if(parser);
    if (parser->current->type == TOKEN_ASYNC || parser->current->type == TOKEN_FUNCTION)
        return parse_function(parser);
    if (parser->current->type == TOKEN_LET || parser->current->type == TOKEN_CONST || parser->current->type == TOKEN_VAR)
        return parse_declaration(parser);
    if (parser->current->type == TOKEN_IMPORT)
        return parse_import(parser);
    if (parser->current->type == TOKEN_EXPORT)
        return parse_export(parser);
    if (parser->current->type == TOKEN_TRY)
        return parse_try(parser);
    if (parser->current->type == TOKEN_RETURN)
        return parse_return(parser);
    if (parser->current->type == TOKEN_BREAK)
        return parse_break(parser);
    if (parser->current->type == TOKEN_CONTINUE)
        return parse_continue(parser);
    if (parser->current->type == TOKEN_THROW)
        return parse_throw(parser);

    /* fallback: expression statement */
    node_t *expr = parse_comma(parser);
    if (!expr) return NULL; /* bubble up error */

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected ';' after expression, got '%s' \n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        /* free expr to avoid leak */
        node_free(expr);
        return NULL;  // bubble up error
    }

    return expr;
}

static node_t *parse_block(parser_t *parser)
{
    node_t *block = calloc(1, sizeof(node_t));
    if (!block) ERROR("Calloc failed!\n");

    block->type = NODE_BLOCK;
    block->block.count = 0;
    
    size_t capacity = 4;
    block->block.statements = malloc(sizeof(node_t*) * capacity);
    if (!block->block.statements)
    {
        free(block);
        ERROR("Malloc failed!\n");
    }

    /* block starting with '{' */
    if (parser_match(parser, TOKEN_LEFT_BRACE))
    {
        while (parser->current->type != TOKEN_RIGHT_BRACE &&
               parser->current->type != TOKEN_EOF)
        {
            node_t *stmt = parse_statement(parser);
            if (!stmt) {
                // free any already parsed statements
                for (size_t i = 0; i < block->block.count; ++i)
                    node_free(block->block.statements[i]);
                free(block->block.statements);
                free(block);

                return NULL;  // bubble error up
            }

            if (block->block.count >= capacity)
            {
                capacity *= 2;
				block->block.statements =
                    realloc(block->block.statements, sizeof(node_t*) * capacity);
            }

            block->block.statements[block->block.count++] = stmt;
        }
        
        if (!parser_match(parser, TOKEN_RIGHT_BRACE))
        {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected '}' to end block, got '%s'\n",
                LOCATION(parser->current->loc),
                parser->current->value
            );

            /* free the block and its statements */
            for (size_t i = 0; i < block->block.count; ++i)
                node_free(block->block.statements[i]);
            free(block->block.statements);
            free(block);

            return NULL;
        }
    }
    /* single statement block */
    else
    {
        node_t *stmt = parse_statement(parser);
        if (stmt == NULL)
        {
            /* free the block and its statements */
            free(block->block.statements);
            free(block);

            return NULL;
        }

        block->block.statements[block->block.count++] = stmt;
    }

    /* TODO: potetional optimazation: flatten single-statement nested blocks */
    return block;
}

static node_t *parse_label(parser_t *parser)
{
    /* label checking */
    if (parser->current->type != TOKEN_IDENTIFIER ||
        parser_peek(parser)->type != TOKEN_COLON)
    {
        UNREACHABLE;
    }

    char *label_name = (char*)parser->current->value;

    parser_advance(parser); /* consume identifier */
    parser_advance(parser); /* consume colon */

    node_t *label_node = calloc(1, sizeof(node_t));
    if (!label_node) ERROR("Calloc failed!\n");

    label_node->type = NODE_LABEL;
    label_node->label.name = strdup(label_name);
    if (!label_node->label.name) ERROR("strdup failed!\n");

    label_node->label.statement = parse_statement(parser);
    if (!label_node->label.statement) {
        /* free resources and bubble up error */
        free(label_node->label.name);
        free(label_node);
        return NULL;
    }

    return label_node;
}

static node_t *parse_if(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_IF))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected 'if', got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        return NULL;
    }

    if (!parser_match(parser, TOKEN_LEFT_PAREN))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected '(' after 'if', got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        return NULL;
    }

    node_t *condition = parse_comma(parser);
    if (!condition) return NULL;

    if (!parser_match(parser, TOKEN_RIGHT_PAREN))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected ')' after 'if' condition, got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(condition);
        return NULL;
    }

    node_t *then_branch = parse_block(parser);
    if (!then_branch) {
        node_free(condition);
        return NULL;
    }

    node_t *else_branch = NULL;
    if (parser_match(parser, TOKEN_ELSE))
    {
        if (parser->current->type == TOKEN_IF)
        {
            else_branch = parse_if(parser);
        }
        else
        {
            else_branch = parse_block(parser);
        }

        if (!else_branch) {
            node_free(condition);
            node_free(then_branch);
            return NULL;
        }
    }

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_IF;
    node->if_stmt.condition = condition;
	node->if_stmt.then_branch = then_branch;
	node->if_stmt.else_branch = else_branch;

    return node;
}

static node_t *parse_switch(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_SWITCH)) {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected 'switch', got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        return NULL;
    }

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_SWITCH;
    node->switch_stmt.expr = NULL;
    node->switch_stmt.cases = NULL;
    node->switch_stmt.cases_count = 0;

    if (!parser_match(parser, TOKEN_LEFT_PAREN)) {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected '(' after 'switch', got '%s'\n",
            LOCATION(parser->lexer->loc),
            parser->current->value
        );
        node_free(node);
        return NULL;
    }

    node->switch_stmt.expr = parse_expression(parser);
    if (!node->switch_stmt.expr) {
        node_free(node);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected ')' after switch expression got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(node);
        return NULL;
    }

    // Expect '{'
    if (!parser_match(parser, TOKEN_LEFT_BRACE)) {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected '{' after switch expression, got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(node);
        return NULL;
    }

    // Parse cases
    while (parser->current->type != TOKEN_RIGHT_BRACE && parser->current->type != TOKEN_EOF) {
        size_t case_index = node->switch_stmt.cases_count++;

        node->switch_stmt.cases = realloc(
            node->switch_stmt.cases,
            sizeof(*node->switch_stmt.cases) * node->switch_stmt.cases_count
        );
        if (!node->switch_stmt.cases) ERROR("Realloc failed!\n");

        node->switch_stmt.cases[case_index].labels = NULL;
        node->switch_stmt.cases[case_index].labels_count = 0;
        node->switch_stmt.cases[case_index].body = NULL;
        node->switch_stmt.cases[case_index].is_default = false;

        // --- CASE ---
        if (parser_match(parser, TOKEN_CASE)) {
            size_t labels_capacity = 2;
            node->switch_stmt.cases[case_index].labels =
                malloc(sizeof(node_t *) * labels_capacity);
            if (!node->switch_stmt.cases[case_index].labels) ERROR("Malloc failed!\n");

            do {
                if (node->switch_stmt.cases[case_index].labels_count >= labels_capacity) {
                    labels_capacity *= 2;
                    node->switch_stmt.cases[case_index].labels = realloc(
                        node->switch_stmt.cases[case_index].labels,
                        sizeof(node_t *) * labels_capacity
                    );
                    if (!node->switch_stmt.cases[case_index].labels) ERROR("Realloc failed!\n");
                }

                node_t *label_expr = parse_comma(parser);
                if (!label_expr) {
                    node_free(node);
                    return NULL;
                }

                node->switch_stmt.cases[case_index]
                    .labels[node->switch_stmt.cases[case_index].labels_count++] = label_expr;

                if (!parser_match(parser, TOKEN_COLON)) {
                    PARSER_ERROR(parser,
                        "[ERROR] [%s:%zu:%zu] Expected ':' after case expression, got '%s'\n",
                        LOCATION(parser->current->loc),
                        parser->current->value
                    );
                    node_free(node);
                    return NULL;
                }
            } while (parser_match(parser, TOKEN_CASE));
        }
        // --- DEFAULT ---
        else if (parser_match(parser, TOKEN_DEFAULT)) {
            // node->switch_stmt.cases[case_index].labels = NULL;
            // node->switch_stmt.cases[case_index].labels_count = 0;
            node->switch_stmt.cases[case_index].is_default = true;

            if (!parser_match(parser, TOKEN_COLON)) {
                PARSER_ERROR(parser,
                    "[ERROR] [%s:%zu:%zu] Expected ':' after 'default' got '%s'\n",
                    LOCATION(parser->current->loc),
                    parser->current->value
                );
                node_free(node);
                return NULL;
            }
        }
        else {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected 'case' or 'default' got '%s'\n",
                LOCATION(parser->current->loc),
                parser->current->value
            );
            node_free(node);
            return NULL;
        }

        // Case body (block node)
        node_t *body_block = calloc(1, sizeof(node_t));
        if (!body_block) ERROR("Malloc failed!\n");

        body_block->type = NODE_BLOCK;
        body_block->block.count = 0;
        body_block->block.statements = NULL;

        node->switch_stmt.cases[case_index].body = body_block;

        // Parse until next case/default/}
        while (parser->current->type != TOKEN_CASE &&
               parser->current->type != TOKEN_DEFAULT &&
               parser->current->type != TOKEN_RIGHT_BRACE &&
               parser->current->type != TOKEN_EOF)
        {

            node_t *stmt = parse_statement(parser);
            if (!stmt) {
                node_free(node);
                return NULL;
            }

            size_t old_count = body_block->block.count;
            body_block->block.statements = realloc(
                body_block->block.statements,
                sizeof(node_t *) * (old_count + 1)
            );
            if (!body_block->block.statements) ERROR("Realloc failed!\n");

            body_block->block.statements[old_count] = stmt;
            body_block->block.count++;
        }
    }

    if (!parser_match(parser, TOKEN_RIGHT_BRACE)) {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected '}' to close switch, got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(node);
        return NULL;
    }

    return node;
}

static node_t *parse_while(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_WHILE))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected 'while' got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        return NULL;
    }

    if (!parser_match(parser, TOKEN_LEFT_PAREN))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected '(' after 'while' got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        return NULL;
    }

    node_t *condition = parse_comma(parser);
    if (!condition) return NULL; // bubble up parsing error

    if (!parser_match(parser, TOKEN_RIGHT_PAREN))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected ')' after condition got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(condition);
        return NULL;
    }

    node_t *body = parse_block(parser);
    if (!body) {
        node_free(condition);
        return NULL;
    }

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_WHILE;
    node->while_stmt.condition = condition;
	node->while_stmt.body = body;

    return node; 
}

static node_t *parse_do_while(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_DO))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected 'do' got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        return NULL;
    }

    // parse body first
    node_t *body = parse_block(parser);
    if (!body) return NULL; // bubble up block parse error

    // expect 'while' after body
    if (!parser_match(parser, TOKEN_WHILE))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected 'while' after 'do' block got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(body);
        return NULL;
    }

    // Expect '(' after 'while'
    if (!parser_match(parser, TOKEN_LEFT_PAREN))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected '(' after 'while' got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(body);
        return NULL;
    }

    node_t *condition = parse_comma(parser);
    if (!condition) {
        node_free(body);
        return NULL;  // bubble up parse error
    }

    if (!parser_match(parser, TOKEN_RIGHT_PAREN))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected ')' after condition got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(body);
        node_free(condition);
        return NULL;
    }

    // Optional semicolon after do-while
    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected ';' after 'do-while' got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(body);
        node_free(condition);
        return NULL;
    }

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_DO_WHILE;
    node->while_stmt.body = body;
    node->while_stmt.condition = condition;

    return node;
}


static node_t *parse_for(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_FOR)) {
        PARSER_ERROR(parser, "[ERROR] Expected 'for'\n");
        return NULL;
    }

    // location_t node_location = parser->previous.location;

    if (!parser_match(parser, TOKEN_LEFT_PAREN)) {
        PARSER_ERROR(parser, "[ERROR] Expected '(' after 'for'\n");
        return NULL;
    }

    node_t *for_node = malloc(sizeof(node_t));
    if (!for_node) ERROR("Malloc failed!\n");

    for_node->type = NODE_FOR;
    // for_node->location = node_location;
    for_node->for_stmt.body = NULL;
    for_node->for_stmt.init = NULL;
    for_node->for_stmt.condition = NULL;
    for_node->for_stmt.increment = NULL;

    // parse initializer
    if (parser->current->type == TOKEN_LET ||
        parser->current->type == TOKEN_CONST ||
        parser->current->type == TOKEN_VAR)
    {
        for_node->for_stmt.init = parse_declaration(parser);
        // semicolon already consumed by `parse_var_declaration` for variable declarations
        if (!for_node->for_stmt.init) {
            free(for_node);
            return NULL;  // bubble up parse error
        }
    }
    else if (parser->current->type != TOKEN_SEMICOLON)
    {
        for_node->for_stmt.init = parse_comma(parser);
        if (!parser_match(parser, TOKEN_SEMICOLON)) {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected ';' after for-loop initializer, got '%s'\n",
                LOCATION(parser->current->loc),
                parser->current->value
            );
            node_free(for_node);
            return NULL;
        }
    }
    else
    {
        // empty initializer
        parser_advance(parser); // consume the semicolon
    }

    // parse condition
    if (parser->current->type != TOKEN_SEMICOLON) {
        for_node->for_stmt.condition = parse_comma(parser);
        if (!for_node->for_stmt.condition) {
            node_free(for_node->for_stmt.init);
            free(for_node);
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected ';' after for-loop condition, got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(for_node->for_stmt.init);
        node_free(for_node->for_stmt.condition);
        free(for_node);
        return NULL;
    }

    // --- Parse increment ---
    if (parser->current->type != TOKEN_RIGHT_PAREN) {
        for_node->for_stmt.increment = parse_comma(parser);
        if (!for_node->for_stmt.increment) {
            node_free(for_node->for_stmt.init);
            node_free(for_node->for_stmt.condition);
            free(for_node);
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected ')' after for-loop, got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(for_node->for_stmt.init);
        node_free(for_node->for_stmt.condition);
        node_free(for_node->for_stmt.increment);
        free(for_node);
        return NULL;
    }

    // parse body
    for_node->for_stmt.body = parse_block(parser);
    if (!for_node->for_stmt.body) {
        node_free(for_node->for_stmt.init);
        node_free(for_node->for_stmt.condition);
        node_free(for_node->for_stmt.increment);
        free(for_node);
        return NULL; // bubble up parse error
    }

    return for_node;
}

static node_t *parse_function(parser_t *parser)
{
    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Calloc failed!\n");

    node->type = NODE_FUNCTION;
    node->function.is_async = false;
    // node->location = parser->current.location;
    node->function.param_count = 0;
    node->function.params = NULL;
    node->function.name = NULL;
    node->function.body = NULL;

    /* Check for 'async' keyword */
    if (parser->current->type == TOKEN_ASYNC)
    {
        node->function.is_async = true;
        parser_advance(parser);
    }

    /* Expect 'function' keyword */ 
    if (!parser_match(parser, TOKEN_FUNCTION)) {
        PARSER_ERROR(parser, "[ERROR] [%s:%zu:%zu] Expected 'function', got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(node);
        return NULL;
    }

    /* Check for generator '*' */
    // if (parser_match(parser, TOKEN_MULTIPLY)) {
    //     node->function.is_generator = true;
    // }

    /* function name (optional for anonymous functions) */
    if (parser->current->type == TOKEN_IDENTIFIER) {
        node->function.name = strdup(parser->current->value);
        parser_advance(parser);
    }

    // Expect '('
    if (!parser_match(parser, TOKEN_LEFT_PAREN)) {
        PARSER_ERROR(parser, "[ERROR] [%s:%zu:%zu] Expected '(' after function name, got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(node);
        return NULL;
    }

    /* parse parameters */
    if (parser->current->type != TOKEN_RIGHT_PAREN) {
        do {
            bool is_rest = false;

            if (parser_match(parser, TOKEN_ELLIPSIS)) {
                is_rest = true;
            }

            if (parser->current->type != TOKEN_IDENTIFIER) {
                PARSER_ERROR(parser,
                    "[ERROR] [%s:%zu:%zu] Expected parameter name, got '%s'\n",
                    LOCATION(parser->current->loc),
                    parser->current->value
                );
                node_free(node);
                return NULL;
            }

            void *tmp = realloc(
                node->function.params,
                sizeof(*node->function.params) * (node->function.param_count + 1)
            );
            if (!tmp) ERROR("Malloc failed!\n");
            node->function.params = tmp;

            node->function.params[node->function.param_count].name = strdup(parser->current->value);
            node->function.params[node->function.param_count].default_value = NULL;
            node->function.params[node->function.param_count].is_rest = is_rest;

            /* consume identifier */
            parser_advance(parser);
            
            /* optional default value */
            if (!is_rest && parser_match(parser, TOKEN_EQUAL)) {
                node_t *default_expr = parse_expression(parser);
                if (!default_expr) {
                    node_free(node);
                    return NULL;
                }
                node->function.params[node->function.param_count].default_value = default_expr;
            } else if (is_rest && parser_match(parser, TOKEN_EQUAL)) {
                PARSER_ERROR(parser,
                    "[ERROR] [%s:%zu:%zu] Rest parameter cannot have a default value.\n",
                    LOCATION(parser->current->loc)
                );
                node_free(node);
                return NULL;
            }

            node->function.param_count++;
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        PARSER_ERROR(parser, "[ERROR] [%s:%zu:%zu] Expected ')' after parameters name, got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        node_free(node);
        return NULL;
    }

    node->function.body = parse_block(parser);
    if (!node->function.body) {
        node_free(node);
        return NULL;
    }

    return node;
}

static node_t *parse_declaration(parser_t *parser)
{
    token_t kind = *parser->current;

    if (!parser_match(parser, TOKEN_LET) &&
        !parser_match(parser, TOKEN_CONST) &&
        !parser_match(parser, TOKEN_VAR))
    {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected 'let', 'const' or 'var', got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        return NULL;
    }

    // location_t location = kind.location;
    node_t *node = malloc(sizeof(node_t));
    if (!node) ERROR("Malloc failed!");

    node->type = NODE_DECLARATION;
    node->declaration.kind = kind;
    node->declaration.count = 0;

    size_t capacity = 2;
    node->declaration.names =
        malloc(sizeof(node_t *) * capacity);
    node->declaration.values =
        malloc(sizeof(node_t *) * capacity);

    if (!node->declaration.names || !node->declaration.values) {
        free(node->declaration.names);
        free(node->declaration.values);
        free(node);
        ERROR("Malloc failed!\n");
    }
    
    while (true)
    {
        node_t *name = parse_identifier(parser);
        if (!name) {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected identifier after '%s', got: '%s'\n",
                LOCATION(parser->current->loc),
                kind.value,
                parser->current->value
            );
            node_free(node); // free previously allocated names and values
            return NULL;
        }

        if (node->declaration.count >= capacity) {
            capacity *= 2;
            void *tmp_names = realloc(node->declaration.names, sizeof(node_t *) * capacity);
            void *tmp_values = realloc(node->declaration.values, sizeof(node_t *) * capacity);
            if (!tmp_names || !tmp_values) {
                free(tmp_names);
                free(tmp_values);
                node_free(node);
                ERROR("Realloc failed!\n");
            }
            node->declaration.names = tmp_names;
            node->declaration.values = tmp_values;
        }

        node->declaration.names[node->declaration.count] = name;

        /* optional assignment */
        if (parser_match(parser, TOKEN_EQUAL)) {
            node_t *value_expr = parse_expression(parser);
            if (!value_expr) {
                node_free(node);
                return NULL;
            }
            node->declaration.values[node->declaration.count] = value_expr;
        } else {
            node->declaration.values[node->declaration.count] = NULL;
        }

        node->declaration.count++;

        if (!parser_match(parser, TOKEN_COMMA)) {
            if (!parser_match(parser, TOKEN_SEMICOLON))
            {
                PARSER_ERROR(parser, "[ERROR] [%s:%zu:%zu] Unexpected token after variable declaration: '%s', expected ';' or ','\n",
                    LOCATION(parser->current->loc),
                    parser->current->value
                );
                node_free(node);
                return NULL;
            }
            break;
        }
    }
    // node->location = location;
    return node;
}

static node_t *parse_import(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_IMPORT)) {
        PARSER_ERROR(parser, "[ERROR] [%s:%zu:%zu] Expected 'import', got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        return NULL;
    }

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");
    node->type = NODE_IMPORT;

    node->import_stmt.default_name = NULL;
    node->import_stmt.named_count = 0;
    node->import_stmt.imported = NULL;
    node->import_stmt.module = NULL;

    // side-effect only import: import "module"
    if (parser->current->type == TOKEN_STRING_LITERAL)
    {
        node->import_stmt.module = strdup(parser->current->value);
        if (!node->import_stmt.module) ERROR("Strdup failed!\n");

        parser_advance(parser); // consume the string literal
        parser_match(parser, TOKEN_SEMICOLON); // optional semicolon
        return node;
    }
    
    /* default import or nothing */
    if (parser->current->type == TOKEN_IDENTIFIER)
    {
        node->import_stmt.default_name = strdup(parser->current->value);
        if (!node->import_stmt.default_name) ERROR("Strdup failed!\n");

        parser_advance(parser); /* consume identifier */

        // check if combined named imports follow
        if (parser_match(parser, TOKEN_COMMA)) {
            if (parser->current->type != TOKEN_LEFT_BRACE) {
                PARSER_ERROR(parser,
                    "[ERROR] Expected '{' after ',' in import statement\n"
                );
                free(node->import_stmt.default_name);
                free(node);
                return NULL;
            }
        }
    }

    /* named imports */
    if (parser_match(parser, TOKEN_LEFT_BRACE)) {
        size_t cap = 4, count = 0;
        node->import_stmt.imported = malloc(sizeof(char*) * cap);
        if (!node->import_stmt.imported) {
            free(node->import_stmt.default_name);
            free(node);
            ERROR("Malloc failed!\n");
        }

        while (parser->current->type != TOKEN_RIGHT_BRACE &&
               parser->current->type != TOKEN_EOF)
        {
            if (parser->current->type != TOKEN_IDENTIFIER) {
                PARSER_ERROR(parser,
                    "[ERROR] Expected identifier in named imports, got: '%s'\n",
                    parser->current->value
                );
                for (size_t i = 0; i < count; ++i) free(node->import_stmt.imported[i]);
                free(node->import_stmt.imported);
                free(node->import_stmt.default_name);
                free(node);
                return NULL;
            }

            char *name = strdup(parser->current->value);
            if (!name) {
                for (size_t i = 0; i < count; ++i) free(node->import_stmt.imported[i]);
                free(node->import_stmt.imported);
                free(node->import_stmt.default_name);
                free(node);
                ERROR("Strdup failed!\n");
            }
            node->import_stmt.imported[count++] = name;
            parser_advance(parser);

            parser_match(parser, TOKEN_COMMA); // skip comma if present

            if (count >= cap) {
                cap *= 2;
                void *tmp = realloc(node->import_stmt.imported, sizeof(char*) * cap);
                if (!tmp) {
                    for (size_t i = 0; i < count; ++i) free(node->import_stmt.imported[i]);
                    free(node->import_stmt.imported);
                    free(node->import_stmt.default_name);
                    free(node);
                    ERROR("Realloc failed!\n");
                }
                node->import_stmt.imported = tmp;
            }
        }
        if (!parser_match(parser, TOKEN_RIGHT_BRACE)) {
            PARSER_ERROR(parser,
                "[ERROR] Expected '}' after named imports\n"
            );
            for (size_t i = 0; i < count; ++i) free(node->import_stmt.imported[i]);
            free(node->import_stmt.imported);
            free(node->import_stmt.default_name);
            free(node);
            return NULL;
        }

        node->import_stmt.named_count = count;
    }

    /* 'from' keyword */
    if (!parser_match(parser, TOKEN_FROM)) {
        PARSER_ERROR(parser,
            "[ERROR] [%s:%zu:%zu] Expected 'from' keyword in import statement, got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        for (size_t i = 0; i < node->import_stmt.named_count; ++i) free(node->import_stmt.imported[i]);
        free(node->import_stmt.imported);
        free(node->import_stmt.default_name);
        free(node);
        return NULL;
    }

    if (parser->current->type != TOKEN_STRING_LITERAL) {
        PARSER_ERROR(parser,
            "[ERROR] Expected module after 'from', got '%s'\n",
            parser->current->value
        );
        for (size_t i = 0; i < node->import_stmt.named_count; ++i) free(node->import_stmt.imported[i]);
        free(node->import_stmt.imported);
        free(node->import_stmt.default_name);
        free(node);
        return NULL;
    }

    // node->location = parser->current.location;
    node->import_stmt.module = strdup(parser->current->value);
    if (!node->import_stmt.module) {
        for (size_t i = 0; i < node->import_stmt.named_count; ++i) free(node->import_stmt.imported[i]);
        free(node->import_stmt.imported);
        free(node->import_stmt.default_name);
        free(node);
        ERROR("Strdup failed!\n");
    }

    parser_advance(parser); // consume the string literal
    parser_match(parser, TOKEN_SEMICOLON); // consume optional semicolon

    return node;
}

static node_t *parse_export(parser_t *parser)
{
    parser_advance(parser); // consume 'export'

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Calloc failed!\n");

    node->type = NODE_EXPORT;
    node->export_stmt.is_default = false;
    node->export_stmt.declaration = NULL;
    node->export_stmt.named_count = 0;
    node->export_stmt.exported = NULL;

    // node->location = parser->current.location;

    // export default
    if (parser_match(parser, TOKEN_DEFAULT))
    {
        node->export_stmt.is_default = true;
        node->export_stmt.declaration = parse_statement(parser);

        if (!node->export_stmt.declaration) {
            PARSER_ERROR(parser, "[ERROR] Expected declaration after 'export default'\n");
            free(node);
            return NULL;
        }
    }
    // named exports: export { a, b, c };
    else if (parser_match(parser, TOKEN_LEFT_BRACE))
    {
        size_t capacity = 4;
        node->export_stmt.exported = malloc(sizeof(char*) * capacity);
        if (!node->export_stmt.exported) ERROR("Malloc failed!\n");
        
        while (parser->current->type != TOKEN_RIGHT_BRACE &&
               parser->current->type != TOKEN_EOF)
        {
            if (!parser_check(parser, TOKEN_IDENTIFIER)) {
                PARSER_ERROR(parser, "[ERROR] Expected identifier in export list\n");
                for (size_t i = 0; i < node->export_stmt.named_count; ++i)
                    free(node->export_stmt.exported[i]);
                free(node->export_stmt.exported);
                free(node);
                return NULL;
            }

            char *name = strdup(parser->current->value);
            if (!name) {
                for (size_t i = 0; i < node->export_stmt.named_count; ++i)
                    free(node->export_stmt.exported[i]);
                free(node->export_stmt.exported);
                free(node);
                ERROR("Strdup failed!\n");
            }

            if (node->export_stmt.named_count >= capacity) {
                capacity *= 2;
                void *tmp = realloc(node->export_stmt.exported, sizeof(char*) * capacity);
                if (!tmp) {
                    for (size_t i = 0; i < node->export_stmt.named_count; ++i)
                        free(node->export_stmt.exported[i]);
                    free(node->export_stmt.exported);
                    free(node);
                    ERROR("Realloc failed!\n");
                }
                node->export_stmt.exported = tmp;
            }

            node->export_stmt.exported[node->export_stmt.named_count++] = name;
            parser_advance(parser); // consume identifier
            parser_match(parser, TOKEN_COMMA); // optional comma
        }
        printf("%d\n", parser->current->type);
        if (!parser_match(parser, TOKEN_RIGHT_BRACE)) {
            PARSER_ERROR(parser, "[ERROR] Expected '}' after export list\n");
            for (size_t i = 0; i < node->export_stmt.named_count; ++i)
                free(node->export_stmt.exported[i]);
            free(node->export_stmt.exported);
            free(node);
            return NULL;
        }

        if (!parser_match(parser, TOKEN_SEMICOLON)) {
            PARSER_ERROR(parser, "[ERROR] Expected ';' after export list\n");
            for (size_t i = 0; i < node->export_stmt.named_count; ++i)
                free(node->export_stmt.exported[i]);
            free(node->export_stmt.exported);
            free(node);
            return NULL;
        }
    }
    // fallback: single declaration without default
    else {
        node->export_stmt.declaration = parse_statement(parser);
        if (!node->export_stmt.declaration) {
            PARSER_ERROR(parser, "[ERROR] Expected declaration after 'export'\n");
            free(node);
            return NULL;
        }
    }

    return node;
}

static node_t *parse_return(parser_t *parser)
{
    parser_advance(parser); // consume 'return'

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_RETURN;
    node->return_stmt.value = NULL;
    // node->location = parser->current.location;

    /* match the semicolon (if no return value) */
    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        node->return_stmt.value = parse_comma(parser);
        if (!node->return_stmt.value) {
            node_free(node);
            return NULL; // bubble up error
        }
        if (!parser_match(parser, TOKEN_SEMICOLON)) {
            node_free(node);
            PARSER_ERROR(parser, "[ERROR] Expected ';' after return value\n");
            return NULL;
        }
    }

    return node;
}

static node_t *parse_break(parser_t *parser)
{
    parser_advance(parser); // consume 'break'

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Calloc failed!\n");

    node->type = NODE_BREAK;
    node->break_stmt.label = NULL;
    node->loc = parser->current->loc;

    if (parser->current->type == TOKEN_IDENTIFIER) {
        node->break_stmt.label = strdup(parser->current->value);
        if (!node->break_stmt.label) {
            node_free(node);
            ERROR("Strdup failed!\n");
        }
        parser_advance(parser); // consume label
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        node_free(node);
        PARSER_ERROR(parser, "[ERROR] Expected ';' after break\n");
        return NULL;
    }

    return node;
}

static node_t *parse_continue(parser_t *parser)
{
    // consume 'continue'
    if (!parser_match(parser, TOKEN_CONTINUE)) {
        PARSER_ERROR(parser, "[ERROR] [%s:%zu:%zu] Expected 'continue', got '%s'\n",
            LOCATION(parser->current->loc),
            parser->current->value
        );
        return NULL;
    }

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Calloc failed!\n");

    node->type = NODE_CONTINUE;
    node->continue_stmt.label = NULL;
    node->loc = parser->previous->loc;

    if (parser->current->type == TOKEN_IDENTIFIER) {
        node->continue_stmt.label = strdup(parser->current->value);
        if (!node->continue_stmt.label) {
            node_free(node);
            ERROR("Strdup failed!\n");
        }
        parser_advance(parser); // consume label
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        PARSER_ERROR(parser, "[ERROR] Expected ';' after continue\n");
        node_free(node);
        return NULL;
    }

    return node;
}

static node_t *parse_throw(parser_t *parser)
{
    parser_advance(parser); // consume 'throw'

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Calloc failed!\n");

    node->type = NODE_THROW;

    if (parser->current->type == TOKEN_SEMICOLON) {
        PARSER_ERROR(parser, "[ERROR] 'throw' must have an expression\n");
        node_free(node);
        return NULL;
    }

    node->throw_stmt.value = parse_comma(parser);
    if (!node->throw_stmt.value) {
        node_free(node);
        return NULL; // bubble error
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        PARSER_ERROR(parser, "[ERROR] Expected ';' after throw expression\n");
        node_free(node);
        return NULL;
    }

    return node;
}

static node_t *parse_try(parser_t *parser)
{
    parser_advance(parser); // consume 'try'

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Calloc failed!\n");

    node->type = NODE_TRY;
    node->try_stmt.try_block = NULL;
    node->try_stmt.catch_param = NULL;
    node->try_stmt.catch_block = NULL;
    node->try_stmt.finally_block = NULL;

    if (parser->current->type != TOKEN_LEFT_BRACE) {
        PARSER_ERROR(parser, "[ERROR] Expected '{' after 'try'\n");
        node_free(node);
        return NULL;
    }
    node->try_stmt.try_block = parse_block(parser);
    if (!node->try_stmt.try_block) {
        node_free(node);
        return NULL;
    }

    if (parser->current->type == TOKEN_CATCH) {
        parser_advance(parser); // consume 'catch'

        if (parser->current->type == TOKEN_LEFT_PAREN) {
            parser_advance(parser); // consume '('

            if (parser->current->type != TOKEN_IDENTIFIER) {
                PARSER_ERROR(parser, "[ERROR] Expected identifier after 'catch('\n");
                node_free(node);
                return NULL;
            }

            node->try_stmt.catch_param = strdup(parser->current->value);
            if (!node->try_stmt.catch_param) {
                node_free(node);
                ERROR("Strdup failed!\n");
            }

            parser_advance(parser); // consume identifier

            if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
                PARSER_ERROR(parser, "[ERROR] Expected ')' after catch parameter, got: '%s'\n",
                             parser->current->value);
                node_free(node);
                return NULL;
            }
        }

        if (parser->current->type != TOKEN_LEFT_BRACE) {
            PARSER_ERROR(parser, "[ERROR] Expected '{' after 'catch'\n");
            node_free(node);
            return NULL;
        }
        
        node->try_stmt.catch_block = parse_block(parser);
        if (!node->try_stmt.catch_block) {
            node_free(node);
            return NULL;
        }
    }

    if (parser->current->type == TOKEN_FINALLY) {
        parser_advance(parser); // consume 'finally'

        if (parser->current->type != TOKEN_LEFT_BRACE) {
            PARSER_ERROR(parser, "[ERROR] Expected '{' after 'finally'\n");
            node_free(node);
            return NULL;
        }

        node->try_stmt.finally_block = parse_block(parser);
        if (!node->try_stmt.finally_block) {
            node_free(node);
            return NULL;
        }
    }

    if (!node->try_stmt.catch_block && !node->try_stmt.finally_block) {
        PARSER_ERROR(parser, "[ERROR] 'try' must have at least a 'catch' or 'finally'\n");
        node_free(node);
        return NULL;
    }

    return node;
}

static node_t *parse_comma(parser_t *parser)
{
    node_t *left = parse_expression(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_COMMA))
    {
        token_t op = *parser->previous;
        node_t *right = parse_expression(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left; 
}

static node_t *parse_expression(parser_t *parser)
{
    return parse_assignment(parser);
}

static node_t *parse_assignment(parser_t *parser)
{
    node_t *left = parse_ternary(parser);
    if (!left) return NULL;

    if (parser_match(parser, TOKEN_EQUAL) ||
        parser_match(parser, TOKEN_PLUS_EQUAL) ||
        parser_match(parser, TOKEN_MINUS_EQUAL) ||
        parser_match(parser, TOKEN_STAR_EQUAL) ||
        parser_match(parser, TOKEN_SLASH_EQUAL) ||
        parser_match(parser, TOKEN_PERCENT_EQUAL) ||
        parser_match(parser, TOKEN_CARET_EQUAL) ||
        parser_match(parser, TOKEN_PIPE_EQUAL) ||
        parser_match(parser, TOKEN_AMPERSAND_EQUAL) ||
        parser_match(parser, TOKEN_LEFT_SHIFT_EQUAL) ||
        parser_match(parser, TOKEN_RIGHT_SHIFT_EQUAL) ||
        parser_match(parser, TOKEN_STAR_STAR_EQUAL) ||
        parser_match(parser, TOKEN_LOGICAL_AND_EQUAL) ||
        parser_match(parser, TOKEN_LOGICAL_OR_EQUAL))
    {
        token_t op = *parser->previous;

        /* right association */
        node_t *right = parse_assignment(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        // node_t *node = calloc(1, sizeof(node_t));
        // if (!node) ERROR("Malloc failed!\n");

        // node->type = NODE_ASSIGNMENT;
        // node->assignment.op = op;
        // node->assignment.target = left;
        // node->assignment.value = right;

        // return node;

        return node_create_assignment(left, op, right, op.loc);
    }

    return left;
}

static node_t *parse_ternary(parser_t *parser)
{
    node_t *condition = parse_logical_or(parser);
    if (!condition) return NULL;

    if (parser_match(parser, TOKEN_QUESTION))
    {
        node_t *true_expr = parse_expression(parser);
        if (!true_expr)
        {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected expression after '?' in ternary expression, got: '%s'\n",
                LOCATION(parser->current->loc),
                parser->current->value
            );

            node_free(condition);
            return NULL;
        }

        if (!parser_match(parser, TOKEN_COLON))
        {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected ':' in ternary expression, got: '%s'\n",
                LOCATION(parser->current->loc),
                parser->current->value
            );
            node_free(condition);
            node_free(true_expr);
            return NULL;
        }

        node_t *false_expr = parse_expression(parser);
        if (!false_expr)
        {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected expression after ':' in ternary expression, got: '%s'\n",
                LOCATION(parser->current->loc),
                parser->current->value
            );
            node_free(condition);
            node_free(true_expr);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_TERNARY;
        node->ternary.condition = condition;
        node->ternary.true_expr = true_expr;
        node->ternary.false_expr = false_expr;

        return node;
    }

    return condition;
}

static node_t *parse_logical_or(parser_t *parser)
{
    node_t *left = parse_logical_and(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_LOGICAL_OR))
    {
        token_t op = *parser->previous;
        node_t *right = parse_logical_and(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}


static node_t *parse_logical_and(parser_t *parser)
{
    node_t *left = parse_bitwise_or(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_LOGICAL_AND))
    {
        token_t op = *parser->previous;
        node_t *right = parse_bitwise_or(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}


static node_t *parse_bitwise_or(parser_t *parser)
{
    node_t *left = parse_bitwise_xor(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_PIPE))
    {
        token_t op = *parser->previous;
        node_t *right = parse_bitwise_xor(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}

static node_t *parse_bitwise_xor(parser_t *parser)
{
    node_t *left = parse_bitwise_and(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_CARET))
    {
        token_t op = *parser->previous;
        node_t *right = parse_bitwise_and(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}


static node_t *parse_bitwise_and(parser_t *parser)
{
    node_t *left = parse_equality(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_AMPERSAND))
    {
        token_t op = *parser->previous;
        node_t *right = parse_equality(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}

static node_t *parse_equality(parser_t *parser)
{
    node_t *left = parse_relational(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_EQUAL_EQUAL) ||
           parser_match(parser, TOKEN_BANG_EQUAL))
    {
        token_t op = *parser->previous;
        node_t *right = parse_relational(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}

static node_t *parse_relational(parser_t *parser)
{
    node_t *left = parse_shift(parser);
    if (!left) return NULL;

    while (
        parser_match(parser, TOKEN_LESS)       || // <
        parser_match(parser, TOKEN_LESS_EQUAL) || // <=
        parser_match(parser, TOKEN_GREATER)    || // >
        parser_match(parser, TOKEN_GREATER_EQUAL)   // >=
    )
    {
        token_t op = *parser->previous;
        node_t *right = parse_shift(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}


static node_t *parse_shift(parser_t *parser)
{
    node_t *left = parse_term(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_LEFT_SHIFT) ||
           parser_match(parser, TOKEN_RIGHT_SHIFT))
    {
        token_t op = *parser->previous;
        node_t *right = parse_term(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}


static node_t *parse_term(parser_t *parser)
{
    node_t *left = parse_factor(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_PLUS) ||
           parser_match(parser, TOKEN_MINUS))
    {
        token_t op = *parser->previous;
        node_t *right = parse_factor(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}

static node_t *parse_factor(parser_t *parser)
{
    node_t *left = parse_exponent(parser);
    if (!left) return NULL;

    while (parser_match(parser, TOKEN_STAR) ||
           parser_match(parser, TOKEN_SLASH) ||
           parser_match(parser, TOKEN_PERCENT))
    {
        token_t op = *parser->previous;
        node_t *right = parse_exponent(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        left = node;
    }

    return left;
}


static node_t *parse_exponent(parser_t *parser)
{
    node_t *left = parse_unary(parser);
    if (!left) return NULL;

    if (parser_match(parser, TOKEN_STAR_STAR))
    {
        token_t op = *parser->previous;

        /* right association */
        node_t *right = parse_exponent(parser);
        if (!right)
        {
            node_free(left);
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_BINARY;
        node->binary.op = op;
        node->binary.left = left;
        node->binary.right = right;

        return node;
    }

    return left;
}

static node_t *parse_unary(parser_t *parser)
{
    if (parser_match(parser, TOKEN_AWAIT)) {
        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_AWAIT;
        node->loc = parser->previous->loc;
        node->await_expr.argument = parse_unary(parser);

        if (!node->await_expr.argument) {
            free(node);
            return NULL;
        }

        return node;
    }

    if (parser_match(parser, TOKEN_BANG) ||
        parser_match(parser, TOKEN_PLUS) ||
        parser_match(parser, TOKEN_MINUS) ||
        parser_match(parser, TOKEN_TILDE) ||
        parser_match(parser, TOKEN_PLUS_PLUS) ||
        parser_match(parser, TOKEN_MINUS_MINUS) ||
        parser_match(parser, TOKEN_TYPEOF) ||
        parser_match(parser, TOKEN_DELETE) ||
        parser_match(parser, TOKEN_VOID))
    {
        token_t op = *parser->previous;
        node_t *right = parse_unary(parser);
        if (!right) {
            return NULL;
        }

        node_t *node = calloc(1, sizeof(node_t));
        if (!node) ERROR("Malloc failed!\n");

        node->type = NODE_UNARY;
        node->loc = op.loc;
        node->unary.op = op;
        node->unary.right = right;

        return node; 
    }

    return parse_postfix(parser);
}

static node_t *parse_postfix(parser_t *parser)
{
    node_t *expr = parse_primary(parser);
    if (!expr) return NULL;

    while (true)
    {
        /* function call */
        if (parser_match(parser, TOKEN_LEFT_PAREN))
        {
            node_t *call = malloc(sizeof(node_t));
            if (!call) ERROR("Malloc failed!\n");

            call->type = NODE_CALL;
            // call->location = parser->previous.location;
            
            size_t argc = 0, cap = 4;
            node_t **args = malloc(sizeof(node_t*) * cap);
            if (!args) ERROR("Malloc failed!\n");

            if (parser->current->type != TOKEN_RIGHT_PAREN)
            {
                do
                {
                    node_t *arg = parse_expression(parser);
                    if (!arg)
                    {
                        /* free already parsed args before returning NULL */
                        for (size_t i = 0; i < argc; ++i) node_free(args[i]);
                        free(args); node_free(call); node_free(expr);
                        return NULL;
                    }
                    if (argc >= cap)
                    {
                        cap *= 2;
                        /* FIXME: assume reallocation never fails */
                        args = realloc(args, sizeof(node_t*) * cap);
                    }

                    args[argc++] = arg;
                } while (parser_match(parser, TOKEN_COMMA));
            }

            if (!parser_match(parser, TOKEN_RIGHT_PAREN))
            {
                PARSER_ERROR(parser,
                    "[ERROR] [%s:%zu:%zu] Expected ',' or ')' after function argument got: '%s'\n",
                        LOCATION(parser->current->loc),
                        parser->current->value
                );

                /* free already parsed args before returning NULL */
                for (size_t i = 0; i < argc; ++i) node_free(args[i]);
                free(args); node_free(call); node_free(expr);

                return NULL;
            }

            call->call.callee = expr;
            call->call.args = args;
            call->call.arg_count = argc;

            expr = call;
        }
        /* indexing */
        else if (parser_match(parser, TOKEN_LEFT_BRACKET))
        {
            node_t *node = malloc(sizeof(node_t));
            if (!node) ERROR("Malloc failed!\n");

            node->type = NODE_INDEX;
            // node->location = parser->previous.location;
            node->index.array = expr;
            node->index.index = parse_comma(parser);
            if (!node->index.index) {
                node_free(node);
                return NULL;
            }

            if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
                PARSER_ERROR(parser,
                    "[ERROR] [%s:%zu:%zu] Expected ']' after array index, got: '%s'\n",
                    LOCATION(parser->current->loc),
                    parser->current->value
                );
                node_free(node);
                return NULL;
            }

            expr = node;
        }
        /* member */
        else if (parser_match(parser, TOKEN_DOT))
        {
            node_t *node = malloc(sizeof(node_t));
            if (!node) ERROR("Malloc failed!\n");

            node->type = NODE_MEMBER;
            // node->location = parser->previous.location; // the '.' token
            node->member.object = expr;                 // the object being accessed

            // Only allow identifier (no arbitrary expression here)
            if (parser->current->type != TOKEN_IDENTIFIER) {
                PARSER_ERROR(parser,
                    "[ERROR] [%s:%zu:%zu] Expected identifier after '.'\n",
                    LOCATION(parser->current->loc)
                );
                node_free(node);
                return NULL;
            }

            node->member.property = parse_identifier(parser);
            expr = node;
        }
        else if (parser_match(parser, TOKEN_PLUS_PLUS) ||
            parser_match(parser, TOKEN_MINUS_MINUS))
        {
            node_t *node = malloc(sizeof(node_t));
            if (!node) ERROR("Malloc failed!\n");

            node->type = NODE_POSTFIX;
            node->postfix.op = *parser->previous;
			node->postfix.left = expr;
            expr = node;
        }
        else
        {
            break;
        }
    }

    return expr;
}

static node_t *parse_primary(parser_t *parser)
{
    if (parser->current->type == TOKEN_NUMBER_LITERAL)  return parse_number(parser);
    if (parser->current->type == TOKEN_STRING_LITERAL)  return parse_string(parser);
    if (parser->current->type == TOKEN_BOOL_LITERAL)    return parse_bool(parser);
    if (parser->current->type == TOKEN_IDENTIFIER)      return parse_identifier(parser);
    if (parser->current->type == TOKEN_ASYNC || parser->current->type == TOKEN_FUNCTION)
        return parse_function(parser);
    if (parser->current->type == TOKEN_NULL)            return parse_null(parser);
    if (parser->current->type == TOKEN_UNDEFINED)       return parse_undefined(parser);
    if (parser->current->type == TOKEN_ELLIPSIS)        return parse_spread(parser);
    if (parser->current->type == TOKEN_LEFT_BRACKET)    return parse_array(parser);
    if (parser->current->type == TOKEN_LEFT_BRACE)      return parse_object(parser);

    if (parser->current->type == TOKEN_THIS) {
        node_t *node = calloc(1, sizeof(node_t));
        node->type = NODE_THIS;
        node->loc = parser->current->loc;
        parser_advance(parser);
        return node;
    }

    if (parser_match(parser, TOKEN_LEFT_PAREN))
    {
        node_t *expr = parse_comma(parser);
        if (!expr) return NULL;

        if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected ')' after expression got: '%s'\n",
                LOCATION(parser->current->loc),
                parser->current->value
            );
            node_free(expr);
            return NULL;
        }

        return expr;
    }

    PARSER_ERROR(parser,
        "[ERROR] [%s:%zu:%zu] Expected primary expression, got '%s'\n",
        LOCATION(parser->current->loc),
        parser->current->value
    );

    return NULL;
}

static node_t *parse_number(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_NUMBER_LITERAL)) return NULL;

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_NUMBER;

    // mpfr_init2(node->number, MPFR_PRECISION);    
    // if (mpfr_set_str(node->number, parser->previous->value, 10, MPFR_RNDN) != 0) {
    //     ERROR("Invalid number literal: %s\n", parser->previous->value);
    // }

    char *endptr = NULL;
    node->number = strtold(parser->previous->value, &endptr);
    if (endptr == parser->previous->value || *endptr != '\0') {
        ERROR("Invalid number literal: %s\n", parser->previous->value);
    }

    return node;
}

static node_t *parse_string(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_STRING_LITERAL)) return NULL;

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_STRING;
    node->string = strdup(parser->previous->value); 
    return node;
}

static node_t *parse_bool(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_BOOL_LITERAL)) return NULL;

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_BOOL;
    node->boolean = strcmp(parser->previous->value, "true") == 0; 
    return node;
}

static node_t *parse_identifier(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_IDENTIFIER)) return NULL;

    const char *name = parser->previous->value;

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    if (strncmp(name, "Infinity", 8) == 0) {
        node->type = NODE_NUMBER;
        node->number = INFINITY;
    } else if (strncmp(name, "NaN", 3) == 0) {
        node->type = NODE_NUMBER;
        node->number = NAN;
    } else {
        node->type = NODE_IDENTIFIER;
        node->identifier = strdup(parser->previous->value);    
    }

    return node;
}

static node_t *parse_undefined(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_UNDEFINED)) return NULL;

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_UNDEFINED;
    return node;
}

static node_t *parse_null(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_NULL)) return NULL;

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_NULL;
    return node;
}

static node_t *parse_spread(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_ELLIPSIS)) return NULL;

    node_t *argument = parse_expression(parser);
    if (!argument) return NULL;

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_SPREAD;
    node->spread.argument = argument;
    return node;
}

static node_t *parse_array(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_LEFT_BRACKET)) return NULL;

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_ARRAY;
    node->array.elements = NULL;
    node->array.count = 0;

    node_t **elements = NULL;
    size_t count = 0;

    if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
        do {
            node_t *element = NULL;

            // spread element
            if (parser_match(parser, TOKEN_ELLIPSIS)) {
                node_t *argument = parse_expression(parser);
                if (!argument) goto fail;

                element = calloc(1, sizeof(node_t));
                if (!element) ERROR("Malloc failed!\n");

                element->type = NODE_SPREAD;
                element->spread.argument = argument;
            }
            // elision (undefined)
            else if (parser->current->type == TOKEN_COMMA) {
                element = calloc(1, sizeof(node_t));
                if (!element) ERROR("Malloc failed!\n");

                element->type = NODE_UNDEFINED;
            }
            else
            {
                element = parse_expression(parser);
                if (!element) goto fail;
            }

            node_t **tmp = realloc(elements, sizeof(node_t *) * (count + 1));
            if (!tmp) ERROR("Realloc failed!\n");
            elements = tmp;
            elements[count++] = element;

        } while (parser_match(parser, TOKEN_COMMA) && parser->current->type != TOKEN_RIGHT_BRACKET);

        if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected ',' or ']' after array element, got '%s'\n",
                LOCATION(parser->current->loc),
                parser->current->value
            );
            goto fail;
        }
    }

    node->array.elements = elements;
    node->array.count = count;
    return node;

fail:
    for (size_t i = 0; i < count; i++) node_free(elements[i]);
    free(elements);
    free(node);
    return NULL;
}

static node_t *parse_object(parser_t *parser)
{
    if (!parser_match(parser, TOKEN_LEFT_BRACE)) return NULL;

    node_t *node = calloc(1, sizeof(node_t));
    if (!node) ERROR("Malloc failed!\n");

    node->type = NODE_OBJECT;
    node->object.keys = NULL;
    node->object.values = NULL;
    node->object.count = 0;

    char **keys = NULL;
    node_t **values = NULL;
    size_t count = 0;

    if (!parser_match(parser, TOKEN_RIGHT_BRACE)) {
        do {
            node_t *value = NULL;
            char *key = NULL;

            // spread property
            // if (parser_match(parser, TOKEN_ELLIPSIS)) {
            //     value = parse_expression(parser);
            //     if (!value) goto fail;
            //     key = NULL;
            // }
            if (parser_check(parser, TOKEN_ELLIPSIS)) {
                value = parse_spread(parser);
                if (!value) goto fail;
                key = NULL;
            }

            else {
                if (!(parser_match(parser, TOKEN_IDENTIFIER) ||
                      parser_match(parser, TOKEN_STRING_LITERAL))) {
                    PARSER_ERROR(parser,
                        "[ERROR] [%s:%zu:%zu] Expected identifier or string literal for object key, got: '%s'\n",
                        LOCATION(parser->current->loc),
                        parser->current->value
                    );
                    goto fail;
                }

                key = strdup(parser->previous->value);
                if (!key) ERROR("Strdup failed!\n");

                if (parser_match(parser, TOKEN_COLON)) {
                    value = parse_expression(parser);
                    if (!value) {
                        free(key);
                        goto fail;
                    }
                }
                else if (parser->current->type == TOKEN_COMMA ||
                         parser->current->type == TOKEN_RIGHT_BRACE) {
                    // shorthand { a, b }
                    value = calloc(1, sizeof(node_t));
                    if (!value) ERROR("Malloc failed!\n");

                    value->type = NODE_IDENTIFIER;
                    value->identifier = strdup(key);
                    if (!value->identifier) ERROR("Strdup failed!\n");
                }
                else {
                    PARSER_ERROR(parser,
                        "[ERROR] [%s:%zu:%zu] Expected ':' or ',' after object key, got '%s'\n",
                        LOCATION(parser->current->loc),
                        parser->current->value
                    );
                    free(key);
                    goto fail;
                }
            }

            char **tmp_keys = realloc(keys, sizeof(char *) * (count + 1));
            node_t **tmp_values = realloc(values, sizeof(node_t *) * (count + 1));
            if (!tmp_keys || !tmp_values) ERROR("Realloc failed!\n");

            keys = tmp_keys;
            values = tmp_values;
            keys[count] = key;
            values[count] = value;
            count++;

        } while (parser_match(parser, TOKEN_COMMA) && parser->current->type != TOKEN_RIGHT_BRACE);

        if (!parser_match(parser, TOKEN_RIGHT_BRACE)) {
            PARSER_ERROR(parser,
                "[ERROR] [%s:%zu:%zu] Expected ',' or '}' after object entry, got '%s'\n",
                LOCATION(parser->current->loc),
                parser->current->value
            );
            goto fail;
        }
    }

    node->object.keys = keys;
    node->object.values = values;
    node->object.count = count;
    return node;

fail:
    for (size_t i = 0; i < count; i++) {
        free(keys[i]);
        node_free(values[i]);
    }
    free(keys);
    free(values);
    free(node);
    return NULL;
}
