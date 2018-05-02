#include <stdlib.h>
#include <stdio.h>

#include "mcc.h"

int main(int argc, char **argv)
{
	int i, ret = EXIT_SUCCESS;
	parser_result_t result;
	semantic_result_t sem;
	const char *name;
	FILE *in;

	for (i = 1; i < argc; ++i) {
		in = fopen(argv[i], "r");
		if (in == NULL) {
			perror(argv[i]);
			ret = EXIT_FAILURE;
			continue;
		}

		result = mcc_parse_file(in);
		fclose(in);

		if (result.status != PARSER_STATUS_OK) {
			switch (result.status) {
			case PARSER_STATUS_OUT_OF_MEMORY:
				fprintf(stderr, "%s: Out of memory\n",
					argv[i]);
				break;
			case PARSER_STATUS_PARSE_ERROR:
				fprintf(stderr, "%s: %u: %s\n",
					argv[i],
					result.program.error_line,
					result.program.error_msg);
				break;
			default:
				fprintf(stderr,
					"%s: Unknown error parsing input\n",
					argv[i]);
				break;
			}

			ret = EXIT_FAILURE;
		}

		sem = mcc_semantic_check(&result.program);

		if (sem.status != SEMANTIC_STATUS_OK) {
			ret = EXIT_FAILURE;
		}

		switch (sem.status) {
		case SEMANTIC_STATUS_OK:
			break;
		case SEMANTIC_FUNCTION_REDEF:
			name = mcc_str_tab_resolve(&result.program.identifiers,
						   sem.u.redef.first->identifier);
			fprintf(stderr, "%s: %u: function '%s' "
				"redefined. Previous definition here: "
				"%u\n", argv[i],
				sem.u.redef.second->line_no,
				name, sem.u.redef.first->line_no);
			break;
		case SEMANTIC_MAIN_MISSING:
			fprintf(stderr,"%s: main function missing\n",argv[i]);
			break;
		case SEMANTIC_MAIN_TYPE:
			fprintf(stderr, "%s: %u: type must be void main()\n",
				argv[i], sem.u.main->line_no);
			break;
		case SEMANTIC_BUILTIN_REDEF:
			fprintf(stderr, "%s: %u: redefinition of built-in\n",
				argv[i], sem.u.redef.second->line_no);
			break;
		case SEMANTIC_VAR_REDEF:
			name = mcc_str_tab_resolve(&result.program.identifiers,
						   sem.u.vredef.first->identifier);
			fprintf(stderr, "%s: %u: variable '%s' redefined. "
				"Previous definition here: %u\n", argv[i],
				sem.u.vredef.second->line_no, name,
				sem.u.vredef.first->line_no);
			break;
		case SEMANTIC_RET_NO_VAL:
			fprintf(stderr, "%s: %u: expected value after "
				"return.\n", argv[i], sem.u.stmt->line_no);
			break;
		case SEMANTIC_RET_VOID:
			fprintf(stderr, "%s: %u: no value expected after "
				"return\n", argv[i], sem.u.stmt->line_no);
			break;
		case SEMANTIC_NO_RET:
			fprintf(stderr, "%s: %u: no return at end of "
				"non-void function.\n", argv[i],
				sem.u.stmt->line_no);
			break;
		case SEMANTIC_CALL_UNRESOLVED:
			name = mcc_str_tab_resolve(&result.program.identifiers,
						   sem.u.expr->u.call.identifier);
			fprintf(stderr, "%s: %u: call to unknown "
				"function '%s'\n", argv[i], sem.u.expr->line_no,
				name);
			break;
		}

		mcc_cleanup_program(&result.program);
	}

	return ret;
}
