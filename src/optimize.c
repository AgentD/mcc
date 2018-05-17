#include "tac.h"


static bool remove_tagged(mcc_tac_inst_t *tac, mcc_tac_inst_t **out)
{
	mcc_tac_inst_t *prev = NULL, *t = tac;
	bool change = false;

	while (t != NULL) {
		if (t->num) {
			if (prev != NULL) {
				prev->next = t->next;
				free(t);
				t = prev->next;
			} else {
				tac = tac->next;
				free(t);
				t = tac;
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

			if (tac->arg[i].u.ref->op == TAC_COPY) {
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
	mcc_tac_inst_t *t;
	size_t i;

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
		case TAC_COPY:
			t->num = 1;
			break;
		default:
			t->num = 0;
			break;
		}
	}

	for (t = tac; t != NULL; t = t->next) {
		for (i = 0; i < sizeof(t->arg) / sizeof(t->arg[0]); ++i) {
			if (t->arg[i].type == TAC_ARG_RESULT)
				t->arg[i].u.ref->num = 0;
			if (t->arg[i].type == TAC_ARG_LABEL)
				t->arg[i].u.ref->num = 0;
			if (t->arg[i].type == TAC_ARG_VAR)
				t->arg[i].u.ref->num = 0;
		}
	}

	return remove_tagged(tac, out);
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

static bool merge_labels(mcc_tac_inst_t *tac, mcc_tac_inst_t **out)
{
	mcc_tac_inst_t *t, *lbl;
	size_t i;

	for (t = tac; t != NULL; t = t->next) {
		t->num = 0;

		if (t->op == TAC_LABEL) {
			if (t->next != NULL && t->next->op == TAC_LABEL)
				t->num = 1;
		}
	}

	for (t = tac; t != NULL; t = t->next) {
		for (i = 0; i < sizeof(t->arg) / sizeof(t->arg[0]); ++i) {
			if (t->arg[i].type == TAC_ARG_LABEL) {
				lbl = t->arg[i].u.ref;

				while (lbl->num != 0)
					lbl = lbl->next;

				t->arg[i].u.ref = lbl;
			}
		}
	}

	return remove_tagged(tac, out);
}

static bool constant_jumps(mcc_tac_inst_t *tac, mcc_tac_inst_t **out)
{
	bool change = false, nonzero;
	mcc_tac_inst_t *t = tac;

	for (; t != NULL; t = t->next) {
		t->num = 0;

		if (t->op == TAC_JMP) {
			if (t->arg[0].type != TAC_ARG_LABEL)
				continue;
			if (t->arg[0].u.ref != t->next)
				continue;
			t->num = 1;
			continue;
		}

		switch (t->arg[0].type) {
		case TAC_ARG_IMM_INT:
			nonzero = (t->arg[0].u.ival != 0);
			break;
		case TAC_ARG_IMM_FLOAT:
			nonzero = (t->arg[0].u.fval != 0.0f);
			break;
		default:
			continue;
		}

		switch (t->op) {
		case TAC_JZ:
			if (!nonzero) {
				t->op = TAC_JMP;
				t->arg[0] = t->arg[1];
				t->arg[1].type = TAC_ARG_UNUSED;
				change = true;
			} else {
				t->num = 1;
			}
			break;
		case TAC_JNZ:
			if (nonzero) {
				t->op = TAC_JMP;
				t->arg[0] = t->arg[1];
				t->arg[1].type = TAC_ARG_UNUSED;
				change = true;
			} else {
				t->num = 1;
			}
			break;
		default:
			break;
		}
	}

	return remove_tagged(tac, out) || change;
}

static bool remove_unreachable(mcc_tac_inst_t *tac, mcc_tac_inst_t **out)
{
	mcc_tac_inst_t *t = tac;

	while (t != NULL) {
		t->num = 0;

		if (t->op == TAC_RET || t->op == TAC_JMP) {
			t = t->next;

			while (t != NULL && t->op != TAC_LABEL &&
			       t->op != TAC_END_FUNCTION) {
				t->num = 1;
				t = t->next;
			}
		} else {
			t = t->next;
		}
	}

	return remove_tagged(tac, out);
}

static bool is_jump_between(mcc_tac_inst_t *start, mcc_tac_inst_t *end)
{
	while (start != end) {
		if (start == NULL)
			return true;
		if (start->op == TAC_JMP || start->op == TAC_RET)
			return true;
		if (start->op == TAC_JZ || start->op == TAC_JNZ)
			return true;
		start = start->next;
	}
	return false;
}

static bool uni_phi(mcc_tac_inst_t *tac)
{
	mcc_tac_inst_t *t, *l, *r, *t1;
	bool change = false;

	for (t = tac; t != NULL; t = t->next) {
		if (t->op != TAC_OP_PHI)
			continue;

		l = t->arg[0].u.ref;
		r = t->arg[1].u.ref;

		if (is_jump_between(l, t) || is_jump_between(r, t))
			continue;

		t->op = TAC_COPY;
		t->arg[0].u.ref = l;
		t->arg[1].type = TAC_ARG_UNUSED;
		change = true;

		for (t1 = l; t1 != NULL && t1 != t; t1 = t1->next) {
			if (t1 == r) {
				t->arg[0].u.ref = r;
				break;
			}
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
		change = merge_labels(tac, &tac) || change;
		change = constant_jumps(tac, &tac) || change;
		change = remove_unreachable(tac, &tac) || change;
		change = uni_phi(tac) || change;
	} while (change);

	return tac;
}
