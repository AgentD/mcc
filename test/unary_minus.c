#include <stdlib.h>

#include "test.h"
#include "mcc.h"

const char *input = 
"int main() {\n"
"	a - - b;\n"
"}\n";

int main(void)
{
	parser_result_t result = mcc_parse_string(input);
	expression_t *e, *l, *r;
	function_def_t *f;
	str_tab_t *ident;
	statement_t *s;

	if (result.status != PARSER_STATUS_OK)
		return EXIT_FAILURE;

	/* assumes test_expr_basic passes */
	f = result.program.functions;
	ident = &result.program.identifiers;
	s = f->body->st.compound_head;
	e = s->st.expr;

	/* the expression tree must be a - (-b) */
	TEST_ASSERT(e != NULL)
	TEST_ASSERT(e->type == BINOP_SUB)

	l = e->u.binary.left;
	r = e->u.binary.right;

	TEST_ASSERT(l != NULL)
	TEST_ASSERT(r != NULL)

	TEST_ASSERT(l->type == SEX_IDENTIFIER)
	TEST_ASSERT(r->type == SEX_UNARY)

	ASSERT_IDENTIFIER(ident, l->u.identifier, "a")

	TEST_ASSERT(r->u.unary.op == UNARY_NEG)
	e = r->u.unary.exp;

	TEST_ASSERT(e != NULL)
	TEST_ASSERT(e->type == SEX_IDENTIFIER)
	ASSERT_IDENTIFIER(ident, e->u.identifier, "b")

	mcc_cleanup_program(&result.program);
	return EXIT_SUCCESS;
}
