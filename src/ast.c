#include <string.h>

#include "ast.h"

void mcc_init_program(program_t *prog)
{
	memset(prog, 0, sizeof(*prog));
}

void mcc_cleanup_program(program_t *prog)
{
	str_tab_cleanup(&prog->strings);
	str_tab_cleanup(&prog->identifiers);
	memset(prog, 0, sizeof(*prog));
}
