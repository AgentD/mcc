#include <stdlib.h>

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
	case STMT_ASSIGN_VAR:
	case STMT_ASSIGN_PARAM:
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
