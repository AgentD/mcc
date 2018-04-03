#ifndef TEST_H
#define TEST_H

#include <string.h>

#define TEST_ASSERT(x) \
	if (!(x)) {\
		fprintf(stderr, "%s: %d: %s\n", __FILE__, __LINE__, #x);\
		exit(EXIT_FAILURE);\
	}

#define ASSERT_IDENTIFIER(ident, off, value) {\
		const char *str;\
		str = mcc_str_tab_resolve(ident, off);\
		TEST_ASSERT(str != NULL);\
		TEST_ASSERT(strcmp(str, value) == 0);\
	}

#endif /* TEST_H */

