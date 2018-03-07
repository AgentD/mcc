%define api.pure full
%define parse.trace
%define parse.error verbose

%lex-param {void *scanner}
%parse-param {void *scanner} {expression_t** result}


%code requires {
#include "mcc.h"

void yyerror(yyscan_t *scanner, expression_t **result, const char *yymsgp);
}

%{
#include <string.h>

#include "token.h"
#define YYSTYPE token_t

#include "scanner.h"
#include "ast.h"
%}
%token TK_END 0 "end of file"

%token <lit> TK_INT_LITERAL "integer literal"
%token <lit> TK_FLOAT_LITERAL "float literal"
%token <lit> TK_BOOL_LITERAL "bool literal"
%token <lit> TK_STRING_LITERAL "string literal"
%token <identifier> TK_IDENTIFIER "identifier"

%token TK_SLASH "/"
%token TK_ASTERISK "*"
%token TK_MINUS "-"
%token TK_PLUS "+"
%token TK_NOT "!"
%token TK_LESS "<"
%token TK_GREATER ">"
%token TK_LEQ "<="
%token TK_GEQ ">="
%token TK_ANL "&&"
%token TK_ORL "||"
%token TK_EQU "=="
%token TK_NEQ "!="
%token TK_ASSIGN "="
%token TK_BRK_OPEN "["
%token TK_BRK_CLOSE "]"
%token TK_BRA_OPEN "{"
%token TK_BRA_CLOSE "}"
%token TK_PAR_OPEN "("
%token TK_PAR_CLOSE ")"
%token TK_COMMA ","
%token TK_SEMICOLON ";"

%token KW_BOOL "bool"
%token KW_INT "int"
%token KW_FLOAT "float"
%token KW_STRING "string"
%token KW_IF "if"
%token KW_ELSE "else"
%token KW_WHILE "while"
%token KW_RETURN "return"
%token KW_VOID "void"



%type <binop> binary_op

%type <exp> expression
%type <sexp> single_expr
%type <lit> literal
%type <unop> unary_op
%type <args> arguments

%start toplevel

%%

toplevel : expression { *result = $1; }
         ;

unary_op  : TK_MINUS    { $$ = UNARY_NEG; }
          | TK_NOT      { $$ = UNARY_INV; }
          ;

binary_op : TK_PLUS     { $$ = BINOP_ADD; }
          | TK_MINUS    { $$ = BINOP_SUB; }
          | TK_ASTERISK { $$ = BINOP_MUL; }
          | TK_SLASH    { $$ = BINOP_DIV; }
          | TK_LESS     { $$ = BINOP_LESS; }
          | TK_GREATER  { $$ = BINOP_GREATER; }
          | TK_LEQ      { $$ = BINOP_LEQ; }
          | TK_GEQ      { $$ = BINOP_GEQ; }
          | TK_ORL      { $$ = BINOP_ORL; }
          | TK_ANL      { $$ = BINOP_ANL; }
          | TK_EQU      { $$ = BINOP_EQU; }
          | TK_NEQ      { $$ = BINOP_NEQU; }
          ;

single_expr : literal                             { $$ = sex_literal($1); }
            | TK_IDENTIFIER                       { $$ = sex_identifier($1); }
            | unary_op expression                 { $$ = sex_unary($1, $2); }
            | TK_PAR_OPEN expression TK_PAR_CLOSE { $$ = sex_nested($2); }
            | TK_IDENTIFIER TK_BRK_OPEN expression TK_BRK_CLOSE { $$ = sex_array_access($1, $3); }
            | TK_IDENTIFIER TK_PAR_OPEN TK_PAR_CLOSE            { $$ = sex_call($1, NULL); }
            | TK_IDENTIFIER TK_PAR_OPEN arguments TK_PAR_CLOSE  { $$ = sex_call($1, $3); }
            ;

arguments : expression                    { $$ = mkarg($1, NULL); }
          | expression TK_COMMA arguments { $$ = mkarg($1, $3); }
          ;

expression : single_expr                      { $$ = mkexp($1, BINOP_NOP, NULL); }
           | single_expr binary_op expression { $$ = mkexp($1, $2, $3);          }
           ;

literal : TK_INT_LITERAL    { $$ = $1; }
        | TK_FLOAT_LITERAL  { $$ = $1; }
        | TK_BOOL_LITERAL   { $$ = $1; }
        | TK_STRING_LITERAL { $$ = $1; }
        ;

%%
#include <assert.h>

void yyerror(yyscan_t *scanner, expression_t **result, const char *yymsgp)
{
	(void)scanner; (void)result; (void)yymsgp;
}

parser_result_t parse_file(FILE *input)
{
	parser_result_t result = { .status = PARSER_STATUS_OK };
	assert(input);

	yyscan_t scanner;

	yylex_init(&scanner);
	yyset_in(input, scanner);

	if (yyparse(scanner, &result.expression) != 0) {
		result.status = PARSER_STATUS_UNKNOWN_ERROR;
	}

	yylex_destroy(scanner);
	return result;
}

parser_result_t parse_string(const char *input)
{
	parser_result_t result;
	assert(input);

	FILE *in = fmemopen((void *)input, strlen(input), "r");

	if (!in) {
		result.status = PARSER_STATUS_UNABLE_TO_OPEN_STREAM;
		return result;
	}

	result = parse_file(in);
	fclose(in);
	return result;
}
