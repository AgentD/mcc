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
	return mcc_sex_array_access(identifier, NULL);
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

	s->type = SEX_VAR_ACCESS;
	s->u.var.identifier = identifier;
	s->u.var.index = index;
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
			break;
		case SEX_RESOLVED_VAR:
			mcc_expr_free(sex->u.var_resolved.index);
			break;
		case SEX_VAR_ACCESS:
			mcc_expr_free(sex->u.var.index);
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

static const char *builtins[] = {
	"print",
	"print_nl",
	"print_int",
	"print_float",
	"read_int",
	"read_float",
};

const char *mcc_builtin_name(E_BUILTIN_FUN id)
{
	if (id < 0 || id > BUILTIN_MAX)
		return NULL;
	return builtins[id];
}
