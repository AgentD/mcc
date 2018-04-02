#include <stdlib.h>

#include "ast.h"


function_def_t *mcc_function(E_TYPE type, off_t identifier, decl_t *parameters,
			     statement_t *body)
{
	function_def_t *f = calloc(1, sizeof(*f));
	decl_t *prev, *current, *next;

	if (f != NULL) {
		prev = next = NULL;
		current = parameters;

		while (current != NULL) {
			next = current->next;
			current->next = prev;
			prev = current;
			current = next;
		}

		f->type = type;
		f->identifier = identifier;
		f->parameters = prev;
		f->body = body;
	}
	return f;
}

void mcc_function_free(function_def_t *f)
{
	decl_t *d;

	while (f->parameters != NULL) {
		d = f->parameters;
		f->parameters = d->next;

		mcc_declaration_free(d);
	}

	mcc_stmt_free(f->body);
	free(f);
}
