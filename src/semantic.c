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

	main_ident = mcc_str_tab_add(&prog->identifiers, "main");

	for (i = 0; i < ARRAY_SIZE(built_in); ++i)
		builtin_ident[i] = mcc_str_tab_add(&prog->identifiers, built_in[i]);
	
	for (f = prog->functions; f != NULL; f = f->next) {
		for (i = 0; i < ARRAY_SIZE(builtin_ident); ++i) {
			if (f->identifier == builtin_ident[i]) {
				ret.status = SEMATNIC_BUILTIN_REDEF;
				ret.u.redef.first = NULL;
				ret.u.redef.second = f;
				return ret;
			}
		}

		for (g = prog->functions; g != f; g = g->next) {
			if (g->identifier == f->identifier) {
				ret.status = SEMANTIC_FUNCTION_REDEF;
				ret.u.redef.first = g;
				ret.u.redef.second = f;
				return ret;
			}
		}

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


semantic_result_t mcc_semantic_check(program_t *prog)
{
	semantic_result_t ret;

	ret = check_functions(prog);
	
	if (ret.status != SEMANTIC_STATUS_OK)
		return ret;
	
	return ret;
}
