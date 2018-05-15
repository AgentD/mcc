#include <stdlib.h>
#include <assert.h>

#include "expr.h"
#include "decl.h"
#include "ast.h"

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

expression_t *mcc_sex_unary(E_EXPR_TYPE op, expression_t *exp)
{
	expression_t *s = calloc(1, sizeof(*s));

	s->type = op;
	s->u.unary = exp;
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
		case SEX_UNARY_NEG:
		case SEX_UNARY_INV:
			mcc_expr_free(sex->u.unary);
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

E_TYPE mcc_builtin_ret_type(E_BUILTIN_FUN id)
{
	switch (id) {
	case BUILTIN_READ_INT:
		return TYPE_INT;
	case BUILTIN_READ_FLOAT:
		return TYPE_FLOAT;
	default:
		break;
	}

	return TYPE_VOID;
}

E_TYPE mcc_builtin_param_type(E_BUILTIN_FUN id, int num)
{
	switch (id) {
	case BUILTIN_PRINT:
		return num == 0 ? TYPE_STRING : TYPE_VOID;
	case BUILTIN_PRINT_INT:
		return num == 0 ? TYPE_INT : TYPE_VOID;
	case BUILTIN_PRINT_FLOAT:
		return num == 0 ? TYPE_FLOAT : TYPE_VOID;
	default:
		break;
	}

	return TYPE_VOID;
}

/*****************************************************************************/

static unsigned int count_args(arg_t *args)
{
	unsigned int i = 0;

	while (args != NULL) {
		args = args->next;
		++i;
	}
	return i;
}

static mcc_tac_inst_t *serialize_args(arg_t *args, mcc_tac_inst_t **results)
{
	mcc_tac_inst_t *list = NULL, *n;
	unsigned int i = 0;

	while (args != NULL) {
		if (list == NULL) {
			list = n = mcc_expr_to_tac(args->expr);
		} else {
			n->next = mcc_expr_to_tac(args->expr);
		}

		while (n->next != NULL)
			n = n->next;

		results[i++] = n;
		args = args->next;
	}

	return list;
}

/*****************************************************************************/

static mcc_tac_inst_t *simple_binary(TAC_OPCODE op, expression_t *expr)
{
	mcc_tac_inst_t *list, *n, *l;

	list = n = mcc_expr_to_tac(expr->u.binary.left);
	while (n->next != NULL)
		n = n->next;
	l = n;
	n->next = mcc_expr_to_tac(expr->u.binary.right);
	while (n->next != NULL)
		n = n->next;

	n->next = mcc_mk_tac_node(op);
	n->next->type = mcc_decl_to_tac_type(expr->datatype);
	n->next->arg[0].type = TAC_ARG_RESULT;
	n->next->arg[0].u.ref = l;
	n->next->arg[1].type = TAC_ARG_RESULT;
	n->next->arg[1].u.ref = n;
	return list;
}

static mcc_tac_inst_t *simple_unary(TAC_OPCODE op, expression_t *expr)
{
	mcc_tac_inst_t *n, *list = mcc_expr_to_tac(expr->u.unary);
	for (n = list; n->next != NULL; n = n->next)
		;
	n->next = mcc_mk_tac_node(op);
	n->next->type = mcc_decl_to_tac_type(expr->datatype);
	n->next->arg[0].type = TAC_ARG_RESULT;
	n->next->arg[0].u.ref = n;
	return list;
}

static mcc_tac_inst_t *short_circuit_binary(expression_t *expr, TAC_OPCODE skip_op)
{
	mcc_tac_inst_t *l, *r, *n, *list, *lbl = mcc_mk_tac_node(TAC_LABEL);

	n = list = mcc_expr_to_tac(expr->u.binary.left);
	while (n->next != NULL)
		n = n->next;
	l = n;

	n->next = mcc_mk_tac_node(skip_op);
	n->next->arg[0].type = TAC_ARG_RESULT;
	n->next->arg[0].u.ref = l;
	n->next->arg[1].type = TAC_ARG_LABEL;
	n->next->arg[1].u.ref = lbl;
	n = n->next;

	n->next = mcc_expr_to_tac(expr->u.binary.right);
	while (n->next != NULL)
		n = n->next;
	r = n;

	n->next = lbl;
	n = n->next;

	n->next = mcc_mk_tac_node(TAC_OP_PHI);
	n->next->type = mcc_decl_to_tac_type(expr->datatype);
	n->next->arg[0].type = TAC_ARG_RESULT;
	n->next->arg[0].u.ref = l;
	n->next->arg[1].type = TAC_ARG_RESULT;
	n->next->arg[1].u.ref = r;
	return list;
}

/*****************************************************************************/

static mcc_tac_inst_t *literal_node(literal_t value)
{
	mcc_tac_inst_t *n = mcc_mk_tac_node(TAC_IMMEDIATE);

	n->type = mcc_decl_to_tac_type(value.type);

	switch (value.type) {
	case TYPE_BOOL:
		n->arg[0].type = TAC_ARG_IMM_INT;
		n->arg[0].u.ival = value.value.b ? 1 : 0;
		break;
	case TYPE_INT:
		n->arg[0].type = TAC_ARG_IMM_INT;
		n->arg[0].u.ival = value.value.i;
		break;
	case TYPE_FLOAT:
		n->arg[0].type = TAC_ARG_IMM_FLOAT;
		n->arg[0].u.fval = value.value.f;
		break;
	case TYPE_STRING:
		n->arg[0].type = TAC_ARG_STR;
		n->arg[0].u.strval = value.value.str;
		break;
	default:
		assert(0);
	}

	return n;
}

static mcc_tac_inst_t *var_ref(decl_t *var, expression_t *index)
{
	mcc_tac_inst_t *list, *n;

	if (index != NULL) {
		list = n = mcc_expr_to_tac(index);
		while (n->next != NULL)
			n = n->next;
		n->next = mcc_mk_tac_node(TAC_OP_ADD);
		n->next->type = mcc_decl_to_tac_type(var->type);
		n->next->type.ptr_level += 1;
		n->next->arg[0].type = TAC_ARG_VAR;
		n->next->arg[0].u.ref = var->user;
		n->next->arg[1].type = TAC_ARG_RESULT;
		n->next->arg[1].u.ref = n;
		n = n->next;

		n->next = mcc_mk_tac_node(TAC_LOAD);
		n->next->type = list->type;
		n->next->type.ptr_level -= 1;
		n->next->arg[0].type = TAC_ARG_RESULT;
		n->next->arg[0].u.ref = n;
	} else {
		list = mcc_mk_tac_node(TAC_LOAD);
		list->type = mcc_decl_to_tac_type(var->type);
		list->arg[0].type = TAC_ARG_VAR;
		list->arg[0].u.ref = var->user;
	}
	return list;
}

static mcc_tac_inst_t *mk_call(off_t ident, E_TYPE rtype)
{
	mcc_tac_inst_t *n = mcc_mk_tac_node(TAC_CALL);

	n->type = mcc_decl_to_tac_type(rtype);
	n->arg[0].type = TAC_ARG_NAME;
	n->arg[0].u.name = ident;
	return n;
}

static mcc_tac_inst_t *mk_builtin_call(expression_t *expr)
{
	mcc_tac_inst_t *list, *n, **args;
	unsigned int i;
	E_TYPE mcctype;
	off_t ident;

	ident = expr->u.call_builtin.identifier;
	i = count_args(expr->u.call_builtin.args);

	if (!i) {
		mcctype = mcc_builtin_ret_type(expr->u.call_builtin.id);
		return mk_call(ident, mcctype);
	}

	args = alloca(i * sizeof(args[0]));
	list = serialize_args(expr->u.call_builtin.args, args);
	for (n = list; n->next != NULL; n = n->next)
		;

	while (i--) {
		mcctype = mcc_builtin_param_type(expr->u.call_builtin.id, i);

		n->next = mcc_mk_tac_node(TAC_PUSH_ARG);
		n->next->type = mcc_decl_to_tac_type(mcctype);
		n->next->arg[0].type = TAC_ARG_RESULT;
		n->next->arg[0].u.ref = args[i];
		n = n->next;
	}

	mcctype = mcc_builtin_ret_type(expr->u.call_builtin.id);
	n->next = mk_call(ident, mcctype);
	return list;
}

static mcc_tac_inst_t *mk_call_expr(expression_t *expr)
{
	mcc_tac_inst_t *list, *n, **args;
	unsigned int i, j;
	decl_t *param;
	off_t ident;

	ident = expr->u.call_resolved.fun->identifier;
	i = count_args(expr->u.call_resolved.args);
	if (!i)
		return mk_call(ident, expr->u.call_resolved.fun->type);

	args = alloca(i * sizeof(args[0]));
	list = serialize_args(expr->u.call_resolved.args, args);
	for (n = list; n->next != NULL; n = n->next)
		;

	while (i--) {
		n->next = mcc_mk_tac_node(TAC_PUSH_ARG);
		n = n->next;

		n->arg[0].type = TAC_ARG_RESULT;
		n->arg[0].u.ref = args[i];

		param = expr->u.call_resolved.fun->parameters;
		for (j = 0; j < i; ++j)
			param = param->next;

		n->type = mcc_decl_to_tac_type(param->type);
		if (param->flags & DECL_FLAG_ARRAY)
			n->type.ptr_level += 1;
	}

	n->next = mk_call(ident, expr->u.call_resolved.fun->type);
	return list;
}

mcc_tac_inst_t *mcc_expr_to_tac(expression_t *expr)
{
	switch (expr->type) {
	case SEX_LITERAL: return literal_node(expr->u.lit);
	case BINOP_ADD: return simple_binary(TAC_OP_ADD, expr);
	case BINOP_SUB: return simple_binary(TAC_OP_SUB, expr);
	case BINOP_MUL: return simple_binary(TAC_OP_MUL, expr);
	case BINOP_DIV: return simple_binary(TAC_OP_DIV, expr);
	case BINOP_LESS: return simple_binary(TAC_OP_LT, expr);
	case BINOP_GREATER: return simple_binary(TAC_OP_GT, expr);
	case BINOP_LEQ: return simple_binary(TAC_OP_LEQ, expr);
	case BINOP_GEQ: return simple_binary(TAC_OP_GEQ, expr);
	case BINOP_EQU: return simple_binary(TAC_OP_EQU, expr);
	case BINOP_NEQU: return simple_binary(TAC_OP_NEQU, expr);
	case SEX_UNARY_NEG: return simple_unary(TAC_OP_NEG, expr);
	case SEX_UNARY_INV: return simple_unary(TAC_OP_INV, expr);
	case BINOP_ANL: return short_circuit_binary(expr, TAC_JZ);
	case BINOP_ORL: return short_circuit_binary(expr, TAC_JNZ);
	case SEX_CALL_BUILTIN: return mk_builtin_call(expr);
	case SEX_CALL_RESOLVED: return mk_call_expr(expr);
	case SEX_RESOLVED_VAR:
		return var_ref(expr->u.var_resolved.var,
			       expr->u.var_resolved.index);
	default:
		break;
	}

	assert(0);
}
