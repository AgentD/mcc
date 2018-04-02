%option batch
%option bison-bridge
%option noinput
%option nounput
%option noyywrap
%option reentrant
%option extra-type="program_t *"

%{
#include "parser.h"
#include "token.h"
#include "ast.h"

#include <stdlib.h>

#define YYSTYPE token_t
%}

identifier    [a-zA-Z_][a-zA-Z0-9_]*
str_literal   \"[^\"]*\"
int_literal   [0-9]+
float_literal [0-9]+\.[0-9]+

%%

"true"            { yylval->lit = literal_bool(true); return TK_BOOL_LITERAL; }
"false"           { yylval->lit = literal_bool(false); return TK_BOOL_LITERAL; }

"bool"            { return KW_BOOL; }
"int"             { return KW_INT; }
"float"           { return KW_FLOAT; }
"string"          { return KW_STRING; }
"if"              { return KW_IF; }
"else"            { return KW_ELSE; }
"while"           { return KW_WHILE; }
"return"          { return KW_RETURN; }
"void"            { return KW_VOID; }

"<="              { return TK_LEQ; }
">="              { return TK_GEQ; }
"&&"              { return TK_ANL; }
"||"              { return TK_ORL; }
"=="              { return TK_EQU; }
"!="              { return TK_NEQ; }
"/"               { return TK_SLASH; }
"*"               { return TK_ASTERISK; }
"-"               { return TK_MINUS; }
"+"               { return TK_PLUS; }
"!"               { return TK_NOT; }
"<"               { return TK_LESS; }
">"               { return TK_GREATER; }
"="               { return TK_ASSIGN; }
"["               { return TK_BRK_OPEN; }
"]"               { return TK_BRK_CLOSE; }
"{"               { return TK_BRA_OPEN; }
"}"               { return TK_BRA_CLOSE; }
"("               { return TK_PAR_OPEN; }
")"               { return TK_PAR_CLOSE; }
","               { return TK_COMMA; }
";"               { return TK_SEMICOLON; }

{identifier}      {
	program_t *p = yyextra;

	yylval->identifier = str_tab_add(&p->identifiers, yytext);
	/* TODO: handle allocation error */

	return TK_IDENTIFIER;
}

{str_literal}     {
	program_t *p = yyextra;

	yylval->lit = literal_string(str_tab_add(&p->strings, yytext));
	/* TODO: handle allocation error */

	return TK_STRING_LITERAL;
}

{int_literal}     {
	yylval->lit = literal_int(strtol(yytext, NULL, 10));
	return TK_INT_LITERAL;
}

{float_literal}   {
	yylval->lit = literal_float(strtof(yytext, NULL));
	return TK_FLOAT_LITERAL;
}

[ \t\r\n]+        { /* ignore */ }

<<EOF>>           { return TK_END; }

. { fprintf(stderr, "invalid character '%c'\n", yytext[0]); }

