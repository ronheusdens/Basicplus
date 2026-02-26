#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

/*
 * Parser context
 */
typedef struct
{
    Token *tokens;
    int num_tokens;
    int pos;
    int error_code;
    char *error_msg;
} Parser;

/* Parser functions */

Parser *parser_create(Token *tokens, int num_tokens);
void parser_free(Parser *parser);

Program *parse_program(Parser *parser);
ProgramLine *parse_line(Parser *parser);
ASTStmt *parse_statement(Parser *parser);
ASTExpr *parse_expression(Parser *parser);

int parser_has_error(Parser *parser);
const char *parser_error_message(Parser *parser);

#endif /* PARSER_H */
