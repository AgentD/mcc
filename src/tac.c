#include <assert.h>

#include "bitmap.h"
#include "mcc.h"
#include "tac.h"

static bool tac_is_used(mcc_tac_inst_t *list, mcc_tac_inst_t *ref)
{
	int i;

	for (; list != NULL; list = list->next) {
		for (i = 0; i < 2; ++i) {
			if (list->arg[i].type == TAC_ARG_RESULT) {
				if (list->arg[i].u.ref == ref)
					return true;
			}
		}
	}

	return false;
}

static bool tac_is_id_used(mcc_tac_inst_t *tac, unsigned int id)
{
	for (; tac != NULL; tac = tac->next) {
		switch (tac->op) {
		case TAC_CALL:
			if (tac->type.type == TAC_TYPE_NONE)
				break;
			/* fall-through */
		case TAC_OP_PHI:
		case TAC_IMMEDIATE:
		case TAC_OP_ADD:
		case TAC_OP_SUB:
		case TAC_OP_MUL:
		case TAC_OP_DIV:
		case TAC_OP_LT:
		case TAC_OP_GT:
		case TAC_OP_LEQ:
		case TAC_OP_GEQ:
		case TAC_OP_EQU:
		case TAC_OP_NEQU:
		case TAC_OP_NEG:
		case TAC_OP_INV:
		case TAC_OP_FTOI:
		case TAC_OP_ITOF:
		case TAC_LOAD:
		case TAC_COPY:
			if (tac->num == id)
				return true;
			break;
		default:
			break;
		}
	}
	return false;
}

static void tac_remap_id(mcc_tac_inst_t *tac, unsigned int id,
			 unsigned int target)
{
	for (; tac != NULL; tac = tac->next) {
		switch (tac->op) {
		case TAC_CALL:
			if (tac->type.type == TAC_TYPE_NONE)
				break;
			/* fall-through */
		case TAC_OP_PHI:
		case TAC_IMMEDIATE:
		case TAC_OP_ADD:
		case TAC_OP_SUB:
		case TAC_OP_MUL:
		case TAC_OP_DIV:
		case TAC_OP_LT:
		case TAC_OP_GT:
		case TAC_OP_LEQ:
		case TAC_OP_GEQ:
		case TAC_OP_EQU:
		case TAC_OP_NEQU:
		case TAC_OP_NEG:
		case TAC_OP_INV:
		case TAC_OP_FTOI:
		case TAC_OP_ITOF:
		case TAC_LOAD:
		case TAC_COPY:
			if (tac->num == id)
				tac->num = target;
			break;
		default:
			break;
		}
	}
}

static void tac_remove_gaps(mcc_tac_inst_t *tac, unsigned int max_id)
{
	unsigned int i, j;

	for (i = 0; i <= max_id; ++i) {
		if (tac_is_id_used(tac, i))
			continue;

		for (j = i + 1; j <= max_id; ++j) {
			if (tac_is_id_used(tac, j)) {
				tac_remap_id(tac, j, i);
				break;
			}
		}
	}
}

static mcc_tac_inst_t *tac_remove_phi(mcc_tac_inst_t *tac)
{
	mcc_tac_inst_t *t, *prev;

	t = tac;
	prev = NULL;

	while (t != NULL) {
		if (t->op == TAC_OP_PHI) {
			if (prev != NULL) {
				prev->next = t->next;
				free(t);
				t = prev->next;
			} else {
				tac = t->next;
				free(t);
				t = tac;
			}
		} else {
			prev = t;
			t = t->next;
		}
	}

	return tac;
}

mcc_tac_inst_t *mcc_tac_allocate_ids(mcc_tac_inst_t *tac)
{
	unsigned int lblcount = 0, varcount = 0, max_id = 0;
	mcc_tac_inst_t *t;
	bitmap_t map;
	size_t out;
	int i;

	mcc_bitmap_init(&map);

	for (t = tac; t != NULL; t = t->next) {
		for (i = 0; i < 2; ++i) {
			if (t->arg[i].type != TAC_ARG_RESULT)
				continue;

			if (!tac_is_used(t->next, t->arg[i].u.ref))
				mcc_bitmap_free(&map, t->arg[i].u.ref->num);
		}

		switch (t->op) {
		case TAC_LABEL:
			t->num = lblcount++;
			break;
		case TAC_ALLOCA:
			t->num = varcount++;
			break;
		case TAC_OP_PHI:
			assert(t->arg[0].type == TAC_ARG_RESULT);
			assert(t->arg[1].type == TAC_ARG_RESULT);
			t->num = t->arg[0].u.ref->num;
			if (t->arg[1].u.ref->num < t->num)
				t->num = t->arg[1].u.ref->num;
			t->arg[0].u.ref->num = t->num;
			t->arg[1].u.ref->num = t->num;
			break;
		case TAC_CALL:
			if (t->type.type == TAC_TYPE_NONE)
				break;
			/* fall-through */
		case TAC_IMMEDIATE:
		case TAC_OP_ADD:
		case TAC_OP_SUB:
		case TAC_OP_MUL:
		case TAC_OP_DIV:
		case TAC_OP_LT:
		case TAC_OP_GT:
		case TAC_OP_LEQ:
		case TAC_OP_GEQ:
		case TAC_OP_EQU:
		case TAC_OP_NEQU:
		case TAC_OP_NEG:
		case TAC_OP_INV:
		case TAC_OP_FTOI:
		case TAC_OP_ITOF:
		case TAC_LOAD:
		case TAC_COPY:
			mcc_bitmap_allocate(&map, &out);
			t->num = out;
			if (t->num > max_id)
				max_id = t->num;
			break;
		default:
			break;
		}
	}

	mcc_bitmap_cleanup(&map);

	tac_remove_gaps(tac, max_id);

	return tac_remove_phi(tac);
}
