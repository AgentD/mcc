#include <string.h>
#include <stdlib.h>

#include "bitmap.h"


void mcc_bitmap_init(bitmap_t *bitmap)
{
	memset(bitmap, 0, sizeof(*bitmap));
}

void mcc_bitmap_cleanup(bitmap_t *bitmap)
{
	free(bitmap->pool);
	memset(bitmap, 0, sizeof(*bitmap));
}

int mcc_bitmap_allocate(bitmap_t *bitmap, size_t *out)
{
	size_t i, new_size;
	uint32_t mask;
	void *new;

	for (i = 0; i < bitmap->pool_size; ++i) {
		if (bitmap->pool[i] != 0xFFFFFFFF) {
			*out = i * 32;

			mask = 0x01;
			while (bitmap->pool[i] & mask) {
				*out += 1;
				mask <<= 1;
			}

			bitmap->pool[i] |= mask;
			return 0;
		}
	}

	if (bitmap->pool_size >= bitmap->max_size) {
		new_size = bitmap->max_size ? bitmap->max_size * 2 : 16;
		new = realloc(bitmap->pool, new_size * sizeof(bitmap->pool[0]));
		if (new == NULL)
			return -1;

		bitmap->pool = new;
		bitmap->max_size = new_size;
	}

	bitmap->pool[i] = 0x01;
	bitmap->pool_size += 1;
	*out = i * 32;
	return 0;
}

void mcc_bitmap_free(bitmap_t *bitmap, size_t index)
{
	uint32_t mask = 0x01;
	size_t i = 0;

	while (index >= 32) {
		++i;
		index -= 32;
	}

	while (index != 0) {
		mask <<= 1;
		index -= 1;
	}

	bitmap->pool[i] &= ~mask;
}
