#include <stdlib.h>

#include "test.h"
#include "mcc.h"

const char *input = 
"int main() {\n"
"	if (42)\n"
"		if (21)\n"
"			foo();\n"
"		else\n"
"			bar();\n"
"	else\n"
"		fred();\n"
"	baz();\n"
"}\n";

int main(void)
{
	parser_result_t result = mcc_parse_string(input);
	statement_t *s, *l, *r;
	function_def_t *f;
	str_tab_t *ident;
	expression_t *e;

	if (result.status != PARSER_STATUS_OK)
		return EXIT_FAILURE;

	ident = &result.program.identifiers;

	f = result.program.functions;
	TEST_ASSERT(f->next == NULL)
	TEST_ASSERT(f->line_no == 1)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 1)

	/* if statement, followed by call to baz */
	s = f->body->st.compound_head;
	TEST_ASSERT(s != NULL)
	TEST_ASSERT(s->type == STMT_IF)
	TEST_ASSERT(s->line_no == 2)

	TEST_ASSERT(s->next != NULL)
	TEST_ASSERT(s->next->type == STMT_EXPR)
	TEST_ASSERT(s->next->next == NULL)
	TEST_ASSERT(s->next->line_no == 9)

	e = s->next->st.expr;
	TEST_ASSERT(e != NULL)
	TEST_ASSERT(e->type == SEX_CALL)
	TEST_ASSERT(e->u.call.args == NULL)
	ASSERT_IDENTIFIER(ident, e->u.call.identifier, "baz")

	/* top level if */
	TEST_ASSERT(s->st.branch.cond != NULL)
	TEST_ASSERT(s->st.branch.exec_true != NULL)
	TEST_ASSERT(s->st.branch.exec_false != NULL)

	e = s->st.branch.cond;
	TEST_ASSERT(e->type == SEX_LITERAL)
	TEST_ASSERT(e->u.lit.type == TYPE_INT)
	TEST_ASSERT(e->u.lit.value.i == 42)

	l = s->st.branch.exec_true;
	r = s->st.branch.exec_false;

	/* true path must be second level if statement */
	TEST_ASSERT(l->type == STMT_IF)
	TEST_ASSERT(l->line_no == 3)
	TEST_ASSERT(l->st.branch.cond != NULL)
	TEST_ASSERT(l->st.branch.exec_true != NULL)
	TEST_ASSERT(l->st.branch.exec_false != NULL)

	e = l->st.branch.cond;
	TEST_ASSERT(e->type == SEX_LITERAL)
	TEST_ASSERT(e->u.lit.type == TYPE_INT)
	TEST_ASSERT(e->u.lit.value.i == 21)

	/* false path must be call to fred */
	TEST_ASSERT(r->type == STMT_EXPR)
	TEST_ASSERT(r->next == NULL)
	TEST_ASSERT(r->line_no == 8)

	e = r->st.expr;
	TEST_ASSERT(e != NULL)
	TEST_ASSERT(e->type == SEX_CALL)
	TEST_ASSERT(e->u.call.args == NULL)
	ASSERT_IDENTIFIER(ident, e->u.call.identifier, "fred")

	/** inspect second level if **/
	s = l;
	l = s->st.branch.exec_true;
	r = s->st.branch.exec_true;

	/* true path must be call to foo */
	TEST_ASSERT(l->type == STMT_EXPR)
	TEST_ASSERT(l->next == NULL)
	TEST_ASSERT(l->line_no == 4)

	e = l->st.expr;
	TEST_ASSERT(e != NULL)
	TEST_ASSERT(e->type == SEX_CALL)
	TEST_ASSERT(e->u.call.args == NULL)
	ASSERT_IDENTIFIER(ident, e->u.call.identifier, "foo")

	/* false path must be call to bar */
	r = s->st.branch.exec_false;
	TEST_ASSERT(r->type == STMT_EXPR)
	TEST_ASSERT(r->next == NULL)
	TEST_ASSERT(r->line_no == 6)

	e = r->st.expr;
	TEST_ASSERT(e != NULL)
	TEST_ASSERT(e->type == SEX_CALL)
	TEST_ASSERT(e->u.call.args == NULL)
	ASSERT_IDENTIFIER(ident, e->u.call.identifier, "bar")

	mcc_cleanup_program(&result.program);
	return EXIT_SUCCESS;
}
