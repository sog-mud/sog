/*
 * $Id: namedp.c,v 1.1 1998-09-01 18:38:01 fjoe Exp $
 */

#include <stdio.h>

#include "typedef.h"
#include "const.h"
#include "namedp.h"
#include "util.h"

NAMEDP *namedp_lookup(NAMEDP *table, const char *name)
{
	for (; table->name; table++)
		if (!str_cmp(table->name, name))
			return table;
	return NULL;
}

char *namedp_name(NAMEDP *table, void *p)
{
	for (; table->name; table++)
		if (table->p == p)
			return table->name;
	return NULL;
}
