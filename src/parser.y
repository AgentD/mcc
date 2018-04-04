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

#define SLOC(astnode, yynode) \
	if (astnode) (astnode)->line_no = (yynode).first_line
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

toplevel         : program                                               { *result = $1; }
                 ;

literal          : TK_INT_LITERAL                                        { $$ = $1; $$.line_no = @1.first_line; }
                 | TK_FLOAT_LITERAL                                      { $$ = $1; $$.line_no = @1.first_line; }
                 | TK_BOOL_LITERAL                                       { $$ = $1; $$.line_no = @1.first_line; }
                 | TK_STRING_LITERAL                                     { $$ = $1; $$.line_no = @1.first_line; }
                 ;

arguments        : expression                                            { $$ = mcc_mkarg($1, NULL); }
                 | expression "," arguments                              { $$ = mcc_mkarg($1, $3); }
                 ;

single_expr      : literal                                               { $$ = mcc_sex_literal($1); $$->line_no = $1.line_no; }
                 | TK_IDENTIFIER                                         { $$ = mcc_sex_identifier($1); SLOC($$, @1); }
                 | "-" expression                                        { $$ = mcc_sex_unary(UNARY_NEG, $2); SLOC($$, @1); }
                 | "!" expression                                        { $$ = mcc_sex_unary(UNARY_INV, $2); SLOC($$, @1); }
                 | "(" expression ")"                                    { $$ = $2; SLOC($$, @1); }
                 | TK_IDENTIFIER "[" expression "]"                      { $$ = mcc_sex_array_access($1, $3); SLOC($$, @2); }
                 | TK_IDENTIFIER "(" ")"                                 { $$ = mcc_sex_call($1, NULL); SLOC($$, @2); }
                 | TK_IDENTIFIER "(" arguments ")"                       { $$ = mcc_sex_call($1, $3); SLOC($$, @2); }
                 ;

mulexpr          : single_expr                                           { $$ = $1; }
                 | mulexpr "*" single_expr                               { $$ = mcc_mkexp($1, BINOP_MUL, $3); SLOC($$, @2); }
                 | mulexpr "/" single_expr                               { $$ = mcc_mkexp($1, BINOP_DIV, $3); SLOC($$, @2); }
                 ;

addexpr          : mulexpr                                               { $$ = $1; }
                 | addexpr "+" mulexpr                                   { $$ = mcc_mkexp($1, BINOP_ADD, $3); SLOC($$, @2); }
                 | addexpr "-" mulexpr                                   { $$ = mcc_mkexp($1, BINOP_SUB, $3); SLOC($$, @2); }
                 ;

cmpexpr          : addexpr                                               { $$ = $1; }
                 | cmpexpr ">" addexpr                                   { $$ = mcc_mkexp($1, BINOP_GREATER, $3); SLOC($$, @2); }
                 | cmpexpr "<" addexpr                                   { $$ = mcc_mkexp($1, BINOP_LESS, $3); SLOC($$, @2); }
                 | cmpexpr ">=" addexpr                                  { $$ = mcc_mkexp($1, BINOP_GEQ, $3); SLOC($$, @2); }
                 | cmpexpr "<=" addexpr                                  { $$ = mcc_mkexp($1, BINOP_LEQ, $3); SLOC($$, @2); }
                 | cmpexpr "==" addexpr                                  { $$ = mcc_mkexp($1, BINOP_EQU, $3); SLOC($$, @2); }
                 | cmpexpr "!=" addexpr                                  { $$ = mcc_mkexp($1, BINOP_NEQU, $3); SLOC($$, @2); }
                 ;

expression       : cmpexpr                                               { $$ = $1; }
                 | expression "||" cmpexpr                               { $$ = mcc_mkexp($1, BINOP_ORL, $3); SLOC($$, @2); }
                 | expression "&&" cmpexpr                               { $$ = mcc_mkexp($1, BINOP_ANL, $3); SLOC($$, @2); }
                 ;



parexpr          : "(" expression ")"                                    { $$ = $2; }
                 ;

if_stmt          : "if" parexpr statement                                { $$ = mcc_stmt_branch($2, $3, NULL); SLOC($$, @1); }
                 | "if" parexpr statement "else" statement               { $$ = mcc_stmt_branch($2, $3, $5); SLOC($$, @1); }
                 ;

while_stmt       : "while" parexpr statement                             { $$ = mcc_stmt_while($2, $3); SLOC($$, @1); }
                 ;

ret_stmt         : "return" ";"                                          { $$ = mcc_stmt_return(NULL); SLOC($$, @1); }
                 | "return" expression ";"                               { $$ = mcc_stmt_return($2); SLOC($$, @1); }
                 ;

stmt_seq         : statement                                             { $$ = $1; }
                 | stmt_seq statement                                    { $2->next = $1; $$ = $2; }
                 ;

compound_stmt    : "{" stmt_seq "}"                                      { $$ = mcc_stmt_compound($2); SLOC($$, @1); }
                 | "{" "}"                                               { $$ = mcc_stmt_compound(NULL); SLOC($$, @1); }
                 ;

statement        : if_stmt                                               { $$ = $1; }
                 | while_stmt                                            { $$ = $1; }
                 | ret_stmt                                              { $$ = $1; }
                 | assignment ";"                                        { $$ = $1; }
                 | compound_stmt                                         { $$ = $1; }
                 | declaration ";"                                       { $$ = mcc_stmt_declaration($1); $$->line_no = $1->line_no; }
                 | expression ";"                                        { $$ = mcc_stmt_expression($1); $$->line_no = $1->line_no; }
                 ;

type             : "bool"                                                { $$ = TYPE_BOOL; }
                 | "int"                                                 { $$ = TYPE_INT; }
                 | "float"                                               { $$ = TYPE_FLOAT; }
                 | "string"                                              { $$ = TYPE_STRING; }
                 ;

declaration      : type TK_IDENTIFIER                                    { $$ = mcc_declaration($1, 1, $2); SLOC($$, @2); }
                 | type "[" TK_INT_LITERAL "]" TK_IDENTIFIER             { $$ = mcc_declaration($1, $3.value.i, $5); SLOC($$, @2); }
                 ;

assignment       : TK_IDENTIFIER "=" expression                          { $$ = mcc_stmt_assignment($1, NULL, $3); SLOC($$, @2); }
                 | TK_IDENTIFIER "[" expression "]" "=" expression       { $$ = mcc_stmt_assignment($1, $3, $6); SLOC($$, @5); }
                 ;

function_def     : type TK_IDENTIFIER "(" ")" compound_stmt              { $$ = mcc_function($1, $2, NULL, $5); SLOC($$, @1); }
                 | "void" TK_IDENTIFIER "(" ")" compound_stmt            { $$ = mcc_function(TYPE_VOID, $2, NULL, $5); SLOC($$, @1); }
                 | type TK_IDENTIFIER "(" parameters ")" compound_stmt   { $$ = mcc_function($1, $2, $4, $6); SLOC($$, @1); }
                 | "void" TK_IDENTIFIER "(" parameters ")" compound_stmt { $$ = mcc_function(TYPE_VOID, $2, $4, $6); SLOC($$, @1); }
                 ;

parameters       : declaration                                           { $$ = $1; }
                 | parameters "," declaration                            { $3->next = $1; $$ = $3; }
                 ;

function_defs    : function_def                                          { $$ = $1; }
                 | function_defs function_def                            { $2->next = $1; $$ = $2; }
                 ;

program          : TK_END                                                { $$ = NULL; }
                 | function_defs TK_END                                  { $$ = $1; }
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
