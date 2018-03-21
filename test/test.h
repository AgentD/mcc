#ifndef TEST_H
#define TEST_H

#define TEST_ASSERT(x) \
	if (!(x)) {\
		fprintf(stderr, "%s: %d: %s\n", __FILE__, __LINE__, #x);\
		exit(EXIT_FAILURE);\
	}

#endif /* TEST_H */

