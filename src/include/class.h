#ifndef _CLASS_H_
#define _CLASS_H_

/*
 * $Id: class.h,v 1.2 1998-09-04 05:27:45 fjoe Exp $
 */

/*--------------------------------------------------------------------
 * class stuff
 */
struct class_skill {
	int 	sn;		/* skill number */
	int 	level;		/* level needed by class */
	int 	rating;		/* how hard it is to learn */
};

struct class_data {
	char *	name;			/* full name */
	char *	file_name;		/* filename */
	char	who_name[4];		/* three-letter name for 'who' */
	int	attr_prime;		/* primary stat */
	int	weapon;			/* school weapon vnum */
	varr *	guild;			/* guild room list */
	int	skill_adept;		/* max skill % */
	int	thac0_00;		/* thac0 for level 0 */
	int	thac0_32;		/* thac0 for level 32 */
	int	hp_rate;		/* hp rate (when gaining level) */
	int	mana_rate;		/* mana rate (when gaining level */
	sflag_t	flags;			/* class flags */
	int	points;			/* cost in exp */
	int	stats[MAX_STATS];	/* stat modifiers */
	int	align;			/* alignment restrictions */
	varr *	skills;			/* varr of class skills */
	char *	titles[MAX_LEVEL+1][2];	/* titles for each levels and sexes */
};

/* class flags */
#define CLASS_MAGIC	(A)	/* gain additional mana/level */

extern varr * classes;

#define CLASS(i)		((CLASS_DATA*) VARR_GET(classes, i))
#define class_lookup(i)		((CLASS_DATA*) varr_get(classes, i))
#define class_skill_lookup(class, sn) \
	((CLASS_SKILL*) varr_bsearch(class->skills, &sn, cmpint))

char *	class_name(int);
char *	class_who_name(int);

int	cln_lookup(const char *name);
char *	title_lookup(CHAR_DATA *ch);

#endif

