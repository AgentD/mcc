#ifndef MCC_EXPR_H
#define MCC_EXPR_H

#include "literal.h"

typedef struct expression_t expression_t;
typedef struct arg_t arg_t;

struct arg_t {
	expression_t *expr;
	arg_t *next;
};

typedef enum {
	BUILTIN_PRINT = 0,
	BUILTIN_PRINT_NL,
	BUILTIN_PRINT_INT,
	BUILTIN_PRINT_FLOAT,
	BUILTIN_READ_INT,
	BUILTIN_READ_FLOAT,

	BUILTIN_MAX,
} E_BUILTIN_FUN;

typedef enum {
	BINOP_ADD,
	BINOP_SUB,
	BINOP_MUL,
	BINOP_DIV,
	BINOP_LESS,
	BINOP_GREATER,
	BINOP_LEQ,	/**< \brief Less or equal */
	BINOP_GEQ,	/**< \brief Greater or equal */
	BINOP_ANL,	/**< \brief Logic and */
	BINOP_ORL,	/**< \brief Logic or */
	BINOP_EQU,	/**< \brief Equality test */
	BINOP_NEQU,	/**< \brief Not equal */

	SEX_LITERAL,
	SEX_VAR_ACCESS,
	SEX_CALL,

	SEX_UNARY_NEG,	/**< \brief Negation. Usually two's complement */
	SEX_UNARY_INV,	/**< \brief Logical inverse */

	/** \brief A call after resolving function names */
	SEX_CALL_RESOLVED,

	/** \brief A call resolved to a builtin function */
	SEX_CALL_BUILTIN,

	/** \brief Access to a variable */
	SEX_RESOLVED_VAR,
} E_EXPR_TYPE;

struct expression_t {
	E_EXPR_TYPE type;
	E_TYPE datatype;
	unsigned int line_no;

	union {
		literal_t lit;

		struct {
			off_t identifier;
			expression_t *index;
		} var;

		struct {
			struct decl_t *var;
			expression_t *index;
		} var_resolved;

		struct {
			off_t identifier;
			arg_t *args;
		} call;

		struct {
			struct function_def_t *fun;
			arg_t *args;
		} call_resolved;

		struct {
			E_BUILTIN_FUN id;
			off_t identifier;
			arg_t *args;
		} call_builtin;

		expression_t *unary;

		struct {
			expression_t *left;
			expression_t *right;
		} binary;
	} u;
};

#ifdef __cplusplus
extern "C" {
#endif

expression_t *mcc_sex_literal(literal_t lit);
expression_t *mcc_sex_identifier(off_t identifier);
expression_t *mcc_sex_unary(E_EXPR_TYPE op, expression_t *exp);
expression_t *mcc_sex_array_access(off_t identifier, expression_t *index);
expression_t *mcc_sex_call(off_t identifier, arg_t *args);

arg_t *mcc_mkarg(expression_t *expr, arg_t *rhs);

expression_t *mcc_mkexp(expression_t *left, E_EXPR_TYPE type,
			expression_t *right);

void mcc_expr_free(expression_t *sex);

const char *mcc_builtin_name(E_BUILTIN_FUN id);

E_TYPE mcc_builtin_ret_type(E_BUILTIN_FUN id);

E_TYPE mcc_builtin_param_type(E_BUILTIN_FUN id, int num);

#ifdef __cplusplus
}
#endif

#endif /* MCC_EXPR_H */

