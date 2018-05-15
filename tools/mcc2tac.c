#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mcc.h"

static void tac_arg_to_str(mcc_tac_inst_t *tac, unsigned int i,
			   str_tab_t *ident, char *ptr)
{
	switch (tac->arg[i].type) {
	case TAC_ARG_UNUSED:
		*ptr = '\0';
		break;
	case TAC_ARG_RESULT:
		sprintf(ptr, "t%u", tac->arg[i].u.ref->num);
		break;
	case TAC_ARG_LABEL:
		sprintf(ptr, "L%u", tac->arg[i].u.ref->num);
		break;
	case TAC_ARG_VAR:
		if (tac->arg[i].u.ref->op == TAC_ALLOCA)
			sprintf(ptr, "V%u", tac->arg[i].u.ref->num);
		else
			sprintf(ptr, "P%u", tac->arg[i].u.ref->num);
		break;
	case TAC_ARG_NAME:
		strcpy(ptr, mcc_str_tab_resolve(ident, tac->arg[i].u.name));
		break;
	case TAC_ARG_IMM_INT:
		sprintf(ptr, "%d", tac->arg[i].u.ival);
		break;
	case TAC_ARG_IMM_FLOAT:
		sprintf(ptr, "%f", tac->arg[i].u.fval);
		break;
	case TAC_ARG_STR:
		sprintf(ptr, "str%ld", (long)tac->arg[i].u.strval);
		break;
	case TAC_ARG_INDEX:
		sprintf(ptr, "%u", tac->arg[i].u.index);
		break;
	}
}

static void tac_print(str_tab_t *ident, mcc_tac_inst_t *tac)
{
	unsigned int lblcount = 0, stmtcount = 0, varcount = 0;
	char arg0[256], arg1[256];
	mcc_tac_inst_t *t;

	for (t = tac; t != NULL; t = t->next) {
		switch (t->op) {
		case TAC_LABEL:
			t->num = lblcount++;
			break;
		case TAC_ALLOCA:
			t->num = varcount++;
			break;
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
		case TAC_OP_PHI:
		case TAC_OP_NEG:
		case TAC_OP_INV:
		case TAC_CALL:
		case TAC_LOAD:
			t->num = stmtcount++;
			break;
		default:
			break;
		}
	}

	while (tac != NULL) {
		tac_arg_to_str(tac, 0, ident, arg0);
		tac_arg_to_str(tac, 1, ident, arg1);

		switch (tac->op) {
		case TAC_BEGIN_FUNCTION:
			printf("%s:\n\tBEGINFUN\n", arg0);
			break;
		case TAC_END_FUNCTION:
			printf("\tENDFUN\n\n\n");
			break;
		case TAC_FUN_PARAM:
			printf("\tPARAM %s %s\n", arg0, arg1);
			break;
		case TAC_ALLOCA:
			printf("\tV%d := ALLOCA %s\n", tac->num, arg0);
			break;
		case TAC_LABEL:
			printf("L%d:\n", tac->num);
			break;
		case TAC_JZ:
			printf("\tJZ %s, %s\n", arg0, arg1);
			break;
		case TAC_JNZ:
			printf("\tJNZ %s, %s\n", arg0, arg1);
			break;
		case TAC_JMP:
			printf("\tJMP %s\n", arg0);
			break;
		case TAC_RET:
			printf("\tRET %s\n", arg0);
			break;
		case TAC_IMMEDIATE:
			printf("\tt%u := %s\n", tac->num, arg0);
			break;
		case TAC_OP_ADD:
			printf("\tt%u := %s + %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_SUB:
			printf("\tt%u := %s - %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_MUL:
			printf("\tt%u := %s * %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_DIV:
			printf("\tt%u := %s / %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_LT:
			printf("\tt%u := %s < %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_GT:
			printf("\tt%u := %s > %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_LEQ:
			printf("\tt%u := %s <= %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_GEQ:
			printf("\tt%u := %s >= %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_EQU:
			printf("\tt%u := %s == %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_NEQU:
			printf("\tt%u := %s != %s\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_PHI:
			printf("\tt%u := phi(%s, %s)\n", tac->num, arg0, arg1);
			break;
		case TAC_OP_NEG:
			printf("\tt%u := -%s\n", tac->num, arg0);
			break;
		case TAC_OP_INV:
			printf("\tt%u := !%s\n", tac->num, arg0);
			break;
		case TAC_CALL:
			printf("\tt%u := CALL %s\n", tac->num, arg0);
			break;
		case TAC_PUSH_ARG:
			printf("\tPUSHARG %s\n", arg0);
			break;
		case TAC_LOAD:
			printf("\tt%u := LOAD %s\n", tac->num, arg0);
			break;
		case TAC_STORE:
			printf("\tSTORE %s <- %s\n", arg0, arg1);
			break;
		}

		tac = tac->next;
	}
}

int main(int argc, char **argv)
{
	parser_result_t result;
	semantic_result_t sem;
	mcc_tac_inst_t *tac;
	function_def_t *fun;
	(void)argv;

	if (argc != 1) {
		fputs(	"Usage: mcc2tac\n\n"
			"mcc source is read from stdin, tac source is\n"
			"written to stdout.\n\n"
			"USE PIPES!\n", stderr);
		return EXIT_FAILURE;
	}

	result = mcc_parse_file(stdin);

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

		mcc_cleanup_program(&result.program);
		return EXIT_FAILURE;
	}

	sem = mcc_semantic_check(&result.program);

	if (sem.status == SEMANTIC_STATUS_OK) {
		sem = mcc_type_check(&result.program);
	} else {
		fputs("semantic error\n", stderr);
		mcc_cleanup_program(&result.program);
		return EXIT_FAILURE;
	}

	for (fun = result.program.functions; fun != NULL; fun = fun->next) {
		tac = mcc_function_to_tac(fun);

		tac_print(&result.program.identifiers, tac);

		mcc_tac_free(tac);
	}

	mcc_cleanup_program(&result.program);
	return EXIT_SUCCESS;
}
