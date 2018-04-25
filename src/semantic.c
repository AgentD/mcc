#include "mcc.h"

static const char *built_in[] = {
	"print",
	"print_nl",
	"print_int",
	"print_float",
	"read_int",
	"read_float",
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

	for (i = 0; i < ARRAY_SIZE(built_in); ++i)
		builtin_ident[i] = mcc_str_tab_add(&prog->identifiers, built_in[i]);

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


semantic_result_t mcc_semantic_check(program_t *prog)
{
	semantic_result_t ret;
	function_def_t *f;

	ret = check_functions(prog);
	
	if (ret.status != SEMANTIC_STATUS_OK)
		return ret;

	for (f = prog->functions; f != NULL; f = f->next) {
		ret = check_var_redef(f->body->st.compound_head);

		if (ret.status != SEMANTIC_STATUS_OK)
			return ret;
	}

	return ret;
}
