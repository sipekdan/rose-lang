#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#include "utils.h"
#include "lexer.h"

char *lexer_strndup(const char *s, size_t n)
{
    size_t len = strlen(s);
    if (len > n) len = n;

    char *new = malloc(len + 1);
    if (new == NULL)
        return NULL;
    
    new[len] = '\0';
    return memcpy(new, s, len);
}

char *lexer_readfile(char *filename)
{
    FILE *f = fopen(filename, "rb");
	if (!f)
	{
		perror("fopen");
		return NULL;
	}

	if (fseek(f, 0, SEEK_END) != 0)
	{
		perror("fseek");
		fclose(f);
		return NULL;
	}

	const long size = ftell(f);
	if (size < 0)
	{
		perror("ftell");
		fclose(f);
		return NULL;
	}

	rewind(f);

	char *buffer = malloc(size + 1);
	if (!buffer)
	{
		fprintf(stderr, "Memory allocation failed\n");
		fclose(f);
		return NULL;
	}

	const size_t read_size = fread(buffer, 1, size, f);
	if (read_size != (size_t) size)
	{
		fprintf(stderr, "Could not read whole file\n");
		free(buffer);
		fclose(f);
		return NULL;
	}

	buffer[size] = '\0';

	fclose(f);
	return buffer;
}

static void lexer_tokenize(lexer_t *lexer);

void lexer_init(lexer_t *lexer, char *filename, char *source)
{
    if (source) {
        lexer->source = strdup(source);
        lexer->loc.filename = filename ? strdup(filename) : strdup("(null)");
    } else if (filename) {
        lexer->source = lexer_readfile(filename);
        if (!lexer->source) {
            lexer->had_error = true;
            return;
        }
        lexer->loc.filename = strdup(filename);
    } else {
        lexer->source = NULL;
        lexer->loc.filename = NULL;
        lexer->had_error = true;
        return;
    }

    // lexer->source = lexer_readfile(filename);
    // if (!lexer->source)
    // {
    //     lexer->had_error = true;
    //     return;
    // }

    // lexer->loc.filename = strdup(filename);
    lexer->length = strlen(lexer->source);
    
    lexer->loc.line = lexer->loc.column = 1;
    lexer->pos = 0;

    lexer->tokens = NULL;
    lexer->index = 0;
    lexer->count = 0;
    lexer->capacity = 0;

    lexer->had_error = false;

    lexer_tokenize(lexer);
}

void lexer_free(lexer_t *lexer)
{
    if (!lexer) return;

    for (size_t i = 0; i < lexer->count; i++)
    {
        token_t token = lexer->tokens[i];
        free(token.loc.filename);
        free(token.value);
        token.value = NULL;
    }

    free(lexer->tokens);
    lexer->tokens = NULL;
    lexer->count = lexer->capacity = 0;

    free(lexer->loc.filename);
    free(lexer->source);
    free(lexer);
}

static char lexer_peek(lexer_t *lex)
{
    if (lex->pos >= lex->length) return '\0';
    return lex->source[lex->pos];
}

static char lexer_advance(lexer_t *lex)
{
    if (lex->pos >= lex->length)
        return '\0';

    unsigned char c = lex->source[lex->pos++];

    if (c > 127) {
        LEXER_ERROR(lex, "Non-ASCII character encountered\n");
        return '\0';
    }

    if (c == '\n')
    {
        lex->loc.line++;
        lex->loc.column = 1;
    } else
    {
        lex->loc.column++;
    }

    return c;
}

static bool lexer_match(lexer_t *lexer, char expected)
{
    if (lexer_peek(lexer) == expected)
    {
        lexer_advance(lexer);
        return true;
    }
    return false;
}

static token_t *lexer_append(lexer_t *lexer, token_t token)
{
    if (lexer->count == lexer->capacity)
    {
        lexer->capacity = lexer->capacity ? lexer->capacity * 2 : 8;
        lexer->tokens = realloc(lexer->tokens, lexer->capacity * sizeof(token_t));
    }
    lexer->tokens[lexer->count++] = token;
    
    printf("[%s:%zu:%zu] %s(\"%s\")\n",
        token.loc.filename,
        token.loc.line,
        token.loc.column,
        token_type_to_string(token.type),
        token.value
    );

    return &lexer->tokens[lexer->count - 1];
}

static void lexer_tokenize(lexer_t *lexer)
{
    static_assert(TOKEN_COUNT == 88, "Fix TOKEN_COUNT in 'lexer_tokenize'");

    while (true)
    {
        /* skip whitespace */
        while (isspace(lexer_peek(lexer)))
            lexer_advance(lexer);

        /* skip comments */
        if (lexer_peek(lexer) == '/' &&
            lexer->pos + 1 < lexer->length)
        {
            char next = lexer->source[lexer->pos + 1];

            /* single-line comment */
            if (next == '/')
            {
                lexer_advance(lexer); /* skip first / */
                lexer_advance(lexer); /* skip second / */
                while (lexer_peek(lexer) != '\n' &&
                        lexer_peek(lexer) != '\0')
                {
                    lexer_advance(lexer);
                }
                continue; /* skip whitespace */
            }

            /* multi-line comment */
            else if (next == '*') {
                lexer_advance(lexer); /* skip first / */
                lexer_advance(lexer); /* skip * */
                while (true) {
                    if (lexer_peek(lexer) == '\0')
                    {
                        LEXER_ERROR(lexer, "Unterminated multi-line comment\n");
                        return;
                    }
                    if (lexer_peek(lexer) == '*' &&
                        lexer->pos + 1 < lexer->length &&
                        lexer->source[lexer->pos + 1] == '/')
                    {
                        lexer_advance(lexer); /* skip * */
                        lexer_advance(lexer); /* skip / */
                        break;
                    }
                    lexer_advance(lexer);
                }
                continue; /* skip whitespace */
            }
        }

        size_t start = lexer->pos;
        size_t start_line = lexer->loc.line;
        size_t start_column = lexer->loc.column;

        char c = lexer_advance(lexer);

        token_t token = {0};
        token.loc.filename = strdup(lexer->loc.filename);
        token.loc.line = start_line;
        token.loc.column = start_column;

        /* end of file */
        if (c == '\0')
        {
            token.type = TOKEN_EOF;
            lexer_append(lexer, token);
            break; /* stop tokenization */
        }

        /* strings */
        if (c == '"' || c == '\'')
        {
            char quote = c;
            size_t buf_capacity = 16, buf_len = 0;
            char *buf = malloc(buf_capacity);

            while (lexer_peek(lexer) != quote && lexer_peek(lexer) != '\0')
            {
                char ch = lexer_advance(lexer);
                if (ch == '\\')
                {
                    char next = lexer_advance(lexer);
                    switch (next)
                    {
                        case 'n': ch = '\n'; break;
                        case 'r': ch = '\r'; break;
                        case 't': ch = '\t'; break;
                        case 'b': ch = '\b'; break;
                        case 'f': ch = '\f'; break;
                        case 'v': ch = '\v'; break;
                        case '\\': ch = '\\'; break;
                        case '\'': ch = '\''; break;
                        case '"': ch = '"'; break;
                        case '0': ch = '\0'; break;
                        case 'u':
                        {
                            if (lexer->pos + 4 > lexer->length) {
                                free(buf);
                                free(token.loc.filename);
                                LEXER_ERROR(lexer, "Incomplete Unicode escape sequence\n");
                                return;
                            }

                            unsigned int codepoint = 0;
                            for (int i = 0; i < 4; i++) {
                                char hex = lexer_advance(lexer);
                                if (!isxdigit(hex)) {
                                    free(buf);
                                    free(token.loc.filename);
                                    LEXER_ERROR(lexer, "Invalid Unicode escape sequence\n");
                                    return;
                                }
                                codepoint <<= 4;
                                if (hex >= '0' && hex <= '9') codepoint |= hex - '0';
                                else if (hex >= 'a' && hex <= 'f') codepoint |= hex - 'a' + 10;
                                else if (hex >= 'A' && hex <= 'F') codepoint |= hex - 'A' + 10;
                            }

                            // Ensure buffer can hold up to 3 bytes
                            if (buf_len + 3 >= buf_capacity) {
                                buf_capacity *= 2;
                                buf = realloc(buf, buf_capacity);
                            }

                            // Encode codepoint into UTF-8
                            if (codepoint <= 0x7F) {
                                buf[buf_len++] = (char)codepoint;
                            } else if (codepoint <= 0x7FF) {
                                buf[buf_len++] = 0xC0 | ((codepoint >> 6) & 0x1F);
                                buf[buf_len++] = 0x80 | (codepoint & 0x3F);
                            } else { // 0x800 .. 0xFFFF
                                buf[buf_len++] = 0xE0 | ((codepoint >> 12) & 0x0F);
                                buf[buf_len++] = 0x80 | ((codepoint >> 6) & 0x3F);
                                buf[buf_len++] = 0x80 | (codepoint & 0x3F);
                            }
                            continue; // already handled
                        }
                        default:
                            free(token.loc.filename);
                            lexer->loc.column -= 1;
                            LEXER_ERROR(lexer, "Unknown escape sequence '\\%c'\n", next);
                            return;
                    }
                }
                if (buf_len + 1 >= buf_capacity) {
                    buf_capacity *= 2;
                    buf = realloc(buf, buf_capacity);
                }
                buf[buf_len++] = ch;
            }

            if (lexer_peek(lexer) != quote)
            {
                free(buf);
                free(token.loc.filename);
                lexer->loc.line = start_line;
                lexer->loc.column = start_column;
                LEXER_ERROR(lexer, "Unterminated string literal\n");
                return;
            }

            lexer_advance(lexer); // closing quote
            buf[buf_len] = '\0';

            token.type = TOKEN_STRING_LITERAL;
            token.length = buf_len;
            token.value = buf;
            lexer_append(lexer, token);
            continue;
        }

        /* numbers */
        if (isdigit(c))
        {
            size_t start = lexer->pos - 1;
            while (isdigit(lexer_peek(lexer)) || lexer_peek(lexer) == '_')
                lexer_advance(lexer);

            if (lexer_peek(lexer) == '.')
            {
                lexer_advance(lexer);
                while (isdigit(lexer_peek(lexer)) || lexer_peek(lexer) == '_')
                    lexer_advance(lexer);
            }

            size_t length = lexer->pos - start;
            char *value = lexer_strndup(&lexer->source[start], length);

            token.type = TOKEN_NUMBER_LITERAL;
            token.value = value;
            token.length = length;

            lexer_append(lexer, token);
            continue;
        }

        /* identifiers and keywords */
        if (isalpha(c) || c == '_' || c == '$')
        {
            while (isalnum(lexer_peek(lexer)) || lexer_peek(lexer) == '_' ||
                   lexer_peek(lexer) == '$')
                lexer_advance(lexer);

            size_t length = lexer->pos - start;
            char *value = lexer_strndup(&lexer->source[start], length);

            token.value = value;
            token.length = length;

            if (length == 2 && strncmp(value, "if", 2) == 0) token.type = TOKEN_IF;
            else if (length == 4 && strncmp(value, "else", 4) == 0) token.type = TOKEN_ELSE;
            else if (length == 5 && strncmp(value, "while", 5) == 0) token.type = TOKEN_WHILE;
            else if (length == 2 && strncmp(value, "do", 2) == 0) token.type = TOKEN_DO;
            else if (length == 3 && strncmp(value, "for", 3) == 0) token.type = TOKEN_FOR;
            else if (length == 8 && strncmp(value, "function", 8) == 0) token.type = TOKEN_FUNCTION;
            else if (length == 6 && strncmp(value, "switch", 6) == 0) token.type = TOKEN_SWITCH;
            else if (length == 4 && strncmp(value, "case", 4) == 0) token.type = TOKEN_CASE;
            else if (length == 7 && strncmp(value, "default", 7) == 0) token.type = TOKEN_DEFAULT;
            else if (length == 3 && strncmp(value, "let", 3) == 0) token.type = TOKEN_LET;
            else if (length == 5 && strncmp(value, "const", 5) == 0) token.type = TOKEN_CONST;
            else if (length == 3 && strncmp(value, "var", 3) == 0) token.type = TOKEN_VAR;
            else if (length == 4 && strncmp(value, "null", 4) == 0) token.type = TOKEN_NULL;
            else if (length == 9 && strncmp(value, "undefined", 9) == 0) token.type = TOKEN_UNDEFINED;
            else if (length == 6 && strncmp(value, "return", 6) == 0) token.type = TOKEN_RETURN;
            else if (length == 5 && strncmp(value, "break", 5) == 0) token.type = TOKEN_BREAK;
            else if (length == 8 && strncmp(value, "continue", 8) == 0) token.type = TOKEN_CONTINUE;
            else if (length == 6 && strncmp(value, "import", 6) == 0) token.type = TOKEN_IMPORT;
            else if (length == 6 && strncmp(value, "export", 6) == 0) token.type = TOKEN_EXPORT;
            else if (length == 3 && strncmp(value, "try", 3) == 0) token.type = TOKEN_TRY;
            else if (length == 5 && strncmp(value, "catch", 5) == 0) token.type = TOKEN_CATCH;
            else if (length == 7 && strncmp(value, "finally", 7) == 0) token.type = TOKEN_FINALLY;
            else if (length == 5 && strncmp(value, "throw", 5) == 0) token.type = TOKEN_THROW;
            else if (length == 6 && strncmp(value, "typeof", 6) == 0) token.type = TOKEN_TYPEOF;
            // else if (length == 2 && strncmp(value, "in", 2) == 0) token.type = TOKEN_IN;
            // else if (length == 2 && strncmp(value, "of", 2) == 0) token.type = TOKEN_OF;
            else if (length == 4 && strncmp(value, "true", 4) == 0) token.type = TOKEN_BOOL_LITERAL;
            else if (length == 5 && strncmp(value, "false", 5) == 0) token.type = TOKEN_BOOL_LITERAL;
            else if (length == 5 && strncmp(value, "async", 5) == 0) token.type = TOKEN_ASYNC;
            else if (length == 5 && strncmp(value, "await", 5) == 0) token.type = TOKEN_AWAIT;
            else if (length == 4 && strncmp(value, "from", 4) == 0) token.type = TOKEN_FROM;
            else if (length == 6 && strncmp(value, "delete", 6) == 0) token.type = TOKEN_DELETE;
            else if (length == 4 && strncmp(value, "this", 4) == 0) token.type = TOKEN_THIS;
            else if (length == 4 && strncmp(value, "void", 4) == 0) token.type = TOKEN_VOID;
            else if (length == 3 && strncmp(value, "new", 3) == 0) token.type = TOKEN_NEW;
            else if (length == 8 && strncmp(value, "debugger", 8) == 0) token.type = TOKEN_DEBUGGER;
            else token.type = TOKEN_IDENTIFIER;

            lexer_append(lexer, token);
            continue;
        }

        /* operators, punctuation, etc. */
        switch (c)
        {
            case '+':
                if (lexer_match(lexer, '+')) { token.type = TOKEN_PLUS_PLUS; token.value = strdup("++"); token.length = 2; }
                else if (lexer_match(lexer, '=')) { token.type = TOKEN_PLUS_EQUAL; token.value = strdup("+="); token.length = 2; }
                else { token.type = TOKEN_PLUS; token.value = strdup("+"); token.length = 1; }
                break;
            case '-':
                if (lexer_match(lexer, '-')) { token.type = TOKEN_MINUS_MINUS; token.value = strdup("--"); token.length = 2; }
                else if (lexer_match(lexer, '=')) { token.type = TOKEN_MINUS_EQUAL; token.value = strdup("-="); token.length = 2; }
                else if (lexer_match(lexer, '>')) { token.type = TOKEN_ARROW; token.value = strdup("=>"); token.length = 2; }
                else { token.type = TOKEN_MINUS; token.value = strdup("-"); token.length = 1; }
                break;
            case '*':
                if (lexer_match(lexer, '*')) {
                    if (lexer_match(lexer, '=')) { token.type = TOKEN_STAR_STAR_EQUAL; token.value = strdup("**="); token.length = 3; }
                    else { token.type = TOKEN_STAR_STAR; token.value = strdup("**"); token.length = 2; }
                }
                else if (lexer_match(lexer, '=')) { token.type = TOKEN_STAR_EQUAL; token.value = strdup("*="); token.length = 2; }
                else { token.type = TOKEN_STAR; token.value = strdup("*"); token.length = 1; }
                break;
            case '/':
                if (lexer_match(lexer, '=')) { token.type = TOKEN_SLASH_EQUAL; token.value = strdup("/="); token.length = 2; }
                else { token.type = TOKEN_SLASH; token.value = strdup("/"); token.length = 1; }
                break;
            case '%':
                if (lexer_match(lexer, '=')) { token.type = TOKEN_PERCENT_EQUAL; token.value = strdup("%="); token.length = 2; }
                else { token.type = TOKEN_PERCENT; token.value = strdup("%"); token.length = 1; }
                break;
            case '&':
                if (lexer_match(lexer, '&')) {
                    if (lexer_match(lexer, '=')) { token.type = TOKEN_LOGICAL_AND_EQUAL; token.value = strdup("&&="); token.length = 3; }
                    else { token.type = TOKEN_LOGICAL_AND; token.value = strdup("&&"); token.length = 2; }
                }
                else if (lexer_match(lexer, '=')) { token.type = TOKEN_AMPERSAND_EQUAL; token.value = strdup("&="); token.length = 2; }
                else { token.type = TOKEN_AMPERSAND; token.value = strdup("&"); token.length = 1; }
                break;
            case '|':
                if (lexer_match(lexer, '|')) {
                    if (lexer_match(lexer, '=')) { token.type = TOKEN_LOGICAL_OR_EQUAL; token.value = strdup("||="); token.length = 3; }
                    else { token.type = TOKEN_LOGICAL_OR; token.value = strdup("||"); token.length = 2; }
                }
                else if (lexer_match(lexer, '=')) { token.type = TOKEN_PIPE_EQUAL; token.value = strdup("|="); token.length = 2; }
                else { token.type = TOKEN_PIPE; token.value = strdup("|"); token.length = 1; }
                break;
            case '^':
                if (lexer_match(lexer, '=')) { token.type = TOKEN_CARET_EQUAL; token.value = strdup("^="); token.length = 2; }
                else { token.type = TOKEN_CARET; token.value = strdup("^"); token.length = 1; }
                break;
            case '~': token.type = TOKEN_TILDE; token.value = strdup("~"); token.length = 1; break;
            case '<':
                if (lexer_match(lexer, '<')) {
                    if (lexer_match(lexer, '=')) { token.type = TOKEN_LEFT_SHIFT_EQUAL; token.value = strdup("<<="); token.length = 3; }
                    else { token.type = TOKEN_LEFT_SHIFT; token.value = strdup("<<"); token.length = 2; }
                }
                else if (lexer_match(lexer, '=')) { token.type = TOKEN_LESS_EQUAL; token.value = strdup("<="); token.length = 2; }
                else { token.type = TOKEN_LESS; token.value = strdup("<"); token.length = 1; }
                break;
            case '>':
                if (lexer_match(lexer, '>')) {
                    if (lexer_match(lexer, '=')) { token.type = TOKEN_RIGHT_SHIFT_EQUAL; token.value = strdup(">>="); token.length = 3; }
                    else { token.type = TOKEN_RIGHT_SHIFT; token.value = strdup(">>"); token.length = 2; }
                }
                else if (lexer_match(lexer, '=')) { token.type = TOKEN_GREATER_EQUAL; token.value = strdup(">="); token.length = 2; }
                else { token.type = TOKEN_GREATER; token.value = strdup(">"); token.length = 1; }
                break;
            case '=':
                if (lexer_match(lexer, '=')) {
                    token.type = TOKEN_EQUAL_EQUAL; token.value = strdup("=="); token.length = 2;
                }
                else { token.type = TOKEN_EQUAL; token.value = strdup("="); token.length = 1; }
                break;
            case '!':
                if (lexer_match(lexer, '=')) {
                    token.type = TOKEN_BANG_EQUAL; token.value = strdup("!="); token.length = 2;
                }
                else { token.type = TOKEN_BANG; token.value = strdup("!"); token.length = 1; }
                break;
            case '(': token.type = TOKEN_LEFT_PAREN; token.value = strdup("("); token.length = 1; break;
            case ')': token.type = TOKEN_RIGHT_PAREN; token.value = strdup(")"); token.length = 1; break;
            case '{': token.type = TOKEN_LEFT_BRACE; token.value = strdup("{"); token.length = 1; break;
            case '}': token.type = TOKEN_RIGHT_BRACE; token.value = strdup("}"); token.length = 1; break;
            case '[': token.type = TOKEN_LEFT_BRACKET; token.value = strdup("["); token.length = 1; break;
            case ']': token.type = TOKEN_RIGHT_BRACKET; token.value = strdup("]"); token.length = 1; break;
            case '.':
                if (lexer_match(lexer, '.') && lexer_match(lexer, '.')) { token.type = TOKEN_ELLIPSIS; token.value = strdup("..."); token.length = 3; }
                else { token.type = TOKEN_DOT; token.value = strdup("."); token.length = 1; }
                break;
            case ',': token.type = TOKEN_COMMA; token.value = strdup(","); token.length = 1; break;
            case ';': token.type = TOKEN_SEMICOLON; token.value = strdup(";"); token.length = 1; break;
            case ':': token.type = TOKEN_COLON; token.value = strdup(":"); token.length = 1; break;
            case '?':
                // if (lexer_match(lexer, '?')) {
                //     token.type = TOKEN_NULLISH_COALESCING; token.value = strdup("??"); token.length = 2;
                // }
                // else { token.type = TOKEN_QUESTION; token.value = strdup("?"); token.length = 1;}
                token.type = TOKEN_QUESTION; token.value = strdup("?"); token.length = 1;
                break; 
            default:
                free(token.loc.filename);
                lexer->loc.line = start_line;
                lexer->loc.column = start_column;
                LEXER_ERROR(lexer, "Unexpected character '%c'\n", c);
                return;
                // token.type = TOKEN_UNKNOWN;
                // token.value = strndup(&lexer->source[start], 1);
                // token.length = 1;
        }

        // token.loc.line = start_line;
        // token.loc.column = start_column;
        lexer_append(lexer, token);
    }
}

token_t *lexer_next(lexer_t *lexer)
{
    if (!lexer || !lexer->tokens || lexer->index >= lexer->count)
        return NULL; // EOF reached

    return &lexer->tokens[lexer->index++];
}

