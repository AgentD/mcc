#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "mcc.h"

static program_t prog;

static void gen_name(const void *ptr, char *buffer)
{
	char *cptr;

	sprintf(buffer, "%p", ptr);

	if (buffer[0] == '0' && (buffer[1] == 'x' || buffer[1] == 'X'))
		memmove(buffer, buffer + 2, strlen(buffer + 2) + 1);

	for (cptr = buffer; *cptr != '\0'; ++cptr) {
		if (isdigit(*cptr))
			*cptr = 'F' + (*cptr - '0');
		if (islower(*cptr))
			*cptr = toupper(*cptr);
	}
}

static void print_arrow(const void *src, const void *dst)
{
	char name1[20], name2[20];

	gen_name(src, name1);
	gen_name(dst, name2);
	printf("\t%s -> %s;\n", name1, name2);
}

static void print_label(const void *ptr, const char *label)
{
	char name[20];

	gen_name(ptr, name);
	printf("\t%s [label=\"%s\"];\n", name, label);
}

static void sex_to_dot(expression_t *sex);

static void args_to_dot(arg_t *args)
{
	print_label(args, "argument");
	sex_to_dot(args->expr);
	print_arrow(args, args->expr);

	if (args->next != NULL) {
		args_to_dot(args->next);
		print_arrow(args, args->next);
	}
}

static void sex_to_dot(expression_t *sex)
{
	const char *str;
	char buffer[32];
	void *ptr;

	switch (sex->type) {
	case SEX_LITERAL:
		switch (sex->u.lit.type) {
		case TYPE_BOOL:
			sprintf(buffer, "bool %s",
				sex->u.lit.value.b ? "true" :"false");
			break;
		case TYPE_INT:
			sprintf(buffer, "int %d", sex->u.lit.value.i);
			break;
		case TYPE_FLOAT:
			sprintf(buffer, "float %f", sex->u.lit.value.f);
			break;
		case TYPE_STRING:
			sprintf(buffer, "string %lu",
				(unsigned long)sex->u.lit.value.str);
			break;
		default:
			assert(0);
		}

		print_label(sex, buffer);
		break;
	case SEX_IDENTIFIER:
		str = str_tab_resolve(&prog.identifiers, sex->u.identifier);
		print_label(sex, str);
		break;
	case SEX_ARRAY_INDEX:
		ptr = &sex->u.array_idx.identifier;
		str = str_tab_resolve(&prog.identifiers,
				      sex->u.array_idx.identifier);

		print_label(sex, "array access");
		print_label(ptr, str);
		sex_to_dot(sex->u.array_idx.index);

		print_arrow(sex, sex->u.array_idx.index);
		print_arrow(sex, ptr);
		break;
	case SEX_CALL:
		ptr = &sex->u.array_idx.identifier;
		str = str_tab_resolve(&prog.identifiers,
				      sex->u.call.identifier);

		print_label(sex, "call");
		print_label(ptr, str);
		print_arrow(sex, ptr);

		if (sex->u.call.args != NULL) {
			args_to_dot(sex->u.call.args);
			print_arrow(sex, sex->u.call.args);
		}
		break;
	case SEX_UNARY:
		switch (sex->u.unary.op) {
		case UNARY_NEG: print_label(sex, "unary -"); break;
		case UNARY_INV: print_label(sex, "unary !"); break;
		default:
			assert(0);
		}
		sex_to_dot(sex->u.unary.exp);
		print_arrow(sex, sex->u.unary.exp);
		break;
	case BINOP_ADD: str = "+"; goto binary;
	case BINOP_SUB: str = "-"; goto binary;
	case BINOP_MUL: str = "*"; goto binary;
	case BINOP_DIV: str = "/"; goto binary;
	case BINOP_LESS: str = "<"; goto binary;
	case BINOP_GREATER: str = ">"; goto binary;
	case BINOP_LEQ: str = "<="; goto binary;
	case BINOP_GEQ: str = ">="; goto binary;
	case BINOP_ANL: str = "&&"; goto binary;
	case BINOP_ORL: str = "||"; goto binary;
	case BINOP_EQU: str = "=="; goto binary;
	case BINOP_NEQU: str = "!="; goto binary;
	default:
		print_label(sex, "unknown node type");
		break;
	}
	return;
binary:
	print_label(sex, str);

	if (sex->u.binary.left != NULL) {
		sex_to_dot(sex->u.binary.left);
		print_arrow(sex, sex->u.binary.left);
	}

	if (sex->u.binary.right != NULL) {
		sex_to_dot(sex->u.binary.right);
		print_arrow(sex, sex->u.binary.right);
	}
}

int main(int argc, char **argv)
{
	parser_result_t result;
	(void)argv;

	if (argc != 1) {
		fputs(	"Usage: mcc2dot\n\n"
			"mcc source is read from stdin, dot source is\n"
			"written to stdout.\n\n"
			"USE PIPES!\n", stderr);
		return EXIT_FAILURE;
	}

	mcc_init_program(&prog);
	mcc_set_program(&prog);

	result = parse_file(stdin);

	if (result.status != PARSER_STATUS_OK) {
		fputs("Error parsing input\n", stderr);
		return EXIT_FAILURE;
	}

	fputs("digraph mcc2dot {\n", stdout);
	sex_to_dot(result.expression);
	fputs("}\n", stdout);

	sex_free(result.expression);
	return EXIT_SUCCESS;
}
