#include <stdlib.h>
#include <stdio.h>

#include "mcc.h"

int main(int argc, char **argv)
{
	int i, ret = EXIT_SUCCESS;
	parser_result_t result;
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

		mcc_cleanup_program(&result.program);
	}

	return ret;
}
