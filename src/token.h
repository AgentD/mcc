#ifndef TOKEN_H
#define TOKEN_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>

#include "ast.h"

typedef union {
	off_t identifier;
	single_expr_t *sexp;
	expression_t *exp;
	E_BINOP binop;
	E_UNARY unop;
	literal_t lit;
	arg_t *args;
} token_t;

#endif /* TOKEN_H */

