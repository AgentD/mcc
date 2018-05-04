#include <stdlib.h>

#include "expr.h"

expression_t *mcc_sex_literal(literal_t lit)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_LITERAL;
	s->u.lit = lit;
	return s;
}

expression_t *mcc_sex_identifier(off_t identifier)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_IDENTIFIER;
	s->u.identifier = identifier;
	return s;
}

expression_t *mcc_sex_unary(E_UNARY op, expression_t *exp)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_UNARY;
	s->u.unary.op = op;
	s->u.unary.exp = exp;
	return s;
}

expression_t *mcc_sex_array_access(off_t identifier, expression_t *index)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_ARRAY_INDEX;
	s->u.array_idx.identifier = identifier;
	s->u.array_idx.index = index;
	return s;
}

expression_t *mcc_sex_call(off_t identifier, arg_t *args)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = SEX_CALL;
	s->u.call.identifier = identifier;
	s->u.call.args = args;
	return s;
}

arg_t *mcc_mkarg(expression_t *expr, arg_t *rhs)
{
	arg_t *arg = calloc(1, sizeof(*arg));

	arg->expr = expr;
	arg->next = rhs;
	return arg;
}

expression_t *mcc_mkexp(expression_t *left, E_EXPR_TYPE type,
			expression_t *right)
{
	expression_t *e = calloc(1, sizeof(*e));

	e->type = type;
	e->u.binary.left = left;
	e->u.binary.right = right;
	return e;
}

void mcc_expr_free(expression_t *sex)
{
	arg_t *arg, *args;

	if (sex != NULL) {
		args = NULL;

		switch (sex->type) {
		case SEX_LITERAL:
		case SEX_IDENTIFIER:
		case SEX_RESOLVED_PARAM:
		case SEX_RESOLVED_VAR:
		case SEX_ARRAY_IDX_VAR:
		case SEX_ARRAY_IDX_PARAM:
			break;
		case SEX_ARRAY_INDEX:
			mcc_expr_free(sex->u.array_idx.index);
			break;
		case SEX_CALL_RESOLVED:
			args = sex->u.call_resolved.args;
			break;
		case SEX_CALL_BUILTIN:
			args = sex->u.call_builtin.args;
			break;
		case SEX_CALL:
			args = sex->u.call.args;
			break;
		case SEX_UNARY:
			mcc_expr_free(sex->u.unary.exp);
			break;
		default:
			mcc_expr_free(sex->u.binary.left);
			mcc_expr_free(sex->u.binary.right);
			break;
		}

		while (args != NULL) {
			arg = args;
			args = arg->next;

			mcc_expr_free(arg->expr);
			free(arg);
		}

		free(sex);
	}
}
