#include <stdlib.h>

#include "decl.h"

decl_t *mcc_declaration(E_TYPE type, int size, off_t identifier,
			unsigned int flags)
{
	decl_t *d = calloc(1, sizeof(*d));

	if (d != NULL) {
		d->type = type;
		d->flags = flags;
		d->array_size = size;
		d->identifier = identifier;
	}
	return d;
}

void mcc_declaration_free(decl_t *d)
{
	free(d);
}
