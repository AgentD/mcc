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
	SEX_IDENTIFIER,
	SEX_ARRAY_INDEX,
	SEX_CALL,
	SEX_UNARY,
} E_EXPR_TYPE;

typedef enum {
	UNARY_NEG,	/**< \brief Negation. Usually two's complement */
	UNARY_INV,	/**< \brief Logical inverse */
} E_UNARY;

struct expression_t {
	E_EXPR_TYPE type;

	union {
		literal_t lit;

		off_t identifier;

		struct {
			off_t identifier;
			expression_t *index;
		} array_idx;

		struct {
			off_t identifier;
			arg_t *args;
		} call;

		struct {
			E_UNARY op;
			expression_t *exp;
		} unary;

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
expression_t *mcc_sex_unary(E_UNARY op, expression_t *exp);
expression_t *mcc_sex_array_access(off_t identifier, expression_t *index);
expression_t *mcc_sex_call(off_t identifier, arg_t *args);

arg_t *mcc_mkarg(expression_t *expr, arg_t *rhs);

expression_t *mcc_mkexp(expression_t *left, E_EXPR_TYPE type,
			expression_t *right);

void mcc_expr_free(expression_t *sex);

#ifdef __cplusplus
}
#endif

#endif /* MCC_EXPR_H */

