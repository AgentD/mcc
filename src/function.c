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

		current = f->parameters;
		while (current != NULL) {
			current->flags |= DECL_FLAG_PARAM;
			current = current->next;
		}
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

mcc_tac_inst_t *mcc_function_to_tac(function_def_t *fun)
{
	mcc_tac_inst_t *n, *list;
	unsigned int i = 0;
	decl_t *param;

	n = list = mcc_mk_tac_node(TAC_BEGIN_FUNCTION);
	list->arg[0].type = TAC_ARG_NAME;
	list->arg[0].u.name = fun->identifier;

	for (param = fun->parameters; param != NULL; param = param->next) {
		n->next = mcc_mk_tac_node(TAC_FUN_PARAM);
		n = n->next;
		param->user = n;

		n->type = mcc_decl_to_tac_type(param->type);
		if (param->flags & DECL_FLAG_ARRAY)
			n->type.ptr_level += 1;
		n->arg[0].type = TAC_ARG_INDEX;
		n->arg[0].u.index = i++;
	}

	n->next = mcc_stmt_to_tac(fun->body);
	while (n->next != NULL)
		n = n->next;
	n->next = mcc_mk_tac_node(TAC_END_FUNCTION);
	return list;
}
