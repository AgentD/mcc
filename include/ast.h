#ifndef MCC_AST_H
#define MCC_AST_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>

#include "str_tab.h"
#include "literal.h"
#include "expr.h"

typedef struct {
	E_TYPE type;
	unsigned int array_size;
	off_t identifier;
} decl_t;

/******************************** statements ********************************/

typedef struct statement_t statement_t;
typedef struct statement_ll_node_t statement_ll_node_t;

struct statement_ll_node_t {
	statement_t *stmt;
	statement_ll_node_t *next;
};

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
			expression_t *ret;
		} ret;

		struct {
			statement_ll_node_t *list;
		} compound;

		struct {
			decl_t *decl;
		} decl;

		struct {
			off_t identifier;
			expression_t *array_index;
			expression_t *value;
		} assignment;

		struct {
			expression_t *expr;
		} expr;
	} st;
};

/****************************************************************************/

typedef struct param_t param_t;
typedef struct function_def_t function_def_t;
typedef struct program_t program_t;

struct param_t {
	decl_t *decl;
	param_t *next;
};

struct function_def_t {
	E_TYPE type;

	off_t identifier;
	param_t *parameters;
	statement_t *body;

	function_def_t *next;
};

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

