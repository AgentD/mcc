#include <stdlib.h>

#include "test.h"
#include "mcc.h"

#define ASSERT_IDENTIFIER(exp, ident) {\
	const char *str;\
	str = str_tab_resolve(&result.program.identifiers, exp->u.identifier);\
	TEST_ASSERT(str != NULL);\
	TEST_ASSERT(strcmp(str, ident) == 0);\
}

int main(void)
{
	const char *input = "a - b - c";
	expression_t *e;

	parser_result_t result = parse_string(input);

	if (result.status != PARSER_STATUS_OK)
		return EXIT_FAILURE;

	e = result.expression;

	TEST_ASSERT(e != NULL);
	TEST_ASSERT(e->type == BINOP_SUB);
	TEST_ASSERT(e->u.binary.left != NULL);
	TEST_ASSERT(e->u.binary.left->type == BINOP_SUB);
	TEST_ASSERT(e->u.binary.right != NULL);
	TEST_ASSERT(e->u.binary.right->type == SEX_IDENTIFIER);

	ASSERT_IDENTIFIER((e->u.binary.right), "c");

	e = e->u.binary.left;
	TEST_ASSERT(e->type == BINOP_SUB);
	TEST_ASSERT(e->u.binary.left != NULL);
	TEST_ASSERT(e->u.binary.left->type == SEX_IDENTIFIER);
	ASSERT_IDENTIFIER((e->u.binary.left), "a");
	TEST_ASSERT(e->u.binary.right != NULL);
	TEST_ASSERT(e->u.binary.right->type == SEX_IDENTIFIER);
	ASSERT_IDENTIFIER((e->u.binary.right), "b");

	expr_free(result.expression);
	mcc_cleanup_program(&result.program);
	return EXIT_SUCCESS;
}
