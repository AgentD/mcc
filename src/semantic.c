#include <string.h>

#include "mcc.h"
#include "symtab.h"

static function_def_t *find_fun(program_t *prog, off_t identifier)
{
	function_def_t *f = prog->functions;

	while (f != NULL && f->identifier != identifier)
		f = f->next;

	return f;
}

static bool find_builtin(program_t *prog, off_t identifier, E_BUILTIN_FUN *ret)
{
	int i;

	for (i = 0; i < BUILTIN_MAX; ++i) {
		if (identifier == prog->builtins[i]) {
			if (ret != NULL)
				*ret = i;
			return true;
		}
	}

	return false;
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
		return check_return_expr(stmt->st.branch.exec_false,
					 expected, parent);
	case STMT_WHILE:
		return check_return_expr(stmt->st.wloop.body, expected, parent);
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
	default:
		break;
	}

	return ret;
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
	case SEX_UNARY_NEG:
	case SEX_UNARY_INV:
	case SEX_UNARY_INT_TO_FLOAT:
	case SEX_UNARY_FLOAT_TO_INT:
		return link_fun_expr(expr->u.unary, prog, symtab);
	case SEX_CALL_RESOLVED:
	case SEX_CALL_BUILTIN:
	case SEX_RESOLVED_VAR:
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
			expr->u.call_builtin.identifier = expr->u.call.identifier;
			expr->u.call_builtin.id = builtin;
			break;
		}

		ret.status = SEMANTIC_CALL_UNRESOLVED;
		ret.u.expr = expr;
		break;
	default:
		ret = link_fun_expr(expr->u.binary.left, prog, symtab);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		ret = link_fun_expr(expr->u.binary.right, prog, symtab);
		break;
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
		return link_fun_stmt(stmt->st.branch.exec_false, prog, symtab);
	case STMT_WHILE:
		ret = link_fun_expr(stmt->st.wloop.cond, prog, symtab);
		if (ret.status != SEMANTIC_STATUS_OK)
			break;
		return link_fun_stmt(stmt->st.wloop.body, prog, symtab);
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
		return link_fun_expr(stmt->st.ret, prog, symtab);
	case STMT_ASSIGN_RESOLVED:
	case STMT_DECL:
		break;
	case STMT_EXPR:
		return link_fun_expr(stmt->st.expr, prog, symtab);
	case STMT_COMPOUND:
		oldhead = symtab;

		for (s = stmt->st.compound_head; s != NULL; s = s->next) {
			if (s->type == STMT_DECL) {
				sym = mcc_symtab_lookup(symtab,
							s->st.decl->identifier);
				if (sym != NULL && sym->parent == stmt) {
					ret.status = SEMANTIC_VAR_REDEF;
					ret.u.vredef.first = sym->decl;
					ret.u.vredef.second = s->st.decl;
					break;
				}

				sym = mcc_mksymbol(s->st.decl, stmt);
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
	}

	return ret;
}

semantic_result_t mcc_semantic_check(program_t *prog)
{
	symbol_t *s, *syms = NULL;
	bool main_found = false;
	semantic_result_t ret;
	function_def_t *f, *g;
	off_t main_ident;
	decl_t *p;

	main_ident = mcc_str_tab_add(&prog->identifiers, "main");

	for (f = prog->functions; f != NULL; f = f->next) {
		if (find_builtin(prog, f->identifier, NULL)) {
			ret.status = SEMANTIC_BUILTIN_REDEF;
			ret.u.redef.first = NULL;
			ret.u.redef.second = f;
			return ret;
		}

		g = find_fun(prog, f->identifier);
		if (g != f) {
			ret.status = SEMANTIC_FUNCTION_REDEF;
			ret.u.redef.first = g;
			ret.u.redef.second = f;
			return ret;
		}

		if (f->identifier == main_ident) {
			main_found = true;

			if (f->type != TYPE_VOID || f->parameters != NULL) {
				ret.status = SEMANTIC_MAIN_TYPE;
				ret.u.main = f;
				return ret;
			}
		}

		ret = check_return_expr(f->body, f->type != TYPE_VOID, NULL);

		if (ret.status != SEMANTIC_STATUS_OK)
			return ret;

		for (p = f->parameters; p != NULL; p = p->next) {
			s = mcc_symtab_lookup(syms, p->identifier);
			if (s != NULL) {
				ret.status = SEMANTIC_VAR_REDEF;
				ret.u.vredef.first = s->decl;
				ret.u.vredef.second = p;
				goto skip_link;
			}

			s = mcc_mksymbol(p, NULL);
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

	if (!main_found)
		ret.status = SEMANTIC_MAIN_MISSING;

	return ret;
}
