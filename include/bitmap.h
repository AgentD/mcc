#ifndef MCC_BITMAP_H
#define MCC_BITMAP_H

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint32_t *pool;
	size_t pool_size;
	size_t max_size;
} bitmap_t;

#ifdef __cplusplus
extern "C" {
#endif

void mcc_bitmap_init(bitmap_t *bitmap);

void mcc_bitmap_cleanup(bitmap_t *bitmap);

int mcc_bitmap_allocate(bitmap_t *bitmap, size_t *out);

void mcc_bitmap_free(bitmap_t *bitmap, size_t index);

#ifdef __cplusplus
}
#endif

#endif /* MCC_BITMAP_H */
