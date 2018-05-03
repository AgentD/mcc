#include <stdlib.h>

#include "mcc.h"
#include "test.h"

const char *input =
"void main()\n"
"{\n"
"    foo();\n"
"}";

int main(void)
{
	semantic_result_t sem;
	expression_t *expr;
	parser_result_t pr;
	str_tab_t *ident;

	pr = mcc_parse_string(input);
	TEST_ASSERT(pr.status == PARSER_STATUS_OK)

	sem = mcc_semantic_check(&pr.program);
	TEST_ASSERT(sem.status == SEMANTIC_CALL_UNRESOLVED);

	ident = &pr.program.identifiers;

	expr = sem.u.expr;
	TEST_ASSERT(expr != NULL)
	TEST_ASSERT(expr->type == SEX_CALL)
	TEST_ASSERT(expr->u.call.args == NULL)
	ASSERT_IDENTIFIER(ident, expr->u.call.identifier, "foo")

	mcc_cleanup_program(&pr.program);
	return EXIT_SUCCESS;
}
