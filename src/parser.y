%define api.pure full
%define parse.trace
%define parse.error verbose

%lex-param {void *scanner}
%parse-param {void *scanner} {statement_t** result}


%code requires {
#include "mcc.h"

void yyerror(yyscan_t *scanner, statement_t **result, const char *yymsgp);
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



%type <exp> expression
%type <exp> single_expr
%type <exp> mulexpr
%type <exp> addexpr
%type <exp> cmpexpr
%type <lit> literal
%type <unop> unary_op
%type <args> arguments

%type <exp> parexpr
%type <stmt> if_stmt while_stmt ret_stmt compound_stmt statement stmt_seq
%type <stmt> assignment
%type <type> type
%type <decl> declaration

%start toplevel

%%

toplevel : statement { *result = $1; }
         ;

unary_op  : TK_MINUS    { $$ = UNARY_NEG; }
          | TK_NOT      { $$ = UNARY_INV; }
          ;

literal : TK_INT_LITERAL    { $$ = $1; }
        | TK_FLOAT_LITERAL  { $$ = $1; }
        | TK_BOOL_LITERAL   { $$ = $1; }
        | TK_STRING_LITERAL { $$ = $1; }
        ;

arguments : expression                    { $$ = mkarg($1, NULL); }
          | expression TK_COMMA arguments { $$ = mkarg($1, $3); }
          ;

single_expr : literal                             { $$ = sex_literal($1); }
            | TK_IDENTIFIER                       { $$ = sex_identifier($1); }
            | unary_op expression                 { $$ = sex_unary($1, $2); }
            | TK_PAR_OPEN expression TK_PAR_CLOSE { $$ = $2; }
            | TK_IDENTIFIER TK_BRK_OPEN expression TK_BRK_CLOSE { $$ = sex_array_access($1, $3); }
            | TK_IDENTIFIER TK_PAR_OPEN TK_PAR_CLOSE            { $$ = sex_call($1, NULL); }
            | TK_IDENTIFIER TK_PAR_OPEN arguments TK_PAR_CLOSE  { $$ = sex_call($1, $3); }
            ;

mulexpr : single_expr                     { $$ = $1; }
        | mulexpr TK_ASTERISK single_expr { $$ = mkexp($1, BINOP_MUL, $3); }
        | mulexpr TK_SLASH single_expr    { $$ = mkexp($1, BINOP_DIV, $3); }
        ;

addexpr : mulexpr                         { $$ = $1; }
        | addexpr TK_PLUS mulexpr         { $$ = mkexp($1, BINOP_ADD, $3); }
        | addexpr TK_MINUS mulexpr        { $$ = mkexp($1, BINOP_SUB, $3); }
        ;

cmpexpr : addexpr                         { $$ = $1; }
        | cmpexpr TK_GREATER addexpr      { $$ = mkexp($1, BINOP_GREATER, $3); }
        | cmpexpr TK_LESS addexpr         { $$ = mkexp($1, BINOP_LESS, $3); }
        | cmpexpr TK_GEQ addexpr          { $$ = mkexp($1, BINOP_GEQ, $3); }
        | cmpexpr TK_LEQ addexpr          { $$ = mkexp($1, BINOP_LEQ, $3); }
        | cmpexpr TK_EQU addexpr          { $$ = mkexp($1, BINOP_EQU, $3); }
        | cmpexpr TK_NEQ addexpr          { $$ = mkexp($1, BINOP_NEQU, $3); }
        ;

expression : cmpexpr                      { $$ = $1; }
           | expression TK_ORL cmpexpr    { $$ = mkexp($1, BINOP_ORL, $3); }
           | expression TK_ANL cmpexpr    { $$ = mkexp($1, BINOP_ANL, $3); }
           ;



parexpr          : TK_PAR_OPEN expression TK_PAR_CLOSE { $$ = $2; }

if_stmt          : KW_IF parexpr statement                   { $$ = stmt_branch($2, $3, NULL); }
                 | KW_IF parexpr statement KW_ELSE statement { $$ = stmt_branch($2, $3, $5); }
                 ;

while_stmt       : KW_WHILE parexpr statement                { $$ = stmt_while($2, $3); }
                 ;

ret_stmt         : KW_RETURN TK_SEMICOLON                    { $$ = stmt_return(NULL); }
                 | KW_RETURN expression TK_SEMICOLON         { $$ = stmt_return($2); }
                 ;

stmt_seq         : statement                                 { $$ = $1; }
                 | stmt_seq statement                        { $2->next = $1; $$ = $2; }
                 ;

compound_stmt    : TK_BRA_OPEN stmt_seq TK_BRA_CLOSE         { $$ = stmt_compound($2); }
                 | TK_BRA_OPEN TK_BRA_CLOSE                  { $$ = stmt_compound(NULL); }
                 ;

statement        : if_stmt                                   { $$ = $1; }
                 | while_stmt                                { $$ = $1; }
                 | ret_stmt                                  { $$ = $1; }
                 | assignment TK_SEMICOLON                   { $$ = $1; }
                 | compound_stmt                             { $$ = $1; }
                 | declaration TK_SEMICOLON                  { $$ = stmt_declaration($1); }
                 | expression TK_SEMICOLON                   { $$ = stmt_expression($1); }
                 ;

type             : KW_BOOL    { $$ = TYPE_BOOL; }
                 | KW_INT     { $$ = TYPE_INT; }
                 | KW_FLOAT   { $$ = TYPE_FLOAT; }
                 | KW_STRING  { $$ = TYPE_STRING; }
                 ;

declaration      : type TK_IDENTIFIER                                          { $$ = declaration($1, 1, $2); }
                 | type TK_BRK_OPEN TK_INT_LITERAL TK_BRK_CLOSE TK_IDENTIFIER  { $$ = declaration($1, $3.value.i, $5); }
                 ;

assignment       : TK_IDENTIFIER TK_ASSIGN expression                                      { $$ = stmt_assignment($1, NULL, $3); }
                 | TK_IDENTIFIER TK_BRK_OPEN expression TK_BRK_CLOSE TK_ASSIGN expression  { $$ = stmt_assignment($1, $3, $6); }
                 ;

/*function_def     : type TK_IDENTIFIER TK_PAR_OPEN TK_PAR_CLOSE compound_stmt
                 | KW_VOID TK_IDENTIFIER TK_PAR_OPEN TK_PAR_CLOSE compound_stmt
                 | type TK_IDENTIFIER TK_PAR_OPEN parameters TK_PAR_CLOSE compound_stmt
                 | KW_VOID TK_IDENTIFIER TK_PAR_OPEN parameters TK_PAR_CLOSE compound_stmt
                 ;

parameters       = declaration
                 | declaration TK_COMMA parameters
                 ;



function_defs    = function_def
                 | function_defs function_def
                 ;

program          = function_defs TK_END
                 | TK_END
                 ;*/

%%
#include <assert.h>

void yyerror(yyscan_t *scanner, statement_t **result, const char *yymsgp)
{
	(void)scanner; (void)result; (void)yymsgp;
}

parser_result_t parse_file(FILE *input)
{
	parser_result_t result = { .status = PARSER_STATUS_OK };
	assert(input);

	yyscan_t scanner;

	mcc_init_program(&result.program);

	yylex_init(&scanner);
	yylex_init_extra(&result.program, &scanner);
	yyset_in(input, scanner);

	if (yyparse(scanner, &result.statement) != 0) {
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
