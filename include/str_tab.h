#ifndef STR_TAB_H
#define STR_TAB_H

#include <sys/types.h>
#include <stddef.h>

typedef struct {
	char *pool;
	size_t pool_size;
	size_t pool_max;
} str_tab_t;

#ifdef __cplusplus
extern "C" {
#endif

void str_tab_init(str_tab_t *tab);

void str_tab_cleanup(str_tab_t *tab);

off_t str_tab_add(str_tab_t *tab, const char *str);

const char *str_tab_resolve(str_tab_t *tab, off_t id);

#ifdef __cplusplus
}
#endif

#endif /* STR_TAB_H */

