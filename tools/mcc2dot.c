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

static void print_dotted_arrow(const void *src, const void *dst,
			       const char *label)
{
	char name1[20], name2[20];

	gen_name(src, name1);
	gen_name(dst, name2);

	if (label) {
		printf("\t%s -> %s [label=\"%s\", style=dotted];\n",
		       name1, name2, label);
	} else {
		printf("\t%s -> %s [style=dotted];\n", name1, name2);
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
	case SEX_VAR_ACCESS:
		ptr = &sex->u.var.identifier;
		str = mcc_str_tab_resolve(&prog->identifiers,
					  sex->u.var.identifier);

		if (sex->u.var.index != NULL) {
			print_label(sex, "array read");
			print_label(ptr, str);
			sex_to_dot(prog, sex->u.var.index);
			print_arrow(sex, ptr, "array");
			print_arrow(sex, sex->u.var.index, "index");
		} else {
			print_label(sex, str);
		}
		break;
	case SEX_RESOLVED_VAR:
		if (sex->u.var_resolved.index != NULL) {
			print_label(sex, "array read");
			sex_to_dot(prog, sex->u.var_resolved.index);
			print_dotted_arrow(sex, sex->u.var_resolved.var,
					   "array");
			print_arrow(sex, sex->u.var_resolved.index, "index");
		} else {
			print_label(sex, "var");
			print_dotted_arrow(sex, sex->u.var_resolved.var, NULL);
		}
		break;
	case SEX_CALL:
		ptr = &sex->u.call.identifier;
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
	case SEX_CALL_RESOLVED:
		print_label(sex, "call");
		print_dotted_arrow(sex, sex->u.call_resolved.fun, NULL);

		if (sex->u.call_resolved.args != NULL) {
			args_to_dot(prog, sex->u.call_resolved.args);
			print_arrow(sex, sex->u.call_resolved.args, NULL);
		}
		break;
	case SEX_CALL_BUILTIN:
		print_label(sex, mcc_builtin_name(sex->u.call_builtin.id));

		if (sex->u.call_builtin.args != NULL) {
			args_to_dot(prog, sex->u.call_builtin.args);
			print_arrow(sex, sex->u.call_builtin.args, NULL);
		}
		break;
	case SEX_UNARY_NEG:
		print_label(sex, "unary -");
		sex_to_dot(prog, sex->u.unary);
		print_arrow(sex, sex->u.unary, NULL);
		break;
	case SEX_UNARY_INV:
		print_label(sex, "unary !");
		sex_to_dot(prog, sex->u.unary);
		print_arrow(sex, sex->u.unary, NULL);
		break;
	case SEX_UNARY_FLOAT_TO_INT:
		print_label(sex, "float-to-int");
		sex_to_dot(prog, sex->u.unary);
		print_arrow(sex, sex->u.unary, NULL);
		break;
	case SEX_UNARY_INT_TO_FLOAT:
		print_label(sex, "int-to-float");
		sex_to_dot(prog, sex->u.unary);
		print_arrow(sex, sex->u.unary, NULL);
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
	case STMT_ASSIGN_RESOLVED:
		print_box(stmt, "ASSIGN");

		print_dotted_arrow(stmt, stmt->st.assign_resolved.target,
				   "LHS");

		sex_to_dot(prog, stmt->st.assign_resolved.value);
		print_arrow(stmt, stmt->st.assign_resolved.value, "RHS");

		if (stmt->st.assign_resolved.array_index) {
			sex_to_dot(prog, stmt->st.assign_resolved.array_index);
			print_arrow(stmt, stmt->st.assign_resolved.array_index,
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
	semantic_result_t sem;
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
		switch (result.status) {
		case PARSER_STATUS_OUT_OF_MEMORY:
			fputs("Out of memory\n", stderr);
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
	if (sem.status != SEMANTIC_STATUS_OK) {
		fputs("Semantic error in input program\n", stderr);
		mcc_cleanup_program(&result.program);
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
