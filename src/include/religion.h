#ifndef _RELIGION_H_
#define _RELIGION_H_

/*
 * $Id: religion.h,v 1.1 1998-09-04 05:27:47 fjoe Exp $
 */

char *	religion_name(int religion);

/* To add god, make tattoo in limbo.are, add OBJ_VNUM_TATTOO_(GOD),
 * add here and add to const.c in the religion_table  also increase
 * MAX_RELIGION in merc.h  and make oprog for the tattoo
 */

#define RELIGION_NONE		0
#define RELIGION_APOLLON	1
#define RELIGION_ZEUS		2
#define RELIGION_SIEBELE	3
#define RELIGION_HEPHAESTUS	4
#define RELIGION_EHRUMEN	5
#define RELIGION_AHRUMAZDA	6
#define RELIGION_DEIMOS 	7
#define RELIGION_PHOBOS 	8
#define RELIGION_ODIN		9
#define RELIGION_MARS		10
#define RELIGION_ATHENA 	11
#define RELIGION_GOKTENGRI	12
#define RELIGION_HERA		13
#define RELIGION_VENUS		14
#define RELIGION_ARES		15
#define RELIGION_PROMETHEUS	16
#define RELIGION_EROS		17

/* Religion structure */
struct religion_type
{
	char *	leader;
	char *	name;
	int	vnum;
};

extern	const	struct religion_type	religion_table	[];

#endif
