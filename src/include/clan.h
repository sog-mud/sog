#ifndef _CLAN_H_
#define _CLAN_H_

/*
 * $Id: clan.h,v 1.1 1998-09-01 18:37:57 fjoe Exp $
 */

/*----------------------------------------------------------------------
 * clan stuff (clan.c)
 */

#define CLAN_NONE 	 0
#define CLAN_RULER	 1
#define CLAN_INVADER	 2
#define CLAN_CHAOS	 3
#define CLAN_SHALAFI	 4
#define CLAN_BATTLE	 5
#define CLAN_KNIGHT	 6
#define CLAN_LIONS	 7
#define CLAN_HUNTER	 8
#define CLAN_CONFEDERACY 9

/* Clan status */
#define CLAN_LEADER	2
#define CLAN_SECOND	1
#define CLAN_COMMON	0

/*
 * Clan structure
 */
struct clan_data
{
	char *		name;		/* clan name */
	char *		file_name;	/* file name */

	int	 	recall_vnum;	/* recall room vnum */
	mlstring *	msg_prays;	/* what to print in clanrecall */
	mlstring *	msg_vanishes;

	varr *		skills;		/* clan skills */

	flag_t		flags;		/* clan flags */

	int	 	obj_vnum;	/* unused (for a while) */
	OBJ_DATA *	obj_ptr;
};

/* clan flags */
#define CLAN_HIDDEN	(A)		/* clan will not appear in who */
#define CLAN_CHANGED	(Z)

int		cn_lookup	(const char* name);
const char*	clan_name	(int cn);

extern varr *	clans;

#define CLAN(cn)		((CLAN_DATA*) VARR_GET(clans, cn))
#define clan_lookup(cn)		((CLAN_DATA*) varr_get(clans, cn))

struct clan_skill {
	int	sn;
	int	level;
};

#define clan_skill_lookup(clan, sn) \
	((CLAN_SKILL*) varr_bsearch(clan->skills, &sn, cmpint))

#endif
