#include <stdlib.h>

#include "ast.h"

decl_t *declaration(E_TYPE type, int size, off_t identifier)
{
	decl_t *d = calloc(1, sizeof(*d));

	if (d != NULL) {
		d->type = type;
		d->array_size = size;
		d->identifier = identifier;
	}
	return d;
}

void declaration_free(decl_t *d)
{
	free(d);
}
