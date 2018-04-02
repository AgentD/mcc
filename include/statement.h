#ifndef STATEMENT_H
#define STATEMENT_H

#include "expr.h"
#include "decl.h"

typedef struct statement_t statement_t;

typedef enum {
	STMT_IF,
	STMT_WHILE,
	STMT_RET,
	STMT_DECL,
	STMT_ASSIGN,
	STMT_EXPR,
	STMT_COMPOUND
} E_STATEMENT;

struct statement_t {
	E_STATEMENT type;
	statement_t *next;

	union {
		struct {
			expression_t *cond;
			statement_t *exec_true;
			statement_t *exec_false;
		} branch;

		struct {
			expression_t *cond;
			statement_t *body;
		} wloop;

		struct {
			off_t identifier;
			expression_t *array_index;
			expression_t *value;
		} assignment;

		expression_t *ret;

		statement_t *compound_head;

		decl_t *decl;

		expression_t *expr;
	} st;
};

#ifdef __cplusplus
extern "C" {
#endif

statement_t *mcc_stmt_branch(expression_t *cond, statement_t *exec_true,
			     statement_t *exec_false);

statement_t *mcc_stmt_while(expression_t *cond, statement_t *body);

statement_t *mcc_stmt_return(expression_t *exp);

statement_t *mcc_stmt_compound(statement_t *list);

statement_t *mcc_stmt_expression(expression_t *expr);

statement_t *mcc_stmt_declaration(decl_t *decl);

statement_t *mcc_stmt_assignment(off_t identifier, expression_t *array_index,
				 expression_t *value);

void mcc_stmt_free(statement_t *stmt);

#ifdef __cplusplus
}
#endif

#endif /* STATEMENT_H */

