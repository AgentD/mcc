/*
  as --32 mccrt.S -o mccrt.o
  ld -m elf_i386 *.o -lc --dynamic-linker=/lib/ld-linux.so.2
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mcc.h"

static void float_const_to_data_section(mcc_tac_inst_t *tac,
					unsigned int funidx, FILE *out)
{
	unsigned int fcidx = 0;
	bool have_flt = false;
	mcc_tac_inst_t *t;
	int i;

	for (t = tac; t != NULL; t = t->next) {
		for (i = 0; i < 2; ++i) {
			if (t->arg[i].type == TAC_ARG_IMM_FLOAT) {
				have_flt = true;
				break;
			}
		}
	}

	if (!have_flt)
		return;

	fprintf(out, "\t.section\t.rodata\n\t.align 4\n");

	for (t = tac; t != NULL; t = t->next) {
		for (i = 0; i < 2; ++i) {
			if (t->arg[i].type == TAC_ARG_IMM_FLOAT) {
				fprintf(out, "F%uF%u:\n", funidx, fcidx++);
				fprintf(out, "\t.float %f\n", t->arg[i].u.fval);
			}
		}
	}
}

static void store_result(FILE *out, mcc_tac_inst_t *t)
{
	if (t->type.type == TAC_TYPE_FLOAT && t->type.ptr_level == 0) {
		fprintf(out, "\tfstps\t-%u(%%ebp)\n\tfwait\n", t->num);
	} else {
		if (t->next != NULL) {
			if (t->next->arg[0].type == TAC_ARG_RESULT &&
			    t->next->arg[0].u.ref == t)
				return;
			if (t->next->arg[1].type == TAC_ARG_RESULT &&
			    t->next->arg[1].u.ref == t) {
				fprintf(out, "\tmovl\t%%eax, %%ebx\n");
				return;
			}
		}

		fprintf(out, "\tmovl\t%%eax, -%u(%%ebp)\n", t->num);
	}
}

static void load_arg_to_reg(mcc_tac_inst_t *t, int idx, unsigned int funidx,
			    unsigned int *flcidx, FILE *out)
{
	const char *gpreg[] = { "%eax", "%ebx" };

	switch (t->arg[idx].type) {
	case TAC_ARG_IMM_INT:
		if (t->type.ptr_level > 0) {
			fprintf(out, "\tmovl\t$%d, %s\n",
				t->arg[idx].u.ival * 4, gpreg[idx]);
		} else {
			fprintf(out, "\tmovl\t$%d, %s\n",
				t->arg[idx].u.ival, gpreg[idx]);
		}
		break;
	case TAC_ARG_STR:
		fprintf(out, "\tmovl\t$STR%lu, %s\n",
			(unsigned long)t->arg[idx].u.strval, gpreg[idx]);
		break;
	case TAC_ARG_IMM_FLOAT:
		fprintf(out, "\tflds\tF%uF%u\n", funidx, (*flcidx)++);
		break;
	case TAC_ARG_RESULT:
		if (t->arg[idx].u.ref->type.type == TAC_TYPE_FLOAT &&
		    t->arg[idx].u.ref->type.ptr_level == 0) {
			fprintf(out, "\tflds\t-%u(%%ebp)\n",
				t->arg[idx].u.ref->num);
		} else if (t->arg[idx].u.ref->next != t) {
			fprintf(out, "\tmovl\t-%u(%%ebp), %s\n",
				t->arg[idx].u.ref->num, gpreg[idx]);
		}

		if (t->type.ptr_level > t->arg[idx].u.ref->type.ptr_level)
			fprintf(out, "\tsall\t$2, %s\n", gpreg[idx]);
		break;
	case TAC_ARG_VAR:
		if (t->arg[idx].u.ref->op == TAC_ALLOCA &&
		    t->arg[idx].u.ref->arg[0].u.index > 1) {
			fprintf(out, "\tleal\t-%u(%%ebp), %s\n",
				t->arg[idx].u.ref->num, gpreg[idx]);
			break;
		}

		if (t->arg[idx].u.ref->type.type == TAC_TYPE_FLOAT &&
		    t->type.ptr_level == 0) {
			if (t->arg[idx].u.ref->op == TAC_ALLOCA) {
				fprintf(out, "\tflds\t-%u(%%ebp)\n",
					t->arg[idx].u.ref->num);
			} else {
				fprintf(out, "\tflds\t%u(%%ebp)\n",
					t->arg[idx].u.ref->num);
			}
		} else {
			if (t->arg[idx].u.ref->op == TAC_ALLOCA) {
				fprintf(out, "\tmovl\t-%u(%%ebp), %s\n",
					t->arg[idx].u.ref->num, gpreg[idx]);
			} else {
				fprintf(out, "\tmovl\t%u(%%ebp), %s\n",
					t->arg[idx].u.ref->num, gpreg[idx]);
			}

			if (t->type.ptr_level > t->arg[idx].u.ref->type.ptr_level)
				fprintf(out, "\tsall\t$2, %s\n", gpreg[idx]);
		}
		break;
	default:
		break;
	}
}

static void compare_values(FILE *out, mcc_tac_inst_t *t, unsigned int funidx,
			   unsigned int *flcidx)
{
	bool isfloat = false;

	switch (t->arg[0].type) {
	case TAC_ARG_RESULT:
	case TAC_ARG_VAR:
		if (t->arg[0].u.ref->type.type == TAC_TYPE_FLOAT)
			isfloat = true;
		break;
	case TAC_ARG_IMM_FLOAT:
		isfloat = true;
		break;
	default:
		break;
	}

	switch (t->arg[1].type) {
	case TAC_ARG_RESULT:
	case TAC_ARG_VAR:
		if (t->arg[1].u.ref->type.type == TAC_TYPE_FLOAT)
			isfloat = true;
		break;
	case TAC_ARG_IMM_FLOAT:
		isfloat = true;
		break;
	default:
		break;
	}

	if (isfloat) {
		load_arg_to_reg(t, 1, funidx, flcidx, out);
		load_arg_to_reg(t, 0, funidx, flcidx, out);
		fprintf(out, "\tfcomip\n\tfstp\t%%st(0)\n");
	} else {
		load_arg_to_reg(t, 0, funidx, flcidx, out);
		load_arg_to_reg(t, 1, funidx, flcidx, out);
		fprintf(out, "\tcmpl\t%%ebx, %%eax\n");
	}
}

static void pusharg(FILE *out, mcc_tac_inst_t *t, unsigned int funidx,
		    unsigned int *flcidx)
{
	switch (t->arg[0].type) {
	case TAC_ARG_IMM_INT:
		fprintf(out, "\tpushl\t$%d\n", t->arg[0].u.ival);
		break;
	case TAC_ARG_STR:
		fprintf(out, "\tpushl\t$STR%lu\n",
			(unsigned long)t->arg[0].u.strval);
		break;
	case TAC_ARG_IMM_FLOAT:
		fprintf(out, "\tpushl\t(F%uF%u)\n", funidx, (*flcidx)++);
		break;
	case TAC_ARG_RESULT:
		if (t->arg[0].u.ref->next == t &&
		    (t->arg[0].u.ref->type.type != TAC_TYPE_FLOAT ||
		     t->arg[0].u.ref->type.ptr_level != 0)) {
			fprintf(out, "\tpushl\t%%eax\n");
		} else {
			fprintf(out, "\tpushl\t-%u(%%ebp)\n",
				t->arg[0].u.ref->num);
		}
		break;
	case TAC_ARG_VAR:
		if (t->arg[0].u.ref->op == TAC_ALLOCA &&
		    t->arg[0].u.ref->arg[0].u.index > 1) {
		}
		if (t->arg[0].u.ref->op == TAC_ALLOCA) {
			fprintf(out, "\tpushl\t-%u(%%ebp)\n",
				t->arg[0].u.ref->num);
		} else {
			fprintf(out, "\tpushl\t%u(%%ebp)\n",
				t->arg[0].u.ref->num);
		}
		break;
	default:
		break;
	}
}

static void store(mcc_tac_inst_t *t, unsigned int funidx,
		  unsigned int *flcidx, FILE *out)
{
	if (t->arg[0].type == TAC_ARG_RESULT) {
		load_arg_to_reg(t, 0, funidx, flcidx, out);
		load_arg_to_reg(t, 1, funidx, flcidx, out);
		fprintf(out, "\tmovl\t%%ebx, (%%eax)\n");
	} else {
		switch (t->arg[1].type) {
		case TAC_ARG_RESULT:
			if (t->arg[1].u.ref->next == t &&
			    (t->arg[1].u.ref->type.type != TAC_TYPE_FLOAT ||
			     t->arg[1].u.ref->type.ptr_level != 0)) {
				break;
			}
			fprintf(out, "\tmovl\t-%u(%%ebp), %%eax\n",
				t->arg[1].u.ref->num);
			break;
		case TAC_ARG_IMM_FLOAT:
			fprintf(out, "\tmovl\t(F%uF%u), %%eax\n", funidx, (*flcidx)++);
			break;
		case TAC_ARG_IMM_INT:
			fprintf(out, "\tmovl\t$%d, %%eax\n", t->arg[1].u.ival);
			break;
		case TAC_ARG_STR:
			fprintf(out, "\tmovl\t$STR%lu, %%eax\n",
				(unsigned long)t->arg[1].u.strval);
			break;
		case TAC_ARG_VAR:
			if (t->arg[1].u.ref->op == TAC_ALLOCA) {
				fprintf(out, "\tmovl\t-%u(%%ebp), %%eax\n",
					t->arg[1].u.ref->num);
			} else {
				fprintf(out, "\tmovl\t%u(%%ebp), %%eax\n",
					t->arg[1].u.ref->num);
			}
			break;
		default:
			assert(0);
		}

		if (t->arg[0].u.ref->op == TAC_ALLOCA) {
			fprintf(out, "\tmovl\t%%eax, -%u(%%ebp)\n",
				t->arg[0].u.ref->num);
		} else {
			fprintf(out, "\tmovl\t%%eax, %u(%%ebp)\n",
				t->arg[0].u.ref->num);
		}
	}
}

static void tac_print(str_tab_t *ident, mcc_tac_inst_t *tac, FILE *out,
		      function_def_t *fun, unsigned int funidx)
{
	unsigned int allocated = 0, argsize = 8, max_retnum = 0;
	unsigned int flcidx = 0;
	mcc_tac_inst_t *t;
	const char *name;

	name = mcc_str_tab_resolve(ident, fun->identifier);

	fprintf(out, "\t.text\n\t.globl\t%s\n", name);
	fprintf(out, "\t.type\t%s, @function\n", name);
	fprintf(out, "%s:\n", name);
	fprintf(out, "\tpushl\t%%ebp\n");
	fprintf(out, "\tmovl\t%%esp, %%ebp\n");

	for (t = tac; t != NULL; t = t->next) {
		switch (t->op) {
		case TAC_ALLOCA:
			allocated += 4 * t->arg[0].u.index;
			t->num = allocated;
			break;
		case TAC_FUN_PARAM:
			t->num = argsize;
			argsize += 4;
			break;
		default:
			break;
		}
	}

	for (t = tac; t != NULL; t = t->next) {
		switch (t->op) {
		case TAC_CALL:
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
		case TAC_LOAD:
		case TAC_COPY:
			if (t->num > max_retnum)
				max_retnum = t->num;
			t->num = allocated + 4 + t->num * 4;
			break;
		default:
			break;
		}
	}

	allocated += (max_retnum + 1) * 4;

	if (allocated)
		fprintf(out, "\tsubl\t$%u, %%esp\n", allocated);

	argsize = 0;

	for (t = tac; t != NULL; t = t->next) {
		switch (t->op) {
		case TAC_BEGIN_FUNCTION:
		case TAC_END_FUNCTION:
		case TAC_FUN_PARAM:
		case TAC_ALLOCA:
		case TAC_OP_PHI:
			break;
		case TAC_LABEL:
			fprintf(out, "F%uL%d:\n", funidx, t->num);
			break;
		case TAC_JMP:
			assert(t->arg[0].type == TAC_ARG_LABEL);

			fprintf(out, "\tjmp\tF%uL%d\n", funidx,
			        t->arg[0].u.ref->num);
			break;
		case TAC_RET:
			load_arg_to_reg(t, 0, funidx, &flcidx, out);
			if (t->next->op != TAC_END_FUNCTION)
				fprintf(out, "\tjmp\tF%uOUT\n", funidx);
			break;
		case TAC_JZ:
			load_arg_to_reg(t, 0, funidx, &flcidx, out);
			fprintf(out, "\tandl\t%%eax, %%eax\n");
			fprintf(out, "\tjz\tF%uL%d\n", funidx,
			        t->arg[1].u.ref->num);
			break;
		case TAC_JNZ:
			load_arg_to_reg(t, 0, funidx, &flcidx, out);
			fprintf(out, "\tandl\t%%eax, %%eax\n");
			fprintf(out, "\tjnz\tF%uL%d\n", funidx,
			        t->arg[1].u.ref->num);
			break;
		case TAC_PUSH_ARG:
			pusharg(out, t, funidx, &flcidx);
			argsize += 4;
			break;
		case TAC_CALL:
			fprintf(out, "\tcall\t%s\n",
				mcc_str_tab_resolve(ident, t->arg[0].u.name));
			if (argsize > 0)
				fprintf(out, "\taddl\t$%d, %%esp\n", argsize);
			argsize = 0;

			if (t->type.type != TAC_TYPE_NONE)
				store_result(out, t);
			break;
		case TAC_IMMEDIATE:
			load_arg_to_reg(t, 0, funidx, &flcidx, out);
			store_result(out, t);
			break;
		case TAC_OP_ADD:
			load_arg_to_reg(t, 0, funidx, &flcidx, out);
			load_arg_to_reg(t, 1, funidx, &flcidx, out);
			if (t->type.type == TAC_TYPE_FLOAT &&
			    t->type.ptr_level == 0) {
				fprintf(out, "\tfaddp\n");
			} else {
				fprintf(out, "\taddl\t%%ebx, %%eax\n");
			}
			store_result(out, t);
			break;
		case TAC_OP_SUB:
			if (t->type.type == TAC_TYPE_FLOAT) {
				load_arg_to_reg(t, 1, funidx, &flcidx, out);
				load_arg_to_reg(t, 0, funidx, &flcidx, out);
				fprintf(out, "\tfsubp\n");
			} else {
				load_arg_to_reg(t, 0, funidx, &flcidx, out);
				load_arg_to_reg(t, 1, funidx, &flcidx, out);
				fprintf(out, "\tsubl\t%%ebx, %%eax\n");
			}
			store_result(out, t);
			break;
		case TAC_OP_MUL:
			load_arg_to_reg(t, 0, funidx, &flcidx, out);
			load_arg_to_reg(t, 1, funidx, &flcidx, out);
			if (t->type.type == TAC_TYPE_FLOAT) {
				fprintf(out, "\tfmulp\n");
			} else {
				fprintf(out, "\timul\t%%ebx, %%eax\n");
			}
			store_result(out, t);
			break;
		case TAC_OP_DIV:
			if (t->type.type == TAC_TYPE_FLOAT) {
				load_arg_to_reg(t, 1, funidx, &flcidx, out);
				load_arg_to_reg(t, 0, funidx, &flcidx, out);
				fprintf(out, "\tfdivp\n");
			} else {
				load_arg_to_reg(t, 0, funidx, &flcidx, out);
				load_arg_to_reg(t, 1, funidx, &flcidx, out);
				fprintf(out, "\tidiv\t%%ebx, %%eax\n");
			}
			store_result(out, t);
			break;
		case TAC_OP_INV:
			load_arg_to_reg(t, 0, funidx, &flcidx, out);
			fprintf(out, "\tnotl\t%%eax\n");
			store_result(out, t);
			break;
		case TAC_STORE:
			store(t, funidx, &flcidx, out);
			break;
		case TAC_LOAD:
			assert(t->arg[0].type == TAC_ARG_VAR);

			if (t->arg[0].u.ref->op == TAC_ALLOCA) {
				fprintf(out, "\tleal\t-%u(%%ebp), %%eax\n",
					t->arg[0].u.ref->num);
			} else {
				fprintf(out, "\tleal\t%u(%%ebp), %%eax\n",
					t->arg[0].u.ref->num);
			}

			switch (t->arg[1].type) {
			case TAC_ARG_UNUSED:
				break;
			case TAC_ARG_IMM_INT:
				fprintf(out, "\taddl\t$%d, %%eax\n",
					t->arg[1].u.ival * 4);
				break;
			case TAC_ARG_RESULT:
				if (t->arg[1].u.ref->next != t) {
					fprintf(out,
						"\tmovl\t-%u(%%ebp), %%ebx\n",
						t->arg[1].u.ref->num);
				}
				fprintf(out, "\tsall\t$2, %%ebx\n");
				fprintf(out, "\taddl\t%%ebx, %%eax\n");
				break;
			case TAC_ARG_VAR:
				if (t->arg[1].u.ref->op == TAC_ALLOCA) {
					fprintf(out, "\tmovl\t-%u(%%ebp), %%ebx\n",
						t->arg[1].u.ref->num);
				} else {
					fprintf(out, "\tmovl\t%u(%%ebp), %%ebx\n",
						t->arg[1].u.ref->num);
				}
				fprintf(out, "\tshll\t$2, %%ebx\n");
				fprintf(out, "\taddl\t%%ebx, %%eax\n");
				break;
			default:
				assert(0);
			}

			fprintf(out, "\tmovl\t(%%eax), %%eax\n");
			store_result(out, t);
			break;
		case TAC_OP_LT:
			compare_values(out, t, funidx, &flcidx);
			fprintf(out, "\tsetb\t%%al\n\tmovzx\t%%al, %%eax\n");
			store_result(out, t);
			break;
		case TAC_OP_GT:
			compare_values(out, t, funidx, &flcidx);
			fprintf(out, "\tseta\t%%al\n\tmovzx\t%%al, %%eax\n");
			store_result(out, t);
			break;
		case TAC_OP_LEQ:
			compare_values(out, t, funidx, &flcidx);
			fprintf(out, "\tsetbe\t%%al\n\tmovzx\t%%al, %%eax\n");
			store_result(out, t);
			break;
		case TAC_OP_GEQ:
			compare_values(out, t, funidx, &flcidx);
			fprintf(out, "\tsetae\t%%al\n\tmovzx\t%%al, %%eax\n");
			store_result(out, t);
			break;
		case TAC_OP_EQU:
			compare_values(out, t, funidx, &flcidx);
			fprintf(out, "\tsete\t%%al\n\tmovzx\t%%al, %%eax\n");
			store_result(out, t);
			break;
		case TAC_OP_NEQU:
			compare_values(out, t, funidx, &flcidx);
			fprintf(out, "\tsetne\t%%al\n\tmovzx\t%%al, %%eax\n");
			store_result(out, t);
			break;
		case TAC_OP_NEG:
			load_arg_to_reg(t, 0, funidx, &flcidx, out);
			if (t->type.type == TAC_TYPE_FLOAT) {
				fprintf(out, "\tfchs\n");
			} else {
				fprintf(out, "\tneg\t%%eax\n");
			}
			store_result(out, t);
			break;
		case TAC_OP_FTOI:
			/* TODO */
		case TAC_OP_ITOF:
			/* TODO */
		default:
			assert(0);
		}
	}

	name = mcc_str_tab_resolve(ident, fun->identifier);

	fprintf(out, "F%uOUT:\n\tleave\n\tret\n", funidx);
	fprintf(out, "\t.size\t%s, .-%s\n", name, name);

	float_const_to_data_section(tac, funidx, out);
	fprintf(out, "\n\n");
}


static void print_semantic_error(semantic_result_t sem, const char* filename,
				 program_t *prog)
{
	const char *name;
	off_t id;

	switch (sem.status) {
	case SEMANTIC_STATUS_OK:
		break;
	case SEMANTIC_FUNCTION_REDEF:
		name = mcc_str_tab_resolve(&prog->identifiers,
					   sem.u.redef.first->identifier);
		fprintf(stderr, "%s: %u: function '%s' redefined. "
			"Previous definition here: %u\n",
			filename, sem.u.redef.second->line_no,
			name, sem.u.redef.first->line_no);
		break;
	case SEMANTIC_MAIN_MISSING:
		fprintf(stderr, "%s: main function missing\n", filename);
		break;
	case SEMANTIC_MAIN_TYPE:
		fprintf(stderr, "%s: %u: type must be void main()\n",
			filename, sem.u.main->line_no);
		break;
	case SEMANTIC_BUILTIN_REDEF:
		fprintf(stderr, "%s: %u: redefinition of built-in\n",
			filename, sem.u.redef.second->line_no);
		break;
	case SEMANTIC_VAR_REDEF:
		name = mcc_str_tab_resolve(&prog->identifiers,
					   sem.u.vredef.first->identifier);
		fprintf(stderr, "%s: %u: variable '%s' redefined. "
			"Previous definition here: %u\n", filename,
			sem.u.vredef.second->line_no, name,
			sem.u.vredef.first->line_no);
		break;
	case SEMANTIC_RET_NO_VAL:
		fprintf(stderr, "%s: %u: expected value after return.\n",
			filename, sem.u.stmt->line_no);
		break;
	case SEMANTIC_RET_VOID:
		fprintf(stderr, "%s: %u: no value expected after return\n",
			filename, sem.u.stmt->line_no);
		break;
	case SEMANTIC_NO_RET:
		fprintf(stderr, "%s: %u: no return at end of non-void "
			"function.\n", filename, sem.u.stmt->line_no);
		break;
	case SEMANTIC_CALL_UNRESOLVED:
		name = mcc_str_tab_resolve(&prog->identifiers,
					sem.u.expr->u.call.identifier);
		fprintf(stderr, "%s: %u: call to unknown function '%s'\n",
			filename, sem.u.expr->line_no, name);
		break;
	case SEMANTIC_OOM:
		fprintf(stderr, "%s: out of memory\n", filename);
		break;
	case SEMANTIC_UNKNOWN_VAR:
		id = sem.u.expr->u.var.identifier;
		name = mcc_str_tab_resolve(&prog->identifiers, id);
		fprintf(stderr, "%s: %u: usage of unknown variable '%s'\n",
			filename, sem.u.expr->line_no, name);
		break;
	case SEMANTIC_UNKNOWN_LHS:
		name = mcc_str_tab_resolve(&prog->identifiers,
					sem.u.stmt->st.assignment.identifier);
		fprintf(stderr,"%s: %u: assignment to unknown variable '%s'\n",
			filename, sem.u.stmt->line_no, name);
		break;
	case SEMANTIC_ASS_MISSING_ARRAY_INDEX:
		fprintf(stderr, "%s: %u: array index in assignment missing\n",
			filename, sem.u.stmt->line_no);
		break;
	case SEMANTIC_ASS_NOT_ARRAY:
		fprintf(stderr, "%s: %u: target in assignment is not array\n",
			filename, sem.u.stmt->line_no);
		break;
	case SEMANTIC_MISSING_ARRAY_INDEX:
		name = mcc_str_tab_resolve(&prog->identifiers,
				sem.u.expr->u.var_resolved.var->identifier);
		fprintf(stderr, "%s: %u: index missing for '%s'\n",
			filename, sem.u.expr->line_no, name);
		break;
	case SEMANTIC_NOT_ARRAY:
		name = mcc_str_tab_resolve(&prog->identifiers,
				sem.u.expr->u.var_resolved.var->identifier);
		fprintf(stderr, "%s: %u: variable '%s' is not array\n",
			filename, sem.u.expr->line_no, name);
		break;
	case SEMANTIC_ARRAY_INDEX_NOT_INT:
		name = mcc_str_tab_resolve(&prog->identifiers,
				sem.u.expr->u.var_resolved.var->identifier);
		fprintf(stderr, "%s: %u: index for '%s' is not int\n",
			filename, sem.u.expr->line_no, name);
		break;
	case SEMANTIC_ASS_MISMATCH:
		fprintf(stderr, "%s: %u: type mismatch in assignment\n",
			filename, sem.u.stmt->line_no);
		break;
	case SEMANTIC_OP_NOT_BOOLEAN:
		fprintf(stderr, "%s: %u: boolean type expected for operator\n",
			filename, sem.u.expr->line_no);
		break;
	case SEMANTIC_OP_NOT_NUMERIC:
		fprintf(stderr, "%s: %u: numeric type expected for operator\n",
			filename, sem.u.expr->line_no);
		break;
	case SEMANTIC_OP_ARG_NUM:
		fprintf(stderr, "%s: %u: wrong number of arguments in call\n",
			filename, sem.u.expr->line_no);
		break;
	case SEMANTIC_OP_ARG_TYPE:
		fprintf(stderr, "%s: %u: wrong argument type in call\n",
			filename, sem.u.expr->line_no);
		break;
	}
}


static void dump_strings(FILE *out, str_tab_t *tab)
{
	size_t i = 0;

	if (tab->pool == NULL || tab->pool_size == 0)
		return;

	fprintf(out, "\t.section .rodata\n");

	while (i < tab->pool_size) {
		fprintf(out, "STR%lu:\n", (unsigned long)i);
		fprintf(out, "\t.string %s\n", tab->pool + i);
		i += strlen(tab->pool + i) + 1;
	}
}


int main(int argc, char **argv)
{
	unsigned int funidx = 0;
	parser_result_t result;
	semantic_result_t sem;
	mcc_tac_inst_t *tac;
	function_def_t *fun;
	FILE *in, *out;

	if (argc != 3) {
		fputs("Usage: mcc <inputfile> <outputfile>\n\n", stderr);
		return EXIT_FAILURE;
	}

	in = fopen(argv[1], "r");
	if (!in) {
		perror(argv[1]);
		return EXIT_FAILURE;
	}

	out = fopen(argv[2], "w");
	if (!out) {
		perror(argv[2]);
		goto fail_out;
	}

	result = mcc_parse_file(in);

	if (result.status != PARSER_STATUS_OK) {
		switch (result.status) {
		case PARSER_STATUS_OUT_OF_MEMORY:
			fputs("out of memory\n", stderr);
			break;
		case PARSER_STATUS_PARSE_ERROR:
			fprintf(stderr, "%u: %s\n",
				result.program.error_line,
				result.program.error_msg);
			break;
		default:
			fputs("Unknown error parsing input\n", stderr);
			break;
		}
		goto fail;
	}

	sem = mcc_semantic_check(&result.program);

	if (sem.status == SEMANTIC_STATUS_OK)
		sem = mcc_type_check(&result.program);

	if (sem.status != SEMANTIC_STATUS_OK) {
		print_semantic_error(sem, argv[1], &result.program);
		mcc_cleanup_program(&result.program);
		goto fail;
	}

	for (fun = result.program.functions; fun != NULL; fun = fun->next) {
		tac = mcc_function_to_tac(fun);
		tac = mcc_optimize_tac(tac);
		tac = mcc_tac_allocate_ids(tac);

		tac_print(&result.program.identifiers, tac, out,
			  fun, funidx++);

		mcc_tac_free(tac);
	}

	dump_strings(out, &result.program.strings);

	mcc_cleanup_program(&result.program);
	return EXIT_SUCCESS;
fail:
	mcc_cleanup_program(&result.program);
	fclose(out);
fail_out:
	fclose(in);
	return EXIT_FAILURE;
}
