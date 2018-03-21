#include <stdlib.h>

#include "test.h"
#include "mcc.h"

int main(void)
{
	const char *input = "a - - b";
	expression_t *e;

	parser_result_t result = parse_string(input);

	if (result.status != PARSER_STATUS_OK)
		return EXIT_FAILURE;

	e = result.expression;

	TEST_ASSERT(e != NULL);
	TEST_ASSERT(e->type == BINOP_SUB);

	TEST_ASSERT(e->u.binary.left != NULL);
	TEST_ASSERT(e->u.binary.left->type == SEX_IDENTIFIER);

	e = e->u.binary.right;
	TEST_ASSERT(e != NULL);
	TEST_ASSERT(e->type == SEX_UNARY);
	TEST_ASSERT(e->u.unary.op == UNARY_NEG);
	TEST_ASSERT(e->u.unary.exp != NULL);
	TEST_ASSERT(e->u.unary.exp->type == SEX_IDENTIFIER);

	expr_free(result.expression);
	mcc_cleanup_program(&result.program);
	return EXIT_SUCCESS;
}
