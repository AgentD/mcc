#ifndef SYMTAB_H
#define SYMTAB_H

#include "decl.h"
#include "literal.h"

#include <stdlib.h>

typedef struct symbol_t {
	decl_t *decl;

	/** \brief Compound statement that this was declared in */
	statement_t *parent;

	struct symbol_t *next;
} symbol_t;

static FORCE_INLINE symbol_t *mcc_mksymbol(decl_t *decl, statement_t *parent)
{
	symbol_t *sym = calloc(1, sizeof(*sym));

	if (sym != NULL) {
		sym->decl = decl;
		sym->parent = parent;
	}

	return sym;
}

static FORCE_INLINE void mcc_free_symbol(symbol_t *sym)
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
