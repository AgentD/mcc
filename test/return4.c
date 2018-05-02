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
"        return;\n"
"    return 42;\n"
"}\n";

int main(void)
{
	semantic_result_t sem;
	parser_result_t pr;
	statement_t *stmt;

	pr = mcc_parse_string(input);
	TEST_ASSERT(pr.status == PARSER_STATUS_OK)

	sem = mcc_semantic_check(&pr.program);
	stmt = sem.u.stmt;
	TEST_ASSERT(sem.status == SEMANTIC_RET_NO_VAL)

	TEST_ASSERT(stmt != NULL)
	TEST_ASSERT(stmt->line_no == 7)
	TEST_ASSERT(stmt->type == STMT_RET)

	mcc_cleanup_program(&pr.program);
	return EXIT_SUCCESS;
}
