#include <assert.h>
#include "token.h"

const char *token_type_to_string(token_type_t type)
{
    static_assert(TOKEN_COUNT == 87, "Fix TOKEN_COUNT in 'token_type_to_string'");

    switch (type)
    {
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_DO: return "DO";
        case TOKEN_FOR: return "FOR";
        case TOKEN_FUNCTION: return "FUNCTION";
        case TOKEN_SWITCH: return "SWITCH";
        case TOKEN_CASE: return "CASE";
        case TOKEN_DEFAULT: return "DEFAULT";
        case TOKEN_LET: return "LET";
        case TOKEN_CONST: return "CONST";
        case TOKEN_VAR: return "VAR";
        case TOKEN_NULL: return "NULL";
        case TOKEN_UNDEFINED: return "UNDEFINED";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_BREAK: return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";
        case TOKEN_IMPORT: return "IMPORT";
        case TOKEN_EXPORT: return "EXPORT";
        case TOKEN_TRY: return "TRY";
        case TOKEN_CATCH: return "CATCH";
        case TOKEN_FINALLY: return "FINALLY";
        case TOKEN_THROW: return "THROW";
        case TOKEN_TYPEOF: return "TYPEOF";
        case TOKEN_ASYNC: return "ASYNC";
        case TOKEN_AWAIT: return "AWAIT";
        case TOKEN_FROM: return "FROM";
        case TOKEN_DELETE: return "DELETE";
        case TOKEN_THIS: return "THIS";
        case TOKEN_VOID: return "VOID";

        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_BOOL_LITERAL: return "BOOL_LITERAL";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";
        case TOKEN_NUMBER_LITERAL: return "NUMBER_LITERAL";

        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_PERCENT: return "PERCENT";
        case TOKEN_AMPERSAND: return "AMPERSAND";
        case TOKEN_PIPE: return "PIPE";
        case TOKEN_CARET: return "CARET";
        case TOKEN_TILDE: return "TILDE";
        case TOKEN_LEFT_SHIFT: return "LEFT_SHIFT";
        case TOKEN_RIGHT_SHIFT: return "RIGHT_SHIFT";
        case TOKEN_STAR_STAR: return "STAR_STAR";

        case TOKEN_PLUS_PLUS: return "PLUS_PLUS";
        case TOKEN_MINUS_MINUS: return "MINUS_MINUS";

        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_PLUS_EQUAL: return "PLUS_EQUAL";
        case TOKEN_MINUS_EQUAL: return "MINUS_EQUAL";
        case TOKEN_STAR_EQUAL: return "STAR_EQUAL";
        case TOKEN_SLASH_EQUAL: return "SLASH_EQUAL";
        case TOKEN_PERCENT_EQUAL: return "PERCENT_EQUAL";
        case TOKEN_STAR_STAR_EQUAL: return "STAR_STAR_EQUAL";
        case TOKEN_AMPERSAND_EQUAL: return "AMPERSAND_EQUAL";
        case TOKEN_PIPE_EQUAL: return "PIPE_EQUAL";
        case TOKEN_CARET_EQUAL: return "CARET_EQUAL";
        case TOKEN_LEFT_SHIFT_EQUAL: return "LEFT_SHIFT_EQUAL";
        case TOKEN_RIGHT_SHIFT_EQUAL: return "RIGHT_SHIFT_EQUAL";
        case TOKEN_LOGICAL_AND_EQUAL: return "LOGICAL_AND_EQUAL";
        case TOKEN_LOGICAL_OR_EQUAL: return "LOGICAL_OR_EQUAL";

        case TOKEN_EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TOKEN_BANG_EQUAL: return "BANG_EQUAL";
        case TOKEN_LESS: return "LESS";
        case TOKEN_GREATER: return "GREATER";
        case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";

        case TOKEN_LOGICAL_AND: return "LOGICAL_AND";
        case TOKEN_LOGICAL_OR: return "LOGICAL_OR";
        case TOKEN_BANG: return "BANG";
        case TOKEN_NULLISH_COALESCING: return "NULLISH_COALESCING";

        case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
        case TOKEN_LEFT_BRACKET: return "LEFT_BRACKET";
        case TOKEN_RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";

        case TOKEN_DOT: return "DOT";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COLON: return "COLON";
        case TOKEN_QUESTION: return "QUESTION";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_ELLIPSIS: return "ELLIPSIS";

        case TOKEN_EOF: return "EOF";
        case TOKEN_UNKNOWN: return "UNKNOWN";

        default: return "UNKNOWN";
    }
}
