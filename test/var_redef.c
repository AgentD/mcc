#include <stdlib.h>

#include "mcc.h"
#include "test.h"

const char *input =
"void foo()\n"
"{\n"
"    int foo;\n"
"    int bar;\n"
"}\n"
"void main()\n"
"{\n"
"    int foo;\n"
"    int bar;\n"
"    int foo;\n"
"}\n";

int main(void)
{
	semantic_result_t sem;
	parser_result_t pr;
	str_tab_t *ident;
	decl_t *d;

	pr = mcc_parse_string(input);
	TEST_ASSERT(pr.status == PARSER_STATUS_OK)

	ident = &pr.program.identifiers;

	sem = mcc_semantic_check(&pr.program);
	TEST_ASSERT(sem.status == SEMANTIC_VAR_REDEF)

	d = sem.u.vredef.first;
	TEST_ASSERT(d != NULL)
	TEST_ASSERT(d->line_no == 8)
	ASSERT_IDENTIFIER(ident, d->identifier, "foo")

	d = sem.u.vredef.second;
	TEST_ASSERT(d != NULL)
	TEST_ASSERT(d->line_no == 10)
	ASSERT_IDENTIFIER(ident, d->identifier, "foo")

	mcc_cleanup_program(&pr.program);
	return EXIT_SUCCESS;
}
