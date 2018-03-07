#include <string.h>

#include "ast.h"

static program_t *program = NULL;

void mcc_init_program(program_t *prog)
{
	memset(prog, 0, sizeof(*prog));
}

program_t *mcc_get_program(void)
{
	return program;
}

void mcc_set_program(program_t *prog)
{
	program = prog;
}
