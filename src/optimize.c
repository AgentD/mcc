#include "tac.h"


static bool value_pull(mcc_tac_inst_t *tac)
{
	bool change = false;
	size_t i;

	for (; tac != NULL; tac = tac->next) {
		if (tac->op == TAC_OP_PHI)
			continue;

		for (i = 0; i < sizeof(tac->arg) / sizeof(tac->arg[0]); ++i) {
			if (tac->arg[i].type != TAC_ARG_RESULT)
				continue;

			if (tac->arg[i].u.ref->op == TAC_IMMEDIATE) {
				tac->arg[i] = tac->arg[i].u.ref->arg[0];
				change = true;
				continue;
			}

			if (tac->arg[i].u.ref->op == TAC_LOAD) {
				if (tac->arg[i].u.ref->arg[0].type !=
				    TAC_ARG_VAR)
					continue;
				if (tac->arg[i].u.ref->arg[1].type !=
				    TAC_ARG_UNUSED)
					continue;

				tac->arg[i] = tac->arg[i].u.ref->arg[0];
				change = true;
				continue;
			}
		}
	}

	return change;
}

static bool remove_unused(mcc_tac_inst_t *tac, mcc_tac_inst_t **out)
{
	mcc_tac_inst_t *t, *prev;
	bool change = false;
	size_t i;

	/* abuse "num" field to mark nodes as used/unused */
	for (t = tac; t != NULL; t = t->next) {
		switch (t->op) {
		case TAC_IMMEDIATE:
		case TAC_LOAD:
		case TAC_LABEL:
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
		case TAC_OP_PHI:
		case TAC_OP_NEG:
		case TAC_OP_INV:
			t->num = 0;
			break;
		default:
			t->num = 1;
			break;
		}
		if (t->next == NULL)
			t->num = 1;
	}

	/* mark nodes as used if their result is used */
	for (t = tac; t != NULL; t = t->next) {
		for (i = 0; i < sizeof(t->arg) / sizeof(t->arg[0]); ++i) {
			if (t->arg[i].type == TAC_ARG_RESULT)
				t->arg[i].u.ref->num |= 1;
			if (t->arg[i].type == TAC_ARG_LABEL)
				t->arg[i].u.ref->num |= 1;
			if (t->arg[i].type == TAC_ARG_VAR)
				t->arg[i].u.ref->num |= 1;
		}
	}

	/* remove unused nodes */
	prev = NULL;
	t = tac;

	while (t != NULL) {
		if (t->num == 0) {
			if (prev == NULL) {
				tac = tac->next;
				free(t);
				t = tac;
			} else {
				prev->next = t->next;
				free(t);
				t = prev->next;
			}
			change = true;
		} else {
			prev = t;
			t = t->next;
		}
	}

	*out = tac;
	return change;
}

static void type_promote(mcc_tac_inst_t *tac)
{
	if (tac->arg[0].type == TAC_ARG_IMM_FLOAT ||
	    tac->arg[1].type == TAC_ARG_IMM_FLOAT) {
		if (tac->arg[0].type == TAC_ARG_IMM_INT) {
			tac->arg[0].type = TAC_ARG_IMM_FLOAT;
			tac->arg[0].u.fval = tac->arg[0].u.ival;
		}
		if (tac->arg[1].type == TAC_ARG_IMM_INT) {
			tac->arg[1].type = TAC_ARG_IMM_FLOAT;
			tac->arg[1].u.fval = tac->arg[0].u.ival;
		}
	}
}

static bool calc_constant(mcc_tac_inst_t *tac)
{
	bool change = false;

	for (; tac != NULL; tac = tac->next) {
		if (tac->arg[0].type != TAC_ARG_IMM_INT &&
		    tac->arg[0].type != TAC_ARG_IMM_FLOAT) {
			continue;
		}

		if (tac->arg[1].type != TAC_ARG_IMM_INT &&
		    tac->arg[1].type != TAC_ARG_IMM_FLOAT &&
		    tac->arg[1].type != TAC_ARG_UNUSED) {
			continue;
		}

		switch (tac->op) {
		case TAC_OP_ADD:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].u.fval += tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival += tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_SUB:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].u.fval -= tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival -= tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_MUL:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].u.fval *= tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival *= tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_DIV:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].u.fval /= tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival /= tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_LT:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].type = TAC_ARG_IMM_INT;
				tac->arg[0].u.ival = tac->arg[0].u.fval < tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival = tac->arg[0].u.ival < tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_GT:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].type = TAC_ARG_IMM_INT;
				tac->arg[0].u.ival = tac->arg[0].u.fval > tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival = tac->arg[0].u.ival > tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_LEQ:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].type = TAC_ARG_IMM_INT;
				tac->arg[0].u.ival = tac->arg[0].u.fval <= tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival = tac->arg[0].u.ival <= tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_GEQ:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].type = TAC_ARG_IMM_INT;
				tac->arg[0].u.ival = tac->arg[0].u.fval >= tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival = tac->arg[0].u.ival >= tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_EQU:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].type = TAC_ARG_IMM_INT;
				tac->arg[0].u.ival = tac->arg[0].u.fval == tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival = tac->arg[0].u.ival == tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_NEQU:
			type_promote(tac);
			if (tac->arg[0].type == TAC_ARG_IMM_FLOAT) {
				tac->arg[0].type = TAC_ARG_IMM_INT;
				tac->arg[0].u.ival = tac->arg[0].u.fval != tac->arg[1].u.fval;
			} else {
				tac->arg[0].u.ival = tac->arg[0].u.ival != tac->arg[1].u.ival;
			}
			break;
		case TAC_OP_NEG:
			if (tac->arg[0].type == TAC_ARG_IMM_INT) {
				tac->arg[0].u.ival = -tac->arg[0].u.ival;
			} else {
				tac->arg[0].u.fval = -tac->arg[0].u.fval;
			}
			break;
		case TAC_OP_INV:
			if (tac->arg[0].type == TAC_ARG_IMM_INT) {
				tac->arg[0].u.ival = tac->arg[0].u.ival == 0;
			} else {
				tac->arg[0].u.fval = (tac->arg[0].u.fval == 0.0f);
			}
			break;
		default:
			continue;
		}

		tac->op = TAC_IMMEDIATE;
		tac->arg[1].type = TAC_ARG_UNUSED;
		change = true;

		switch (tac->arg[0].type) {
		case TAC_ARG_IMM_INT:
			if (tac->type.type == TAC_TYPE_FLOAT) {
				tac->arg[0].u.ival=tac->arg[0].u.fval;
				change = true;
			}
			break;
		case TAC_ARG_IMM_FLOAT:
			if (tac->type.type == TAC_TYPE_INT) {
				tac->arg[0].u.fval=tac->arg[0].u.ival;
				change = true;
			}
			break;
		default:
			break;
		}
	}

	return change;
}

mcc_tac_inst_t *mcc_optimize_tac(mcc_tac_inst_t *tac)
{
	bool change;

	do {
		change = value_pull(tac);
		change = calc_constant(tac) || change;
		change = remove_unused(tac, &tac) || change;
	} while (change);

	return tac;
}
