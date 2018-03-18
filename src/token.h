#ifndef TOKEN_H
#define TOKEN_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>

#include "ast.h"

typedef union {
	off_t identifier;
	expression_t *exp;
	E_UNARY unop;
	literal_t lit;
	arg_t *args;
} token_t;

#endif /* TOKEN_H */

