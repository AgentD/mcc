#include <stdlib.h>

#include "mcc.h"

int main(void)
{
	const char *input = "192 + 3.14";

	parser_result_t result = parse_string(input);

	if (result.status != PARSER_STATUS_OK)
		return EXIT_FAILURE;

	expr_free(result.expression);
	return EXIT_SUCCESS;
}
