#include "lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* Keyword table for recognition */
static const struct
{
    const char *name;
    TokenType type;
} keyword_table[] = {
    {"PRINT", TOK_PRINT},
    {"USING", TOK_USING},
    {"INPUT", TOK_INPUT},
    {"LET", TOK_LET},
    {"IF", TOK_IF},
    {"THEN", TOK_THEN},
    {"ELSE", TOK_ELSE},
    {"ON", TOK_ON},
    {"GOTO", TOK_GOTO},
    {"GOSUB", TOK_GOSUB},
    {"RETURN", TOK_RETURN},
    {"PROCEDURE", TOK_PROCEDURE},
    {"CLASS", TOK_CLASS},
    {"NEW", TOK_NEW},
    {"FOR", TOK_FOR},
    {"TO", TOK_TO},
    {"STEP", TOK_STEP},
    {"NEXT", TOK_NEXT},
    {"DIM", TOK_DIM},
    {"DATA", TOK_DATA},
    {"READ", TOK_READ},
    {"RESTORE", TOK_RESTORE},
    {"OPEN", TOK_OPEN},
    {"CLOSE", TOK_CLOSE},
    {"WRITE", TOK_WRITE},
    {"GET", TOK_GET},
    {"PUT", TOK_PUT},
    {"LINE", TOK_LINE},
    {"AS", TOK_AS},
    {"OUTPUT", TOK_OUTPUT},
    {"APPEND", TOK_APPEND},
    {"POKE", TOK_POKE},
    {"SAVE", TOK_SAVE},
    {"DELETE", TOK_DELETE},
    {"MERGE", TOK_MERGE},
    {"ERROR", TOK_ERROR},
    {"RESUME", TOK_RESUME},
    {"SLEEP", TOK_SLEEP},
    {"BEEP", TOK_BEEP},
    {"CLS", TOK_CLS},
    {"DEF", TOK_DEF},
    {"DEFINT", TOK_DEFINT},
    {"DEFSNG", TOK_DEFSNG},
    {"DEFDBL", TOK_DEFDBL},
    {"DEFSTR", TOK_DEFSTR},
    {"TRON", TOK_TRON},
    {"TROFF", TOK_TROFF},
    {"STOP", TOK_STOP},
    {"CONT", TOK_CONT},
    {"SOUND", TOK_SOUND},
    {"TAB", TOK_TAB},
    {"WHILE", TOK_WHILE},
    {"WEND", TOK_WEND},
    {"DO", TOK_DO},
    {"LOOP", TOK_LOOP},
    {"UNTIL", TOK_UNTIL},
    {"EXIT", TOK_EXIT},
    {"CLEAR", TOK_CLEAR},
    {"AT", TOK_AT},
    {"END", TOK_END},
    {"ENDIF", TOK_ENDIF},
    {"REM", TOK_REM},
    {"CASE", TOK_CASE},
    {"OF", TOK_OF},
    {"WHEN", TOK_WHEN},
    {"OTHERWISE", TOK_OTHERWISE},
    {"ENDCASE", TOK_ENDCASE},
    {"AND", TOK_AND},
    {"OR", TOK_OR},
    {"NOT", TOK_NOT},
    {"MOD", TOK_MOD},
    {NULL, TOK_UNKNOWN}};

/* Helper to check if string is a keyword */
static TokenType lookup_keyword(const char *str)
{
    for (int i = 0; keyword_table[i].name != NULL; i++)
    {
        if (strcasecmp(str, keyword_table[i].name) == 0)
        {
            return keyword_table[i].type;
        }
    }
    return TOK_IDENTIFIER;
}

/* Helper to add token to lexer */
static void add_token(Lexer *lexer, TokenType type, const char *value,
                      double num_value, const char *str_value,
                      int line, int col)
{
    if (lexer->num_tokens >= lexer->capacity)
    {
        lexer->capacity *= 2;
        lexer->tokens = xrealloc(lexer->tokens, lexer->capacity * sizeof(Token));
    }

    Token *tok = &lexer->tokens[lexer->num_tokens++];
    tok->type = type;
    tok->value = value ? xstrdup(value) : NULL;
    tok->num_value = num_value;
    tok->str_value = str_value ? xstrdup(str_value) : NULL;
    tok->line_number = line;
    tok->column_number = col;
}

/* Skip whitespace */
static void skip_whitespace(Lexer *lexer)
{
    while (lexer->input[lexer->pos] == ' ' || lexer->input[lexer->pos] == '\t')
    {
        lexer->pos++;
        lexer->column++;
    }
}

/* Lex a number */
static void lex_number(Lexer *lexer)
{
    int start_col = lexer->column;
    char buffer[64];
    int idx = 0;

    /* Integer part */
    while (isdigit(lexer->input[lexer->pos]) && idx < 63)
    {
        buffer[idx++] = lexer->input[lexer->pos++];
        lexer->column++;
    }

    /* Decimal part */
    if (lexer->input[lexer->pos] == '.')
    {
        buffer[idx++] = lexer->input[lexer->pos++];
        lexer->column++;

        while (isdigit(lexer->input[lexer->pos]) && idx < 63)
        {
            buffer[idx++] = lexer->input[lexer->pos++];
            lexer->column++;
        }
    }

    /* Exponent part */
    if ((lexer->input[lexer->pos] == 'e' || lexer->input[lexer->pos] == 'E') && idx < 60)
    {
        buffer[idx++] = lexer->input[lexer->pos++];
        lexer->column++;

        if (lexer->input[lexer->pos] == '+' || lexer->input[lexer->pos] == '-')
        {
            buffer[idx++] = lexer->input[lexer->pos++];
            lexer->column++;
        }

        while (isdigit(lexer->input[lexer->pos]) && idx < 63)
        {
            buffer[idx++] = lexer->input[lexer->pos++];
            lexer->column++;
        }
    }

    buffer[idx] = '\0';
    double num = strtod(buffer, NULL);

    add_token(lexer, TOK_NUMBER, buffer, num, NULL, lexer->line, start_col);
}

/* Lex a string literal */
static void lex_string(Lexer *lexer)
{
    int start_col = lexer->column;
    char buffer[512];
    int idx = 0;

    lexer->pos++; /* Skip opening quote */
    lexer->column++;

    while (lexer->input[lexer->pos] != '\0' &&
           lexer->input[lexer->pos] != '"' &&
           lexer->input[lexer->pos] != '\n' &&
           idx < 511)
    {
        buffer[idx++] = lexer->input[lexer->pos++];
        lexer->column++;
    }

    if (lexer->input[lexer->pos] == '"')
    {
        lexer->pos++; /* Skip closing quote */
        lexer->column++;
    }

    buffer[idx] = '\0';
    add_token(lexer, TOK_STRING, buffer, 0.0, buffer, lexer->line, start_col);
}

/* Lex an identifier or keyword */
static void lex_identifier(Lexer *lexer)
{
    int start_col = lexer->column;
    char buffer[128];
    int idx = 0;

    /* First character (letter or underscore) */
    while ((isalnum(lexer->input[lexer->pos]) ||
            lexer->input[lexer->pos] == '_' ||
            lexer->input[lexer->pos] == '$' ||
            lexer->input[lexer->pos] == '%' ||
            lexer->input[lexer->pos] == '!' ||
            lexer->input[lexer->pos] == '#') &&
           idx < 127)
    {
        buffer[idx++] = toupper(lexer->input[lexer->pos]);
        lexer->pos++;
        lexer->column++;
    }

    buffer[idx] = '\0';

    /* Check if it's a keyword */
    TokenType type = lookup_keyword(buffer);
    add_token(lexer, type, buffer, 0.0, NULL, lexer->line, start_col);
}

/* Main tokenization function */
Token *lexer_tokenize(Lexer *lexer)
{
    if (lexer == NULL || lexer->input == NULL)
    {
        return NULL;
    }

    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->num_tokens = 0;

    while (lexer->input[lexer->pos] != '\0')
    {
        char ch = lexer->input[lexer->pos];

        /* Skip whitespace */
        if (ch == ' ' || ch == '\t')
        {
            skip_whitespace(lexer);
            continue;
        }

        /* Newline */
        if (ch == '\n' || ch == '\r')
        {
            add_token(lexer, TOK_NEWLINE, "\\n", 0.0, NULL, lexer->line, lexer->column);
            lexer->pos++;
            if (ch == '\r' && lexer->input[lexer->pos] == '\n')
            {
                lexer->pos++; /* Skip \r\n */
            }
            lexer->line++;
            lexer->column = 1;
            continue;
        }

        /* Numbers */
        if (isdigit(ch))
        {
            lex_number(lexer);
            continue;
        }

        /* String literals */
        if (ch == '"')
        {
            lex_string(lexer);
            continue;
        }

        /* Identifiers and keywords */
        if (isalpha(ch) || ch == '_')
        {
            lex_identifier(lexer);
            continue;
        }

        /* Operators and punctuation */
        int start_col = lexer->column;

        switch (ch)
        {
        case '+':
            add_token(lexer, TOK_PLUS, "+", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '-':
            add_token(lexer, TOK_MINUS, "-", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '*':
            add_token(lexer, TOK_STAR, "*", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '/':
            add_token(lexer, TOK_SLASH, "/", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '^':
            add_token(lexer, TOK_CARET, "^", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '=':
            add_token(lexer, TOK_EQ, "=", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '<':
            lexer->pos++;
            lexer->column++;
            if (lexer->input[lexer->pos] == '=')
            {
                add_token(lexer, TOK_LE, "<=", 0.0, NULL, lexer->line, start_col);
                lexer->pos++;
                lexer->column++;
            }
            else if (lexer->input[lexer->pos] == '>')
            {
                add_token(lexer, TOK_NE, "<>", 0.0, NULL, lexer->line, start_col);
                lexer->pos++;
                lexer->column++;
            }
            else
            {
                add_token(lexer, TOK_LT, "<", 0.0, NULL, lexer->line, start_col);
            }
            break;

        case '>':
            lexer->pos++;
            lexer->column++;
            if (lexer->input[lexer->pos] == '=')
            {
                add_token(lexer, TOK_GE, ">=", 0.0, NULL, lexer->line, start_col);
                lexer->pos++;
                lexer->column++;
            }
            else
            {
                add_token(lexer, TOK_GT, ">", 0.0, NULL, lexer->line, start_col);
            }
            break;

        case '(':
            add_token(lexer, TOK_LPAREN, "(", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case ')':
            add_token(lexer, TOK_RPAREN, ")", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case ',':
            add_token(lexer, TOK_COMMA, ",", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case ';':
            add_token(lexer, TOK_SEMICOLON, ";", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case ':':
            add_token(lexer, TOK_COLON, ":", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '#':
            add_token(lexer, TOK_HASH, "#", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '@':
            add_token(lexer, TOK_AT, "@", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '.':
            add_token(lexer, TOK_DOT, ".", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '?':
            add_token(lexer, TOK_QUESTION, "?", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '$':
            add_token(lexer, TOK_DOLLAR, "$", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '%':
            add_token(lexer, TOK_PERCENT, "%", 0.0, NULL, lexer->line, start_col);
            lexer->pos++;
            lexer->column++;
            break;

        case '\'':
            /* Apostrophe as comment - skip to end of line */
            while (lexer->input[lexer->pos] != '\0' &&
                   lexer->input[lexer->pos] != '\n' &&
                   lexer->input[lexer->pos] != '\r')
            {
                lexer->pos++;
                lexer->column++;
            }
            break;

        default:
            /* Unknown character - skip it */
            lexer->pos++;
            lexer->column++;
            break;
        }
    }

    /* Add EOF token */
    add_token(lexer, TOK_EOF, NULL, 0.0, NULL, lexer->line, lexer->column);

    return lexer->tokens;
}

/* Create lexer */
Lexer *lexer_create(const char *input)
{
    Lexer *lexer = xcalloc(1, sizeof(Lexer));
    lexer->input = input;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->capacity = 1024;
    lexer->tokens = xmalloc(lexer->capacity * sizeof(Token));
    lexer->num_tokens = 0;
    return lexer;
}

/* Free lexer */
void lexer_free(Lexer *lexer)
{
    if (lexer == NULL)
    {
        return;
    }
    if (lexer->tokens != NULL)
    {
        for (int i = 0; i < lexer->num_tokens; i++)
        {
            if (lexer->tokens[i].value != NULL)
            {
                free(lexer->tokens[i].value);
            }
            if (lexer->tokens[i].str_value != NULL)
            {
                free(lexer->tokens[i].str_value);
            }
        }
        free(lexer->tokens);
    }
    free(lexer);
}

/* Get token count */
int lexer_token_count(Lexer *lexer)
{
    return lexer ? lexer->num_tokens : 0;
}

/* Peek at current token */
Token lexer_peek(Lexer *lexer)
{
    if (lexer == NULL || lexer->pos >= lexer->num_tokens)
    {
        Token eof_token = {TOK_EOF, NULL, 0.0, NULL, 0, 0};
        return eof_token;
    }
    return lexer->tokens[lexer->pos];
}

/* Get next token */
Token lexer_next(Lexer *lexer)
{
    if (lexer == NULL || lexer->pos >= lexer->num_tokens)
    {
        Token eof_token = {TOK_EOF, NULL, 0.0, NULL, 0, 0};
        return eof_token;
    }
    return lexer->tokens[lexer->pos++];
}

/* Get current token */
Token lexer_current(Lexer *lexer)
{
    if (lexer == NULL || lexer->pos == 0 || lexer->pos > lexer->num_tokens)
    {
        Token eof_token = {TOK_EOF, NULL, 0.0, NULL, 0, 0};
        return eof_token;
    }
    return lexer->tokens[lexer->pos - 1];
}

/* Get token type name */
const char *token_type_name(TokenType type)
{
    switch (type)
    {
    case TOK_EOF:
        return "EOF";
    case TOK_NUMBER:
        return "NUMBER";
    case TOK_STRING:
        return "STRING";
    case TOK_IDENTIFIER:
        return "IDENTIFIER";
    case TOK_KEYWORD:
        return "KEYWORD";
    case TOK_PLUS:
        return "PLUS";
    case TOK_MINUS:
        return "MINUS";
    case TOK_STAR:
        return "STAR";
    case TOK_SLASH:
        return "SLASH";
    case TOK_CARET:
        return "CARET";
    case TOK_MOD:
        return "MOD";
    case TOK_EQ:
        return "EQ";
    case TOK_NE:
        return "NE";
    case TOK_LT:
        return "LT";
    case TOK_LE:
        return "LE";
    case TOK_GT:
        return "GT";
    case TOK_GE:
        return "GE";
    case TOK_AND:
        return "AND";
    case TOK_OR:
        return "OR";
    case TOK_NOT:
        return "NOT";
    case TOK_LPAREN:
        return "LPAREN";
    case TOK_RPAREN:
        return "RPAREN";
    case TOK_COMMA:
        return "COMMA";
    case TOK_SEMICOLON:
        return "SEMICOLON";
    case TOK_COLON:
        return "COLON";
    case TOK_HASH:
        return "HASH";
    case TOK_AT:
        return "AT";
    case TOK_QUESTION:
        return "QUESTION";
    case TOK_DOLLAR:
        return "DOLLAR";
    case TOK_PERCENT:
        return "PERCENT";
    case TOK_PRINT:
        return "PRINT";
    case TOK_INPUT:
        return "INPUT";
    case TOK_LET:
        return "LET";
    case TOK_IF:
        return "IF";
    case TOK_THEN:
        return "THEN";
    case TOK_ELSE:
        return "ELSE";
    case TOK_ON:
        return "ON";
    case TOK_GOTO:
        return "GOTO";
    case TOK_GOSUB:
        return "GOSUB";
    case TOK_RETURN:
        return "RETURN";
    case TOK_FOR:
        return "FOR";
    case TOK_TO:
        return "TO";
    case TOK_STEP:
        return "STEP";
    case TOK_NEXT:
        return "NEXT";
    case TOK_DIM:
        return "DIM";
    case TOK_DATA:
        return "DATA";
    case TOK_READ:
        return "READ";
    case TOK_RESTORE:
        return "RESTORE";
    case TOK_OPEN:
        return "OPEN";
    case TOK_CLOSE:
        return "CLOSE";
    case TOK_WRITE:
        return "WRITE";
    case TOK_GET:
        return "GET";
    case TOK_PUT:
        return "PUT";
    case TOK_LINE:
        return "LINE";
    case TOK_AS:
        return "AS";
    case TOK_OUTPUT:
        return "OUTPUT";
    case TOK_APPEND:
        return "APPEND";
    case TOK_POKE:
        return "POKE";
    case TOK_ERROR:
        return "ERROR";
    case TOK_RESUME:
        return "RESUME";
    case TOK_DEFINT:
        return "DEFINT";
    case TOK_DEFSNG:
        return "DEFSNG";
    case TOK_DEFDBL:
        return "DEFDBL";
    case TOK_DEFSTR:
        return "DEFSTR";
    case TOK_END:
        return "END";
    case TOK_ENDIF:
        return "ENDIF";
    case TOK_REM:
        return "REM";
    case TOK_WHILE:
        return "WHILE";
    case TOK_WEND:
        return "WEND";
    case TOK_DO:
        return "DO";
    case TOK_LOOP:
        return "LOOP";
    case TOK_UNTIL:
        return "UNTIL";
    case TOK_EXIT:
        return "EXIT";
    case TOK_SAVE:
        return "SAVE";
    case TOK_CLEAR:
        return "CLEAR";
    case TOK_DELETE:
        return "DELETE";
    case TOK_MERGE:
        return "MERGE";
    case TOK_SLEEP:
        return "SLEEP";
    case TOK_BEEP:
        return "BEEP";
    case TOK_CLS:
        return "CLS";
    case TOK_TRON:
        return "TRON";
    case TOK_TROFF:
        return "TROFF";
    case TOK_STOP:
        return "STOP";
    case TOK_CONT:
        return "CONT";
    case TOK_SOUND:
        return "SOUND";
    case TOK_TAB:
        return "TAB";
    case TOK_DEF:
        return "DEF";
    case TOK_PROCEDURE:
        return "PROCEDURE";
    case TOK_CLASS:
        return "CLASS";
    case TOK_NEW:
        return "NEW";
    case TOK_DOT:
        return "DOT";
    case TOK_NEWLINE:
        return "NEWLINE";
    default:
        return "UNKNOWN";
    }
}
