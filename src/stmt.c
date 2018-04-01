#include <stdlib.h>

#include "ast.h"


statement_t *stmt_branch(expression_t *cond, statement_t *exec_true,
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

statement_t *stmt_while(expression_t *cond, statement_t *body)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_WHILE;
		s->st.wloop.cond = cond;
		s->st.wloop.body = body;
	}
	return s;
}

statement_t *stmt_return(expression_t *exp)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_RET;
		s->st.ret.ret = exp;
	}
	return s;
}

statement_t *stmt_compound(statement_t *list)
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
		s->st.compound.head = prev;
	}
	return s;
}

statement_t *stmt_expression(expression_t *expr)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_EXPR;
		s->st.expr.expr = expr;
	}
	return s;
}

statement_t *stmt_declaration(decl_t *decl)
{
	statement_t *s = calloc(1, sizeof(*s));

	if (s != NULL) {
		s->type = STMT_DECL;
		s->st.decl.decl = decl;
	}
	return s;
}

statement_t *stmt_assignment(off_t identifier, expression_t *array_index,
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

void stmt_free(statement_t *stmt)
{
	statement_t *n;

	if (stmt == NULL)
		return;

	switch (stmt->type) {
	case STMT_IF:
		expr_free(stmt->st.branch.cond);
		stmt_free(stmt->st.branch.exec_true);
		stmt_free(stmt->st.branch.exec_false);
		break;
	case STMT_WHILE:
		expr_free(stmt->st.wloop.cond);
		stmt_free(stmt->st.wloop.body);
		break;
	case STMT_RET:
		expr_free(stmt->st.ret.ret);
		break;
	case STMT_DECL:
		declaration_free(stmt->st.decl.decl);
		break;
	case STMT_ASSIGN:
		expr_free(stmt->st.assignment.array_index);
		expr_free(stmt->st.assignment.value);
		break;
	case STMT_EXPR:
		expr_free(stmt->st.expr.expr);
		break;
	case STMT_COMPOUND:
		while (stmt->st.compound.head != NULL) {
			n = stmt->st.compound.head;
			stmt->st.compound.head = n->next;

			stmt_free(n);
		}
		break;
	}

	free(stmt);
}
