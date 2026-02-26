#include "parser.h"
#include "compat.h"
#include "common.h"
#include "eval.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>

/* Forward declarations */
static Token *current_token(Parser *parser);
static void advance(Parser *parser);
static int match(Parser *parser, TokenType type);
static Token *peek_next_token(Parser *parser);
static int expect(Parser *parser, TokenType type, const char *msg);
static void parser_error(Parser *parser, const char *msg);

/* Expression parsing */
static ASTExpr *parse_or_expr(Parser *parser);
static ASTExpr *parse_and_expr(Parser *parser);
static ASTExpr *parse_not_expr(Parser *parser);
static ASTExpr *parse_relational_expr(Parser *parser);
static ASTExpr *parse_additive_expr(Parser *parser);
static ASTExpr *parse_multiplicative_expr(Parser *parser);
static ASTExpr *parse_power_expr(Parser *parser);
static ASTExpr *parse_unary_expr(Parser *parser);
static ASTExpr *parse_primary_expr(Parser *parser);

static int is_builtin_function(const char *name)
{
    if (!name)
        return 0;
    const char *funcs[] = {
        "ABS", "SIN", "COS", "TAN", "ATN", "EXP", "LOG", "LN", "SQR", "INT", "SGN", "RND",
        "VAL", "ASC", "LEN", "CHR$", "STR$", "LEFT$", "RIGHT$", "MID$", "STRING$", "SPACE$", "INKEY$",
        "EOF", "PEEK", "FRE", "POS", "LOC", "LOF", "VARPTR", "USR", "GETA", "GETB", "POINT", "INSTR", NULL};
    for (int i = 0; funcs[i]; i++)
    {
        if (strcasecmp(name, funcs[i]) == 0)
            return 1;
    }
    return 0;
}

/* Statement parsing */
static ASTStmt *parse_print_stmt(Parser *parser);
static ASTStmt *parse_input_stmt(Parser *parser);
static ASTStmt *parse_line_input_stmt(Parser *parser);
static ASTStmt *parse_let_stmt(Parser *parser);
static ASTStmt *parse_if_stmt(Parser *parser);
static ASTStmt *parse_on_stmt(Parser *parser);
static ASTStmt *parse_goto_stmt(Parser *parser);
static ASTStmt *parse_gosub_stmt(Parser *parser);
static ASTStmt *parse_return_stmt(Parser *parser);
static ASTStmt *parse_error_stmt(Parser *parser);
static ASTStmt *parse_resume_stmt(Parser *parser);
static ASTStmt *parse_sleep_stmt(Parser *parser);
static ASTStmt *parse_beep_stmt(Parser *parser);
static ASTStmt *parse_cls_stmt(Parser *parser);
static ASTStmt *parse_clear_stmt(Parser *parser);
static ASTStmt *parse_for_stmt(Parser *parser);
static ASTStmt *parse_next_stmt(Parser *parser);
static ASTStmt *parse_while_stmt(Parser *parser);
static ASTStmt *parse_wend_stmt(Parser *parser);
static ASTStmt *parse_do_loop_stmt(Parser *parser);
static ASTStmt *parse_loop_stmt(Parser *parser);
static ASTStmt *parse_exit_stmt(Parser *parser);
static ASTStmt *parse_dim_stmt(Parser *parser);
static ASTStmt *parse_read_stmt(Parser *parser);
static ASTStmt *parse_data_stmt(Parser *parser);
static ASTStmt *parse_restore_stmt(Parser *parser);
static ASTStmt *parse_open_stmt(Parser *parser);
static ASTStmt *parse_close_stmt(Parser *parser);
static ASTStmt *parse_write_stmt(Parser *parser);
static ASTStmt *parse_get_stmt(Parser *parser);
static ASTStmt *parse_put_stmt(Parser *parser);
static ASTStmt *parse_poke_stmt(Parser *parser);
static ASTStmt *parse_save_stmt(Parser *parser);
static ASTStmt *parse_delete_stmt(Parser *parser);
static ASTStmt *parse_merge_stmt(Parser *parser);
static ASTStmt *parse_end_stmt(Parser *parser);
static ASTStmt *parse_rem_stmt(Parser *parser);
static ASTStmt *parse_def_stmt(Parser *parser, StmtType type);
static ASTStmt *parse_def_fn_stmt(Parser *parser);
static ASTStmt *parse_color_stmt(Parser *parser);
static ASTStmt *parse_pcolor_stmt(Parser *parser);
static ASTStmt *parse_set_stmt(Parser *parser);
static ASTStmt *parse_reset_stmt(Parser *parser);
static ASTStmt *parse_line_stmt(Parser *parser);
static ASTStmt *parse_circle_stmt(Parser *parser);
static ASTStmt *parse_paint_stmt(Parser *parser);
static ASTStmt *parse_screen_stmt(Parser *parser);
static ASTStmt *parse_case_stmt(Parser *parser);
static ASTStmt *parse_sound_stmt(Parser *parser);

Parser *parser_create(Token *tokens, int num_tokens)
{
    Parser *parser = xcalloc(1, sizeof(Parser));
    parser->tokens = tokens;
    parser->num_tokens = num_tokens;
    parser->pos = 0;
    parser->error_code = 0;
    parser->error_msg = NULL;
    return parser;
}

void parser_free(Parser *parser)
{
    if (parser == NULL)
    {
        return;
    }
    if (parser->error_msg != NULL)
    {
        free(parser->error_msg);
    }
    free(parser);
}

/* Token utilities */

static Token *current_token(Parser *parser)
{
    if (parser->pos >= parser->num_tokens)
    {
        return NULL;
    }
    return &parser->tokens[parser->pos];
}

static void advance(Parser *parser)
{
    if (parser->pos < parser->num_tokens)
    {
        parser->pos++;
    }
}

static Token *peek_next_token(Parser *parser)
{
    if (parser->pos + 1 >= parser->num_tokens)
    {
        return NULL;
    }
    return &parser->tokens[parser->pos + 1];
}

static int match(Parser *parser, TokenType type)
{
    Token *tok = current_token(parser);
    if (tok && tok->type == type)
    {
        advance(parser);
        return 1;
    }
    return 0;
}

static int expect(Parser *parser, TokenType type, const char *msg)
{
    if (match(parser, type))
    {
        return 1;
    }
    parser_error(parser, msg);
    return 0;
}

static void parser_error(Parser *parser, const char *msg)
{
    parser->error_code = 1;
    if (parser->error_msg)
    {
        free(parser->error_msg);
    }

    Token *tok = current_token(parser);
    if (tok && tok->line_number > 0)
    {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "%s (line %d)", msg, tok->line_number);
        parser->error_msg = xstrdup(buffer);
    }
    else
    {
        parser->error_msg = xstrdup(msg);
    }
}

/* Program parsing */

Program *parse_program(Parser *parser)
{
    if (parser == NULL)
    {
        return NULL;
    }

    Program *prog = ast_program_create();

    while (parser->pos < parser->num_tokens && !parser_has_error(parser))
    {
        Token *tok = current_token(parser);
        if (!tok)
        {
            break;
        }

        /* Skip leading newlines and EOF */
        if (tok->type == TOK_NEWLINE || tok->type == TOK_EOF)
        {
            advance(parser);
            continue;
        }

        ProgramLine *line = parse_line(parser);
        if (line)
        {
            ast_program_add_line(prog, line);
        }
        else
        {
            /* If we couldn't parse a line, advance to avoid infinite loop */
            if (parser->pos < parser->num_tokens)
            {
                advance(parser);
            }
        }

        /* Skip newlines between lines */
        while (match(parser, TOK_NEWLINE))
            ;
    }

    return prog;
}

ProgramLine *parse_line(Parser *parser)
{
    Token *tok = current_token(parser);
    if (!tok || tok->type != TOK_NUMBER)
    {
        return NULL; /* Not a line number, skip */
    }

    int line_num = ((int)tok->num_value);
    advance(parser);

    // Skip any ENDIF tokens after line number (directive, not statement)
    while (current_token(parser) && current_token(parser)->type == TOK_ENDIF)
    {
        advance(parser);
        // Optionally consume trailing newline
        if (current_token(parser) && current_token(parser)->type == TOK_NEWLINE)
            advance(parser);
        // If ENDIF is the only thing on this line, treat as REM/no-op
        if (!current_token(parser) || current_token(parser)->type == TOK_NUMBER || current_token(parser)->type == TOK_EOF)
        {
            ASTStmt *empty = ast_stmt_create(STMT_REM);
            return ast_program_line_create(line_num, empty);
        }
    }

    if (current_token(parser) && current_token(parser)->type == TOK_NEWLINE)
    {
        advance(parser);
        ASTStmt *empty = ast_stmt_create(STMT_REM);
        return ast_program_line_create(line_num, empty);
    }

    ASTStmt *stmt = parse_statement(parser);
    if (!stmt)
    {
        return NULL;
    }

    ASTStmt *tail = stmt;
    while (match(parser, TOK_COLON))
    {
        ASTStmt *next_stmt = parse_statement(parser);
        if (!next_stmt)
        {
            break;
        }
        tail->next = next_stmt;
        tail = next_stmt;
    }

    /* Consume trailing newline if present */
    match(parser, TOK_NEWLINE);

    return ast_program_line_create(line_num, stmt);
}

ASTStmt *parse_statement(Parser *parser)
{
    Token *tok = current_token(parser);
    if (!tok)
    {
        return NULL;
    }

    switch (tok->type)
    {
    case TOK_PRINT:
        return parse_print_stmt(parser);
    case TOK_INPUT:
        return parse_input_stmt(parser);
    case TOK_LINE:
    {
        Token *next = peek_next_token(parser);
        if (next && next->type == TOK_INPUT)
        {
            return parse_line_input_stmt(parser);
        }
        return parse_line_stmt(parser);
    }
    case TOK_LET:
        return parse_let_stmt(parser);
    case TOK_IF:
        return parse_if_stmt(parser);
    case TOK_ON:
        return parse_on_stmt(parser);
    case TOK_ERROR:
        return parse_error_stmt(parser);
    case TOK_RESUME:
        return parse_resume_stmt(parser);
    case TOK_SLEEP:
        return parse_sleep_stmt(parser);
    case TOK_BEEP:
        return parse_beep_stmt(parser);
    case TOK_CLS:
        return parse_cls_stmt(parser);
    case TOK_CLEAR:
        return parse_clear_stmt(parser);
    case TOK_SAVE:
        return parse_save_stmt(parser);
    case TOK_DELETE:
        return parse_delete_stmt(parser);
    case TOK_MERGE:
        return parse_merge_stmt(parser);
    case TOK_GOTO:
        return parse_goto_stmt(parser);
    case TOK_GOSUB:
        return parse_gosub_stmt(parser);
    case TOK_RETURN:
        return parse_return_stmt(parser);
    case TOK_FOR:
        return parse_for_stmt(parser);
    case TOK_NEXT:
        return parse_next_stmt(parser);
    case TOK_WHILE:
        return parse_while_stmt(parser);
    case TOK_WEND:
        return parse_wend_stmt(parser);
    case TOK_DIM:
        return parse_dim_stmt(parser);
    case TOK_READ:
        return parse_read_stmt(parser);
    case TOK_DATA:
        return parse_data_stmt(parser);
    case TOK_RESTORE:
        return parse_restore_stmt(parser);
    case TOK_OPEN:
        return parse_open_stmt(parser);
    case TOK_CLOSE:
        return parse_close_stmt(parser);
    case TOK_WRITE:
        return parse_write_stmt(parser);
    case TOK_GET:
        return parse_get_stmt(parser);
    case TOK_PUT:
        return parse_put_stmt(parser);
    case TOK_POKE:
        return parse_poke_stmt(parser);
    case TOK_END:
        return parse_end_stmt(parser);
    case TOK_REM:
        return parse_rem_stmt(parser);
    case TOK_DEF:
        return parse_def_fn_stmt(parser);
    case TOK_DEFINT:
        return parse_def_stmt(parser, STMT_DEFINT);
    case TOK_DEFSNG:
        return parse_def_stmt(parser, STMT_DEFSNG);
    case TOK_DEFDBL:
        return parse_def_stmt(parser, STMT_DEFDBL);
    case TOK_DEFSTR:
        return parse_def_stmt(parser, STMT_DEFSTR);
    case TOK_TRON:
        advance(parser); /* consume TRON */
        return ast_stmt_create(STMT_TRON);
    case TOK_TROFF:
        advance(parser); /* consume TROFF */
        return ast_stmt_create(STMT_TROFF);
    case TOK_COLOR:
        return parse_color_stmt(parser);
    case TOK_PCOLOR:
        return parse_pcolor_stmt(parser);
    case TOK_SET:
        return parse_set_stmt(parser);
    case TOK_RESET:
        return parse_reset_stmt(parser);
    case TOK_CIRCLE:
        return parse_circle_stmt(parser);
    case TOK_PAINT:
        return parse_paint_stmt(parser);
    case TOK_SCREEN:
        return parse_screen_stmt(parser);
    case TOK_CASE:
        return parse_case_stmt(parser);
    case TOK_STOP:
        advance(parser); /* consume STOP */
        return ast_stmt_create(STMT_STOP);
    case TOK_CONT:
        advance(parser); /* consume CONT */
        return ast_stmt_create(STMT_CONT);
    case TOK_DO:
        return parse_do_loop_stmt(parser);
    case TOK_LOOP:
        return parse_loop_stmt(parser);
    case TOK_EXIT:
        return parse_exit_stmt(parser);
    case TOK_SOUND:
        return parse_sound_stmt(parser);
    case TOK_IDENTIFIER:
        /* Implicit LET: A=5 instead of LET A=5 */
        return parse_let_stmt(parser);
    case TOK_ENDIF:
        /* Ignore ENDIF as standalone statement (no-op) */
        advance(parser);
        return ast_stmt_create(STMT_REM); /* treat as comment/no-op */
    default:
        parser_error(parser, "Unknown statement");
        return NULL;
    }
}

/* Statement implementations */
// Stub implementations for CoCo graphics/color statements
static ASTStmt *parse_color_stmt(Parser *parser)
{
    advance(parser); // consume COLOR
    ASTStmt *stmt = ast_stmt_create(STMT_COLOR);
    // Parse: COLOR foreground [, background]
    ASTExpr *foreground = parse_expression(parser);
    if (foreground)
        ast_stmt_add_expr(stmt, foreground);
    if (match(parser, TOK_COMMA))
    {
        ASTExpr *background = parse_expression(parser);
        if (background)
            ast_stmt_add_expr(stmt, background);
    }
    return stmt;
}

static ASTStmt *parse_pcolor_stmt(Parser *parser)
{
    advance(parser); // consume PCOLOR
    ASTStmt *stmt = ast_stmt_create(STMT_PCOLOR);
    // Parse: PCOLOR palette_index
    ASTExpr *palette = parse_expression(parser);
    if (palette)
        ast_stmt_add_expr(stmt, palette);
    return stmt;
}

static ASTStmt *parse_set_stmt(Parser *parser)
{
    advance(parser); // consume SET
    ASTStmt *stmt = ast_stmt_create(STMT_SET);
    // Parse: SET x, y [, color]
    ASTExpr *x = parse_expression(parser);
    if (x)
        ast_stmt_add_expr(stmt, x);
    if (match(parser, TOK_COMMA))
    {
        ASTExpr *y = parse_expression(parser);
        if (y)
            ast_stmt_add_expr(stmt, y);
        if (match(parser, TOK_COMMA))
        {
            ASTExpr *color = parse_expression(parser);
            if (color)
                ast_stmt_add_expr(stmt, color);
        }
    }
    return stmt;
}

static ASTStmt *parse_reset_stmt(Parser *parser)
{
    advance(parser); // consume RESET
    ASTStmt *stmt = ast_stmt_create(STMT_RESET);
    // Parse: RESET x, y
    ASTExpr *x = parse_expression(parser);
    if (x)
        ast_stmt_add_expr(stmt, x);
    if (match(parser, TOK_COMMA))
    {
        ASTExpr *y = parse_expression(parser);
        if (y)
            ast_stmt_add_expr(stmt, y);
    }
    return stmt;
}

static ASTStmt *parse_line_stmt(Parser *parser)
{
    advance(parser); /* consume LINE */
    ASTStmt *stmt = ast_stmt_create(STMT_LINE);
    /* Parse: LINE x1, y1, x2, y2 [, color] or polygon: LINE x1,y1, x2,y2, ... */
    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE &&
           current_token(parser)->type != TOK_COLON && current_token(parser)->type != TOK_EOF)
    {
        if (match(parser, TOK_COMMA))
        {
            continue;
        }
        ASTExpr *expr = parse_expression(parser);
        if (expr)
        {
            ast_stmt_add_expr(stmt, expr);
        }
        else
        {
            break;
        }
        if (!match(parser, TOK_COMMA))
        {
            break;
        }
    }
    return stmt;
}

static ASTStmt *parse_circle_stmt(Parser *parser)
{
    advance(parser); // consume CIRCLE
    ASTStmt *stmt = ast_stmt_create(STMT_CIRCLE);
    // Parse: CIRCLE x, y, radius [, color]
    ASTExpr *x = parse_expression(parser);
    if (x)
        ast_stmt_add_expr(stmt, x);
    if (match(parser, TOK_COMMA))
    {
        ASTExpr *y = parse_expression(parser);
        if (y)
            ast_stmt_add_expr(stmt, y);
        if (match(parser, TOK_COMMA))
        {
            ASTExpr *radius = parse_expression(parser);
            if (radius)
                ast_stmt_add_expr(stmt, radius);
            if (match(parser, TOK_COMMA))
            {
                ASTExpr *color = parse_expression(parser);
                if (color)
                    ast_stmt_add_expr(stmt, color);
            }
        }
    }
    return stmt;
}

static ASTStmt *parse_paint_stmt(Parser *parser)
{
    advance(parser); // consume PAINT
    ASTStmt *stmt = ast_stmt_create(STMT_PAINT);
    // Parse: PAINT x, y [, color]
    ASTExpr *x = parse_expression(parser);
    if (x)
        ast_stmt_add_expr(stmt, x);
    if (match(parser, TOK_COMMA))
    {
        ASTExpr *y = parse_expression(parser);
        if (y)
            ast_stmt_add_expr(stmt, y);
        if (match(parser, TOK_COMMA))
        {
            ASTExpr *color = parse_expression(parser);
            if (color)
                ast_stmt_add_expr(stmt, color);
        }
    }
    return stmt;
}

static ASTStmt *parse_screen_stmt(Parser *parser)
{
    advance(parser); // consume SCREEN
    ASTStmt *stmt = ast_stmt_create(STMT_SCREEN);
    // Parse: SCREEN mode
    ASTExpr *mode = parse_expression(parser);
    if (mode)
        ast_stmt_add_expr(stmt, mode);
    return stmt;
}

static ASTStmt *parse_case_stmt(Parser *parser)
{
    advance(parser); /* consume CASE */

    /* Parse: CASE expression OF */
    ASTExpr *case_expr = parse_expression(parser);
    if (!case_expr)
    {
        parser_error(parser, "Expected expression after CASE");
        return NULL;
    }

    if (!match(parser, TOK_OF))
    {
        parser_error(parser, "Expected OF after CASE expression");
        ast_expr_free(case_expr);
        return NULL;
    }

    Token *tok = current_token(parser);

    /* Multi-line CASE/OF/WHEN/ENDCASE block (when OF is followed by NEWLINE) */
    if (tok && tok->type == TOK_NEWLINE)
    {
        advance(parser); /* consume NEWLINE after OF */

        ASTStmt *root_if = NULL;
        ASTStmt *current_else = NULL;
        ASTStmt *otherwise_body = NULL;

        while (1)
        {
            Token *ctok = current_token(parser);

            /* End of input */
            if (!ctok || ctok->type == TOK_EOF)
                break;

            /* Expect numbered line for WHEN/OTHERWISE/ENDCASE */
            if (ctok->type != TOK_NUMBER)
            {
                advance(parser);
                continue;
            }

            advance(parser); /* skip line number */
            Token *peek = current_token(parser);

            if (peek && peek->type == TOK_ENDCASE)
            {
                /* Found ENDCASE, end the block */
                advance(parser);
                if (current_token(parser) && current_token(parser)->type == TOK_NEWLINE)
                    advance(parser);
                break;
            }
            else if (peek && peek->type == TOK_OTHERWISE)
            {
                /* Parse OTHERWISE block */
                advance(parser);
                while (match(parser, TOK_NEWLINE))
                    ;

                ASTStmt *otherwise_block = NULL;
                ASTStmt *otherwise_tail = NULL;

                while (1)
                {
                    Token *ctok2 = current_token(parser);
                    if (!ctok2 || ctok2->type == TOK_EOF)
                        break;
                    if (ctok2->type != TOK_NUMBER)
                    {
                        advance(parser);
                        continue;
                    }

                    int save_pos = parser->pos;
                    advance(parser);
                    Token *peek2 = current_token(parser);

                    if (peek2 && peek2->type == TOK_ENDCASE)
                    {
                        parser->pos = save_pos;
                        break;
                    }

                    parser->pos = save_pos;
                    ProgramLine *pline = parse_line(parser);
                    if (pline && pline->stmt)
                    {
                        if (!otherwise_block)
                        {
                            otherwise_block = pline->stmt;
                            otherwise_tail = pline->stmt;
                        }
                        else
                        {
                            otherwise_tail->next = pline->stmt;
                            otherwise_tail = pline->stmt;
                        }
                    }
                }

                otherwise_body = otherwise_block;
                continue;
            }
            else if (peek && peek->type == TOK_WHEN)
            {
                /* Parse WHEN clause */
                advance(parser);

                ASTExpr *when_value = parse_expression(parser);
                if (!when_value)
                {
                    parser_error(parser, "Expected value after WHEN");
                    continue;
                }

                /* Create equality comparison: CASE_EXPR = WHEN_VALUE */
                ASTExpr *comparison = ast_expr_create(EXPR_BINARY_OP);
                comparison->op = OP_EQ;
                ast_expr_add_child(comparison, ast_expr_copy(case_expr));
                ast_expr_add_child(comparison, when_value);

                while (match(parser, TOK_NEWLINE))
                    ;

                /* Parse statements for this WHEN clause until next WHEN/OTHERWISE/ENDCASE */
                ASTStmt *when_block = NULL;
                ASTStmt *when_tail = NULL;

                while (1)
                {
                    Token *ctok2 = current_token(parser);
                    if (!ctok2 || ctok2->type == TOK_EOF)
                        break;
                    if (ctok2->type != TOK_NUMBER)
                    {
                        advance(parser);
                        continue;
                    }

                    int save_pos = parser->pos;
                    advance(parser);
                    Token *peek2 = current_token(parser);

                    if (peek2 && (peek2->type == TOK_WHEN || peek2->type == TOK_OTHERWISE || peek2->type == TOK_ENDCASE))
                    {
                        parser->pos = save_pos;
                        break;
                    }

                    parser->pos = save_pos;
                    ProgramLine *pline = parse_line(parser);
                    if (pline && pline->stmt)
                    {
                        if (!when_block)
                        {
                            when_block = pline->stmt;
                            when_tail = pline->stmt;
                        }
                        else
                        {
                            when_tail->next = pline->stmt;
                            while (when_tail->next)
                                when_tail = when_tail->next;
                        }
                    }
                }

                /* Create IF statement for this WHEN */
                ASTStmt *if_stmt = ast_stmt_create(STMT_IF);
                ast_stmt_add_expr(if_stmt, comparison);
                ast_stmt_set_body(if_stmt, when_block);

                /* Chain to previous IF */
                if (!root_if)
                {
                    root_if = if_stmt;
                    current_else = if_stmt;
                }
                else
                {
                    current_else->else_body = if_stmt;
                    current_else = if_stmt;
                }
            }
            else
            {
                advance(parser);
            }
        }

        /* Set OTHERWISE as final else_body */
        if (otherwise_body && current_else)
        {
            current_else->else_body = otherwise_body;
        }

        /* Free the original case expression */
        ast_expr_free(case_expr);

        return root_if ? root_if : ast_stmt_create(STMT_BLOCK);
    }

    /* Single-line CASE (all on one line - not supported yet, same as multi-line) */
    parser_error(parser, "CASE statement must span multiple lines with WHEN clauses");
    ast_expr_free(case_expr);
    return NULL;
}

static ASTStmt *parse_sound_stmt(Parser *parser)
{
    advance(parser); /* consume SOUND */
    ASTStmt *stmt = ast_stmt_create(STMT_SOUND);

    /* Parse: SOUND base_freq ; [harmonic, intensity ; ...] ; duration
       or traditional: SOUND base_freq, duration */

    ASTExpr *base_freq = parse_expression(parser);
    if (base_freq)
        ast_stmt_add_expr(stmt, base_freq);

    /* Check if semicolon (new harmonic syntax) or comma (traditional syntax) */
    if (match(parser, TOK_SEMICOLON))
    {
        /* New harmonics syntax: SOUND base_freq ; h1, i1 ; h2, i2 ; ... ; duration */
        while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE &&
               current_token(parser)->type != TOK_COLON && current_token(parser)->type != TOK_EOF)
        {
            ASTExpr *expr = parse_expression(parser);
            if (expr)
                ast_stmt_add_expr(stmt, expr);

            if (match(parser, TOK_COMMA))
            {
                /* Harmonic number, intensity pair */
                ASTExpr *intensity = parse_expression(parser);
                if (intensity)
                    ast_stmt_add_expr(stmt, intensity);

                /* Check for next semicolon or end */
                if (!match(parser, TOK_SEMICOLON))
                {
                    break;
                }
            }
            else if (match(parser, TOK_SEMICOLON))
            {
                /* Continue to next harmonic or duration */
                continue;
            }
            else
            {
                /* No more harmonics, this must be the duration */
                break;
            }
        }
    }
    else if (match(parser, TOK_COMMA))
    {
        /* Traditional syntax: SOUND frequency, duration */
        ASTExpr *duration = parse_expression(parser);
        if (duration)
            ast_stmt_add_expr(stmt, duration);
    }

    return stmt;
}

static ASTStmt *parse_print_stmt(Parser *parser)
{
    advance(parser); /* consume PRINT */

    /* PRINT@ position, expr */
    if (match(parser, TOK_AT))
    {
        ASTStmt *stmt = ast_stmt_create(STMT_PRINT_AT);
        ASTExpr *pos_expr = parse_expression(parser);
        if (!pos_expr)
        {
            return stmt;
        }
        ast_stmt_add_expr(stmt, pos_expr);

        /* Expect comma before value */
        if (!match(parser, TOK_COMMA))
        {
            parser_error(parser, "Expected ',' after PRINT@ position");
            return stmt;
        }

        ASTExpr *value_expr = parse_expression(parser);
        if (value_expr)
        {
            ast_stmt_add_expr(stmt, value_expr);
        }
        return stmt;
    }

    /* PRINT USING format_string; value */
    if (match(parser, TOK_USING))
    {
        ASTStmt *stmt = ast_stmt_create(STMT_PRINT_USING);

        /* Get format string */
        ASTExpr *format_expr = parse_expression(parser);
        if (!format_expr)
        {
            parser_error(parser, "Expected format string after USING");
            return stmt;
        }
        ast_stmt_add_expr(stmt, format_expr);

        /* Expect semicolon */
        if (!match(parser, TOK_SEMICOLON))
        {
            parser_error(parser, "Expected ';' after USING format string");
            return stmt;
        }

        /* Get value to format */
        ASTExpr *value_expr = parse_expression(parser);
        if (!value_expr)
        {
            parser_error(parser, "Expected value after USING format");
            return stmt;
        }
        ast_stmt_add_expr(stmt, value_expr);

        return stmt;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_PRINT);

    /* PRINT #n, ... */
    if (match(parser, TOK_HASH))
    {
        Token *tok = current_token(parser);
        if (tok && tok->type == TOK_NUMBER)
        {
            stmt->file_handle = (int)tok->num_value;
            advance(parser);
            match(parser, TOK_COMMA);
        }
        else
        {
            parser_error(parser, "Expected file handle after #");
            return stmt;
        }
    }

    /* Parse print items */
    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE && current_token(parser)->type != TOK_ELSE && current_token(parser)->type != TOK_COLON)
    {
        if (match(parser, TOK_SEMICOLON))
        {
            ASTExpr *sep = ast_expr_create(EXPR_PRINT_SEP);
            sep->str_value = xstrdup(";");
            ast_stmt_add_expr(stmt, sep);
            continue;
        }
        if (match(parser, TOK_COMMA))
        {
            ASTExpr *sep = ast_expr_create(EXPR_PRINT_SEP);
            sep->str_value = xstrdup(",");
            ast_stmt_add_expr(stmt, sep);
            continue;
        }

        /* Check for TAB(expr) */
        if (match(parser, TOK_TAB))
        {
            if (!match(parser, TOK_LPAREN))
            {
                parser_error(parser, "Expected '(' after TAB");
                break;
            }
            ASTExpr *tab_expr = parse_expression(parser);
            if (!tab_expr)
            {
                parser_error(parser, "Expected expression in TAB()");
                break;
            }
            if (!match(parser, TOK_RPAREN))
            {
                parser_error(parser, "Expected ')' after TAB expression");
                break;
            }
            /* Create EXPR_TAB node with the position as a child */
            ASTExpr *tab_node = ast_expr_create(EXPR_TAB);
            ast_expr_add_child(tab_node, tab_expr);
            ast_stmt_add_expr(stmt, tab_node);
            continue;
        }

        ASTExpr *expr = parse_expression(parser);
        if (expr)
        {
            ast_stmt_add_expr(stmt, expr);
        }
        else
        {
            break;
        }
    }

    return stmt;
}

static ASTStmt *parse_input_stmt(Parser *parser)
{
    advance(parser); /* consume INPUT */

    ASTStmt *stmt = ast_stmt_create(STMT_INPUT);

    /* INPUT #n, ... */
    if (match(parser, TOK_HASH))
    {
        Token *tok = current_token(parser);
        if (tok && tok->type == TOK_NUMBER)
        {
            stmt->file_handle = (int)tok->num_value;
            advance(parser);
            match(parser, TOK_COMMA);
        }
        else
        {
            parser_error(parser, "Expected file handle after #");
            return stmt;
        }
    }

    /* Parse optional prompt string */
    Token *tok = current_token(parser);
    if (tok && tok->type == TOK_STRING)
    {
        ASTExpr *prompt = ast_expr_create(EXPR_STRING);
        prompt->str_value = xstrdup(tok->str_value);
        ast_stmt_add_expr(stmt, prompt);
        advance(parser);

        if (match(parser, TOK_SEMICOLON) || match(parser, TOK_COMMA))
        {
            /* Separator after prompt */
        }
    }

    /* Parse variable list */
    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE && current_token(parser)->type != TOK_COLON)
    {
        if (match(parser, TOK_COMMA))
        {
            continue;
        }

        tok = current_token(parser);
        if (tok && tok->type == TOK_IDENTIFIER)
        {
            ASTExpr *var = ast_expr_create(EXPR_VAR);
            var->var_name = xstrdup(tok->value);
            ast_stmt_add_expr(stmt, var);
            advance(parser);
        }
        else
        {
            break;
        }
    }

    return stmt;
}

static ASTStmt *parse_line_input_stmt(Parser *parser)
{
    advance(parser); /* consume LINE */

    if (!match(parser, TOK_INPUT))
    {
        parser_error(parser, "Expected INPUT after LINE");
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_LINE_INPUT);

    if (match(parser, TOK_HASH))
    {
        Token *tok = current_token(parser);
        if (tok && tok->type == TOK_NUMBER)
        {
            stmt->file_handle = (int)tok->num_value;
            advance(parser);
            match(parser, TOK_COMMA);
        }
        else
        {
            parser_error(parser, "Expected file handle after #");
            return stmt;
        }
    }

    Token *tok = current_token(parser);
    if (tok && tok->type == TOK_IDENTIFIER)
    {
        ASTExpr *var = ast_expr_create(EXPR_VAR);
        var->var_name = xstrdup(tok->value);
        ast_stmt_add_expr(stmt, var);
        advance(parser);
    }
    else
    {
        parser_error(parser, "Expected variable after LINE INPUT");
    }

    return stmt;
}

static ASTStmt *parse_let_stmt(Parser *parser)
{
    /* Optional LET keyword */
    if (current_token(parser) && current_token(parser)->type == TOK_LET)
    {
        advance(parser);
    }

    Token *tok = current_token(parser);
    if (!tok || tok->type != TOK_IDENTIFIER)
    {
        parser_error(parser, "Expected variable name");
        return NULL;
    }

    char *var_name = xstrdup(tok->value);
    advance(parser);

    /* Check for array subscript */
    ASTExpr *lhs = ast_expr_create(EXPR_VAR);
    lhs->var_name = var_name;

    if (match(parser, TOK_LPAREN))
    {
        /* Array assignment: A(I) = value */
        lhs->type = EXPR_ARRAY;
        while (current_token(parser) && current_token(parser)->type != TOK_RPAREN)
        {
            ASTExpr *subscript = parse_expression(parser);
            if (subscript)
            {
                ast_expr_add_child(lhs, subscript);
            }
            if (!match(parser, TOK_COMMA))
            {
                break;
            }
        }
        expect(parser, TOK_RPAREN, "Expected ')' after array subscript");
    }

    if (!expect(parser, TOK_EQ, "Expected '=' in assignment"))
    {
        ast_expr_free(lhs);
        return NULL;
    }

    ASTExpr *rhs = parse_expression(parser);
    if (!rhs)
    {
        ast_expr_free(lhs);
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_LET);
    ast_stmt_add_expr(stmt, lhs);
    ast_stmt_add_expr(stmt, rhs);

    return stmt;
}

static ASTStmt *parse_if_stmt(Parser *parser)
{
    advance(parser); /* consume IF */

    ASTExpr *condition = parse_expression(parser);
    if (!condition)
    {
        return NULL;
    }

    int has_then = match(parser, TOK_THEN);
    if (!has_then)
    {
        Token *peek = current_token(parser);
        if (!(peek && (peek->type == TOK_GOTO || peek->type == TOK_GOSUB)))
        {
            ast_expr_free(condition);
            parser_error(parser, "Expected THEN or GOTO/GOSUB after IF condition");
            return NULL;
        }
    }

    Token *tok = current_token(parser);
    ASTStmt *then_stmt = NULL;

    // Multi-line IF/THEN/ENDIF extension (block form)
    if (has_then && tok && tok->type == TOK_NEWLINE)
    {
        if (compat_is_strict(g_compat_state))
        {
            ast_expr_free(condition);
            parser_error(parser, "Multi-line IF/THEN/ENDIF not allowed in strict TRS-80 mode");
            return NULL;
        }
        advance(parser); // consume NEWLINE after THEN
        ASTStmt *block_head = NULL;
        ASTStmt *block_tail = NULL;
        ASTStmt *else_block_head = NULL;
        ASTStmt *else_block_tail = NULL;
        int in_else_block = 0;

        while (1)
        {
            Token *ctok = current_token(parser);
            // Only accept NUMBER as start of each block line
            if (!ctok || ctok->type == TOK_EOF)
                break;
            if (ctok->type == TOK_NUMBER)
            {
                int save_pos = parser->pos;
                advance(parser); // skip line number
                Token *peek = current_token(parser);

                // Check for ELSE keyword
                if (peek && peek->type == TOK_ELSE)
                {
                    advance(parser); // skip ELSE
                    in_else_block = 1;
                    // Consume optional newline after ELSE
                    if (current_token(parser) && current_token(parser)->type == TOK_NEWLINE)
                        advance(parser);
                    continue;
                }

                // Check for ENDIF keyword
                if (peek && peek->type == TOK_ENDIF)
                {
                    // Found numbered ENDIF line, treat as block end
                    advance(parser); // skip ENDIF
                    // Optionally consume trailing NEWLINE
                    if (current_token(parser) && current_token(parser)->type == TOK_NEWLINE)
                        advance(parser);
                    break;
                }

                // Parse as statement in current block (THEN or ELSE)
                parser->pos = save_pos;
                // Parse the line as a normal BASIC line
                ProgramLine *pline = parse_line(parser);
                if (pline && pline->stmt)
                {
                    if (!in_else_block)
                    {
                        // Add to THEN block
                        if (!block_head)
                        {
                            block_head = pline->stmt;
                            block_tail = pline->stmt;
                        }
                        else
                        {
                            block_tail->next = pline->stmt;
                            block_tail = pline->stmt;
                        }
                    }
                    else
                    {
                        // Add to ELSE block
                        if (!else_block_head)
                        {
                            else_block_head = pline->stmt;
                            else_block_tail = pline->stmt;
                        }
                        else
                        {
                            else_block_tail->next = pline->stmt;
                            else_block_tail = pline->stmt;
                        }
                    }
                }
                else
                {
                    // If parse_line fails, skip to next NUMBER
                    while (current_token(parser) && current_token(parser)->type != TOK_NUMBER && current_token(parser)->type != TOK_EOF)
                        advance(parser);
                }
                continue;
            }
            // Skip anything else (should not occur in well-formed BASIC)
            advance(parser);
        }

        then_stmt = block_head;
        ASTStmt *stmt = ast_stmt_create(STMT_IF);
        ast_stmt_add_expr(stmt, condition);
        ast_stmt_set_body(stmt, then_stmt);
        stmt->else_body = else_block_head;
        return stmt;
    }

    // Single-line IF/THEN/ELSE and IF/THEN/GOTO/GOSUB (classic forms)
    if (tok && tok->type == TOK_NUMBER)
    {
        then_stmt = ast_stmt_create(STMT_GOTO);
        then_stmt->target_line = ((int)tok->num_value);
        advance(parser);
    }
    else if (tok && tok->type == TOK_GOTO)
    {
        then_stmt = parse_goto_stmt(parser);
    }
    else if (tok && tok->type == TOK_GOSUB)
    {
        then_stmt = parse_gosub_stmt(parser);
    }
    else
    {
        then_stmt = parse_statement(parser);
        if (then_stmt)
        {
            ASTStmt *tail = then_stmt;
            while (current_token(parser) && current_token(parser)->type == TOK_COLON)
            {
                int saved_pos = parser->pos;
                advance(parser);
                Token *next = current_token(parser);
                if (next && next->type == TOK_ELSE)
                {
                    parser->pos = saved_pos;
                    break;
                }
                ASTStmt *next_stmt = parse_statement(parser);
                if (!next_stmt)
                {
                    break;
                }
                tail->next = next_stmt;
                tail = next_stmt;
            }
        }
    }

    if (!then_stmt)
    {
        ast_expr_free(condition);
        return NULL;
    }

    ASTStmt *else_stmt = NULL;
    if (match(parser, TOK_ELSE))
    {
        else_stmt = parse_statement(parser);
        if (else_stmt)
        {
            ASTStmt *tail = else_stmt;
            while (current_token(parser) && current_token(parser)->type == TOK_COLON)
            {
                advance(parser);
                ASTStmt *next_stmt = parse_statement(parser);
                if (!next_stmt)
                {
                    break;
                }
                tail->next = next_stmt;
                tail = next_stmt;
            }
        }
    }

    ASTStmt *stmt = ast_stmt_create(STMT_IF);
    ast_stmt_add_expr(stmt, condition);
    ast_stmt_set_body(stmt, then_stmt);
    stmt->else_body = else_stmt;
    return stmt;
}

static ASTStmt *parse_on_stmt(Parser *parser)
{
    advance(parser); /* consume ON */

    if (match(parser, TOK_ERROR))
    {
        /* Compatibility check: ON ERROR GOTO not in TRS-80 Level II BASIC */
        if (g_compat_state)
        {
            compat_record_violation(g_compat_state, COMPAT_ERROR_HANDLING, 0,
                                    "ON ERROR GOTO not in TRS-80 Level II BASIC");
            if (compat_is_strict(g_compat_state))
            {
                parser_error(parser, "ON ERROR GOTO not allowed in strict TRS-80 mode");
                return NULL;
            }
        }

        if (!match(parser, TOK_GOTO))
        {
            parser_error(parser, "Expected GOTO after ON ERROR");
            return NULL;
        }

        Token *tok = current_token(parser);
        if (!tok || tok->type != TOK_NUMBER)
        {
            parser_error(parser, "Expected line number after ON ERROR GOTO");
            return NULL;
        }

        ASTStmt *stmt = ast_stmt_create(STMT_ON_ERROR);
        stmt->target_line = (int)tok->num_value;
        advance(parser);
        return stmt;
    }

    ASTExpr *selector = parse_expression(parser);
    if (!selector)
    {
        return NULL;
    }

    int is_gosub = 0;
    if (match(parser, TOK_GOTO))
    {
        is_gosub = 0;
    }
    else if (match(parser, TOK_GOSUB))
    {
        is_gosub = 1;
    }
    else
    {
        ast_expr_free(selector);
        parser_error(parser, "Expected GOTO or GOSUB after ON expression");
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_ON_GOTO);
    stmt->mode = is_gosub;
    ast_stmt_add_expr(stmt, selector);

    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE && current_token(parser)->type != TOK_COLON)
    {
        Token *tok = current_token(parser);
        if (tok && tok->type == TOK_NUMBER)
        {
            ASTExpr *line_expr = ast_expr_create(EXPR_NUMBER);
            line_expr->num_value = tok->num_value;
            ast_stmt_add_expr(stmt, line_expr);
            advance(parser);
        }
        if (!match(parser, TOK_COMMA))
        {
            break;
        }
    }

    return stmt;
}

static ASTStmt *parse_goto_stmt(Parser *parser)
{
    advance(parser); /* consume GOTO */

    Token *tok = current_token(parser);
    if (!tok || tok->type != TOK_NUMBER)
    {
        parser_error(parser, "Expected line number after GOTO");
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_GOTO);
    stmt->target_line = ((int)tok->num_value);
    advance(parser);

    return stmt;
}

static ASTStmt *parse_gosub_stmt(Parser *parser)
{
    advance(parser); /* consume GOSUB */

    Token *tok = current_token(parser);
    if (!tok || tok->type != TOK_NUMBER)
    {
        parser_error(parser, "Expected line number after GOSUB");
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_GOSUB);
    stmt->target_line = ((int)tok->num_value);
    advance(parser);

    return stmt;
}

static ASTStmt *parse_return_stmt(Parser *parser)
{
    advance(parser); /* consume RETURN */
    return ast_stmt_create(STMT_RETURN);
}

static ASTStmt *parse_error_stmt(Parser *parser)
{
    /* Compatibility check: ERROR statement not in TRS-80 Level II BASIC */
    if (g_compat_state)
    {
        compat_record_violation(g_compat_state, COMPAT_ERROR_HANDLING, 0,
                                "ERROR statement not in TRS-80 Level II BASIC");
        if (compat_is_strict(g_compat_state))
        {
            parser_error(parser, "ERROR statement not allowed in strict TRS-80 mode");
            return NULL;
        }
    }

    advance(parser); /* consume ERROR */

    Token *tok = current_token(parser);
    if (!tok || tok->type != TOK_NUMBER)
    {
        parser_error(parser, "Expected error code after ERROR");
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_ERROR);
    ASTExpr *code_expr = ast_expr_create(EXPR_NUMBER);
    code_expr->num_value = tok->num_value;
    ast_stmt_add_expr(stmt, code_expr);
    advance(parser);

    return stmt;
}

static ASTStmt *parse_resume_stmt(Parser *parser)
{
    /* Compatibility check: RESUME statement not in TRS-80 Level II BASIC */
    if (g_compat_state)
    {
        compat_record_violation(g_compat_state, COMPAT_ERROR_HANDLING, 0,
                                "RESUME statement not in TRS-80 Level II BASIC");
        if (compat_is_strict(g_compat_state))
        {
            parser_error(parser, "RESUME statement not allowed in strict TRS-80 mode");
            return NULL;
        }
    }

    advance(parser); /* consume RESUME */

    ASTStmt *stmt = ast_stmt_create(STMT_RESUME);

    if (match(parser, TOK_NEXT))
    {
        stmt->mode = 1;
        return stmt;
    }

    Token *tok = current_token(parser);
    if (tok && tok->type == TOK_NUMBER)
    {
        int line_num = (int)tok->num_value;
        advance(parser);
        if (line_num > 0)
        {
            stmt->mode = 2;
            stmt->target_line = line_num;
        }
        return stmt;
    }

    return stmt;
}

static ASTStmt *parse_sleep_stmt(Parser *parser)
{
    /* Compatibility check: SLEEP is not in TRS-80 Level II BASIC */
    if (g_compat_state)
    {
        compat_record_violation(g_compat_state, COMPAT_MODERN_KEYWORD, 0,
                                "SLEEP statement not in TRS-80 Level II BASIC");
        if (compat_is_strict(g_compat_state))
        {
            parser_error(parser, "SLEEP not allowed in strict TRS-80 mode");
            return NULL;
        }
    }

    advance(parser); /* consume SLEEP */

    ASTExpr *duration = parse_expression(parser);
    if (!duration)
    {
        parser_error(parser, "Expected duration after SLEEP");
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_SLEEP);
    ast_stmt_add_expr(stmt, duration);
    return stmt;
}

static ASTStmt *parse_beep_stmt(Parser *parser)
{
    advance(parser); /* consume BEEP */
    ASTExpr *duration = parse_expression(parser);
    if (!duration)
    {
        parser_error(parser, "Expected duration (ms) after BEEP");
        return NULL;
    }
    ASTStmt *stmt = ast_stmt_create(STMT_BEEP);
    ast_stmt_add_expr(stmt, duration);
    if (match(parser, TOK_COMMA))
    {
        ASTExpr *freq = parse_expression(parser);
        if (freq)
            ast_stmt_add_expr(stmt, freq);
    }
    return stmt;
}

static ASTStmt *parse_cls_stmt(Parser *parser)
{
    advance(parser); /* consume CLS */
    ASTStmt *stmt = ast_stmt_create(STMT_CLS);
    return stmt;
}

static ASTStmt *parse_clear_stmt(Parser *parser)
{
    advance(parser); /* consume CLEAR */
    ASTStmt *stmt = ast_stmt_create(STMT_CLEAR);

    /* CLEAR can have an optional memory size argument */
    Token *tok = current_token(parser);
    if (tok && tok->type != TOK_NEWLINE && tok->type != TOK_SEMICOLON &&
        tok->type != TOK_COLON && tok->type != TOK_EOF)
    {
        ASTExpr *size = parse_expression(parser);
        if (size)
        {
            ast_stmt_add_expr(stmt, size);
        }
    }

    return stmt;
}

static ASTStmt *parse_for_stmt(Parser *parser)
{
    advance(parser); /* consume FOR */

    Token *tok = current_token(parser);
    if (!tok || (tok->type != TOK_IDENTIFIER && tok->type != TOK_LOOP))
    {
        parser_error(parser, "Expected variable after FOR");
        return NULL;
    }

    char *var_name = xstrdup(tok->value);
    advance(parser);

    if (!expect(parser, TOK_EQ, "Expected '=' in FOR statement"))
    {
        free(var_name);
        return NULL;
    }

    ASTExpr *start = parse_expression(parser);
    if (!start)
    {
        free(var_name);
        return NULL;
    }

    if (!expect(parser, TOK_TO, "Expected TO in FOR statement"))
    {
        free(var_name);
        ast_expr_free(start);
        return NULL;
    }

    ASTExpr *end = parse_expression(parser);
    if (!end)
    {
        free(var_name);
        ast_expr_free(start);
        return NULL;
    }

    /* Optional STEP */
    ASTExpr *step = NULL;
    if (match(parser, TOK_STEP))
    {
        step = parse_expression(parser);
        if (!step)
        {
            free(var_name);
            ast_expr_free(start);
            ast_expr_free(end);
            return NULL;
        }
    }

    ASTStmt *stmt = ast_stmt_create(STMT_FOR);
    ASTExpr *var = ast_expr_create(EXPR_VAR);
    var->var_name = var_name;

    ast_stmt_add_expr(stmt, var);
    ast_stmt_add_expr(stmt, start);
    ast_stmt_add_expr(stmt, end);
    if (step)
    {
        ast_stmt_add_expr(stmt, step);
    }

    return stmt;
}

static ASTStmt *parse_next_stmt(Parser *parser)
{
    advance(parser); /* consume NEXT */

    ASTStmt *stmt = ast_stmt_create(STMT_NEXT);

    Token *tok = current_token(parser);
    while (tok && tok->type == TOK_IDENTIFIER)
    {
        ASTExpr *var = ast_expr_create(EXPR_VAR);
        var->var_name = xstrdup(tok->value);
        ast_stmt_add_expr(stmt, var);
        advance(parser);

        if (!match(parser, TOK_COMMA))
        {
            break;
        }
        tok = current_token(parser);
    }

    return stmt;
}

static ASTStmt *parse_dim_stmt(Parser *parser)
{
    advance(parser); /* consume DIM */

    ASTStmt *stmt = ast_stmt_create(STMT_DIM);

    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE && current_token(parser)->type != TOK_COLON)
    {
        if (match(parser, TOK_COMMA))
        {
            continue;
        }

        Token *tok = current_token(parser);
        if (!tok || tok->type != TOK_IDENTIFIER)
        {
            parser_error(parser, "Expected array name in DIM");
            break;
        }

        ASTExpr *array = ast_expr_create(EXPR_ARRAY);
        array->var_name = xstrdup(tok->value);
        advance(parser);

        if (!expect(parser, TOK_LPAREN, "Expected '(' after array name"))
        {
            ast_expr_free(array);
            break;
        }

        /* Parse dimensions */
        while (current_token(parser) && current_token(parser)->type != TOK_RPAREN)
        {
            ASTExpr *dim = parse_expression(parser);
            if (dim)
            {
                ast_expr_add_child(array, dim);
            }
            if (!match(parser, TOK_COMMA))
            {
                break;
            }
        }

        expect(parser, TOK_RPAREN, "Expected ')' after array dimensions");
        ast_stmt_add_expr(stmt, array);
    }

    return stmt;
}

static ASTStmt *parse_read_stmt(Parser *parser)
{
    advance(parser); /* consume READ */

    ASTStmt *stmt = ast_stmt_create(STMT_READ);

    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE && current_token(parser)->type != TOK_COLON)
    {
        if (match(parser, TOK_COMMA))
        {
            continue;
        }

        Token *tok = current_token(parser);
        if (!tok || tok->type != TOK_IDENTIFIER)
        {
            parser_error(parser, "Expected variable in READ");
            break;
        }

        ASTExpr *var = ast_expr_create(EXPR_VAR);
        var->var_name = xstrdup(tok->value);
        advance(parser);

        if (match(parser, TOK_LPAREN))
        {
            var->type = EXPR_ARRAY;
            while (current_token(parser) && current_token(parser)->type != TOK_RPAREN)
            {
                ASTExpr *idx = parse_expression(parser);
                if (idx)
                {
                    ast_expr_add_child(var, idx);
                }
                if (!match(parser, TOK_COMMA))
                {
                    break;
                }
            }
            expect(parser, TOK_RPAREN, "Expected ')' after array indices");
        }

        ast_stmt_add_expr(stmt, var);
    }

    return stmt;
}

static ASTStmt *parse_data_stmt(Parser *parser)
{
    advance(parser); /* consume DATA */

    ASTStmt *stmt = ast_stmt_create(STMT_DATA);

    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE && current_token(parser)->type != TOK_COLON)
    {
        if (match(parser, TOK_COMMA))
        {
            continue;
        }

        Token *tok = current_token(parser);
        if (!tok)
        {
            break;
        }

        if (tok->type == TOK_STRING)
        {
            ASTExpr *expr = ast_expr_create(EXPR_STRING);
            expr->str_value = xstrdup(tok->str_value);
            ast_stmt_add_expr(stmt, expr);
            advance(parser);
        }
        else if (tok->type == TOK_NUMBER)
        {
            ASTExpr *expr = ast_expr_create(EXPR_NUMBER);
            expr->num_value = tok->num_value;
            ast_stmt_add_expr(stmt, expr);
            advance(parser);
        }
        else if (tok->type == TOK_MINUS)
        {
            advance(parser);
            Token *num = current_token(parser);
            if (num && num->type == TOK_NUMBER)
            {
                ASTExpr *expr = ast_expr_create(EXPR_NUMBER);
                expr->num_value = -num->num_value;
                ast_stmt_add_expr(stmt, expr);
                advance(parser);
            }
        }
        else
        {
            parser_error(parser, "Expected literal in DATA");
            break;
        }
    }

    return stmt;
}

static ASTStmt *parse_restore_stmt(Parser *parser)
{
    advance(parser); /* consume RESTORE */
    ASTStmt *stmt = ast_stmt_create(STMT_RESTORE);
    Token *tok = current_token(parser);
    if (tok && tok->type == TOK_NUMBER)
    {
        stmt->target_line = (int)tok->num_value;
        advance(parser);
    }
    return stmt;
}

static ASTStmt *parse_open_stmt(Parser *parser)
{
    advance(parser); /* consume OPEN */

    Token *tok = current_token(parser);
    if (!tok || tok->type != TOK_STRING)
    {
        parser_error(parser, "Expected filename after OPEN");
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_OPEN);
    ASTExpr *fname = ast_expr_create(EXPR_STRING);
    fname->str_value = xstrdup(tok->str_value);
    ast_stmt_add_expr(stmt, fname);
    advance(parser);

    if (!expect(parser, TOK_FOR, "Expected FOR in OPEN statement"))
    {
        return stmt;
    }

    if (match(parser, TOK_INPUT))
    {
        stmt->mode = 1;
    }
    else if (match(parser, TOK_OUTPUT))
    {
        stmt->mode = 2;
    }
    else if (match(parser, TOK_APPEND))
    {
        stmt->mode = 3;
    }
    else
    {
        parser_error(parser, "Expected INPUT/OUTPUT/APPEND after FOR");
        return stmt;
    }

    if (!expect(parser, TOK_AS, "Expected AS in OPEN statement"))
    {
        return stmt;
    }

    if (!expect(parser, TOK_HASH, "Expected # in OPEN statement"))
    {
        return stmt;
    }

    tok = current_token(parser);
    if (tok && tok->type == TOK_NUMBER)
    {
        stmt->file_handle = (int)tok->num_value;
        advance(parser);
    }
    else
    {
        parser_error(parser, "Expected file handle in OPEN statement");
    }

    return stmt;
}

static ASTStmt *parse_close_stmt(Parser *parser)
{
    advance(parser); /* consume CLOSE */

    ASTStmt *stmt = ast_stmt_create(STMT_CLOSE);
    if (match(parser, TOK_HASH))
    {
        Token *tok = current_token(parser);
        if (tok && tok->type == TOK_NUMBER)
        {
            stmt->file_handle = (int)tok->num_value;
            advance(parser);
        }
        else
        {
            parser_error(parser, "Expected file handle after #");
        }
    }

    return stmt;
}

static ASTStmt *parse_write_stmt(Parser *parser)
{
    advance(parser); /* consume WRITE */

    ASTStmt *stmt = ast_stmt_create(STMT_WRITE);

    if (match(parser, TOK_HASH))
    {
        Token *tok = current_token(parser);
        if (tok && tok->type == TOK_NUMBER)
        {
            stmt->file_handle = (int)tok->num_value;
            advance(parser);
            match(parser, TOK_COMMA);
        }
        else
        {
            parser_error(parser, "Expected file handle after #");
            return stmt;
        }
    }

    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE && current_token(parser)->type != TOK_COLON)
    {
        if (match(parser, TOK_COMMA))
        {
            continue;
        }
        ASTExpr *expr = parse_expression(parser);
        if (expr)
        {
            ast_stmt_add_expr(stmt, expr);
        }
        else
        {
            break;
        }
    }

    return stmt;
}

static ASTStmt *parse_get_stmt(Parser *parser)
{
    advance(parser); /* consume GET */

    ASTStmt *stmt = ast_stmt_create(STMT_GET);
    if (match(parser, TOK_HASH))
    {
        Token *tok = current_token(parser);
        if (tok && tok->type == TOK_NUMBER)
        {
            stmt->file_handle = (int)tok->num_value;
            advance(parser);
            match(parser, TOK_COMMA);
        }
        else
        {
            parser_error(parser, "Expected file handle after #");
            return stmt;
        }
    }

    Token *tok = current_token(parser);
    if (tok && tok->type == TOK_IDENTIFIER)
    {
        ASTExpr *var = ast_expr_create(EXPR_VAR);
        var->var_name = xstrdup(tok->value);
        ast_stmt_add_expr(stmt, var);
        advance(parser);
    }
    else
    {
        parser_error(parser, "Expected variable in GET");
    }

    return stmt;
}

static ASTStmt *parse_put_stmt(Parser *parser)
{
    advance(parser); /* consume PUT */

    ASTStmt *stmt = ast_stmt_create(STMT_PUT);
    if (match(parser, TOK_HASH))
    {
        Token *tok = current_token(parser);
        if (tok && tok->type == TOK_NUMBER)
        {
            stmt->file_handle = (int)tok->num_value;
            advance(parser);
            match(parser, TOK_COMMA);
        }
        else
        {
            parser_error(parser, "Expected file handle after #");
            return stmt;
        }
    }

    ASTExpr *expr = parse_expression(parser);
    if (expr)
    {
        ast_stmt_add_expr(stmt, expr);
    }
    else
    {
        parser_error(parser, "Expected value in PUT");
    }

    return stmt;
}

static ASTStmt *parse_poke_stmt(Parser *parser)
{
    advance(parser); /* consume POKE */

    ASTStmt *stmt = ast_stmt_create(STMT_POKE);
    ASTExpr *addr = parse_expression(parser);
    if (!addr)
    {
        return stmt;
    }
    ast_stmt_add_expr(stmt, addr);

    if (!expect(parser, TOK_COMMA, "Expected ',' after POKE address"))
    {
        return stmt;
    }

    ASTExpr *value = parse_expression(parser);
    if (value)
    {
        ast_stmt_add_expr(stmt, value);
    }

    return stmt;
}

static ASTStmt *parse_save_stmt(Parser *parser)
{
    advance(parser); /* consume SAVE */

    ASTStmt *stmt = ast_stmt_create(STMT_SAVE);
    ASTExpr *filename = parse_expression(parser);
    if (!filename)
    {
        return stmt;
    }
    ast_stmt_add_expr(stmt, filename);

    return stmt;
}

static ASTStmt *parse_delete_stmt(Parser *parser)
{
    advance(parser); /* consume DELETE */

    ASTStmt *stmt = ast_stmt_create(STMT_DELETE);

    Token *tok = current_token(parser);
    if (!tok || tok->type == TOK_NEWLINE || tok->type == TOK_SEMICOLON || tok->type == TOK_COLON || tok->type == TOK_EOF)
    {
        parser_error(parser, "DELETE requires a line number or range");
        return stmt;
    }

    /* Parse line number range */
    /* Possible forms: n, n-m, -m, . */

    double start_line = -1.0;
    double end_line = -1.0;

    /* Check for dot (current line) */
    if (tok && tok->type == TOK_IDENTIFIER && strcmp(tok->value, ".") == 0)
    {
        advance(parser);
        /* Store dot as a special value */
        ASTExpr *expr = ast_expr_create(EXPR_NUMBER);
        expr->num_value = -1.0; /* Special marker for current line */
        ast_stmt_add_expr(stmt, expr);
        return stmt;
    }

    /* Check for minus (delete up to m) */
    if (tok && tok->type == TOK_MINUS)
    {
        advance(parser);
        tok = current_token(parser);
        if (!tok || tok->type != TOK_NUMBER)
        {
            parser_error(parser, "Invalid DELETE syntax");
            return stmt;
        }
        end_line = tok->num_value;
        advance(parser);

        /* Store range as two values: -1 for start, end_line for end */
        ASTExpr *expr_start = ast_expr_create(EXPR_NUMBER);
        expr_start->num_value = -2.0; /* Special marker: delete from first to end */
        ASTExpr *expr_end = ast_expr_create(EXPR_NUMBER);
        expr_end->num_value = end_line;
        ast_stmt_add_expr(stmt, expr_start);
        ast_stmt_add_expr(stmt, expr_end);
        return stmt;
    }

    /* Otherwise, must be a number */
    if (!tok || tok->type != TOK_NUMBER)
    {
        parser_error(parser, "Invalid DELETE syntax");
        return stmt;
    }

    start_line = tok->num_value;
    advance(parser);

    tok = current_token(parser);
    if (tok && tok->type == TOK_MINUS)
    {
        advance(parser);
        tok = current_token(parser);
        if (!tok || tok->type != TOK_NUMBER)
        {
            parser_error(parser, "Invalid DELETE range");
            return stmt;
        }
        end_line = tok->num_value;
        advance(parser);
    }
    else
    {
        end_line = start_line;
    }

    ASTExpr *expr_start = ast_expr_create(EXPR_NUMBER);
    expr_start->num_value = start_line;
    ASTExpr *expr_end = ast_expr_create(EXPR_NUMBER);
    expr_end->num_value = end_line;
    ast_stmt_add_expr(stmt, expr_start);
    ast_stmt_add_expr(stmt, expr_end);

    return stmt;
}

static ASTStmt *parse_merge_stmt(Parser *parser)
{
    advance(parser); /* consume MERGE */

    Token *tok = current_token(parser);
    if (!tok || tok->type != TOK_STRING)
    {
        parser_error(parser, "MERGE requires a string filename");
        return ast_stmt_create(STMT_MERGE);
    }

    ASTStmt *stmt = ast_stmt_create(STMT_MERGE);
    ASTExpr *filename_expr = ast_expr_create(EXPR_STRING);
    filename_expr->str_value = xstrdup(tok->value);
    ast_stmt_add_expr(stmt, filename_expr);
    advance(parser);

    return stmt;
}

static ASTStmt *parse_end_stmt(Parser *parser)
{
    advance(parser); /* consume END */
    return ast_stmt_create(STMT_END);
}

static ASTStmt *parse_rem_stmt(Parser *parser)
{
    Token *tok = current_token(parser);
    advance(parser); /* consume REM */

    ASTStmt *stmt = ast_stmt_create(STMT_REM);

    /* Capture rest of line as comment */
    if (tok && tok->str_value)
    {
        stmt->comment = xstrdup(tok->str_value);
    }

    /* Skip to end of line */
    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE)
    {
        advance(parser);
    }

    return stmt;
}

static ASTStmt *parse_def_stmt(Parser *parser, StmtType type)
{
    /* Compatibility check: DEFxxx statements are not in TRS-80 Level II BASIC */
    if (g_compat_state)
    {
        const char *stmt_name = (type == STMT_DEFINT) ? "DEFINT" : (type == STMT_DEFSNG) ? "DEFSNG"
                                                               : (type == STMT_DEFDBL)   ? "DEFDBL"
                                                                                         : "DEFSTR";
        char msg[128];
        snprintf(msg, sizeof(msg), "%s statement not in TRS-80 Level II BASIC", stmt_name);
        compat_record_violation(g_compat_state, COMPAT_MODERN_KEYWORD, 0, msg);
        if (compat_is_strict(g_compat_state))
        {
            parser_error(parser, msg);
            return NULL;
        }
    }

    advance(parser); /* consume DEFxxx */

    ASTStmt *stmt = ast_stmt_create(type);

    while (current_token(parser) && current_token(parser)->type != TOK_NEWLINE && current_token(parser)->type != TOK_COLON)
    {
        Token *tok = current_token(parser);
        if (!tok || tok->type != TOK_IDENTIFIER || !tok->value || strlen(tok->value) != 1)
        {
            parser_error(parser, "DEFxxx requires single-letter identifiers");
            return stmt;
        }

        char start_letter = (char)toupper((unsigned char)tok->value[0]);
        char end_letter = start_letter;
        advance(parser);

        if (match(parser, TOK_MINUS))
        {
            Token *end_tok = current_token(parser);
            if (!end_tok || end_tok->type != TOK_IDENTIFIER || !end_tok->value || strlen(end_tok->value) != 1)
            {
                parser_error(parser, "DEFxxx requires valid letter ranges");
                return stmt;
            }
            end_letter = (char)toupper((unsigned char)end_tok->value[0]);
            advance(parser);
        }

        if (start_letter < 'A' || start_letter > 'Z' || end_letter < 'A' || end_letter > 'Z' || start_letter > end_letter)
        {
            parser_error(parser, "DEFxxx requires valid letter ranges");
            return stmt;
        }

        char range[4];
        if (start_letter == end_letter)
        {
            range[0] = start_letter;
            range[1] = '\0';
        }
        else
        {
            range[0] = start_letter;
            range[1] = '-';
            range[2] = end_letter;
            range[3] = '\0';
        }

        ASTExpr *expr = ast_expr_create(EXPR_STRING);
        expr->str_value = xstrdup(range);
        ast_stmt_add_expr(stmt, expr);

        if (!match(parser, TOK_COMMA))
        {
            break;
        }
    }

    return stmt;
}

static ASTStmt *parse_def_fn_stmt(Parser *parser)
{
    advance(parser); /* consume DEF */

    Token *fn_tok = current_token(parser);
    if (!fn_tok || fn_tok->type != TOK_IDENTIFIER)
    {
        parser_error(parser, "Expected function name after DEF");
        return NULL;
    }

    /* Function names in BASIC start with FN */
    if (!fn_tok->value || strncasecmp(fn_tok->value, "FN", 2) != 0)
    {
        parser_error(parser, "Function names must start with FN");
        return NULL;
    }

    char *fn_name = xstrdup(fn_tok->value);
    advance(parser);

    if (!expect(parser, TOK_LPAREN, "Expected '(' after function name"))
    {
        free(fn_name);
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_DEF_FN);

    /* Store function name as first expression */
    ASTExpr *name_expr = ast_expr_create(EXPR_STRING);
    name_expr->str_value = fn_name;
    ast_stmt_add_expr(stmt, name_expr);

    /* Parse parameter list */
    while (current_token(parser) && current_token(parser)->type != TOK_RPAREN)
    {
        Token *param_tok = current_token(parser);
        if (!param_tok || param_tok->type != TOK_IDENTIFIER)
        {
            parser_error(parser, "Expected parameter name");
            return stmt;
        }

        ASTExpr *param = ast_expr_create(EXPR_STRING);
        param->str_value = xstrdup(param_tok->value);
        ast_stmt_add_expr(stmt, param);
        advance(parser);

        if (!match(parser, TOK_COMMA))
        {
            break;
        }
    }

    if (!expect(parser, TOK_RPAREN, "Expected ')' after parameters"))
    {
        return stmt;
    }

    if (!expect(parser, TOK_EQ, "Expected '=' after parameters"))
    {
        return stmt;
    }

    /* Parse function body expression */
    ASTExpr *body_expr = parse_expression(parser);
    if (!body_expr)
    {
        parser_error(parser, "Expected expression after '=' in DEF FN");
        return stmt;
    }

    ast_stmt_add_expr(stmt, body_expr);

    return stmt;
}

/* Expression parsing - precedence climbing */

ASTExpr *parse_expression(Parser *parser)
{
    return parse_or_expr(parser);
}

static ASTExpr *parse_or_expr(Parser *parser)
{
    ASTExpr *left = parse_and_expr(parser);
    if (!left)
        return NULL;

    while (match(parser, TOK_OR))
    {
        ASTExpr *right = parse_and_expr(parser);
        if (!right)
        {
            ast_expr_free(left);
            return NULL;
        }

        ASTExpr *expr = ast_expr_create(EXPR_BINARY_OP);
        expr->op = OP_OR;
        ast_expr_add_child(expr, left);
        ast_expr_add_child(expr, right);
        left = expr;
    }

    return left;
}

static ASTExpr *parse_and_expr(Parser *parser)
{
    ASTExpr *left = parse_not_expr(parser);
    if (!left)
        return NULL;

    while (match(parser, TOK_AND))
    {
        ASTExpr *right = parse_not_expr(parser);
        if (!right)
        {
            ast_expr_free(left);
            return NULL;
        }

        ASTExpr *expr = ast_expr_create(EXPR_BINARY_OP);
        expr->op = OP_AND;
        ast_expr_add_child(expr, left);
        ast_expr_add_child(expr, right);
        left = expr;
    }

    return left;
}

static ASTExpr *parse_not_expr(Parser *parser)
{
    if (match(parser, TOK_NOT))
    {
        ASTExpr *operand = parse_not_expr(parser);
        if (!operand)
            return NULL;

        ASTExpr *expr = ast_expr_create(EXPR_UNARY_OP);
        expr->op = OP_NOT;
        ast_expr_add_child(expr, operand);
        return expr;
    }

    return parse_relational_expr(parser);
}

static ASTExpr *parse_relational_expr(Parser *parser)
{
    ASTExpr *left = parse_additive_expr(parser);
    if (!left)
        return NULL;

    Token *tok = current_token(parser);
    if (tok)
    {
        OpType op = OP_NONE;

        if (tok->type == TOK_EQ || tok->type == TOK_EQUAL)
            op = OP_EQ;
        else if (tok->type == TOK_LT || tok->type == TOK_LESS)
            op = OP_LT;
        else if (tok->type == TOK_GT || tok->type == TOK_GREATER)
            op = OP_GT;
        else if (tok->type == TOK_LE || tok->type == TOK_LESSEQUAL)
            op = OP_LE;
        else if (tok->type == TOK_GE || tok->type == TOK_GREATEREQUAL)
            op = OP_GE;
        else if (tok->type == TOK_NE || tok->type == TOK_NOTEQUAL)
            op = OP_NE;

        if (op != OP_NONE)
        {
            advance(parser);
            ASTExpr *right = parse_additive_expr(parser);
            if (!right)
            {
                ast_expr_free(left);
                return NULL;
            }

            ASTExpr *expr = ast_expr_create(EXPR_BINARY_OP);
            expr->op = op;
            ast_expr_add_child(expr, left);
            ast_expr_add_child(expr, right);
            return expr;
        }
    }

    return left;
}

static ASTExpr *parse_additive_expr(Parser *parser)
{
    ASTExpr *left = parse_multiplicative_expr(parser);
    if (!left)
        return NULL;

    while (1)
    {
        Token *tok = current_token(parser);
        if (!tok)
            break;

        OpType op = OP_NONE;
        if (tok->type == TOK_PLUS)
        {
            /* Check if this is string concatenation */
            if (is_string_expr(left))
                op = OP_CONCAT;
            else
                op = OP_ADD;
        }
        else if (tok->type == TOK_MINUS)
            op = OP_SUB;
        else
            break;

        advance(parser);
        ASTExpr *right = parse_multiplicative_expr(parser);
        if (!right)
        {
            ast_expr_free(left);
            return NULL;
        }

        ASTExpr *expr = ast_expr_create(EXPR_BINARY_OP);
        expr->op = op;
        ast_expr_add_child(expr, left);
        ast_expr_add_child(expr, right);
        left = expr;
    }

    return left;
}

static ASTExpr *parse_multiplicative_expr(Parser *parser)
{
    ASTExpr *left = parse_power_expr(parser);
    if (!left)
        return NULL;

    while (1)
    {
        Token *tok = current_token(parser);
        if (!tok)
            break;

        OpType op = OP_NONE;
        if (tok->type == TOK_STAR)
            op = OP_MUL;
        else if (tok->type == TOK_SLASH)
            op = OP_DIV;
        else if (tok->type == TOK_MOD)
            op = OP_MOD;
        else
            break;

        advance(parser);
        ASTExpr *right = parse_power_expr(parser);
        if (!right)
        {
            ast_expr_free(left);
            return NULL;
        }

        ASTExpr *expr = ast_expr_create(EXPR_BINARY_OP);
        expr->op = op;
        ast_expr_add_child(expr, left);
        ast_expr_add_child(expr, right);
        left = expr;
    }

    return left;
}

static ASTExpr *parse_power_expr(Parser *parser)
{
    ASTExpr *left = parse_unary_expr(parser);
    if (!left)
        return NULL;

    Token *tok = current_token(parser);
    if (tok && tok->type == TOK_CARET)
    {
        advance(parser);
        ASTExpr *right = parse_power_expr(parser); /* right-associative */
        if (!right)
        {
            ast_expr_free(left);
            return NULL;
        }
        ASTExpr *expr = ast_expr_create(EXPR_BINARY_OP);
        expr->op = OP_POWER;
        ast_expr_add_child(expr, left);
        ast_expr_add_child(expr, right);
        return expr;
    }

    return left;
}

static ASTExpr *parse_unary_expr(Parser *parser)
{
    Token *tok = current_token(parser);
    if (tok && (tok->type == TOK_PLUS || tok->type == TOK_MINUS))
    {
        OpType op = (tok->type == TOK_PLUS) ? OP_PLUS : OP_NEG;
        advance(parser);

        ASTExpr *operand = parse_unary_expr(parser);
        if (!operand)
            return NULL;

        ASTExpr *expr = ast_expr_create(EXPR_UNARY_OP);
        expr->op = op;
        ast_expr_add_child(expr, operand);
        return expr;
    }

    return parse_primary_expr(parser);
}

static ASTExpr *parse_primary_expr(Parser *parser)
{
    Token *tok = current_token(parser);
    if (!tok)
    {
        parser_error(parser, "Unexpected end of input");
        return NULL;
    }

    /* Number literal */
    if (tok->type == TOK_NUMBER)
    {
        ASTExpr *expr = ast_expr_create(EXPR_NUMBER);
        expr->num_value = tok->num_value;
        advance(parser);
        return expr;
    }

    /* String literal */
    if (tok->type == TOK_STRING)
    {
        ASTExpr *expr = ast_expr_create(EXPR_STRING);
        expr->str_value = xstrdup(tok->str_value);
        advance(parser);
        return expr;
    }

    /* Variable or function call */
    if (tok->type == TOK_IDENTIFIER)
    {
        char *name = xstrdup(tok->value);
        advance(parser);

        /* Check for function call or array access */
        if (match(parser, TOK_LPAREN))
        {
            /* User-defined functions start with FN, or it's a builtin */
            int is_function = is_builtin_function(name) ||
                              (name && strlen(name) >= 2 && strncasecmp(name, "FN", 2) == 0);

            if (is_function)
            {
                ASTExpr *expr = ast_expr_create(EXPR_FUNC_CALL);
                expr->var_name = name;

                /* Parse arguments */
                while (current_token(parser) && current_token(parser)->type != TOK_RPAREN)
                {
                    ASTExpr *arg = parse_expression(parser);
                    if (arg)
                    {
                        ast_expr_add_child(expr, arg);
                    }
                    if (!match(parser, TOK_COMMA))
                    {
                        break;
                    }
                }

                expect(parser, TOK_RPAREN, "Expected ')' after function arguments");
                return expr;
            }
            else
            {
                ASTExpr *expr = ast_expr_create(EXPR_ARRAY);
                expr->var_name = name;

                while (current_token(parser) && current_token(parser)->type != TOK_RPAREN)
                {
                    ASTExpr *idx = parse_expression(parser);
                    if (idx)
                    {
                        ast_expr_add_child(expr, idx);
                    }
                    if (!match(parser, TOK_COMMA))
                    {
                        break;
                    }
                }

                expect(parser, TOK_RPAREN, "Expected ')' after array indices");
                return expr;
            }
        }

        /* Simple variable or no-paren builtin function */
        if (is_builtin_function(name) && strcasecmp(name, "INKEY$") == 0)
        {
            ASTExpr *expr = ast_expr_create(EXPR_FUNC_CALL);
            expr->var_name = name;
            return expr;
        }

        ASTExpr *expr = ast_expr_create(EXPR_VAR);
        expr->var_name = name;
        return expr;
    }

    /* Parenthesized expression */
    if (match(parser, TOK_LPAREN))
    {
        ASTExpr *expr = parse_expression(parser);
        expect(parser, TOK_RPAREN, "Expected ')' after expression");
        return expr;
    }

    parser_error(parser, "Unexpected token in expression");
    return NULL;
}

static ASTStmt *parse_while_stmt(Parser *parser)
{
    advance(parser); /* consume WHILE */

    ASTExpr *condition = parse_expression(parser);
    if (!condition)
    {
        parser_error(parser, "Expected condition after WHILE");
        return NULL;
    }

    ASTStmt *stmt = ast_stmt_create(STMT_WHILE);
    ast_stmt_add_expr(stmt, condition);
    return stmt;
}

static ASTStmt *parse_wend_stmt(Parser *parser)
{
    advance(parser); /* consume WEND */
    return ast_stmt_create(STMT_WEND);
}

static ASTStmt *parse_do_loop_stmt(Parser *parser)
{
    advance(parser); /* consume DO */

    ASTStmt *stmt = ast_stmt_create(STMT_DO_LOOP);
    stmt->is_loop_end = 0; /* Mark this as DO (not LOOP) */

    /* Check for DO WHILE condition */
    if (match(parser, TOK_WHILE))
    {
        /* match() already consumed WHILE, now parse the condition */
        ASTExpr *condition = parse_expression(parser);
        if (!condition)
        {
            parser_error(parser, "Expected condition after DO WHILE");
            return NULL;
        }
        ast_stmt_add_expr(stmt, condition);
        stmt->data.condition_type = 1; /* pre-test while */
    }
    else
    {
        stmt->data.condition_type = 0; /* infinite loop, no condition */
    }

    return stmt;
}

static ASTStmt *parse_loop_stmt(Parser *parser)
{
    advance(parser); /* consume LOOP */

    ASTStmt *stmt = ast_stmt_create(STMT_DO_LOOP);
    stmt->is_loop_end = 1; /* Mark this as LOOP (not DO) */

    /* Check for LOOP UNTIL or LOOP WHILE condition */
    if (match(parser, TOK_UNTIL))
    {
        /* match() already consumed UNTIL, now parse the condition */
        ASTExpr *condition = parse_expression(parser);
        if (!condition)
        {
            parser_error(parser, "Expected condition after LOOP UNTIL");
            return NULL;
        }
        ast_stmt_add_expr(stmt, condition);
        stmt->data.condition_type = 3; /* post-test until */
    }
    else if (match(parser, TOK_WHILE))
    {
        /* match() already consumed WHILE, now parse the condition */
        ASTExpr *condition = parse_expression(parser);
        if (!condition)
        {
            parser_error(parser, "Expected condition after LOOP WHILE");
            return NULL;
        }
        ast_stmt_add_expr(stmt, condition);
        stmt->data.condition_type = 2; /* post-test while */
    }
    else
    {
        stmt->data.condition_type = 0; /* plain LOOP */
    }

    return stmt;
}

static ASTStmt *parse_exit_stmt(Parser *parser)
{
    advance(parser); /* consume EXIT */
    return ast_stmt_create(STMT_EXIT);
}

int parser_has_error(Parser *parser)
{
    return parser ? parser->error_code != 0 : 0;
}

const char *parser_error_message(Parser *parser)
{
    return parser ? parser->error_msg : NULL;
}
