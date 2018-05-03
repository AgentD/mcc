#include <stdlib.h>

#include "mcc.h"
#include "test.h"

const char *input =
"void main()\n"
"{\n"
"    foo();\n"
"    bar();\n"
"    print_nl();\n"
"}\n"
"void foo()\n"
"{\n"
"}\n"
"int bar()\n"
"{\n"
"    return 0;\n"
"}\n";

int main(void)
{
	semantic_result_t sem;
	expression_t *expr;
	parser_result_t pr;
	function_def_t *f;
	statement_t *stmt;
	str_tab_t *ident;

	pr = mcc_parse_string(input);
	TEST_ASSERT(pr.status == PARSER_STATUS_OK)

	sem = mcc_semantic_check(&pr.program);
	TEST_ASSERT(sem.status == SEMANTIC_STATUS_OK)

	f = pr.program.functions;
	ident = &pr.program.identifiers;

	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_VOID)
	TEST_ASSERT(f->line_no == 1)
	TEST_ASSERT(f->parameters == NULL)
	TEST_ASSERT(f->body != NULL)
	TEST_ASSERT(f->body->type == STMT_COMPOUND)
	TEST_ASSERT(f->body->line_no == 2)
	ASSERT_IDENTIFIER(ident, f->identifier, "main")

	stmt = f->body->st.compound_head;
	TEST_ASSERT(stmt != NULL)
	TEST_ASSERT(stmt->type == STMT_EXPR)

	expr = stmt->st.expr;
	TEST_ASSERT(expr->type == SEX_CALL_RESOLVED)
	TEST_ASSERT(expr->u.call_resolved.args == NULL)

	f = expr->u.call_resolved.fun;
	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_VOID)
	TEST_ASSERT(f->line_no == 7)
	TEST_ASSERT(f->parameters == NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "foo")

	stmt = stmt->next;
	TEST_ASSERT(stmt != NULL)
	TEST_ASSERT(stmt->type == STMT_EXPR)

	expr = stmt->st.expr;
	TEST_ASSERT(expr->type == SEX_CALL_RESOLVED)
	TEST_ASSERT(expr->u.call_resolved.args == NULL)

	f = expr->u.call_resolved.fun;
	TEST_ASSERT(f != NULL)
	TEST_ASSERT(f->type == TYPE_INT)
	TEST_ASSERT(f->line_no == 10)
	TEST_ASSERT(f->parameters == NULL)
	ASSERT_IDENTIFIER(ident, f->identifier, "bar")

	stmt = stmt->next;
	TEST_ASSERT(stmt != NULL)
	TEST_ASSERT(stmt->type == STMT_EXPR)

	expr = stmt->st.expr;
	TEST_ASSERT(expr->type == SEX_CALL_BUILTIN)
	TEST_ASSERT(expr->u.call_builtin.args == NULL)
	TEST_ASSERT(expr->u.call_builtin.id == BUILTIN_PRINT_NL)

	TEST_ASSERT(stmt->next == NULL)

	mcc_cleanup_program(&pr.program);
	return EXIT_SUCCESS;
}
