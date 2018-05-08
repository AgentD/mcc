#include <stdlib.h>

#include "mcc.h"
#include "test.h"

const char *input =
"int foo()\n"
"{\n"
"    return 0;\n"
"}\n"
"\n"
"void bar()\n"
"{\n"
"}\n"
"\n"
"void foo()\n"
"{\n"
"}\n";

int main(void)
{
	semantic_result_t sem;
	parser_result_t pr;
	function_def_t *f;
	str_tab_t *ident;

	pr = mcc_parse_string(input);
	TEST_ASSERT(pr.status == PARSER_STATUS_OK)

	f = pr.program.functions;
	ident = &pr.program.identifiers;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_INT)
	TEST_ASSERT(f->line_no == 1)
	TEST_ASSERT(f->parameters == NULL)
	TEST_ASSERT(f->body != NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "foo")

	f = f->next;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_VOID)
	TEST_ASSERT(f->line_no == 6)
	TEST_ASSERT(f->parameters == NULL)
	TEST_ASSERT(f->body != NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "bar")

	f = f->next;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_VOID)
	TEST_ASSERT(f->line_no == 10)
	TEST_ASSERT(f->parameters == NULL)
	TEST_ASSERT(f->body != NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "foo")

	f = f->next;
	TEST_ASSERT(f == NULL)

	sem = mcc_semantic_check(&pr.program);
	TEST_ASSERT(sem.status == SEMANTIC_FUNCTION_REDEF)

	f = sem.u.redef.first;
	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_INT)
	TEST_ASSERT(f->line_no == 1)
	TEST_ASSERT(f->parameters == NULL)
	TEST_ASSERT(f->body != NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "foo")

	f = sem.u.redef.second;
	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_VOID)
	TEST_ASSERT(f->line_no == 10)
	TEST_ASSERT(f->parameters == NULL)
	TEST_ASSERT(f->body != NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "foo")

	mcc_cleanup_program(&pr.program);
	return EXIT_SUCCESS;
}
