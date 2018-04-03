#ifndef DECL_H
#define DECL_H

#include <sys/types.h>

#include "literal.h"

typedef struct decl_t {
	E_TYPE type;
	int array_size;
	off_t identifier;

	unsigned int line_no;

	struct decl_t *next;
} decl_t;

#ifdef __cplusplus
extern "C" {
#endif

decl_t *mcc_declaration(E_TYPE type, int size, off_t identifier);

void mcc_declaration_free(decl_t *d);

#ifdef __cplusplus
}
#endif

#endif /* DECL_H */

