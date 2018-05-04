#ifndef MCC_H
#define MCC_H

#include <stdio.h>

#include "ast.h"

typedef enum {
	PARSER_STATUS_OK,
	PARSER_STATUS_UNABLE_TO_OPEN_STREAM,
	PARSER_STATUS_UNKNOWN_ERROR,
	PARSER_STATUS_OUT_OF_MEMORY,
	PARSER_STATUS_PARSE_ERROR,
} E_PARSER_STATUS;

typedef enum {
	SEMANTIC_STATUS_OK,
	SEMANTIC_FUNCTION_REDEF,
	SEMANTIC_MAIN_MISSING,
	SEMANTIC_MAIN_TYPE,
	SEMANTIC_BUILTIN_REDEF,
	SEMANTIC_VAR_REDEF,

	/** \brief Return without value inside non-void function */
	SEMANTIC_RET_NO_VAL,

	/** \brief Return with value inside void function */
	SEMANTIC_RET_VOID,

	/** \brief No return statement at end of non-void function */
	SEMANTIC_NO_RET,

	SEMANTIC_CALL_UNRESOLVED,

	/** \brief Out of memory */
	SEMANTIC_OOM,

	SEMANTIC_UNKNOWN_VAR,

	/** \brief Unknown left hand side in assignment statement */
	SEMANTIC_UNKNOWN_LHS,
} E_SEMANTIC_STATUS;

typedef struct {
	E_PARSER_STATUS status;
	program_t program;
} parser_result_t;

typedef struct {
	E_SEMANTIC_STATUS status;

	union {
		/** \brief Function redefinition */
		struct {
			function_def_t *first;
			function_def_t *second;
		} redef;

		/** \brief Variable redefinition */
		struct {
			decl_t *first;
			decl_t *second;
		} vredef;

		/** \brief Invalid 'main' function */
		function_def_t *main;

		/** \brief Offending statement */
		statement_t *stmt;

		/** \brief Offending expression node */
		expression_t *expr;
	} u;
} semantic_result_t;

#ifdef __cplusplus
extern "C" {
#endif

parser_result_t mcc_parse_file(FILE *input);
parser_result_t mcc_parse_string(const char *input);

semantic_result_t mcc_semantic_check(program_t *prog);

#ifdef __cplusplus
}
#endif

#endif /* MCC_H */

