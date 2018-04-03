#include <stdlib.h>

#include "test.h"
#include "mcc.h"

const char *input = 
"int main() {\n"
"	42;\n"
"}\n";

int main(void)
{
	parser_result_t result = mcc_parse_string(input);
	function_def_t *f;
	expression_t *e;
	statement_t *s;

	if (result.status != PARSER_STATUS_OK)
		return EXIT_FAILURE;

	f = result.program.functions;

	TEST_ASSERT(f->next == NULL)
	TEST_ASSERT(f->line_no == 1)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 1)

	s = f->body->st.compound_head;
	TEST_ASSERT(s != NULL)
	TEST_ASSERT(s->type == STMT_EXPR)
	TEST_ASSERT(s->next == NULL)
	TEST_ASSERT(s->line_no == 2)

	e = s->st.expr;
	TEST_ASSERT(e != NULL)
	TEST_ASSERT(e->type == SEX_LITERAL)
	TEST_ASSERT(e->line_no == 2)
	TEST_ASSERT(e->u.lit.type == TYPE_INT)
	TEST_ASSERT(e->u.lit.value.i == 42)

	mcc_cleanup_program(&result.program);
	return EXIT_SUCCESS;
}
