#ifndef _NAMEDP_H_
#define _NAMEDP_H_

/*
 * $Id: namedp.h,v 1.1 1998-09-01 18:38:01 fjoe Exp $
 */

/*
 * named pointers
 */

struct namedp {
	char *	name;
	void *	p;
	bool	touched;
};

NAMEDP *	namedp_lookup	(NAMEDP *table, const char *name);
char *		namedp_name	(NAMEDP *table, void *p);

#endif
