#include <string.h>
#include <stdlib.h>

#include "ast.h"

void mcc_init_program(program_t *prog)
{
	memset(prog, 0, sizeof(*prog));
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
