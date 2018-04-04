#ifndef MCC_H
#define MCC_H

#include <stdio.h>

#include "ast.h"

typedef enum {
	PARSER_STATUS_OK,
	PARSER_STATUS_UNABLE_TO_OPEN_STREAM,
	PARSER_STATUS_UNKNOWN_ERROR,
	PARSER_STATUS_OUT_OF_MEMORY,
	PARSER_STATUS_PARSE_ERROR,
} E_PARSER_STATUS;

typedef struct {
	E_PARSER_STATUS status;
	program_t program;
} parser_result_t;

#ifdef __cplusplus
extern "C" {
#endif

parser_result_t mcc_parse_file(FILE *input);
parser_result_t mcc_parse_string(const char *input);

#ifdef __cplusplus
}
#endif

#endif /* MCC_H */

