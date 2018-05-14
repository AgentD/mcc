#ifndef DECL_H
#define DECL_H

#include <sys/types.h>

#include "literal.h"

typedef enum {
	/** \brief If set, declare an array */
	DECL_FLAG_ARRAY = 0x01,

	/** \brief If set, declaration is a function paramter */
	DECL_FLAG_PARAM = 0x02,
} E_DECL_FLAG;

typedef struct decl_t {
	E_TYPE type;
	int array_size;
	off_t identifier;

	unsigned int flags;
	unsigned int line_no;

	/**
	 * \brief An opaque user data pointer
	 *
	 * Can be used by code generation to store, for instance, allocation
	 * information for simplified, faster lookup.
	 */
	void *user;

	struct decl_t *next;
} decl_t;

#ifdef __cplusplus
extern "C" {
#endif

decl_t *mcc_declaration(E_TYPE type, int size, off_t identifier,
			unsigned int flags);

void mcc_declaration_free(decl_t *d);

#ifdef __cplusplus
}
#endif

#endif /* DECL_H */

