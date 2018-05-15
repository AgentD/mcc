#ifndef MCC_TAC_H
#define MCC_TAC_H

#include <stdlib.h>

#include "literal.h"

typedef enum {
	TAC_TYPE_NONE,
	TAC_TYPE_INT,
	TAC_TYPE_FLOAT,
	TAC_TYPE_BYTE,
} TAC_TYPE;

typedef struct {
	TAC_TYPE type;
	unsigned int ptr_level;
} tac_type_t;

typedef enum {
	TAC_BEGIN_FUNCTION = 0,
	TAC_END_FUNCTION,

	TAC_FUN_PARAM,
	TAC_ALLOCA,

	TAC_LABEL,
	TAC_JZ,
	TAC_JNZ,
	TAC_JMP,
	TAC_RET,
	TAC_STORE,

	/** \brief Load address of variable */
	TAC_LOAD_ADDRESS,
	/** \brief Load immediate value */
	TAC_IMMEDIATE,
	/** \brief Dereference value */
	TAC_LOAD,

	TAC_OP_ADD,
	TAC_OP_SUB,
	TAC_OP_MUL,
	TAC_OP_DIV,
	TAC_OP_LT,
	TAC_OP_GT,
	TAC_OP_LEQ,
	TAC_OP_GEQ,
	TAC_OP_EQU,
	TAC_OP_NEQU,
	TAC_OP_PHI,

	TAC_OP_NEG,
	TAC_OP_INV,

	TAC_PUSH_ARG,
	TAC_CALL,
} TAC_OPCODE;

typedef enum {
	TAC_ARG_UNUSED = 0,

	/** \brief Intermediate result of a previois operation */
	TAC_ARG_RESULT,

	/** \brief Identifier table entry */
	TAC_ARG_NAME,

	/** \brief Immediate integer */
	TAC_ARG_IMM_INT,

	/** \brief Immediate float */
	TAC_ARG_IMM_FLOAT,

	/** \brief String table entry */
	TAC_ARG_STR,

	/** \brief Pointer to label node */
	TAC_ARG_LABEL,

	/** \brief Parameter index or array size */
	TAC_ARG_INDEX,

	/** \brief Pointer to a declaration node */
	TAC_ARG_VAR,
} TAC_ARG_TYPE;

typedef struct mcc_tac_inst_t {
	TAC_OPCODE op;
	tac_type_t type;
	struct mcc_tac_inst_t *next;
	unsigned int num;

	struct {
		TAC_ARG_TYPE type;

		union {
			struct mcc_tac_inst_t *ref;
			off_t name;
			int ival;
			float fval;
			off_t strval;
			unsigned int index;
		} u;
	} arg[2];
} mcc_tac_inst_t;

static FORCE_INLINE void mcc_tac_free(mcc_tac_inst_t *tac)
{
	mcc_tac_inst_t *old;

	while (tac != NULL) {
		old = tac;
		tac = tac->next;
		free(old);
	}
}

static FORCE_INLINE mcc_tac_inst_t *mcc_mk_tac_node(TAC_OPCODE op)
{
	mcc_tac_inst_t *node = calloc(1, sizeof(*node));

	if (node != NULL) {
		node->op = op;
	}

	return node;
}

static FORCE_INLINE tac_type_t mcc_decl_to_tac_type(E_TYPE type)
{
	tac_type_t ret = { .ptr_level = 0 };

	switch (type) {
	case TYPE_BOOL:
	case TYPE_INT:
		ret.type = TAC_TYPE_INT;
		break;
	case TYPE_FLOAT:
		ret.type = TAC_TYPE_FLOAT;
		break;
	case TYPE_STRING:
		ret.ptr_level = 1;
		ret.type = TAC_TYPE_BYTE;
		break;
	default:
		ret.type = TAC_TYPE_NONE;
		break;
	}
	return ret;
}

#endif /* MCC_TAC_H */
