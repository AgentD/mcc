#include <string.h>
#include <assert.h>

#include "mcc.h"
#include "symtab.h"


static const struct {
	const char *name;
	E_BUILTIN_FUN id;
} built_in[] = {
	{ "print", BUILTIN_PRINT },
	{ "print_nl", BUILTIN_PRINT_NL },
	{ "print_int", BUILTIN_PRINT_INT },
	{ "print_float", BUILTIN_PRINT_FLOAT },
	{ "read_int", BUILTIN_READ_INT },
	{ "read_float", BUILTIN_READ_FLOAT },
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static semantic_result_t check_functions(program_t *prog)
{
	off_t main_ident, builtin_ident[ARRAY_SIZE(built_in)];
	semantic_result_t ret = { .status = SEMANTIC_STATUS_OK };
	bool main_found = false;
	function_def_t *f, *g;
	size_t i;

	/* generate/obtain IDs for main and built in functions */
	main_ident = mcc_str_tab_add(&prog->identifiers, "main");

	for (i = 0; i < ARRAY_SIZE(built_in); ++i) {
		builtin_ident[i] = mcc_str_tab_add(&prog->identifiers,
						   built_in[i].name);
	}

	for (f = prog->functions; f != NULL; f = f->next) {
		/* check if function uses the name of a built in */
		for (i = 0; i < ARRAY_SIZE(builtin_ident); ++i) {
			if (f->identifier == builtin_ident[i]) {
				ret.status = SEMANTIC_BUILTIN_REDEF;
				ret.u.redef.first = NULL;
				ret.u.redef.second = f;
				return ret;
			}
		}

		/* check if it re-uses a previously used function name */
		for (g = prog->functions; g != f; g = g->next) {
			if (g->identifier == f->identifier) {
				ret.status = SEMANTIC_FUNCTION_REDEF;
				ret.u.redef.first = g;
				ret.u.redef.second = f;
				return ret;
			}
		}

		/* if it is 'main', if yes, check function signature */
		if (f->identifier == main_ident) {
			main_found = true;

			if (f->type != TYPE_VOID || f->parameters != NULL) {
				ret.status = SEMANTIC_MAIN_TYPE;
				ret.u.main = f;
				return ret;
			}
		}
	}

	if (!main_found) {
		ret.status = SEMANTIC_MAIN_MISSING;
		return ret;
	}

	return ret;
}

static semantic_result_t check_var_redef(statement_t *list)
{
	semantic_result_t ret = { .status = SEMANTIC_STATUS_OK };
	statement_t *s, *head = list;
	off_t id;

	for (; list != NULL; list = list->next) {
		switch (list->type) {
		case STMT_COMPOUND:
			ret = check_var_redef(list->st.compound_head);
			if (ret.status != SEMANTIC_STATUS_OK)
				goto out;
			break;
		case STMT_DECL:
			id = list->st.decl->identifier;

			for (s = head; s != list; s = s->next) {
				if (s->type != STMT_DECL)
					continue;
				if (s->st.decl->identifier == id) {
					ret.status = SEMANTIC_VAR_REDEF;
					ret.u.vredef.first = s->st.decl;
					ret.u.vredef.second = list->st.decl;
					goto out;
				}
			}
			break;
		default:
			break;
		}
	}
out:
	return ret;
}

static semantic_result_t check_return_expr(statement_t *stmt, bool expected,
					   statement_t *parent)
{
	semantic_result_t ret = { .status = SEMANTIC_STATUS_OK };
	statement_t *s;

	if (stmt == NULL)
		return ret;

	switch (stmt->type) {
	case STMT_RET:
		if (stmt->st.ret == NULL && expected)
			ret.status = SEMANTIC_RET_NO_VAL;

		if (stmt->st.ret != NULL && !expected)
			ret.status = SEMANTIC_RET_VOID;

		ret.u.stmt = stmt;
		break;
	case STMT_IF:
		ret = check_return_expr(stmt->st.branch.exec_true,
					expected, parent);
		if (ret.status != SEMANTIC_STATUS_OK)
			return ret;
		ret = check_return_expr(stmt->st.branch.exec_false,
					expected, parent);
		if (ret.status != SEMANTIC_STATUS_OK)
			return ret;
		break;
	case STMT_WHILE:
		ret = check_return_expr(stmt->st.wloop.body, expected, parent);
		break;
	case STMT_DECL:
	case STMT_ASSIGN:
	case STMT_EXPR:
	case STMT_ASSIGN_RESOLVED:
		break;
	case STMT_COMPOUND:
		for (s = stmt->st.compound_head; s != NULL; s = s->next) {
			ret = check_return_expr(s, expected, stmt);
			if (ret.status != SEMANTIC_STATUS_OK)
				return ret;
			if (s->type == STMT_RET)
				return ret;
		}

		/* end of non-void function, we didn't find a return */
		if (expected && parent == NULL) {
			ret.status = SEMANTIC_NO_RET;
			ret.u.stmt = stmt;
		}
		break;
	}

	return ret;
}

static function_def_t *find_fun(program_t *prog, off_t identifier)
{
	function_def_t *f;

	for (f = prog->functions; f != NULL; f = f->next) {
		if (f->identifier == identifier)
			return f;
	}

	return NULL;
}

static bool find_builtin(program_t *prog, off_t identifier, E_BUILTIN_FUN *ret)
{
	const char *str = mcc_str_tab_resolve(&prog->identifiers, identifier);
	size_t i;

	for (i = 0; i < ARRAY_SIZE(built_in); ++i) {
		if (!strcmp(str, built_in[i].name)) {
			*ret = built_in[i].id;
			return true;
		}
	}

	return false;
}

static semantic_result_t link_fun_expr(expression_t *expr, program_t *prog,
				       symbol_t *symtab)
{
	semantic_result_t ret = { .status = SEMANTIC_STATUS_OK };
	E_BUILTIN_FUN builtin;
	function_def_t *f;
	symbol_t *sym;
	arg_t *a;

	if (expr == NULL)
		return ret;

	switch (expr->type) {
	case BINOP_ADD:
	case BINOP_SUB:
	case BINOP_MUL:
	case BINOP_DIV:
	case BINOP_LESS:
	case BINOP_GREATER:
	case BINOP_LEQ:
	case BINOP_GEQ:
	case BINOP_ANL:
	case BINOP_ORL:
	case BINOP_EQU:
	case BINOP_NEQU:
		ret = link_fun_expr(expr->u.binary.left, prog, symtab);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		ret = link_fun_expr(expr->u.binary.right, prog, symtab);
		break;
	case SEX_UNARY:
		ret = link_fun_expr(expr->u.unary.exp, prog, symtab);
		break;
	case SEX_LITERAL:
		break;
	case SEX_VAR_ACCESS:
		if (expr->u.var.index != NULL) {
			ret = link_fun_expr(expr->u.var.index, prog, symtab);
			if (ret.status != SEMANTIC_STATUS_OK)
				break;
		}

		sym = mcc_symtab_lookup(symtab, expr->u.var.identifier);
		if (sym == NULL) {
			ret.status = SEMANTIC_UNKNOWN_VAR;
			ret.u.expr = expr;
			break;
		}

		expr->type = SEX_RESOLVED_VAR;
		expr->u.var_resolved.index = expr->u.var.index;
		expr->u.var_resolved.var = sym->decl;
		break;
	case SEX_CALL:
		for (a = expr->u.call.args; a != NULL; a = a->next) {
			ret = link_fun_expr(a->expr, prog, symtab);
			if (ret.status != SEMANTIC_STATUS_OK)
				return ret;
		}

		f = find_fun(prog, expr->u.call.identifier);
		if (f != NULL) {
			expr->type = SEX_CALL_RESOLVED;
			expr->u.call_resolved.args = expr->u.call.args;
			expr->u.call_resolved.fun = f;
			break;
		}

		if (find_builtin(prog, expr->u.call.identifier, &builtin)) {
			expr->type = SEX_CALL_BUILTIN;
			expr->u.call_builtin.args = expr->u.call.args;
			expr->u.call_builtin.id = builtin;
			break;
		}

		ret.status = SEMANTIC_CALL_UNRESOLVED;
		ret.u.expr = expr;
		break;
	case SEX_CALL_RESOLVED:
	case SEX_CALL_BUILTIN:
	case SEX_RESOLVED_VAR:
		assert(0);
	}

	return ret;
}

static semantic_result_t link_fun_stmt(statement_t *stmt, program_t *prog,
				       symbol_t *symtab)
{
	semantic_result_t ret = { .status = SEMANTIC_STATUS_OK };
	symbol_t *oldhead, *sym;
	expression_t *a, *b;
	statement_t *s;

	if (stmt == NULL)
		return ret;

	switch (stmt->type) {
	case STMT_IF:
		ret = link_fun_expr(stmt->st.branch.cond, prog, symtab);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		ret = link_fun_stmt(stmt->st.branch.exec_true, prog, symtab);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		ret = link_fun_stmt(stmt->st.branch.exec_false, prog, symtab);
		break;
	case STMT_WHILE:
		ret = link_fun_expr(stmt->st.wloop.cond, prog, symtab);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		ret = link_fun_stmt(stmt->st.wloop.body, prog, symtab);
		break;
	case STMT_ASSIGN:
		ret = link_fun_expr(stmt->st.assignment.array_index, prog,
				    symtab);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		ret = link_fun_expr(stmt->st.assignment.value, prog, symtab);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;

		sym = mcc_symtab_lookup(symtab, stmt->st.assignment.identifier);
		if (sym == NULL) {
			ret.status = SEMANTIC_UNKNOWN_LHS;
			ret.u.stmt = stmt;
			break;
		}

		a = stmt->st.assignment.array_index;
		b = stmt->st.assignment.value;

		stmt->type = STMT_ASSIGN_RESOLVED;
		stmt->st.assign_resolved.target = sym->decl;
		stmt->st.assign_resolved.array_index = a;
		stmt->st.assign_resolved.value = b;
		break;
	case STMT_RET:
		ret = link_fun_expr(stmt->st.ret, prog, symtab);
		break;
	case STMT_DECL:
		break;
	case STMT_EXPR:
		ret = link_fun_expr(stmt->st.expr, prog, symtab);
		break;
	case STMT_COMPOUND:
		oldhead = symtab;

		for (s = stmt->st.compound_head; s != NULL; s = s->next) {
			if (s->type == STMT_DECL) {
				sym = mcc_mksymbol(s->st.decl);
				if (sym == NULL) {
					ret.status = SEMANTIC_OOM;
					break;
				}
				symtab = mcc_symtab_prepend(symtab, sym);
			}

			ret = link_fun_stmt(s, prog, symtab);
			if (ret.status != SEMANTIC_STATUS_OK)
				break;
		}

		while (symtab != oldhead)
			symtab = mcc_symtab_drop(symtab);
		break;
	case STMT_ASSIGN_RESOLVED:
		assert(0);
	}

	return ret;
}

semantic_result_t mcc_semantic_check(program_t *prog)
{
	symbol_t *s, *syms = NULL;
	semantic_result_t ret;
	function_def_t *f;
	decl_t *p;

	ret = check_functions(prog);

	if (ret.status != SEMANTIC_STATUS_OK)
		return ret;

	for (f = prog->functions; f != NULL; f = f->next) {
		ret = check_var_redef(f->body->st.compound_head);

		if (ret.status != SEMANTIC_STATUS_OK)
			return ret;

		ret = check_return_expr(f->body, f->type != TYPE_VOID, NULL);

		if (ret.status != SEMANTIC_STATUS_OK)
			return ret;

		for (p = f->parameters; p != NULL; p = p->next) {
			s = mcc_mksymbol(p);
			if (s == NULL) {
				ret.status = SEMANTIC_OOM;
				goto skip_link;
			}

			syms = mcc_symtab_prepend(syms, s);
		}

		ret = link_fun_stmt(f->body, prog, syms);
	skip_link:
		while (syms != NULL)
			syms = mcc_symtab_drop(syms);

		if (ret.status != SEMANTIC_STATUS_OK)
			return ret;
	}

	return ret;
}
