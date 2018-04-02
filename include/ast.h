#ifndef MCC_AST_H
#define MCC_AST_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>

#include "str_tab.h"
#include "literal.h"
#include "expr.h"

typedef struct decl_t {
	E_TYPE type;
	int array_size;
	off_t identifier;

	struct decl_t *next;
} decl_t;

decl_t *declaration(E_TYPE type, int size, off_t identifier);

void declaration_free(decl_t *d);

/******************************** statements ********************************/

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

statement_t *stmt_branch(expression_t *cond, statement_t *exec_true,
			 statement_t *exec_false);

statement_t *stmt_while(expression_t *cond, statement_t *body);

statement_t *stmt_return(expression_t *exp);

statement_t *stmt_compound(statement_t *list);

statement_t *stmt_expression(expression_t *expr);

statement_t *stmt_declaration(decl_t *decl);

statement_t *stmt_assignment(off_t identifier, expression_t *array_index,
			     expression_t *value);

void stmt_free(statement_t *stmt);

/****************************************************************************/

typedef struct function_def_t function_def_t;
typedef struct program_t program_t;

struct function_def_t {
	E_TYPE type;

	off_t identifier;
	decl_t *parameters;
	statement_t *body;

	function_def_t *next;
};

function_def_t *function(E_TYPE type, off_t identifier,
			 decl_t *parameters, statement_t *body);

void function_free(function_def_t *f);

struct program_t {
	function_def_t *functions;

	str_tab_t strings;
	str_tab_t identifiers;
};

#ifdef __cplusplus
extern "C" {
#endif

void mcc_init_program(program_t *prog);

void mcc_cleanup_program(program_t *prog);

#ifdef __cplusplus
}
#endif

#endif /* MCC_AST_H */

