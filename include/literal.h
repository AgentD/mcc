#ifndef MCC_LITERAL_H
#define MCC_LITERAL_H

#include <stdbool.h>

#ifndef FORCE_INLINE
	#ifdef _MSC_VER
		#define FORCE_INLINE __forceinline
	#else
		#define FORCE_INLINE __inline__ __attribute__((always_inline))
	#endif
#endif

/**
 * \enum E_TYPE
 *
 * \brief Type of a literal (\ref literal_t), return type of a
 *        function (\ref function_def_t) or variable type (\ref decl_t)
 */
typedef enum {
	TYPE_VOID,	/**< \brief Only valid for \ref function_def_t */
	TYPE_BOOL,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING
} E_TYPE;

typedef struct {
	E_TYPE type;
	unsigned int line_no;

	union {
		bool b;
		int i;
		float f;
		off_t str;
	} value;
} literal_t;

static FORCE_INLINE literal_t mcc_literal_bool(bool value)
{
	literal_t l = { .type = TYPE_BOOL, .value = { .b = value } };

	return l;
}

static FORCE_INLINE literal_t mcc_literal_int(int value)
{
	literal_t l = { .type = TYPE_INT, .value = { .i = value } };

	return l;
}

static FORCE_INLINE literal_t mcc_literal_float(float value)
{
	literal_t l = { .type = TYPE_FLOAT, .value = { .f = value } };

	return l;
}

static FORCE_INLINE literal_t mcc_literal_string(off_t value)
{
	literal_t l = { .type = TYPE_STRING, .value = { .str = value } };

	return l;
}

#endif /* MCC_LITERAL_H */

