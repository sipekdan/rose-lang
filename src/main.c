#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <getopt.h>
#include <stdlib.h>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "eval.h"

#define VERSION "0.1.0"

#define MAX_BUFFER 8192

bool is_input_complete(const char *buffer) {
    int parens = 0, braces = 0, brackets = 0;
    bool in_string = false;
    char string_char = 0;

    for (const char *p = buffer; *p; p++) {
        char c = *p;
        if (in_string) {
            if (c == string_char && *(p-1) != '\\') in_string = false;
            continue;
        }

        if (c == '"' || c == '\'') {
            in_string = true;
            string_char = c;
        } else if (c == '(') parens++;
        else if (c == ')') parens--;
        else if (c == '{') braces++;
        else if (c == '}') braces--;
        else if (c == '[') brackets++;
        else if (c == ']') brackets--;
    }

    return parens <= 0 && braces <= 0 && brackets <= 0 && !in_string;
}

char *read_line(const char *prompt, char *buffer, size_t max_len) {
    printf("%s", prompt);
    if (!fgets(buffer, max_len, stdin)) return NULL;
    buffer[strcspn(buffer, "\n")] = '\0';
    return buffer;
}

int main(int argc, char **argv)
{
    // printf("%Lf\n", -1.0L / 0.0L);
    // return 0;
    goto normal;

    char input_buffer[MAX_BUFFER] = "";
    char line_buffer[1024];

    printf("MiniJS REPL (type 'exit' to quit)\n");

    while (1) {
        char *prompt = is_input_complete(input_buffer) ? "> " : "... ";
        if (!read_line(prompt, line_buffer, sizeof(line_buffer))) break;
        if (strcmp(line_buffer, "exit") == 0) break;

        // Save buffer length before appending
        size_t prev_len = strlen(input_buffer);

        // Append the new line
        strcat(input_buffer, line_buffer);
        strcat(input_buffer, "\n");

        if (!is_input_complete(input_buffer))
            continue;

        lexer_t *lexer = malloc(sizeof(lexer_t));
        parser_t *parser = malloc(sizeof(parser_t));
        node_t *ast = NULL;

        lexer_init(lexer, NULL, input_buffer);
        if (lexer->had_error) {
            printf("Lexing failed!\n");
            free(lexer);
            free(parser);
            // revert only last line
            input_buffer[prev_len] = '\0';
            continue;
        }

        parser_init(parser, lexer);
        ast = parse_program(parser);
        if (!ast) {
            printf("Parsing failed!\n");
            parser_free(parser);
            lexer_free(lexer);
            // revert only last line
            input_buffer[prev_len] = '\0';
            continue;
        }

        // Optional AST printing
        node_print(ast);

        // Semantic analysis
        sema_t *sema = malloc(sizeof(sema_t));
        sema_init(sema, ast);
        sema_analyze(sema);
        if (sema->had_error) {
            printf("Semantic analysis failed!\n");
            node_free(ast);
            parser_free(parser);
            lexer_free(lexer);
            sema_free(sema);
            // revert only last line
            input_buffer[prev_len] = '\0';
            continue;
        }

        // Optional evaluation
        // value_t result = evaluate(ast, persistent_env);
        // print_value(result);
        // printf("\n");
        // Cleanup
        node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        sema_free(sema);
    }
    printf("Bye!\n");

    // assert(argc == 2);
normal:

    int opt;
    static struct option long_options[] = {
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "v", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v':
                printf("Rose interpreter version %s\n", VERSION);
                return 0;
            default:
                fprintf(stderr, "Usage: %s [--version|-v] [file]\n", argv[0]);
                return 1;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc == 0) {
        fprintf(stderr, "No input file provided\n");
        return 1;
    }

    char *input_file = argv[0];

    clock_t start, end;
    double duration;
    int result = 0;

    lexer_t *lexer = NULL;
    parser_t *parser = NULL;
    node_t *program = NULL;
    sema_t *sema = NULL;

    start = clock();
    lexer = malloc(sizeof(lexer_t));
    lexer_init(lexer, input_file, NULL);
    // lexer_init(lexer, argv[1], NULL);
    if (lexer->had_error)
    {
        result = 1;
        printf("Lexering failed...\n");
        goto cleanup;
    }
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Lexing took %.6f seconds\n", duration);

    start = clock();
    parser = malloc(sizeof(parser_t));
    parser_init(parser, lexer);

    program = parse_program(parser);
    if (!program)
    {
        result = 1;
        printf("Parsing failed...\n");
        goto cleanup;
    }
    node_print(program);
    node_build(program);

    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Parsing took %.6f seconds\n", duration);

    start = clock();
    sema = malloc(sizeof(sema_t));
    sema_init(sema, program);
    sema_analyze(sema);
    if (sema->had_error)
    {
        result = 1;
        printf("Sema check failed...\n");
        goto cleanup;
    }

    printf("Semantic analysis succeeded!\n");

    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Semantic analysis took %.6f seconds\n", duration);

cleanup:
    if (sema) sema_free(sema);
    if (program) node_free(program);
    if (parser) parser_free(parser);
    if (lexer) lexer_free(lexer);

    // mpfr_free_cache();
    return result;
}
