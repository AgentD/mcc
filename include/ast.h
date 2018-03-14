#ifndef MCC_AST_H
#define MCC_AST_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>

#include "str_tab.h"

#ifdef _MSC_VER
	#define FORCE_INLINE __forceinline
#else
	#define FORCE_INLINE __inline__ __attribute__((always_inline))
#endif

/**
 * \enum E_TYPE
 *
 * \brief Type of a literal (\ref literal_t), return type of a
 *        function (\ref function_def_t) or variable type (\ref decl_t)
 */
typedef enum {
	TYPE_VOID,	/**< \brief Only valid for \ref function_def_t */
	TYPE_BOOL,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING
} E_TYPE;

typedef struct {
	E_TYPE type;

	union {
		bool b;
		int i;
		float f;
		off_t str;
	} value;
} literal_t;

static FORCE_INLINE literal_t literal_bool(bool value)
{
	literal_t l = { .type = TYPE_BOOL, .value = { .b = value } };

	return l;
}

static FORCE_INLINE literal_t literal_int(int value)
{
	literal_t l = { .type = TYPE_INT, .value = { .i = value } };

	return l;
}

static FORCE_INLINE literal_t literal_float(float value)
{
	literal_t l = { .type = TYPE_FLOAT, .value = { .f = value } };

	return l;
}

static FORCE_INLINE literal_t literal_string(off_t value)
{
	literal_t l = { .type = TYPE_STRING, .value = { .str = value } };

	return l;
}

/****************************************************************************/

typedef struct {
	E_TYPE type;
	unsigned int array_size;
	off_t identifier;
} decl_t;

/******************************** expression ********************************/

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

expression_t *sex_literal(literal_t lit);
expression_t *sex_identifier(off_t identifier);
expression_t *sex_unary(E_UNARY op, expression_t *exp);
expression_t *sex_array_access(off_t identifier, expression_t *index);
expression_t *sex_call(off_t identifier, arg_t *args);

arg_t *mkarg(expression_t *expr, arg_t *rhs);

expression_t *mkexp(expression_t *left, E_EXPR_TYPE type, expression_t *right);

void expr_free(expression_t *sex);

#ifdef __cplusplus
}
#endif

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

program_t *mcc_get_program(void);

void mcc_set_program(program_t *prog);

#ifdef __cplusplus
}
#endif

#endif /* MCC_AST_H */

