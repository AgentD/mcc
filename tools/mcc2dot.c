#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "mcc.h"

static void gen_name(const void *ptr, char *buffer)
{
	sprintf(buffer, "A%016lX", (unsigned long)ptr);
}

static const char *type_to_str(E_TYPE tp)
{
	switch (tp) {
	case TYPE_VOID:   return "void";
	case TYPE_BOOL:   return "bool";
	case TYPE_INT:    return "int";
	case TYPE_FLOAT:  return "float";
	case TYPE_STRING: return "string";
	}

	return "(unknown)";
}

static void print_arrow(const void *src, const void *dst, const char *label)
{
	char name1[20], name2[20];

	gen_name(src, name1);
	gen_name(dst, name2);

	if (label) {
		printf("\t%s -> %s [label=\"%s\"];\n", name1, name2, label);
	} else {
		printf("\t%s -> %s;\n", name1, name2);
	}
}

static void print_label(const void *ptr, const char *label)
{
	char name[20];

	gen_name(ptr, name);
	printf("\t%s [label=\"%s\"];\n", name, label);
}

static void print_box(const void *ptr, const char *label)
{
	char name[20];

	gen_name(ptr, name);
	printf("\t%s [shape=box, label=\"%s\"];\n", name, label);
}


static void sex_to_dot(program_t *prog, expression_t *sex);

static void args_to_dot(program_t *prog, arg_t *args)
{
	print_label(args, "argument");
	sex_to_dot(prog, args->expr);
	print_arrow(args, args->expr, NULL);

	if (args->next != NULL) {
		args_to_dot(prog, args->next);
		print_arrow(args, args->next, NULL);
	}
}

static void sex_to_dot(program_t *prog, expression_t *sex)
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
		str = mcc_str_tab_resolve(&prog->identifiers,
					  sex->u.identifier);
		print_label(sex, str);
		break;
	case SEX_ARRAY_INDEX:
		ptr = &sex->u.array_idx.identifier;
		str = mcc_str_tab_resolve(&prog->identifiers,
					  sex->u.array_idx.identifier);

		print_label(sex, "array access");
		print_label(ptr, str);
		sex_to_dot(prog, sex->u.array_idx.index);

		print_arrow(sex, sex->u.array_idx.index, NULL);
		print_arrow(sex, ptr, NULL);
		break;
	case SEX_CALL:
		ptr = &sex->u.array_idx.identifier;
		str = mcc_str_tab_resolve(&prog->identifiers,
					  sex->u.call.identifier);

		print_label(sex, "call");
		print_label(ptr, str);
		print_arrow(sex, ptr, NULL);

		if (sex->u.call.args != NULL) {
			args_to_dot(prog, sex->u.call.args);
			print_arrow(sex, sex->u.call.args, NULL);
		}
		break;
	case SEX_UNARY:
		switch (sex->u.unary.op) {
		case UNARY_NEG: print_label(sex, "unary -"); break;
		case UNARY_INV: print_label(sex, "unary !"); break;
		default:
			assert(0);
		}
		sex_to_dot(prog, sex->u.unary.exp);
		print_arrow(sex, sex->u.unary.exp, NULL);
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
		sex_to_dot(prog, sex->u.binary.left);
		print_arrow(sex, sex->u.binary.left, NULL);
	}

	if (sex->u.binary.right != NULL) {
		sex_to_dot(prog, sex->u.binary.right);
		print_arrow(sex, sex->u.binary.right, NULL);
	}
}

static void decl_to_dot(program_t *prog, decl_t *decl)
{
	const char *tpstr, *name;
	char boxname[20];

	tpstr = type_to_str(decl->type);
	name = mcc_str_tab_resolve(&prog->identifiers, decl->identifier);

	gen_name(decl, boxname);
	printf("\t%s [shape=box, label=\"declare %s, type %s, count %d\"];\n",
		boxname, name, tpstr, decl->array_size);
}

static void stmt_to_dot(program_t *prog, statement_t *stmt)
{
	statement_t *s, *prev;
	const char *str;
	void *ptr;

	switch (stmt->type) {
	case STMT_IF:
		print_box(stmt, "IF");

		sex_to_dot(prog, stmt->st.branch.cond);
		print_arrow(stmt, stmt->st.branch.cond, "condition");

		stmt_to_dot(prog, stmt->st.branch.exec_true);
		print_arrow(stmt, stmt->st.branch.exec_true, "true");

		if (stmt->st.branch.exec_false) {
			stmt_to_dot(prog, stmt->st.branch.exec_false);
			print_arrow(stmt, stmt->st.branch.exec_false, "false");
		}
		break;
	case STMT_WHILE:
		print_box(stmt, "WHILE");

		sex_to_dot(prog, stmt->st.wloop.cond);
		stmt_to_dot(prog, stmt->st.wloop.body);

		print_arrow(stmt, stmt->st.wloop.cond, "condition");
		print_arrow(stmt, stmt->st.wloop.body, "body");
		break;
	case STMT_RET:
		print_box(stmt, "RETURN");

		if (stmt->st.ret) {
			sex_to_dot(prog, stmt->st.ret);
			print_arrow(stmt, stmt->st.ret, NULL);
		}
		break;
	case STMT_DECL:
		print_box(stmt, "DECLARE");
		print_arrow(stmt, stmt->st.decl, "what");
		decl_to_dot(prog, stmt->st.decl);
		break;
	case STMT_ASSIGN:
		print_box(stmt, "ASSIGN");

		ptr = &(stmt->st.assignment.identifier);
		str = mcc_str_tab_resolve(&prog->identifiers,
					  stmt->st.assignment.identifier);
		print_label(ptr, str);
		print_arrow(stmt, ptr, "LHS");

		sex_to_dot(prog, stmt->st.assignment.value);
		print_arrow(stmt, stmt->st.assignment.value, "RHS");

		if (stmt->st.assignment.array_index) {
			sex_to_dot(prog, stmt->st.assignment.array_index);
			print_arrow(stmt, stmt->st.assignment.array_index,
				    "INDEX");
		}
		break;
	case STMT_EXPR:
		print_box(stmt, "EXPR");
		sex_to_dot(prog, stmt->st.expr);
		print_arrow(stmt, stmt->st.expr, NULL);
		break;
	case STMT_COMPOUND:
		print_box(stmt, "COMPOUND");

		prev = stmt;
		s = stmt->st.compound_head;

		while (s != NULL) {
			stmt_to_dot(prog, s);
			print_arrow(prev, s, "sequence");

			prev = s;
			s = s->next;
		}
		break;
	}
}

void fun_to_dot(program_t *prog, function_def_t *def)
{
	const char *tpstr, *name;
	char boxname[20];
	decl_t *d, *prev;

	tpstr = type_to_str(def->type);
	name = mcc_str_tab_resolve(&prog->identifiers, def->identifier);

	gen_name(def, boxname);
	printf("\t%s [shape=box, label=\"function %s, type %s\"];\n",
		boxname, name, tpstr);

	if (def->parameters) {
		print_arrow(def, def->parameters, "parameters");

		d = def->parameters;
		prev = NULL;

		while (d != NULL) {
			decl_to_dot(prog, d);
			if (prev)
				print_arrow(prev, d, "next");

			prev = d;
			d = d->next;
		}
	}

	stmt_to_dot(prog, def->body);
	print_arrow(def, def->body, "body");
}

int main(int argc, char **argv)
{
	parser_result_t result;
	function_def_t *f;
	(void)argv;

	if (argc != 1) {
		fputs(	"Usage: mcc2dot\n\n"
			"mcc source is read from stdin, dot source is\n"
			"written to stdout.\n\n"
			"USE PIPES!\n", stderr);
		return EXIT_FAILURE;
	}

	result = mcc_parse_file(stdin);

	if (result.status != PARSER_STATUS_OK) {
		fputs("Error parsing input\n", stderr);
		return EXIT_FAILURE;
	}

	fputs("digraph mcc2dot {\n", stdout);

	for (f = result.program.functions; f != NULL; f = f->next) {
		fun_to_dot(&result.program, f);

		if (f->next)
			print_arrow(f, f->next, "next");
	}

	fputs("}\n", stdout);

	mcc_cleanup_program(&result.program);
	return EXIT_SUCCESS;
}
