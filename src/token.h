#ifndef TOKEN_H
#define TOKEN_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>

#include "ast.h"

typedef union {
	off_t identifier;
	expression_t *exp;
	E_TYPE type;
	literal_t lit;
	arg_t *args;
	decl_t *decl;
	statement_t *stmt;
	function_def_t *fun;
} token_t;

#endif /* TOKEN_H */

