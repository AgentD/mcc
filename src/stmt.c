#include <stdlib.h>
#include <assert.h>

#include "statement.h"


statement_t *mcc_stmt_branch(expression_t *cond, statement_t *exec_true,
			     statement_t *exec_false)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_IF;
		s->st.branch.cond = cond;
		s->st.branch.exec_true = exec_true;
		s->st.branch.exec_false = exec_false;
	}
	return s;
}

statement_t *mcc_stmt_while(expression_t *cond, statement_t *body)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_WHILE;
		s->st.wloop.cond = cond;
		s->st.wloop.body = body;
	}
	return s;
}

statement_t *mcc_stmt_return(expression_t *exp)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_RET;
		s->st.ret = exp;
	}
	return s;
}

statement_t *mcc_stmt_compound(statement_t *list)
{
	statement_t *s = calloc(1, sizeof(*s));
	statement_t *prev, *current, *next;

	if (s != NULL) {
		prev = next = NULL;
		current = list;

		while (current != NULL) {
			next = current->next;
			current->next = prev;
			prev = current;
			current = next;
		}

		s->type = STMT_COMPOUND;
		s->st.compound_head = prev;
	}
	return s;
}

statement_t *mcc_stmt_expression(expression_t *expr)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_EXPR;
		s->st.expr = expr;
	}
	return s;
}

statement_t *mcc_stmt_declaration(decl_t *decl)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_DECL;
		s->st.decl = decl;
	}
	return s;
}

statement_t *mcc_stmt_assignment(off_t identifier, expression_t *array_index,
				 expression_t *value)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_ASSIGN;
		s->st.assignment.identifier = identifier;
		s->st.assignment.array_index = array_index;
		s->st.assignment.value = value;
	}
	return s;
}

void mcc_stmt_free(statement_t *stmt)
{
	statement_t *n;

	if (stmt == NULL)
		return;

	switch (stmt->type) {
	case STMT_IF:
		mcc_expr_free(stmt->st.branch.cond);
		mcc_stmt_free(stmt->st.branch.exec_true);
		mcc_stmt_free(stmt->st.branch.exec_false);
		break;
	case STMT_WHILE:
		mcc_expr_free(stmt->st.wloop.cond);
		mcc_stmt_free(stmt->st.wloop.body);
		break;
	case STMT_RET:
		mcc_expr_free(stmt->st.ret);
		break;
	case STMT_DECL:
		mcc_declaration_free(stmt->st.decl);
		break;
	case STMT_ASSIGN:
		mcc_expr_free(stmt->st.assignment.array_index);
		mcc_expr_free(stmt->st.assignment.value);
		break;
	case STMT_ASSIGN_RESOLVED:
		mcc_expr_free(stmt->st.assign_resolved.array_index);
		mcc_expr_free(stmt->st.assign_resolved.value);
		break;
	case STMT_EXPR:
		mcc_expr_free(stmt->st.expr);
		break;
	case STMT_COMPOUND:
		while (stmt->st.compound_head != NULL) {
			n = stmt->st.compound_head;
			stmt->st.compound_head = n->next;

			mcc_stmt_free(n);
		}
		break;
	}

	free(stmt);
}

/*****************************************************************************/

static mcc_tac_inst_t *var_to_tac(decl_t *decl)
{
	mcc_tac_inst_t *n = mcc_mk_tac_node(TAC_ALLOCA);

	n->type = mcc_decl_to_tac_type(decl->type);
	n->arg[0].type = TAC_ARG_INDEX;

	if (decl->flags & DECL_FLAG_ARRAY) {
		n->arg[0].u.index = decl->array_size;
	} else {
		n->arg[0].u.index = 1;
	}

	decl->user = n;
	return n;
}

static mcc_tac_inst_t *if_to_tac(statement_t *stmt)
{
	mcc_tac_inst_t *l1, *l2, *n, *list = mcc_expr_to_tac(stmt->st.branch.cond);

	for (n = list; n->next != NULL; n = n->next)
		;

	l1 = mcc_mk_tac_node(TAC_LABEL);

	n->next = mcc_mk_tac_node(TAC_JZ);
	n->next->arg[0].type = TAC_ARG_RESULT;
	n->next->arg[0].u.ref = n;
	n->next->arg[1].type = TAC_ARG_LABEL;
	n->next->arg[1].u.ref = l1;
	n = n->next;

	n->next = mcc_stmt_to_tac(stmt->st.branch.exec_true);
	while (n->next != NULL)
		n = n->next;

	if (stmt->st.branch.exec_false == NULL) {
		n->next = l1;
		return list;
	}

	l2 = mcc_mk_tac_node(TAC_LABEL);

	n->next = mcc_mk_tac_node(TAC_JMP);
	n->next->arg[0].type = TAC_ARG_LABEL;
	n->next->arg[0].u.ref = l2;
	n = n->next;

	n->next = l1;
	n = n->next;

	n->next = mcc_stmt_to_tac(stmt->st.branch.exec_false);
	while (n->next != NULL)
		n = n->next;

	n->next = l2;
	return list;
}

static mcc_tac_inst_t *while_to_tac(statement_t *stmt)
{
	mcc_tac_inst_t *l1 = mcc_mk_tac_node(TAC_LABEL);
	mcc_tac_inst_t *l2 = mcc_mk_tac_node(TAC_LABEL);
	mcc_tac_inst_t *n, *list = l1;

	l1->next = mcc_expr_to_tac(stmt->st.wloop.cond);
	for (n = l1; n->next != NULL; n = n->next)
		;

	n->next = mcc_mk_tac_node(TAC_JZ);
	n->next->arg[0].type = TAC_ARG_RESULT;
	n->next->arg[0].u.ref = n;
	n->next->arg[1].type = TAC_ARG_LABEL;
	n->next->arg[1].u.ref = l2;
	n = n->next;

	n->next = mcc_stmt_to_tac(stmt->st.wloop.body);
	while (n->next != NULL)
		n = n->next;

	n->next = mcc_mk_tac_node(TAC_JMP);
	n->next->arg[0].type = TAC_ARG_LABEL;
	n->next->arg[0].u.ref = l1;
	n = n->next;
	n->next = l2;
	return list;
}

static mcc_tac_inst_t *stmt_list_to_tac(statement_t *stmt)
{
	mcc_tac_inst_t *n, *list = NULL;

	while (stmt != NULL) {
		if (list == NULL) {
			n = list = mcc_stmt_to_tac(stmt);
		} else {
			n->next = mcc_stmt_to_tac(stmt);
		}

		while (n->next != NULL)
			n = n->next;

		stmt = stmt->next;
	}
	return list;
}

static mcc_tac_inst_t *assign_to_tac(statement_t *stmt)
{
	mcc_tac_inst_t *n, *list, *val, *addr;

	n = list = mcc_expr_to_tac(stmt->st.assign_resolved.value);
	for (n = list; n->next != NULL; n = n->next)
		;
	val = n;

	n->next = mcc_mk_tac_node(TAC_LOAD_ADDRESS);
	n->next->type = mcc_decl_to_tac_type(stmt->st.assign_resolved.target->type);
	n->next->type.ptr_level += 1;
	n->next->arg[0].type = TAC_ARG_VAR;
	n->next->arg[0].u.ref = stmt->st.assign_resolved.target->user;
	n = n->next;
	addr = n;

	if (stmt->st.assign_resolved.array_index != NULL) {
		n->next = mcc_expr_to_tac(stmt->st.assign_resolved.array_index);
		while (n->next != NULL)
			n = n->next;
		n->next = mcc_mk_tac_node(TAC_OP_ADD);
		n->next->type = addr->type;
		n->next->arg[0].type = TAC_ARG_RESULT;
		n->next->arg[0].u.ref = addr;
		n->next->arg[1].type = TAC_ARG_RESULT;
		n->next->arg[1].u.ref = n;
		n = n->next;
		addr = n;
	}

	n->next = mcc_mk_tac_node(TAC_STORE);
	n->next->arg[0].type = TAC_ARG_RESULT;
	n->next->arg[0].u.ref = addr;
	n->next->arg[1].type = TAC_ARG_RESULT;
	n->next->arg[1].u.ref = val;
	return list;
}

static mcc_tac_inst_t *ret_to_tac(statement_t *stmt)
{
	mcc_tac_inst_t *list, *n;

	if (stmt->st.ret == NULL)
		return mcc_mk_tac_node(TAC_RET);

	list = mcc_expr_to_tac(stmt->st.ret);
	for (n = list; n->next != NULL; n = n->next)
		;

	n->next = mcc_mk_tac_node(TAC_RET);
	n->next->arg[0].type = TAC_ARG_RESULT;
	n->next->arg[0].u.ref = n;
	return list;
}

mcc_tac_inst_t *mcc_stmt_to_tac(statement_t *stmt)
{
	switch (stmt->type) {
	case STMT_IF: return if_to_tac(stmt);
	case STMT_WHILE: return while_to_tac(stmt);
	case STMT_COMPOUND: return stmt_list_to_tac(stmt->st.compound_head);
	case STMT_EXPR: return mcc_expr_to_tac(stmt->st.expr);
	case STMT_DECL: return var_to_tac(stmt->st.decl);
	case STMT_ASSIGN_RESOLVED: return assign_to_tac(stmt);
	case STMT_RET: return ret_to_tac(stmt);
	default:
		assert(0);
	}
}
