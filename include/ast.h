#ifndef MCC_AST_H
#define MCC_AST_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>

#include "statement.h"
#include "str_tab.h"
#include "literal.h"
#include "decl.h"
#include "expr.h"
#include "tac.h"

typedef struct function_def_t function_def_t;
typedef struct program_t program_t;

struct function_def_t {
	E_TYPE type;

	off_t identifier;
	decl_t *parameters;
	statement_t *body;

	unsigned int line_no;

	function_def_t *next;
};

struct program_t {
	function_def_t *functions;
	off_t builtins[BUILTIN_MAX];

	str_tab_t strings;
	str_tab_t identifiers;

	char *error_msg;
	unsigned int error_line;
};

#ifdef __cplusplus
extern "C" {
#endif

void mcc_init_program(program_t *prog);

void mcc_cleanup_program(program_t *prog);

function_def_t *mcc_function(E_TYPE type, off_t identifier,
			     decl_t *parameters, statement_t *body);

void mcc_function_free(function_def_t *f);

mcc_tac_inst_t *mcc_function_to_tac(function_def_t *fun);

#ifdef __cplusplus
}
#endif

#endif /* MCC_AST_H */

