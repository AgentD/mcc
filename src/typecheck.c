#include "mcc.h"


static bool cast_possible(E_TYPE target, E_TYPE current)
{
	switch (target) {
	case TYPE_VOID:
		return false;
	case TYPE_BOOL:
	case TYPE_STRING:
		return current == target;
	case TYPE_INT:
		if (current == TYPE_BOOL)
			return true;
		/* fall-through */
	case TYPE_FLOAT:
		return (current == TYPE_INT || current == TYPE_FLOAT);
	}
	return false;
}

static bool is_numeric(E_TYPE type)
{
	return type == TYPE_INT || type == TYPE_FLOAT;
}

static semantic_result_t type_check_expr(expression_t *expr);

static semantic_result_t check_array_access(decl_t *var, expression_t *index,
					    bool assignment)
{
	semantic_result_t ret = { .status = SEMANTIC_STATUS_OK };

	if ((var->flags & DECL_FLAG_ARRAY) && index == NULL) {
		ret.status = assignment ? SEMANTIC_ASS_MISSING_ARRAY_INDEX :
			     SEMANTIC_MISSING_ARRAY_INDEX;
		return ret;
	}

	if (!(var->flags & DECL_FLAG_ARRAY) && index != NULL) {
		ret.status = assignment ? SEMANTIC_ASS_NOT_ARRAY :
			     SEMANTIC_NOT_ARRAY;
		return ret;
	}

	ret = type_check_expr(index);
	if (ret.status != SEMANTIC_STATUS_OK)
		return ret;

	if (index != NULL && index->datatype != TYPE_INT) {
		ret.status = SEMANTIC_ARRAY_INDEX_NOT_INT;
		ret.u.expr = index;
		return ret;
	}

	return ret;
}

static semantic_result_t type_check_expr(expression_t *expr)
{
	semantic_result_t ret = { .status = SEMANTIC_STATUS_OK };
	E_TYPE type = TYPE_VOID, ltype, rtype;
	expression_t *idx;
	decl_t *var;
	arg_t *arg;
	int i;

	if (expr == NULL)
		return ret;

	switch (expr->type) {
	case SEX_CALL:
		ret.status = SEMANTIC_CALL_UNRESOLVED;
		break;
	case SEX_VAR_ACCESS:
		ret.status = SEMANTIC_UNKNOWN_VAR;
		break;
	case SEX_LITERAL:
		type = expr->u.lit.type;
		break;
	case SEX_RESOLVED_VAR:
		idx = expr->u.var_resolved.index;
		var = expr->u.var_resolved.var;
		type = var->type;
		ret = check_array_access(var, idx, false);
		if (ret.status == SEMANTIC_MISSING_ARRAY_INDEX ||
			ret.status == SEMANTIC_NOT_ARRAY) {
			ret.u.expr = expr;
		}
		break;
	case SEX_UNARY_INV:
		type = TYPE_BOOL;
		ret = type_check_expr(expr->u.unary);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		if (!cast_possible(TYPE_BOOL, expr->u.unary->datatype)) {
			ret.status = SEMANTIC_OP_NOT_BOOLEAN;
			ret.u.expr = expr;
		}
		break;
	case SEX_UNARY_NEG:
		ret = type_check_expr(expr->u.unary);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		type = expr->u.unary->datatype;
		if (!is_numeric(type)) {
			ret.status = SEMANTIC_OP_NOT_NUMERIC;
			ret.u.expr = expr;
		}
		break;
	case BINOP_ANL:
	case BINOP_ORL:
		ret = type_check_expr(expr->u.binary.left);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		ret = type_check_expr(expr->u.binary.right);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;

		ltype = expr->u.binary.left->datatype;
		rtype = expr->u.binary.right->datatype;
		type = TYPE_BOOL;

		if (ltype != TYPE_BOOL || rtype != TYPE_BOOL) {
			ret.status = SEMANTIC_OP_NOT_BOOLEAN;
			ret.u.expr = expr;
		}
		break;
	case BINOP_EQU:
	case BINOP_NEQU:
	case BINOP_LESS:
	case BINOP_GREATER:
	case BINOP_LEQ:
	case BINOP_GEQ:
		type = TYPE_BOOL;
		/* fall-through */
	case BINOP_ADD:
	case BINOP_SUB:
	case BINOP_MUL:
	case BINOP_DIV:
		ret = type_check_expr(expr->u.binary.left);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		ret = type_check_expr(expr->u.binary.right);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;

		ltype = expr->u.binary.left->datatype;
		rtype = expr->u.binary.right->datatype;

		if (!is_numeric(ltype) || !is_numeric(rtype)) {
			ret.status = SEMANTIC_OP_NOT_NUMERIC;
			ret.u.expr = expr;
			break;
		}

		if (type != TYPE_BOOL) {
			if (ltype == TYPE_FLOAT || rtype == TYPE_FLOAT) {
				type = TYPE_FLOAT;
			} else {
				type = TYPE_INT;
			}
		}
		break;
	case SEX_CALL_RESOLVED:
		var = expr->u.call_resolved.fun->parameters;
		arg = expr->u.call_resolved.args;

		while (var != NULL || arg != NULL) {
			if ((var && !arg) || (!var && arg)) {
				ret.status = SEMANTIC_OP_ARG_NUM;
				ret.u.expr = expr;
				break;
			}

			ret = type_check_expr(arg->expr);
			if (ret.status != SEMANTIC_STATUS_OK)
				break;

			if (!cast_possible(var->type, arg->expr->datatype)) {
				ret.status = SEMANTIC_OP_ARG_TYPE;
				ret.u.expr = expr;
				break;
			}

			var = var->next;
			arg = arg->next;
		}

		type = expr->u.call_resolved.fun->type;
		break;
	case SEX_CALL_BUILTIN:
		arg = expr->u.call_builtin.args;
		i = 0;

		for (;;) {
			type = mcc_builtin_param_type(expr->u.call_builtin.id, i);

			if ((!arg && type != TYPE_VOID) ||
			    (arg && type == TYPE_VOID)) {
				ret.status = SEMANTIC_OP_ARG_NUM;
				ret.u.expr = expr;
				break;
			}
			if (!arg)
				break;

			ret = type_check_expr(arg->expr);
			if (ret.status != SEMANTIC_STATUS_OK)
				break;

			if (!cast_possible(type, arg->expr->datatype)) {
				ret.status = SEMANTIC_OP_ARG_TYPE;
				ret.u.expr = expr;
				break;
			}

			arg = arg->next;
			++i;
		}

		type = mcc_builtin_ret_type(expr->u.call_builtin.id);
		break;
	}

	expr->datatype = type;
	return ret;
}

static semantic_result_t type_check_stmt(statement_t *stmt)
{
	semantic_result_t ret = { .status = SEMANTIC_STATUS_OK };

	if (stmt == NULL)
		return ret;

	switch (stmt->type) {
	case STMT_IF:
		ret = type_check_expr(stmt->st.branch.cond);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		ret = type_check_stmt(stmt->st.branch.exec_true);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		return type_check_stmt(stmt->st.branch.exec_false);
	case STMT_WHILE:
		ret = type_check_expr(stmt->st.wloop.cond);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		return type_check_stmt(stmt->st.wloop.body);
	case STMT_RET:
		return type_check_expr(stmt->st.ret);
	case STMT_EXPR:
		return type_check_expr(stmt->st.expr);
	case STMT_DECL:
		break;
	case STMT_ASSIGN:
		ret.status = SEMANTIC_UNKNOWN_LHS;
		ret.u.stmt = stmt;
		break;
	case STMT_COMPOUND:
		stmt = stmt->st.compound_head;

		while (stmt != NULL && ret.status == SEMANTIC_STATUS_OK) {
			ret = type_check_stmt(stmt);
			stmt = stmt->next;
		}
		break;
	case STMT_ASSIGN_RESOLVED: {
		decl_t *target = stmt->st.assign_resolved.target;
		expression_t *key = stmt->st.assign_resolved.array_index;
		expression_t *value = stmt->st.assign_resolved.value;

		ret = check_array_access(target, key, true);
		ret.u.stmt = stmt;
		if (ret.status != SEMANTIC_STATUS_OK)
			break;

		ret = type_check_expr(value);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;

		if (!cast_possible(target->type, value->datatype)) {
			ret.status = SEMANTIC_ASS_MISMATCH;
			ret.u.stmt = stmt;
			break;
		}
		break;
	}
	}

	return ret;
}

semantic_result_t mcc_type_check(program_t *prog)
{
	semantic_result_t ret = { .status = SEMANTIC_STATUS_OK };
	function_def_t *f;

	for (f = prog->functions; f != NULL; f = f->next) {
		ret = type_check_stmt(f->body);

		if (ret.status != SEMANTIC_STATUS_OK)
			break;
	}

	return ret;
}
