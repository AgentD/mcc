#include <string.h>
#include <stdlib.h>

#include "ast.h"

void mcc_init_program(program_t *prog)
{
	const char *name;
	int i;

	memset(prog, 0, sizeof(*prog));

	/* prepare built in function names */
	for (i = 0; i < BUILTIN_MAX; ++i) {
		name = mcc_builtin_name(i);
		prog->builtins[i] = mcc_str_tab_add(&prog->identifiers, name);
	}
}

void mcc_cleanup_program(program_t *prog)
{
	function_def_t *f;

	while (prog->functions != NULL) {
		f = prog->functions;
		prog->functions = f->next;

		mcc_function_free(f);
	}

	mcc_str_tab_cleanup(&prog->strings);
	mcc_str_tab_cleanup(&prog->identifiers);
	free(prog->error_msg);
	memset(prog, 0, sizeof(*prog));
}
