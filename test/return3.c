#include <stdlib.h>

#include "mcc.h"
#include "test.h"

const char *input =
"void main()\n"
"{\n"
"}\n"
"int foo()\n"
"{\n"
"    if (true)\n"
"        return 42;\n"
"}\n";

int main(void)
{
	semantic_result_t sem;
	parser_result_t pr;

	pr = mcc_parse_string(input);
	TEST_ASSERT(pr.status == PARSER_STATUS_OK)

	sem = mcc_semantic_check(&pr.program);
	TEST_ASSERT(sem.status == SEMANTIC_NO_RET)

	mcc_cleanup_program(&pr.program);
	return EXIT_SUCCESS;
}
