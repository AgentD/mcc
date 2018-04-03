%define api.pure full
%define parse.trace
%define parse.error verbose
%locations

%lex-param {void *scanner}
%parse-param {void *scanner} {function_def_t** result}


%code requires {
#include "mcc.h"
}

%{
#include <string.h>

#include "token.h"
#define YYSTYPE token_t

#include "parser.h"
#include "scanner.h"
#include "ast.h"

void yyerror(YYLTYPE *sloc, yyscan_t *scanner,
	     function_def_t **result, const char *yymsgp);
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



%type <exp> expression
%type <exp> single_expr
%type <exp> mulexpr
%type <exp> addexpr
%type <exp> cmpexpr
%type <lit> literal
%type <args> arguments

%type <exp> parexpr
%type <stmt> if_stmt while_stmt ret_stmt compound_stmt statement stmt_seq
%type <stmt> assignment
%type <type> type
%type <decl> declaration parameters
%type <fun> function_def function_defs program

%start toplevel

%%

toplevel : program { *result = $1; }
         ;

literal : TK_INT_LITERAL    { $$ = $1; $$.line_no = @1.first_line; }
        | TK_FLOAT_LITERAL  { $$ = $1; $$.line_no = @1.first_line; }
        | TK_BOOL_LITERAL   { $$ = $1; $$.line_no = @1.first_line; }
        | TK_STRING_LITERAL { $$ = $1; $$.line_no = @1.first_line; }
        ;

arguments : expression                    { $$ = mcc_mkarg($1, NULL); }
          | expression TK_COMMA arguments { $$ = mcc_mkarg($1, $3); }
          ;

single_expr : literal                                           { $$ = mcc_sex_literal($1); $$->line_no = $1.line_no; }
            | TK_IDENTIFIER                                     { $$ = mcc_sex_identifier($1); $$->line_no = @1.first_line; }
            | TK_MINUS expression                               { $$ = mcc_sex_unary(UNARY_NEG, $2); $$->line_no = @1.first_line; }
            | TK_NOT expression                                 { $$ = mcc_sex_unary(UNARY_INV, $2); $$->line_no = @1.first_line; }
            | TK_PAR_OPEN expression TK_PAR_CLOSE               { $$ = $2; $$->line_no = @1.first_line; }
            | TK_IDENTIFIER TK_BRK_OPEN expression TK_BRK_CLOSE { $$ = mcc_sex_array_access($1, $3); $$->line_no = @2.first_line; }
            | TK_IDENTIFIER TK_PAR_OPEN TK_PAR_CLOSE            { $$ = mcc_sex_call($1, NULL); $$->line_no = @2.first_line; }
            | TK_IDENTIFIER TK_PAR_OPEN arguments TK_PAR_CLOSE  { $$ = mcc_sex_call($1, $3); $$->line_no = @2.first_line; }
            ;

mulexpr : single_expr                     { $$ = $1; }
        | mulexpr TK_ASTERISK single_expr { $$ = mcc_mkexp($1, BINOP_MUL, $3); $$->line_no = @2.first_line; }
        | mulexpr TK_SLASH single_expr    { $$ = mcc_mkexp($1, BINOP_DIV, $3); $$->line_no = @2.first_line; }
        ;

addexpr : mulexpr                         { $$ = $1; }
        | addexpr TK_PLUS mulexpr         { $$ = mcc_mkexp($1, BINOP_ADD, $3); $$->line_no = @2.first_line; }
        | addexpr TK_MINUS mulexpr        { $$ = mcc_mkexp($1, BINOP_SUB, $3); $$->line_no = @2.first_line; }
        ;

cmpexpr : addexpr                         { $$ = $1; }
        | cmpexpr TK_GREATER addexpr      { $$ = mcc_mkexp($1, BINOP_GREATER, $3); $$->line_no = @2.first_line; }
        | cmpexpr TK_LESS addexpr         { $$ = mcc_mkexp($1, BINOP_LESS, $3); $$->line_no = @2.first_line; }
        | cmpexpr TK_GEQ addexpr          { $$ = mcc_mkexp($1, BINOP_GEQ, $3); $$->line_no = @2.first_line; }
        | cmpexpr TK_LEQ addexpr          { $$ = mcc_mkexp($1, BINOP_LEQ, $3); $$->line_no = @2.first_line; }
        | cmpexpr TK_EQU addexpr          { $$ = mcc_mkexp($1, BINOP_EQU, $3); $$->line_no = @2.first_line; }
        | cmpexpr TK_NEQ addexpr          { $$ = mcc_mkexp($1, BINOP_NEQU, $3); $$->line_no = @2.first_line; }
        ;

expression : cmpexpr                      { $$ = $1; }
           | expression TK_ORL cmpexpr    { $$ = mcc_mkexp($1, BINOP_ORL, $3); $$->line_no = @2.first_line; }
           | expression TK_ANL cmpexpr    { $$ = mcc_mkexp($1, BINOP_ANL, $3); $$->line_no = @2.first_line; }
           ;



parexpr          : TK_PAR_OPEN expression TK_PAR_CLOSE { $$ = $2; }
                 ;

if_stmt          : KW_IF parexpr statement                   { $$ = mcc_stmt_branch($2, $3, NULL); $$->line_no = @1.first_line; }
                 | KW_IF parexpr statement KW_ELSE statement { $$ = mcc_stmt_branch($2, $3, $5); $$->line_no = @1.first_line; }
                 ;

while_stmt       : KW_WHILE parexpr statement                { $$ = mcc_stmt_while($2, $3); $$->line_no = @1.first_line; }
                 ;

ret_stmt         : KW_RETURN TK_SEMICOLON                    { $$ = mcc_stmt_return(NULL); $$->line_no = @1.first_line; }
                 | KW_RETURN expression TK_SEMICOLON         { $$ = mcc_stmt_return($2); $$->line_no = @1.first_line; }
                 ;

stmt_seq         : statement                                 { $$ = $1; }
                 | stmt_seq statement                        { $2->next = $1; $$ = $2; }
                 ;

compound_stmt    : TK_BRA_OPEN stmt_seq TK_BRA_CLOSE         { $$ = mcc_stmt_compound($2); $$->line_no = @1.first_line; }
                 | TK_BRA_OPEN TK_BRA_CLOSE                  { $$ = mcc_stmt_compound(NULL); $$->line_no = @1.first_line; }
                 ;

statement        : if_stmt                                   { $$ = $1; }
                 | while_stmt                                { $$ = $1; }
                 | ret_stmt                                  { $$ = $1; }
                 | assignment TK_SEMICOLON                   { $$ = $1; }
                 | compound_stmt                             { $$ = $1; }
                 | declaration TK_SEMICOLON                  { $$ = mcc_stmt_declaration($1); $$->line_no = $1->line_no; }
                 | expression TK_SEMICOLON                   { $$ = mcc_stmt_expression($1); $$->line_no = $1->line_no; }
                 ;

type             : KW_BOOL    { $$ = TYPE_BOOL; }
                 | KW_INT     { $$ = TYPE_INT; }
                 | KW_FLOAT   { $$ = TYPE_FLOAT; }
                 | KW_STRING  { $$ = TYPE_STRING; }
                 ;

declaration      : type TK_IDENTIFIER                                          { $$ = mcc_declaration($1, 1, $2); $$->line_no = @2.first_line; }
                 | type TK_BRK_OPEN TK_INT_LITERAL TK_BRK_CLOSE TK_IDENTIFIER  { $$ = mcc_declaration($1, $3.value.i, $5); $$->line_no = @2.first_line; }
                 ;

assignment       : TK_IDENTIFIER TK_ASSIGN expression                                      { $$ = mcc_stmt_assignment($1, NULL, $3); $$->line_no = @2.first_line; }
                 | TK_IDENTIFIER TK_BRK_OPEN expression TK_BRK_CLOSE TK_ASSIGN expression  { $$ = mcc_stmt_assignment($1, $3, $6); $$->line_no = @5.first_line; }
                 ;

function_def     : type TK_IDENTIFIER TK_PAR_OPEN TK_PAR_CLOSE compound_stmt               { $$ = mcc_function($1, $2, NULL, $5); $$->line_no = @1.first_line; }
                 | KW_VOID TK_IDENTIFIER TK_PAR_OPEN TK_PAR_CLOSE compound_stmt            { $$ = mcc_function(TYPE_VOID, $2, NULL, $5); $$->line_no = @1.first_line; }
                 | type TK_IDENTIFIER TK_PAR_OPEN parameters TK_PAR_CLOSE compound_stmt    { $$ = mcc_function($1, $2, $4, $6); $$->line_no = @1.first_line; }
                 | KW_VOID TK_IDENTIFIER TK_PAR_OPEN parameters TK_PAR_CLOSE compound_stmt { $$ = mcc_function(TYPE_VOID, $2, $4, $6); $$->line_no = @1.first_line; }
                 ;

parameters       : declaration                      { $$ = $1; }
                 | parameters TK_COMMA declaration  { $3->next = $1; $$ = $3; }
                 ;

function_defs    : function_def                     { $$ = $1; }
                 | function_defs function_def       { $2->next = $1; $$ = $2; }
                 ;

program          : TK_END                           { $$ = NULL; }
                 | function_defs TK_END             { $$ = $1; }
                 ;
%%
#include <assert.h>

void yyerror(YYLTYPE *sloc, yyscan_t *scanner,
	     function_def_t **result, const char *yymsgp)
{
	(void)sloc; (void)scanner; (void)result; (void)yymsgp;
}

parser_result_t mcc_parse_file(FILE *input)
{
	parser_result_t result = { .status = PARSER_STATUS_OK };
	function_def_t *list = NULL, *prev, *current, *next;
	assert(input);

	yyscan_t scanner;

	mcc_init_program(&result.program);

	yylex_init_extra(&result.program, &scanner);
	yyset_in(input, scanner);

	if (yyparse(scanner, &list) != 0) {
		result.status = PARSER_STATUS_UNKNOWN_ERROR;
	}

	prev = next = NULL;
	current = list;

	while (current != NULL) {
		next = current->next;
		current->next = prev;
		prev = current;
		current = next;
	}

	result.program.functions = prev;
	yylex_destroy(scanner);
	return result;
}

parser_result_t mcc_parse_string(const char *input)
{
	parser_result_t result;
	assert(input);

	FILE *in = fmemopen((void *)input, strlen(input), "r");

	if (!in) {
		result.status = PARSER_STATUS_UNABLE_TO_OPEN_STREAM;
		return result;
	}

	result = mcc_parse_file(in);
	fclose(in);
	return result;
}
