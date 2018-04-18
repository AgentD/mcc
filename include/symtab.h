#ifndef SYMTAB_H
#define SYMTAB_H

#include "decl.h"
#include "literal.h"

#include <stdlib.h>

typedef enum {
	SYM_TYPE_ARG = 0,
	SYM_TYPE_VAR,
	SYM_TYPE_FUN,
} E_SYM_TYPE;

typedef struct symbol_t {
	E_SYM_TYPE type;

	union {
		decl_t *decl;
		function_def_t *fun;
	} u;

	struct symbol_t *next;
} symbol_t;

static FORCE_INLINE symbol_t *mcc_mksymbol(E_SYM_TYPE type, decl_t *decl)
{
	symbol_t *sym = calloc(1, sizeof(*sym));

	if (sym != NULL) {
		sym->type = type;
		sym->u.decl = decl;
	}

	return sym;
}

static FORCE_INLINE symbol_t *mcc_mkfunsymbol(function_def_t *fun)
{
	symbol_t *sym = calloc(1, sizeof(*sym));

	if (sym != NULL) {
		sym->type = SYM_TYPE_FUN;
		sym->u.fun = fun;
	}

	return sym;
}

static FORCE_INLINE mcc_free_symbol(symbol_t *sym)
{
	free(sym);
}

static FORCE_INLINE symbol_t *mcc_symtab_lookup(symbol_t *head, off_t identifier)
{
	while (head != NULL && head->decl->identifier != identifier)
		head = head->next;

	return head;
}

static FORCE_INLINE symbol_t *mcc_symtab_prepend(symbol_t *head, symbol_t *n)
{
	n->next = head;
	return n;
}

static FORCE_INLINE symbol_t *mcc_symtab_drop(symbol_t *head)
{
	symbol_t *n = head;

	if (n != NULL) {
		head = n->next;
		mcc_free_symbol(n);
	}

	return head;
}

#endif
