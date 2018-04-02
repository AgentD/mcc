#include <string.h>
#include <stdlib.h>

#include "str_tab.h"


#define STR_TAB_INCREMENT 4096


void mcc_str_tab_init(str_tab_t *tab)
{
	memset(tab, 0, sizeof(*tab));
}

void mcc_str_tab_cleanup(str_tab_t *tab)
{
	free(tab->pool);
}

off_t mcc_str_tab_add(str_tab_t *tab, const char *str)
{
	size_t i, slen;
	void *new;
	off_t ret;

	for (i = 0; i < tab->pool_size; ) {
		if (!strcmp(tab->pool + i, str))
			return (off_t)i;

		i += strlen(tab->pool + i) + 1;
	}

	slen = strlen(str) + 1;

	if ((tab->pool_size + slen) >= tab->pool_max) {
		new = realloc(tab->pool, tab->pool_max + STR_TAB_INCREMENT);

		if (new == NULL)
			return (off_t)-1;

		tab->pool = new;
		tab->pool_max += STR_TAB_INCREMENT;
	}

	ret = tab->pool_size;

	memcpy(tab->pool + tab->pool_size, str, slen);
	tab->pool_size += slen;
	return ret;
}

const char *mcc_str_tab_resolve(str_tab_t *tab, off_t id)
{
	if (id >= 0 && (size_t)id < tab->pool_size)
		return tab->pool + id;

	return NULL;
}
