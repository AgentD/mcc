#include <stdlib.h>

#include "mcc.h"
#include "test.h"

const char *input =
"void main()\n"
"{\n"
"    return 0;"
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
	TEST_ASSERT(sem.status == SEMANTIC_RET_VOID)

	TEST_ASSERT(stmt != NULL)
	TEST_ASSERT(stmt->type == STMT_RET)
	TEST_ASSERT(stmt->line_no == 3)

	mcc_cleanup_program(&pr.program);
	return EXIT_SUCCESS;
}
