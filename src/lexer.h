#ifndef LEXER_H
#define LEXER_H

#include "common.h"

/*
 * Token type enumeration
 */
typedef enum
{
    TOK_EOF = 0,
    TOK_NUMBER,
    TOK_STRING,
    TOK_IDENTIFIER,
    TOK_KEYWORD,

    /* Operators */
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_CARET,
    TOK_MOD,
    TOK_PERCENT,

    /* Comparison */
    TOK_EQ,
    TOK_NE,
    TOK_LT,
    TOK_LE,
    TOK_GT,
    TOK_GE,

    /* Logical */
    TOK_AND,
    TOK_OR,
    TOK_NOT,

    /* Delimiters */
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_COLON,
    TOK_DOLLAR,
    TOK_HASH,
    TOK_AMPERSAND,
    TOK_AT,
    TOK_QUESTION,
    TOK_EQUAL,
    TOK_LESS,
    TOK_GREATER,
    TOK_LESSEQUAL,
    TOK_GREATEREQUAL,
    TOK_NOTEQUAL,

    /* Keywords */
    TOK_PRINT,
    TOK_USING,
    TOK_INPUT,
    TOK_LET,
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_ON,
    TOK_GOTO,
    TOK_GOSUB,
    TOK_RETURN,
    TOK_PROCEDURE,
    TOK_CLASS,
    TOK_NEW,
    TOK_DOT,
    TOK_FOR,
    TOK_TO,
    TOK_STEP,
    TOK_NEXT,
    TOK_DIM,
    TOK_DATA,
    TOK_READ,
    TOK_RESTORE,
    TOK_OPEN,
    TOK_CLOSE,
    TOK_WRITE,
    TOK_GET,
    TOK_PUT,
    TOK_LINE,
    TOK_AS,
    TOK_OUTPUT,
    TOK_APPEND,
    TOK_POKE,
    TOK_ERROR,
    TOK_RESUME,
    TOK_SLEEP,
    TOK_BEEP,
    TOK_CLS,
    TOK_DEF,
    TOK_FN,
    TOK_DEFINT,
    TOK_DEFSNG,
    TOK_DEFDBL,
    TOK_DEFSTR,
    TOK_TRON,
    TOK_TROFF,
    TOK_STOP,
    TOK_CONT,
    TOK_SOUND,
    TOK_TAB,
    TOK_WHILE,
    TOK_WEND,
    TOK_DO,
    TOK_LOOP,
    TOK_UNTIL,
    TOK_EXIT,
    TOK_SAVE,
    TOK_CLEAR,
    TOK_DELETE,
    TOK_MERGE,
    TOK_END,
    TOK_ENDIF,
    TOK_REM,
    TOK_CASE,
    TOK_OF,
    TOK_WHEN,
    TOK_OTHERWISE,
    TOK_ENDCASE,

    /* Special */
    TOK_NEWLINE,
    TOK_UNKNOWN,
    TOK_WOB,
    TOK_BOW
} TokenType;

/*
 * Token structure
 */
typedef struct
{
    TokenType type;
    char *value;      /* String value for identifiers, strings */
    double num_value; /* Numeric value for numbers */
    char *str_value;  /* String content for string literals */
    int line_number;
    int column_number;
} Token;

/*
 * Lexer context
 */
typedef struct
{
    const char *input;
    int pos;
    int line;
    int column;
    Token *tokens;
    int num_tokens;
    int capacity;
} Lexer;

/* Lexer functions */

Lexer *lexer_create(const char *input);
void lexer_free(Lexer *lexer);
Token *lexer_tokenize(Lexer *lexer);
int lexer_token_count(Lexer *lexer);

Token lexer_peek(Lexer *lexer);
Token lexer_next(Lexer *lexer);
Token lexer_current(Lexer *lexer);

const char *token_type_name(TokenType type);

#endif /* LEXER_H */
