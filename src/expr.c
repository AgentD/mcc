#include <stdlib.h>

#include "expr.h"

expression_t *sex_literal(literal_t lit)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_LITERAL;
	s->u.lit = lit;
	return s;
}

expression_t *sex_identifier(off_t identifier)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_IDENTIFIER;
	s->u.identifier = identifier;
	return s;
}

expression_t *sex_unary(E_UNARY op, expression_t *exp)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_UNARY;
	s->u.unary.op = op;
	s->u.unary.exp = exp;
	return s;
}

expression_t *sex_array_access(off_t identifier, expression_t *index)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_ARRAY_INDEX;
	s->u.array_idx.identifier = identifier;
	s->u.array_idx.index = index;
	return s;
}

expression_t *sex_call(off_t identifier, arg_t *args)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_CALL;
	s->u.call.identifier = identifier;
	s->u.call.args = args;
	return s;
}

arg_t *mkarg(expression_t *expr, arg_t *rhs)
{
	arg_t *arg = calloc(1, sizeof(*arg));

	arg->expr = expr;
	arg->next = rhs;
	return arg;
}

expression_t *mkexp(expression_t *left, E_EXPR_TYPE type, expression_t *right)
{
	expression_t *e = calloc(1, sizeof(*e));

	e->type = type;
	e->u.binary.left = left;
	e->u.binary.right = right;
	return e;
}

void expr_free(expression_t *sex)
{
	arg_t *arg;

	if (sex != NULL) {
		switch (sex->type) {
		case SEX_LITERAL:
		case SEX_IDENTIFIER:
			break;
		case SEX_ARRAY_INDEX:
			expr_free(sex->u.array_idx.index);
			break;
		case SEX_CALL:
			while (sex->u.call.args != NULL) {
				arg = sex->u.call.args;
				sex->u.call.args = arg->next;

				expr_free(arg->expr);
				free(arg);
			}
			break;
		case SEX_UNARY:
			expr_free(sex->u.unary.exp);
			break;
		default:
			expr_free(sex->u.binary.left);
			expr_free(sex->u.binary.right);
			break;
		}

		free(sex);
	}
}
