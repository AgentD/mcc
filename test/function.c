#include <stdlib.h>

#include "mcc.h"
#include "test.h"

const char *input =
"int foo()\n"
"{\n"
"}\n"
"\n"
"void bar()\n"
"{\n"
"}\n"
"\n"
"int baz(int a)\n"
"{\n"
"}\n"
"\n"
"void qux(int a)\n"
"{\n"
"}\n"
"\n"
"int fred(int a, float b)\n"
"{\n"
"}\n"
"\n"
"void\n"
"george(int a, float b)\n"
"{\n"
"}\n"
"\n"
"float joe(int a, float b,\n"
"          string c)\n"
"{\n"
"}\n";

int main(void)
{
	parser_result_t result = mcc_parse_string(input);
	function_def_t *f;
	str_tab_t *ident;

	if (result.status != PARSER_STATUS_OK)
		return EXIT_FAILURE;

	f = result.program.functions;
	ident = &result.program.identifiers;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_INT)
	TEST_ASSERT(f->line_no == 1)
	TEST_ASSERT(f->parameters == NULL)
	TEST_ASSERT(f->body != NULL)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 2)
	TEST_ASSERT(f->body->next == NULL)
	TEST_ASSERT(f->body->st.compound_head == NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "foo")

	f = f->next;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_VOID)
	TEST_ASSERT(f->line_no == 5)
	TEST_ASSERT(f->parameters == NULL)
	TEST_ASSERT(f->body != NULL)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 6)
	TEST_ASSERT(f->body->next == NULL)
	TEST_ASSERT(f->body->st.compound_head == NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "bar")

	f = f->next;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_INT)
	TEST_ASSERT(f->line_no == 9)
	TEST_ASSERT(f->parameters != NULL)
	TEST_ASSERT(f->parameters->type == TYPE_INT)
	TEST_ASSERT(f->parameters->array_size == 1)
	TEST_ASSERT(f->parameters->line_no == 9)
	TEST_ASSERT(f->parameters->next == NULL)
	ASSERT_IDENTIFIER(ident, f->parameters->identifier, "a")
	TEST_ASSERT(f->body != NULL)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 10)
	TEST_ASSERT(f->body->next == NULL)
	TEST_ASSERT(f->body->st.compound_head == NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "baz")

	f = f->next;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_VOID)
	TEST_ASSERT(f->line_no == 13)
	TEST_ASSERT(f->parameters != NULL)
	TEST_ASSERT(f->parameters->type == TYPE_INT)
	TEST_ASSERT(f->parameters->array_size == 1)
	TEST_ASSERT(f->parameters->line_no == 13)
	TEST_ASSERT(f->parameters->next == NULL)
	ASSERT_IDENTIFIER(ident, f->parameters->identifier, "a")
	TEST_ASSERT(f->body != NULL)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 14)
	TEST_ASSERT(f->body->next == NULL)
	TEST_ASSERT(f->body->st.compound_head == NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "qux")

	f = f->next;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_INT)
	TEST_ASSERT(f->line_no == 17)
	TEST_ASSERT(f->parameters != NULL)
	TEST_ASSERT(f->parameters->type == TYPE_INT)
	TEST_ASSERT(f->parameters->array_size == 1)
	TEST_ASSERT(f->parameters->line_no == 17)
	TEST_ASSERT(f->parameters->next != NULL)
	ASSERT_IDENTIFIER(ident, f->parameters->identifier, "a")
	TEST_ASSERT(f->parameters->next->type == TYPE_FLOAT)
	TEST_ASSERT(f->parameters->next->array_size == 1)
	TEST_ASSERT(f->parameters->next->line_no == 17)
	TEST_ASSERT(f->parameters->next->next == NULL)
	ASSERT_IDENTIFIER(ident, f->parameters->next->identifier, "b")
	TEST_ASSERT(f->body != NULL)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 18)
	TEST_ASSERT(f->body->next == NULL)
	TEST_ASSERT(f->body->st.compound_head == NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "fred")

	f = f->next;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_VOID)
	TEST_ASSERT(f->line_no == 21)
	TEST_ASSERT(f->parameters != NULL)
	TEST_ASSERT(f->parameters->type == TYPE_INT)
	TEST_ASSERT(f->parameters->array_size == 1)
	TEST_ASSERT(f->parameters->line_no == 22)
	TEST_ASSERT(f->parameters->next != NULL)
	ASSERT_IDENTIFIER(ident, f->parameters->identifier, "a")
	TEST_ASSERT(f->parameters->next->type == TYPE_FLOAT)
	TEST_ASSERT(f->parameters->next->array_size == 1)
	TEST_ASSERT(f->parameters->next->line_no == 22)
	TEST_ASSERT(f->parameters->next->next == NULL)
	ASSERT_IDENTIFIER(ident, f->parameters->next->identifier, "b")
	TEST_ASSERT(f->body != NULL)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 23)
	TEST_ASSERT(f->body->next == NULL)
	TEST_ASSERT(f->body->st.compound_head == NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "george")

	f = f->next;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_FLOAT)
	TEST_ASSERT(f->line_no == 26)
	TEST_ASSERT(f->parameters != NULL)
	TEST_ASSERT(f->parameters->type == TYPE_INT)
	TEST_ASSERT(f->parameters->array_size == 1)
	TEST_ASSERT(f->parameters->line_no == 26)
	TEST_ASSERT(f->parameters->next != NULL)
	ASSERT_IDENTIFIER(ident, f->parameters->identifier, "a")
	TEST_ASSERT(f->parameters->next->type == TYPE_FLOAT)
	TEST_ASSERT(f->parameters->next->array_size == 1)
	TEST_ASSERT(f->parameters->next->line_no == 26)
	TEST_ASSERT(f->parameters->next->next != NULL)
	ASSERT_IDENTIFIER(ident, f->parameters->next->identifier, "b")
	TEST_ASSERT(f->parameters->next->next->type == TYPE_STRING)
	TEST_ASSERT(f->parameters->next->next->array_size == 1)
	TEST_ASSERT(f->parameters->next->next->line_no == 27)
	TEST_ASSERT(f->parameters->next->next->next == NULL)
	ASSERT_IDENTIFIER(ident, f->parameters->next->next->identifier, "c")
	TEST_ASSERT(f->body != NULL)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 28)
	TEST_ASSERT(f->body->next == NULL)
	TEST_ASSERT(f->body->st.compound_head == NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "joe")

	f = f->next;
	TEST_ASSERT(f == NULL)

	mcc_cleanup_program(&result.program);
	return EXIT_SUCCESS;
}
