#include <stdlib.h>

#include "ast.h"

single_expr_t *sex_literal(literal_t lit)
{
	single_expr_t *s = calloc(1, sizeof(*s));

	s->type = SEX_LITERAL;
	s->u.lit = lit;
	return s;
}

single_expr_t *sex_identifier(off_t identifier)
{
	single_expr_t *s = calloc(1, sizeof(*s));

	s->type = SEX_IDENTIFIER;
	s->u.identifier = identifier;
	return s;
}

single_expr_t *sex_nested(expression_t *nested)
{
	single_expr_t *s = calloc(1, sizeof(*s));

	s->type = SEX_NESTED;
	s->u.nested = nested;
	return s;
}

single_expr_t *sex_unary(E_UNARY op, expression_t *exp)
{
	single_expr_t *s = calloc(1, sizeof(*s));

	s->type = SEX_UNARY;
	s->u.unary.op = op;
	s->u.unary.exp = exp;
	return s;
}

single_expr_t *sex_array_access(off_t identifier, expression_t *index)
{
	single_expr_t *s = calloc(1, sizeof(*s));

	s->type = SEX_ARRAY_INDEX;
	s->u.array_idx.identifier = identifier;
	s->u.array_idx.index = index;
	return s;
}

single_expr_t *sex_call(off_t identifier, arg_t *args)
{
	single_expr_t *s = calloc(1, sizeof(*s));

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

void sex_free(single_expr_t *sex)
{
	arg_t *arg;

	if (sex != NULL) {
		switch (sex->type) {
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
		case SEX_NESTED:
			expr_free(sex->u.nested);
			break;
		default:
			break;
		}

		free(sex);
	}
}

expression_t *mkexp(single_expr_t *left, E_BINOP binop, expression_t *right)
{
	expression_t *e = calloc(1, sizeof(*e));

	e->operation = binop;
	e->left = left;
	e->right = right;
	return e;
}

void expr_free(expression_t *expr)
{
	if (expr != NULL) {
		sex_free(expr->left);
		expr_free(expr->right);
		free(expr);
	}
}
