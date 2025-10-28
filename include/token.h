
#ifndef __TOKEN_H
#define __TOKEN_H

#include <stddef.h>

#include "location.h"

typedef enum token_type
{
    // keywords
    TOKEN_IF = 0,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_FOR,
    TOKEN_FUNCTION,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_LET,
    TOKEN_CONST,
    TOKEN_VAR,
    TOKEN_NULL,
    TOKEN_UNDEFINED,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_IMPORT,
    TOKEN_EXPORT,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_FINALLY,
    TOKEN_THROW,
    TOKEN_TYPEOF,
    TOKEN_ASYNC,
    TOKEN_AWAIT,
    // TOKEN_IN,
    // TOKEN_OF,
    TOKEN_FROM,
    TOKEN_DELETE,
    TOKEN_THIS,
    TOKEN_VOID,
    TOKEN_NEW,
    TOKEN_DEBUGGER,

	// literals
	TOKEN_IDENTIFIER,
	TOKEN_BOOL_LITERAL,
	TOKEN_STRING_LITERAL,
	TOKEN_NUMBER_LITERAL,

	// operators
	TOKEN_PLUS,         // +
	TOKEN_MINUS,        // -
	TOKEN_STAR,         // *
	TOKEN_SLASH,        // /
	TOKEN_PERCENT,      // %
	TOKEN_AMPERSAND,    // &
    TOKEN_PIPE,         // |
    TOKEN_CARET,        // ^
    TOKEN_TILDE,        // ~
    TOKEN_LEFT_SHIFT,   // <<
    TOKEN_RIGHT_SHIFT,  // >>
    TOKEN_STAR_STAR,    // **

    TOKEN_PLUS_PLUS,    // ++
    TOKEN_MINUS_MINUS,  // --

    TOKEN_EQUAL,            // =
    TOKEN_PLUS_EQUAL,       // +=
    TOKEN_MINUS_EQUAL,      // -=
    TOKEN_STAR_EQUAL,       // *=
    TOKEN_SLASH_EQUAL,      // /=
    TOKEN_PERCENT_EQUAL,    // %=
    TOKEN_STAR_STAR_EQUAL,  // **=
    TOKEN_AMPERSAND_EQUAL,  // &=
    TOKEN_PIPE_EQUAL,       // |=
    TOKEN_CARET_EQUAL,      // ^=
    TOKEN_LEFT_SHIFT_EQUAL, // <<=
    TOKEN_RIGHT_SHIFT_EQUAL,// >>=
    TOKEN_LOGICAL_AND_EQUAL,// &&=
    TOKEN_LOGICAL_OR_EQUAL, // ||=

    // comparison
    TOKEN_EQUAL_EQUAL,      // ==
    TOKEN_BANG_EQUAL,       // !=
    TOKEN_LESS,             // <
    TOKEN_GREATER,          // >
    TOKEN_LESS_EQUAL,       // <=
    TOKEN_GREATER_EQUAL,    // >=

    // logical
    TOKEN_LOGICAL_AND,  // &&
    TOKEN_LOGICAL_OR,   // ||
    TOKEN_BANG,         // !
    // TOKEN_NULLISH_COALESCING,   // ??

    // grouping
    TOKEN_LEFT_PAREN,       // (
    TOKEN_RIGHT_PAREN,      // )
    TOKEN_LEFT_BRACKET,     // [
    TOKEN_RIGHT_BRACKET,    // ]
    TOKEN_LEFT_BRACE,       // {
    TOKEN_RIGHT_BRACE,      // }

    // punctuation
    TOKEN_DOT,          // .
    TOKEN_COMMA,        // ,
    TOKEN_SEMICOLON,    // ;
    TOKEN_COLON,        // :
    TOKEN_QUESTION,     // ?
    TOKEN_ARROW,        // =>
    TOKEN_ELLIPSIS,     // ...

    // special
    TOKEN_EOF,  
    TOKEN_UNKNOWN,

	TOKEN_COUNT,
} token_type_t;


typedef struct token
{
    char *value;
    size_t length;
    token_type_t type;
    
    // size_t line;
    // size_t column;
    location_t loc;
} token_t;

const char *token_type_to_string(token_type_t type);

#endif /* !__TOKEN_H */
