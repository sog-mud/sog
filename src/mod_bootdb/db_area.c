/*
 * $Id: db_area.c,v 1.124 2001-08-21 11:35:14 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <stdio.h>
#include <string.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_area);
DECLARE_DBLOAD_FUN(load_areadata);
DECLARE_DBLOAD_FUN(load_helps);
DECLARE_DBLOAD_FUN(load_mobiles);
DECLARE_DBLOAD_FUN(load_objects);
DECLARE_DBLOAD_FUN(load_resets);
DECLARE_DBLOAD_FUN(load_rooms);
DECLARE_DBLOAD_FUN(load_shops);
DECLARE_DBLOAD_FUN(load_olimits);
DECLARE_DBLOAD_FUN(load_specials);
DECLARE_DBLOAD_FUN(load_practicers);
DECLARE_DBLOAD_FUN(load_resetmsg);
DECLARE_DBLOAD_FUN(load_aflag);

DECLARE_DBINIT_FUN(init_area);

DBFUN dbfun_areas[] = {
	{ "AREA",		load_area,	NULL	},	// notrans
	{ "AREADATA",		load_areadata,	NULL	},	// notrans
	{ "HELPS",		load_helps,	NULL	},	// notrans
	{ "MOBILES",		load_mobiles,	NULL	},	// notrans
	{ "OBJECTS",		load_objects,	NULL	},	// notrans
	{ "RESETS",		load_resets,	NULL	},	// notrans
	{ "ROOMS",		load_rooms,	NULL	},	// notrans
	{ "SHOPS",		load_shops,	NULL	},	// notrans
	{ "OLIMITS",		load_olimits,	NULL	},	// notrans
	{ "SPECIALS",		load_specials,	NULL	},	// notrans
	{ "PRACTICERS",		load_practicers,NULL	},	// notrans
	{ "RESETMESSAGE",	load_resetmsg,	NULL	},	// notrans
	{ "FLAG",		load_aflag,	NULL	},	// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_areas = { dbfun_areas, init_area, 0 };

AREA_DATA *		area_current;

static void vnum_check(AREA_DATA *area, int vnum);

#define ff	((int64_t) 1 << 31)

#define gg	((int64_t) 1 << 32)
#define hh	((int64_t) 1 << 33)
#define ii	((int64_t) 1 << 34)
#define jj	((int64_t) 1 << 35)
#define kk	((int64_t) 1 << 36)
#define ll	((int64_t) 1 << 37)
#define mm	((int64_t) 1 << 38)
#define nn	((int64_t) 1 << 39)

#define oo	((int64_t) 1 << 40)
#define pp	((int64_t) 1 << 41)
#define qq	((int64_t) 1 << 42)
#define rr	((int64_t) 1 << 43)
#define ss	((int64_t) 1 << 44)
#define tt	((int64_t) 1 << 45)
#define uu	((int64_t) 1 << 46)
#define vv	((int64_t) 1 << 47)

#define ww	((int64_t) 1 << 48)
#define xx	((int64_t) 1 << 49)
#define yy	((int64_t) 1 << 50)
#define zz	((int64_t) 1 << 51)

typedef struct flag_subst_t {
	int64_t from;
	flag_t to;
} flag_subst_t;

static flag_t
subst_flag(flag_subst_t *fs, int64_t from)
{
	flag_t rv = 0;

	while (fs->from) {
		if (IS_SET(from, fs->from))
			rv |= fs->to;
		fs++;
	}

	return rv;
}

static void
init_resistances(int16_t *resists)
{
	int i;

	for (i = 0; i < MAX_RESIST; i++)
		resists[i] = RES_UNDEF;
}

#define V0_RES_SUMMON		(A)
#define V0_RES_CHARM		(B)
#define V0_RES_MAGIC		(C)
#define V0_RES_WEAPON		(D)
#define V0_RES_BASH		(E)
#define V0_RES_PIERCE		(F)
#define V0_RES_SLASH		(G)
#define V0_RES_FIRE		(H)
#define V0_RES_COLD		(I)
#define V0_RES_LIGHTNING	(J)
#define V0_RES_ACID		(K)
#define V0_RES_POISON		(L)
#define V0_RES_NEGATIVE		(M)
#define V0_RES_HOLY		(N)
#define V0_RES_ENERGY		(O)
#define V0_RES_MENTAL		(P)
#define V0_RES_DISEASE		(Q)
#define V0_RES_DROWNING		(R)
#define V0_RES_LIGHT		(S)
#define V0_RES_SOUND		(T)
#define V0_RES_STEAL		(U)
#define V0_RES_WOOD		(X)
#define V0_RES_SILVER		(Y)
#define V0_RES_IRON		(Z)

#define RES(imm, res, vul, f)						\
	(IS_SET((imm), (f)) && !IS_SET((vul), (f)) ? 100 :		\
	 IS_SET((imm), (f)) && IS_SET((vul), (f)) ? 33 :		\
	 IS_SET((res), (f)) && IS_SET((vul), (f)) ? RES_UNDEF :		\
	 IS_SET((vul), (f)) ? -50 :					\
	 IS_SET((res), (f)) ? 33 : RES_UNDEF)

static void
set_percent_resistances(flag_t imm, flag_t res, flag_t vul, int16_t *resist)
{
	/*
	 * weapon resistances
	 */
	resist[DAM_BASH]	= RES(imm, res, vul, V0_RES_BASH);
	resist[DAM_SLASH]	= RES(imm, res, vul, V0_RES_SLASH);
	resist[DAM_PIERCE]	= RES(imm, res, vul, V0_RES_PIERCE);

	/*
	 * magic resistances
	 */
	resist[DAM_FIRE]	= RES(imm, res, vul, V0_RES_FIRE);
	resist[DAM_COLD]	= RES(imm, res, vul, V0_RES_COLD);
	resist[DAM_LIGHTNING]	= RES(imm, res, vul, V0_RES_LIGHTNING);
	resist[DAM_ACID]	= RES(imm, res, vul, V0_RES_ACID);
	resist[DAM_POISON]	= RES(imm, res, vul, V0_RES_POISON);
	resist[DAM_NEGATIVE]	= RES(imm, res, vul, V0_RES_NEGATIVE);
	resist[DAM_HOLY]	= RES(imm, res, vul, V0_RES_HOLY);
	resist[DAM_ENERGY]	= RES(imm, res, vul, V0_RES_ENERGY);
	resist[DAM_MENTAL]	= RES(imm, res, vul, V0_RES_MENTAL);
	resist[DAM_DISEASE]	= RES(imm, res, vul, V0_RES_DISEASE);
	resist[DAM_LIGHT]	= RES(imm, res, vul, V0_RES_LIGHT);
	resist[DAM_CHARM]	= RES(imm, res, vul, V0_RES_CHARM);
	resist[DAM_SOUND]	= RES(imm, res, vul, V0_RES_SOUND);
	resist[DAM_HARM]	= RES_UNDEF;
	resist[DAM_OTHER]	= RES_UNDEF;

	/*
	 * other resistances
	 */
	resist[DAM_WOOD]	= RES(imm, res, vul, V0_RES_WOOD);
	resist[DAM_SILVER]	= RES(imm, res, vul, V0_RES_SILVER);
	resist[DAM_IRON]	= RES(imm, res, vul, V0_RES_IRON);

	/*
	 * defaults
	 */
	resist[DAM_WEAPON]	= RES(imm, res, vul, V0_RES_WEAPON);
	resist[DAM_MAGIC]	= RES(imm, res, vul, V0_RES_MAGIC);
}

int damtbl[] = {
	DAM_BASH, DAM_PIERCE, DAM_SLASH, DAM_FIRE, DAM_COLD, DAM_LIGHTNING,
	DAM_ACID, DAM_HOLY, DAM_NEGATIVE, DAM_ENERGY, DAM_MENTAL, DAM_SOUND,
	DAM_DISEASE, DAM_POISON, DAM_CHARM, DAM_HARM, DAM_LIGHT
};
#define DAMTBL_SZ	(sizeof(damtbl) / sizeof(*damtbl))

DBINIT_FUN(init_area)
{
	if (DBDATA_VALID(dbdata)) {
		if (area_current != NULL) {
			if (IS_SET(area_current->area_flags, AREA_TAGGED)) {
				log(LOG_INFO, "UNTAGGED: %s", area_current->file_name);
				REMOVE_BIT(area_current->area_flags,
					   AREA_TAGGED);
				TOUCH_AREA(area_current);
			}
			area_current = NULL;
		}
	}
}

/*
 * Snarf an 'area' header line.
 */
DBLOAD_FUN(load_area)
{
	AREA_DATA *pArea;

	pArea = new_area();
	free_string(fread_string(fp));		/* file name */
	free_string(pArea->file_name);
	pArea->file_name	= get_filename(bootdb_filename);

	pArea->security		= 9;

	free_string(pArea->name);
	pArea->name		= fread_string(fp);
	fread_letter(fp);			/* '{' */
	pArea->min_level	= fread_number(fp);
	pArea->max_level	= fread_number(fp);
	fread_letter(fp);			/* '}' */
	pArea->credits		= fread_sword(fp);
	free_string(fread_string(fp));
	pArea->min_vnum		= fread_number(fp);
	pArea->max_vnum		= fread_number(fp);
	pArea->age		= 15;
	pArea->empty		= FALSE;

	if (area_first == NULL)
		area_first = pArea;
	if (area_last != NULL)
		area_last->next = pArea;

	area_last	= pArea;
	pArea->next	= NULL;
	area_current	= pArea;
}

/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
DBLOAD_FUN(load_areadata)
{
	AREA_DATA *	pArea;

	pArea			= new_area();
	pArea->age		= 15;
	pArea->empty		= FALSE;
	free_string(pArea->file_name);
	pArea->file_name	= get_filename(bootdb_filename);
	pArea->security		= 9;                    /* 9 -- Hugin */

	for (; ;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'B':
			SKEY("Builders", pArea->builders, fread_string(fp));
			break;
		case 'C':
			KEY("Clan", pArea->clan,
			    fread_strkey(fp, &clans, "load_areadata")); // notrans
			SKEY("Credits", pArea->credits, fread_string(fp));
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				fMatch = TRUE;
				if (area_first == NULL)
					area_first = pArea;
				if (area_last  != NULL)
					area_last->next = pArea;
				area_last	= pArea;
				pArea->next	= NULL;
				area_current	= pArea;
				return;
			}
			break;
		case 'F':
			KEY("Flags", pArea->area_flags,
			    (pArea->ver < 4 ? fread_fstring(area_flags, fp) :
					      fread_flags(fp)));
			break;
		case 'L':
			if (IS_TOKEN(fp, "LevelRange")) {
				pArea->min_level = fread_number(fp);
				pArea->max_level = fread_number(fp);
				fMatch = TRUE;
			}
			break;
		case 'N':
			SKEY("Name", pArea->name, fread_string(fp));
			break;
		case 'R':
			MLSKEY("ResetMessage", pArea->resetmsg);
			break;
		case 'S':
			KEY("Security", pArea->security, fread_number(fp));
			break;
		case 'V':
			KEY("Ver", pArea->ver, fread_number(fp));
			if (IS_TOKEN(fp, "VNUMs")) {
				pArea->min_vnum = fread_number(fp);
				pArea->max_vnum = fread_number(fp);
				fMatch = TRUE;
			}
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_areadata: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

static
MLSTR_FOREACH_FUN(cb_strip_nl, lang, p, ap)
{
	char buf[MAX_STRING_LENGTH];
	size_t len, oldlen;

	AREA_DATA *pArea = va_arg(ap, AREA_DATA *);

	if (*p == NULL
	||  (len = strlen(*p)) == 0)
		return NULL;

	oldlen = len;
	while ((*p)[len-1] == '\n') {
		if (len < 2
		||  (*p)[len-2] != '\n')
			break;
		len--;
	}

	if (oldlen != len) {
		strnzncpy(buf, sizeof(buf), *p, len);
		free_string(*p);
		*p = str_dup(buf);
		TOUCH_AREA(pArea);
	}

	return NULL;
}

/*
 * Snarf a help section.
 */
DBLOAD_FUN(load_helps)
{
	HELP_DATA *pHelp;
	int level;
	const char *keyword;

	if (!area_current) {  /* OLC */
		log(LOG_ERROR, "load_helps: no #AREA seen yet.");
		return;
	}

	for (; ;) {
		level		= fread_fword(level_table, fp);
		keyword		= fread_string(fp);

		if (keyword[0] == '$')
			break;

		pHelp		= help_new();
		pHelp->level	= level;
		pHelp->keyword	= keyword;
		mlstr_fread(fp, &pHelp->text);

		mlstr_foreach(&pHelp->text, cb_strip_nl, area_current);
		help_add(area_current, pHelp);
	}
}

/*
 * Snarf a reset section.
 */
DBLOAD_FUN(load_resets)
{
	ROOM_INDEX_DATA *pLastRoom = NULL;

	if (area_current == NULL) {
		log(LOG_ERROR, "load_resets: no #AREA seen yet.");
		return;
	}

	for (; ;) {
		RESET_DATA *pReset;

		ROOM_INDEX_DATA *pRoom;
		char letter;

		if ((letter = fread_letter(fp)) == 'S')
			break;

		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}

		pReset		= reset_new();
		pReset->command	= letter;
		pReset->arg0	= fread_number(fp);
		pReset->arg1	= fread_number(fp);
		pReset->arg2	= fread_number(fp);
		pReset->arg3	= (letter == 'G' || letter == 'R') ?
				  0 : fread_number(fp);
		pReset->arg4	= (letter == 'P' || letter == 'M') ?
				  fread_number(fp) : 0;
		fread_to_eol(fp);

		/*
		 * Validate parameters.
		 */
		switch (letter) {
		default:
			log(LOG_ERROR, "load_resets: bad command '%c'.", letter);
		case 'D':
			reset_free(pReset);
			break;

		case 'M':
		case 'O':
			if ((pRoom = get_room_index(pReset->arg3)) == NULL) {
				log(LOG_ERROR, "load_reset: %d: no such room",
				    pReset->arg1);
				break;
			}

			pLastRoom = pRoom;
			reset_add(pRoom, pReset, NULL);
			break;

		case 'P':
		case 'G':
		case 'E':
			if (!pLastRoom) {
				log(LOG_ERROR, "load_resets: room undefined");
				break;
			}

			reset_add(pLastRoom, pReset, NULL);
			break;

		case 'R':
			if ((pRoom = get_room_index(pReset->arg1)) == NULL) {
				log(LOG_ERROR, "load_reset: %d: no such room",
				    pReset->arg1);
				break;
			}

			if (pReset->arg2 < 0 || pReset->arg2 > MAX_DIR) {
				log(LOG_ERROR, "load_resets: 'R': bad exit %d.",
				    pReset->arg2);
				break;
			}

			reset_add(pRoom, pReset, NULL);
			break;
		}
	}
}

/*
 * Snarf a room section.
 */
DBLOAD_FUN(load_rooms)
{
	ROOM_INDEX_DATA *pRoomIndex;

	if (area_current == NULL) {
		log(LOG_ERROR, "load_rooms: no #AREA seen yet.");
		return;
	}

	for (; ;) {
		int vnum;
		char letter;
		int door;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#') {
			log(LOG_ERROR, "load_rooms: # not found.");
			return;
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		if (get_room_index(vnum)) {
			log(LOG_ERROR, "load_rooms: vnum %d duplicated.", vnum);
			return;
		}

		pRoomIndex		= new_room_index();

		pRoomIndex->area	= area_current;
		pRoomIndex->vnum	= vnum;
		mlstr_fread(fp, &pRoomIndex->name);
		mlstr_fread(fp, &pRoomIndex->description);
		/* Area number */	  fread_number(fp);
		pRoomIndex->room_flags	= fread_flags(fp);
		pRoomIndex->sector_type	= fread_fword(sector_types, fp);
		if (pRoomIndex->sector_type < 0
		||  pRoomIndex->sector_type >= MAX_SECT) {
			log(LOG_ERROR, "load_rooms: vnum %d: invalid sector type",
			    pRoomIndex->vnum);
		}

		for (; ;) {
			letter = fread_letter(fp);

			if (letter == 'S')
				break;

			if (letter == 'H') /* healing room */
				pRoomIndex->heal_rate = fread_number(fp);

			else if (letter == 'M') /* mana room */
				pRoomIndex->mana_rate = fread_number(fp);

			else if (letter == 'D') {
				EXIT_DATA *pexit;
				int locks;
				char c;

				door = fread_number(fp);
				if (door < 0 || door > 5) {
					log(LOG_ERROR, "load_rooms: vnum %d has bad door number.", vnum);
					return;
				}

				pexit			= new_exit();
                                c = xgetc(fp);
				if (c == ' ')
					pexit->size = fread_fword(size_table, fp);
				mlstr_fread(fp, &pexit->description);
				if (mlstr_addnl(&pexit->description))
					TOUCH_AREA(area_current);

				pexit->keyword		= fread_string(fp);
				if (area_current->ver < 2) {
					char buf[MAX_STRING_LENGTH];
					char buf2[MAX_STRING_LENGTH];

					one_argument(pexit->keyword,
						     buf, sizeof(buf));
					if (buf[0] != '\0') {
						snprintf(buf2, sizeof(buf2),
							 "the %s", // notrans
							 buf);
						mlstr_init2(&pexit->short_descr.ml, buf2);
					}
				} else {
					mlstr_fread(fp, &pexit->short_descr.ml);
					mlstr_fread(fp, &pexit->short_descr.gender);
				}
				pexit->exit_info	= 0;
				pexit->rs_flags		= 0;	/* OLC */
				locks			= fread_flags(fp);
				pexit->key		= fread_number(fp);
				pexit->to_room.vnum	= fread_number(fp);
				pexit->orig_door	= door;	/* OLC */

				if (!IS_SET(locks, EX_BITVAL)) {
					switch (locks) {
					case 1:
						pexit->exit_info = EX_ISDOOR;
						pexit->rs_flags  = EX_ISDOOR;
						break;
					case 2:
						pexit->exit_info = EX_ISDOOR |
								   EX_PICKPROOF;
						pexit->rs_flags  = EX_ISDOOR |
								   EX_PICKPROOF;
						break;
					case 3:
						pexit->exit_info = EX_ISDOOR |
								   EX_NOPASS;
						pexit->rs_flags  = EX_ISDOOR |
								   EX_NOPASS;
						break;
					case 4:
						pexit->exit_info = EX_ISDOOR |
								   EX_NOPASS |
								   EX_PICKPROOF;
						pexit->rs_flags  = EX_ISDOOR |
								   EX_NOPASS |
								   EX_PICKPROOF;
						break;
					}
				}
				else {
					pexit->exit_info = locks;
					pexit->rs_flags = locks;
				}

				pRoomIndex->exit[door] = pexit;
			} else if (letter == 'E')
				ed_fread(fp, &pRoomIndex->ed);
			else if (letter == 'O') {
				log(LOG_ERROR, "load rooms: owner present");
				return;

			} else {
				log(LOG_ERROR, "load_rooms: vnum %d has flag '%c' (not 'DES').", vnum, letter);
				return;
			}
		}

		iHash			= vnum % MAX_KEY_HASH;
		pRoomIndex->next	= room_index_hash[iHash];
		room_index_hash[iHash]	= pRoomIndex;
		vnum_check(area_current, vnum);			/* OLC */
		if (vnum > top_vnum_room)
			top_vnum_room = vnum;
	}
}

/*
 * Snarf a shop section.
 */
DBLOAD_FUN(load_shops)
{
	SHOP_DATA *pShop;

	for (; ;) {
		MOB_INDEX_DATA *pMobIndex;
		int iTrade;

		pShop			= new_shop();
		pShop->keeper		= fread_number(fp);
		if (pShop->keeper == 0)
			break;
		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
			pShop->buy_type[iTrade]	= fread_number(fp);
		pShop->profit_buy	= fread_number(fp);
		pShop->profit_sell	= fread_number(fp);
		pShop->open_hour	= fread_number(fp);
		pShop->close_hour	= fread_number(fp);
					  fread_to_eol(fp);
		pMobIndex		= get_mob_index(pShop->keeper);
		if (pMobIndex == NULL) {
			log(LOG_ERROR, "load_shops: %d: no such mob", pShop->keeper);
			continue;
		}
		pMobIndex->pShop	= pShop;

		if (shop_first == NULL)
			shop_first = pShop;
		if (shop_last  != NULL)
			shop_last->next = pShop;

		shop_last	= pShop;
		pShop->next	= NULL;
	}
}

/*
 * Snarf spec proc declarations.
 */
DBLOAD_FUN(load_specials)
{
	for (; ;) {
		MOB_INDEX_DATA *pMobIndex;
		char letter;
		int vnum;
		const char *spec;

		switch (letter = fread_letter(fp)) {
		default:
			log(LOG_ERROR, "load_specials: letter '%c' not *MS.", letter);
			return;

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			vnum = fread_number(fp);
			if ((pMobIndex = get_mob_index(vnum)) == NULL) {
				log(LOG_ERROR, "load_specials: %d: no such mob", vnum);
				break;
			}

			fread_word(fp);
			spec = rfile_tok(fp);
			pMobIndex->spec_fun = mob_spec_lookup(spec);
			if (pMobIndex->spec_fun == NULL) {
				log(LOG_ERROR, "load_specials: %s: no such spec", spec);
				break;
			}
			break;
		}

		fread_to_eol(fp);
	}
}

/*
 * Snarf can prac declarations.
 */
DBLOAD_FUN(load_practicers)
{
	for (; ;) {
		MOB_INDEX_DATA *pMobIndex;
		char letter;
		int vnum;

		switch (letter = fread_letter(fp)) {
		default:
			log(LOG_ERROR, "load_practicers: letter '%c' not *MS.",
				 letter);
			return;

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			vnum = fread_number(fp);
			if ((pMobIndex = get_mob_index(vnum)) == NULL) {
				log(LOG_ERROR, "load_practicer: %d: no such mob", vnum);
				break;
			}
			SET_BIT(pMobIndex->practicer,
				area_current->ver < 4 ?
					fread_fstring(skill_groups, fp) :
					fread_flags(fp));
			break;
		}

		fread_to_eol(fp);
	}
}

DBLOAD_FUN(load_olimits)
{
	char ch;

	for (ch = fread_letter(fp); ch != 'S'; ch = fread_letter(fp)) {
		int vnum;
		OBJ_INDEX_DATA *pObjIndex;

		switch(ch) {
		case 'O':
			vnum = fread_number(fp);
			if ((pObjIndex = get_obj_index(vnum)) == NULL) {
				log(LOG_ERROR, "load_olimits: %d: no such obj", vnum);
				break;
			}
			pObjIndex->limit = fread_number(fp);
			break;

		case '*':
			break;

		default:
			log(LOG_ERROR, "load_olimits: bad command '%c'", ch);
			break;;
		}

		fread_to_eol(fp);
	}
}

DBLOAD_FUN(load_resetmsg)
{
	mlstr_fread(fp, &area_current->resetmsg);
}

DBLOAD_FUN(load_aflag)
{
	area_current->area_flags = fread_flags(fp);
}

#define V0_ACT_SUMMONED		(gg)
#define V0_ACT_FAMILIAR		(jj)
#define V0_ACT_IMMSTEAL		(kk)
#define V0_ACT_IMMSUMMON	(ll)

#define V4_ACT_UNDEAD		(O)

flag_subst_t v0_subst_act[] =
{
	{ ACT_SENTINEL,		ACT_SENTINEL		},
	{ ACT_SCAVENGER,	ACT_SCAVENGER		},
	{ ACT_AGGRESSIVE,	ACT_AGGRESSIVE		},
	{ ACT_STAY_AREA,	ACT_STAY_AREA		},
	{ ACT_WIMPY,		ACT_WIMPY		},
	{ ACT_PET,		ACT_PET			},
	{ ACT_HUNTER,		ACT_HUNTER		},
	{ V4_ACT_UNDEAD,	V4_ACT_UNDEAD		},
	{ ACT_CLERIC,		ACT_CLERIC		},
	{ ACT_MAGE,		ACT_MAGE		},
	{ ACT_THIEF,		ACT_THIEF		},
	{ ACT_WARRIOR,		ACT_WARRIOR		},
	{ ACT_NOALIGN,		ACT_NOALIGN		},
	{ ACT_NOPURGE,		ACT_NOPURGE		},
	{ ACT_OUTDOORS,		ACT_OUTDOORS		},
	{ ACT_INDOORS,		ACT_INDOORS		},
	{ ACT_RIDEABLE,		ACT_RIDEABLE		},
	{ ACT_UPDATE_ALWAYS,	ACT_UPDATE_ALWAYS	},
	{ ACT_NOTRACK,		ACT_NOTRACK		},
	{ V0_ACT_SUMMONED,	ACT_SUMMONED		},
	{ V0_ACT_FAMILIAR,	ACT_FAMILIAR		},
	{ V0_ACT_IMMSTEAL,	ACT_IMMSTEAL		},
	{ V0_ACT_IMMSUMMON,	ACT_IMMSUMMON		},
	{ 0, 0 }
};

#define V0_ACT_CHANGER		(dd)
#define V0_ACT_GAIN		(bb)
#define V0_ACT_TRAIN		(J)
#define V0_ACT_PRACTICE		(K)
#define V0_ACT_QUESTOR		(X)
#define V0_ACT_REPAIRMAN	(ii)
#define V0_ACT_SAGE		(hh)
#define V0_ACT_HEALER		(aa)
#define V0_ACT_CLAN_GUARD	(ff)

flag_subst_t v0_subst_mob[] =
{
	{ V0_ACT_CHANGER,	MOB_CHANGER			},
	{ V0_ACT_GAIN,		MOB_GAIN			},
	{ V0_ACT_TRAIN,		MOB_TRAIN			},
	{ V0_ACT_PRACTICE,	MOB_PRACTICE			},
	{ V0_ACT_QUESTOR,	MOB_QUESTOR			},
	{ V0_ACT_REPAIRMAN,	MOB_REPAIRMAN			},
	{ V0_ACT_SAGE,		MOB_SAGE			},
	{ V0_ACT_HEALER,	MOB_HEALER			},
	{ V0_ACT_CLAN_GUARD,	MOB_CLAN_GUARD			},
	{ 0, 0 }
};

#define V0_AFF_SCREAM			(hh)
#define V0_AFF_BLOODTHIRST		(ii)
#define V0_AFF_STUN			(jj)
#define V0_AFF_WEAK_STUN		(kk)
#define V0_AFF_DETECT_FEAR		(oo)
#define V0_AFF_DETECT_WEB		(rr)
#define V0_AFF_BLACK_SHROUD		(uu)
#define V0_AFF_QUESTTARGET		(vv)
#define V0_AFF_TURNED			(yy)

flag_subst_t v0_subst_aff[] =
{
	{ AFF_BLIND,		AFF_BLIND		},
	{ AFF_SANCTUARY,	AFF_SANCTUARY		},
	{ AFF_FAERIE_FIRE ,	AFF_FAERIE_FIRE		},
	{ AFF_CURSE,		AFF_CURSE		},
	{ AFF_CORRUPTION,	AFF_CORRUPTION		},
	{ AFF_POISON,		AFF_POISON		},
	{ AFF_PROTECT_EVIL,	AFF_PROTECT_EVIL	},
	{ AFF_PROTECT_GOOD,	AFF_PROTECT_GOOD	},
	{ AFF_SLEEP,		AFF_SLEEP		},
	{ AFF_CHARM,		AFF_CHARM		},
	{ AFF_FLYING,		AFF_FLYING		},
	{ AFF_PASS_DOOR,	AFF_PASS_DOOR		},
	{ AFF_HASTE,		AFF_HASTE		},
	{ AFF_CALM,		AFF_CALM		},
	{ AFF_PLAGUE,		AFF_PLAGUE		},
	{ AFF_WEAKEN,		AFF_WEAKEN		},
	{ AFF_BERSERK,		AFF_BERSERK		},
	{ AFF_REGENERATION,	AFF_REGENERATION	},
	{ AFF_SLOW,		AFF_SLOW		},
	{ V0_AFF_SCREAM,	AFF_SCREAM		},
	{ V0_AFF_BLOODTHIRST,	AFF_BLOODTHIRST		},
	{ V0_AFF_STUN,		AFF_STUN		},
	{ V0_AFF_WEAK_STUN,	AFF_WEAK_STUN		},
	{ V0_AFF_DETECT_FEAR,	AFF_FEAR		},
	{ V0_AFF_DETECT_WEB,	AFF_WEB			},
	{ V0_AFF_BLACK_SHROUD,	AFF_BLACK_SHROUD	},
	{ V0_AFF_QUESTTARGET,	AFF_QUESTTARGET		},
	{ V0_AFF_TURNED,	AFF_TURNED		},

	{ 0, 0 }
};

#define V0_AFF_INVIS			(B)
#define V0_AFF_SNEAK			(P)
#define V0_AFF_HIDE			(Q)
#define V0_AFF_CAMOUFLAGE		(ee)
#define V0_AFF_IMP_INVIS		(ff)
#define V0_AFF_FADE			(gg)
#define V0_AFF_BLEND			(ww)

flag_subst_t v0_subst_invis[] =
{
	{ V0_AFF_INVIS,		ID_INVIS	},
	{ V0_AFF_SNEAK,		ID_SNEAK	},
	{ V0_AFF_HIDE,		ID_HIDDEN	},
	{ V0_AFF_CAMOUFLAGE,	ID_CAMOUFLAGE	},
	{ V0_AFF_IMP_INVIS,	ID_IMP_INVIS	},
	{ V0_AFF_FADE,		ID_FADE		},
	{ V0_AFF_BLEND,		ID_BLEND	},
	{ 0, 0 }
};

#define V0_AFF_DETECT_EVIL		(C)
#define V0_AFF_DETECT_INVIS		(D)
#define V0_AFF_DETECT_MAGIC		(E)
#define V0_AFF_DETECT_HIDDEN		(F)
#define V0_AFF_DETECT_GOOD		(G)
#define V0_AFF_INFRARED			(J)
#define V0_AFF_DETECT_IMP_INVIS		(ll)
#define V0_AFF_DETECT_FADE		(mm)
#define V0_AFF_DETECT_UNDEAD		(nn)
#define V0_AFF_DETECT_LIFE		(ss)
#define V0_AFF_ACUTE_VISION		(tt)
#define V0_AFF_AWARENESS		(xx)

flag_subst_t v0_subst_detect[] =
{
	{ V0_AFF_DETECT_EVIL,		ID_EVIL		},
	{ V0_AFF_DETECT_INVIS,		ID_INVIS	},
	{ V0_AFF_DETECT_MAGIC,		ID_MAGIC	},
	{ V0_AFF_DETECT_HIDDEN,		ID_HIDDEN	},
	{ V0_AFF_DETECT_GOOD,		ID_GOOD		},
	{ V0_AFF_INFRARED,		ID_INFRARED	},
	{ V0_AFF_DETECT_IMP_INVIS,	ID_IMP_INVIS	},
	{ V0_AFF_DETECT_FADE,		ID_FADE		},
	{ V0_AFF_DETECT_UNDEAD,		ID_UNDEAD	},
	{ V0_AFF_DETECT_LIFE,		ID_LIFE		},
	{ V0_AFF_ACUTE_VISION,		ID_CAMOUFLAGE	},
	{ V0_AFF_AWARENESS,		ID_BLEND	},
	{ 0, 0 }
};

/*
 * Snarf a mob section.  new style
 */
DBLOAD_FUN(load_mobiles)
{
	MOB_INDEX_DATA *pMobIndex;

	if (!area_current) {
		log(LOG_ERROR, "load_mobiles: no #AREA seen yet.");
		return;
	}

	for (; ;) {
		race_t *r;
		int vnum;
		char letter;
		int iHash;
		flag_t imm = 0;
		flag_t res = 0;
		flag_t vul = 0;

		letter = fread_letter(fp);
		if (letter != '#') {
			log(LOG_ERROR, "load_mobiles: # not found.");
			return;
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		if (get_mob_index(vnum)) {
			log(LOG_ERROR, "load_mobiles: vnum %d duplicated.",
			    vnum);
			return;
		}

		pMobIndex                       = new_mob_index();

		pMobIndex->vnum                 = vnum;
		pMobIndex->name			= fread_string(fp);
		mlstr_fread(fp, &pMobIndex->short_descr);
		mlstr_fread(fp, &pMobIndex->long_descr);
		mlstr_stripnl(&pMobIndex->long_descr);
		mlstr_fread(fp, &pMobIndex->description);
		free_string(pMobIndex->race);
		pMobIndex->race			= fread_string(fp);
		STRKEY_CHECK(&races, pMobIndex->race, "load_mob"); // notrans
		r = race_lookup(pMobIndex->race);

		if (area_current->ver > 0) {
			pMobIndex->act = fread_flags(fp);
			pMobIndex->mob_flags = fread_flags(fp);
		} else {
			int64_t f = fread_flags64(fp);
			pMobIndex->act = subst_flag(v0_subst_act, f);
			pMobIndex->mob_flags = subst_flag(v0_subst_mob, f);
		}
		pMobIndex->act |= (r ? r->act : 0);

		if (area_current->ver > 0) {
			pMobIndex->affected_by = fread_flags(fp);
			pMobIndex->has_invis = fread_flags(fp);
			pMobIndex->has_detect = fread_flags(fp);
		} else {
			int64_t f = fread_flags64(fp);
			pMobIndex->affected_by = subst_flag(v0_subst_aff, f);
			pMobIndex->has_invis = subst_flag(v0_subst_invis, f);
			pMobIndex->has_detect = subst_flag(v0_subst_detect, f);
		}

		pMobIndex->affected_by |= (r ? r->aff : 0);
		pMobIndex->has_invis |= (r ? r->has_invis : 0);
		pMobIndex->has_detect |= (r ? r->has_detect : 0);

		pMobIndex->practicer		= 0;
		pMobIndex->pShop                = NULL;

		pMobIndex->alignment            = fread_number(fp);
		pMobIndex->group                = fread_number(fp);

		pMobIndex->level                = fread_number(fp);
		pMobIndex->hitroll              = fread_number(fp);

		/* read hit dice */
		pMobIndex->hit[DICE_NUMBER]     = fread_number(fp);
		/* 'd'          */                fread_letter(fp);
		pMobIndex->hit[DICE_TYPE]	= fread_number(fp);
		/* '+'          */                fread_letter(fp);
		pMobIndex->hit[DICE_BONUS]      = fread_number(fp);

		/* read mana dice */
		pMobIndex->mana[DICE_NUMBER]	= fread_number(fp);
		fread_letter(fp);
		pMobIndex->mana[DICE_TYPE]	= fread_number(fp);
		fread_letter(fp);
		pMobIndex->mana[DICE_BONUS]	= fread_number(fp);

		/* read damage dice */
		pMobIndex->damage[DICE_NUMBER]	= fread_number(fp);
		fread_letter(fp);
		pMobIndex->damage[DICE_TYPE]	= fread_number(fp);
		fread_letter(fp);
		pMobIndex->damage[DICE_BONUS]	= fread_number(fp);
		pMobIndex->damtype	= fread_strkey(
		    fp, &damtypes, "load_mobiles");		// notrans

		/* read armor class */
		pMobIndex->ac[AC_PIERCE]	= fread_number(fp) * 10;
		pMobIndex->ac[AC_BASH]		= fread_number(fp) * 10;
		pMobIndex->ac[AC_SLASH]		= fread_number(fp) * 10;
		pMobIndex->ac[AC_EXOTIC]	= fread_number(fp) * 10;

		/* read flags and add in data from the race table */
		pMobIndex->off_flags		=
		    fread_flags(fp) | (r ? r->off : 0);
		if (area_current->ver == 0) {
			imm = fread_flags(fp);
			res = fread_flags(fp);
			vul = fread_flags(fp);
		}

		/* vital statistics */
		pMobIndex->start_pos	= fread_fword(position_table, fp);
		pMobIndex->default_pos	= fread_fword(position_table, fp);

		fread_word(fp);
		mlstr_destroy(&pMobIndex->gender);
		mlstr_init2(&pMobIndex->gender, rfile_tok(fp));

		pMobIndex->wealth	= fread_number(fp);

		pMobIndex->form		= fread_flags(fp) | (r ? r->form : 0);
		if (area_current->ver < 5
		&&  IS_SET(pMobIndex->act, V4_ACT_UNDEAD)
		&&  !IS_SET(pMobIndex->form, FORM_UNDEAD)) {
			REMOVE_BIT(pMobIndex->act, V4_ACT_UNDEAD);
			SET_BIT(pMobIndex->form, FORM_UNDEAD);
		}

		pMobIndex->parts	= fread_flags(fp) | (r ? r->parts : 0);
		/* size */
		pMobIndex->size		= fread_fword(size_table, fp);
		free_string(pMobIndex->material);
		pMobIndex->material	= fread_sword(fp);

		for (; ;) {
			letter = fread_letter(fp);

			if (letter == 'a') {
				AFFECT_DATA *paf;

				if (area_current->ver < 6) {
					paf = aff_fread_v5(fp);

					if (area_current->ver < 4
					&&  IS_APPLY_AFFECT(paf)
					&&  INT(paf->location) >= 27) {
						AFFECT_DATA *paf2 = aff_new(
						    TO_RESISTS, str_empty);

						INT(paf2->location) = damtbl[
						    UMIN(INT(paf->location)-27,
							 (int) DAMTBL_SZ-1)];
						paf2->modifier = paf->modifier;

						paf2->level = pMobIndex->level;
						paf2->duration = -1;
						SLIST_ADD(AFFECT_DATA,
						    pMobIndex->affected, paf2);

						INT(paf->location) = APPLY_NONE;
						paf->modifier = 0;
					}
				} else {
					paf = aff_fread(fp, AFF_X_NOLD);

					if (area_current->ver == 6) {
						fread_number(fp);
						fread_number(fp);
					}
				}

				paf->level = pMobIndex->level;
				paf->duration = -1;
				SLIST_ADD(AFFECT_DATA,
				     pMobIndex->affected, paf);
			} else if (letter == 'f') {
				AFFECT_DATA *paf = aff_fread(
				    fp, AFF_X_NOTYPE | AFF_X_NOLD);

				if (area_current->ver == 6) {
					fread_number(fp);
					fread_number(fp);
				}

				paf->level = pMobIndex->level;
				paf->duration = -1;
				SLIST_ADD(AFFECT_DATA,
				     pMobIndex->affected, paf);
			} else if (letter == 'A') {
				fread_word(fp);
				if (!IS_TOKEN(fp, "det")) {
					SET_BIT(pMobIndex->affected_by,
					    fread_flags(fp));
				}
			} else if (letter == 'C') {
				if (!IS_NULLSTR(pMobIndex->clan)) {
					log(LOG_ERROR, "load_mobiles: duplicate clan.");
					return;
				}
				pMobIndex->clan = fread_strkey(
				    fp, &clans, "load_mobiles"); // notrans
			} else if (letter == 'W')
				pMobIndex->invis_level = fread_number(fp);
			else if (letter == 'I')
				pMobIndex->incog_level = fread_number(fp);
			else if (letter == 'V')
				pMobIndex->fvnum = fread_number(fp);
			else if (letter == 'F') {
				fread_word(fp);
				if (IS_TOKEN(fp, "act")) {
					REMOVE_BIT(pMobIndex->act,
					    fread_flags(fp));
				} else if (IS_TOKEN(fp, "aff")) {
					REMOVE_BIT(pMobIndex->affected_by,
					    fread_flags(fp));
				} else if (IS_TOKEN(fp, "inv")) {
					REMOVE_BIT(pMobIndex->has_invis,
					    fread_flags(fp));
				} else if (IS_TOKEN(fp, "det")) {
					REMOVE_BIT(pMobIndex->has_detect,
					    fread_flags(fp));
				} else if (IS_TOKEN(fp, "off")) {
					REMOVE_BIT(pMobIndex->off_flags,
					    fread_flags(fp));
				} else if (IS_TOKEN(fp, "imm"))
					fread_flags(fp);
				else if (IS_TOKEN(fp, "res"))
					fread_flags(fp);
				else if (IS_TOKEN(fp, "vul"))
					fread_flags(fp);
				else if (IS_TOKEN(fp, "for")) {
					REMOVE_BIT(pMobIndex->form,
					    fread_flags(fp));
				} else if (IS_TOKEN(fp, "par"))
					REMOVE_BIT(pMobIndex->parts,
					    fread_flags(fp));
				else {
					log(LOG_ERROR, "flag remove: flag not found.");
					return;
				}
			} else if (letter == 'M') {
				/* XXX */
				int trig_vnum;
				const char *phrase;
#if 0
				int type;

				if ((type = fread_fword(mptrig_types, fp)) == 0) {
					log(LOG_ERROR, "load_mobiles: vnum %d: invalid mob prog trigger",
					    pMobIndex->vnum);
					return;
				}
#endif
				fread_word(fp);
				trig_vnum = fread_number(fp);
				phrase = fread_string(fp);
				free_string(phrase);
			} else if (letter == 'g')
				mlstr_fread(fp, &pMobIndex->gender);
			else if (letter == 'r') {   /* Resists */
				int _res = fread_fword(dam_classes, fp);
				if (_res < 0 || _res == DAM_NONE) {
					log(LOG_ERROR, "load_mobiles: unknown resist name");
					fread_number(fp);
				}
				pMobIndex->resists[_res] = fread_number(fp);
			} else {
				xungetc(fp);
				break;
			}
		}

		if (IS_SET(pMobIndex->affected_by, AFF_SANCTUARY)
		&&  IS_EVIL(pMobIndex)) {
			TOUCH_VNUM(pMobIndex->vnum);
			REMOVE_BIT(pMobIndex->affected_by, AFF_SANCTUARY);
			SET_BIT(pMobIndex->affected_by, AFF_BLACK_SHROUD);
		}

		if (area_current->ver == 0
		&&  (imm || res || vul)) {
			if (IS_SET(imm, V0_RES_SUMMON))
				SET_BIT(pMobIndex->act, ACT_IMMSUMMON);

			if (IS_SET(imm, V0_RES_STEAL))
				SET_BIT(pMobIndex->act, ACT_IMMSTEAL);

			set_percent_resistances(
			    imm, res, vul, pMobIndex->resists);
		}

		iHash                   = vnum % MAX_KEY_HASH;
		pMobIndex->next         = mob_index_hash[iHash];
		mob_index_hash[iHash]   = pMobIndex;
		vnum_check(area_current, vnum);
		if (vnum > top_vnum_mob)
			top_vnum_mob = vnum;
	}
}

flag_subst_t v0_subst_stat[] =
{
	{ ITEM_GLOW,		ITEM_GLOW		},
	{ ITEM_HUM,		ITEM_HUM		},
	{ ITEM_DARK,		ITEM_DARK		},
	{ ITEM_EVIL,		ITEM_EVIL		},
	{ ITEM_INVIS,		ITEM_INVIS		},
	{ ITEM_MAGIC,		ITEM_MAGIC		},
	{ ITEM_NODROP,		ITEM_NODROP		},
	{ ITEM_BLESS,		ITEM_BLESS		},
	{ ITEM_ANTI_GOOD,	ITEM_ANTI_GOOD		},
	{ ITEM_ANTI_EVIL,	ITEM_ANTI_EVIL		},
	{ ITEM_ANTI_NEUTRAL,	ITEM_ANTI_NEUTRAL	},
	{ ITEM_NOREMOVE,	ITEM_NOREMOVE		},
	{ ITEM_ROT_DEATH,	ITEM_ROT_DEATH		},
	{ ITEM_VIS_DEATH,	ITEM_VIS_DEATH		},
	{ ITEM_MELT_DROP,	ITEM_MELT_DROP		},
	{ ITEM_BURN_PROOF,	ITEM_BURN_PROOF		},
	{ ITEM_NOT_EDIBLE,	ITEM_NOT_EDIBLE		},
	{ 0, 0 }
};

#define V0_ITEM_NOPURGE		(O)
#define V0_ITEM_NOSAC		(R)
#define V0_ITEM_NOLOCATE	(T)
#define V0_ITEM_SELL_EXTRACT	(W)
#define V0_ITEM_NOUNCURSE	(Z)
#define V0_ITEM_NOSELL		(aa)
#define V0_ITEM_QUEST		(cc)
#define V0_ITEM_CLAN		(ee)
#define V0_ITEM_QUIT_DROP	(ff)
#define V0_ITEM_PIT		(gg)
#define V0_ITEM_CHQUEST		(hh)
#define V0_ITEM_NOFIND		(ii)

#define V0_ITEM_NO_SAC		(P)

flag_subst_t v0_subst_obj[] =
{
	{ V0_ITEM_NOPURGE,	OBJ_NOPURGE		},
	{ V0_ITEM_NOSAC,	OBJ_NOSAC		},
	{ V0_ITEM_NOLOCATE,	OBJ_NOLOCATE		},
	{ V0_ITEM_SELL_EXTRACT,	OBJ_SELL_EXTRACT	},
	{ V0_ITEM_NOUNCURSE,	OBJ_NOUNCURSE		},
	{ V0_ITEM_NOSELL,	OBJ_NOSELL		},
	{ V0_ITEM_QUEST,	OBJ_QUEST		},
	{ V0_ITEM_CLAN,		OBJ_CLAN		},
	{ V0_ITEM_QUIT_DROP,	OBJ_QUIT_DROP		},
	{ V0_ITEM_PIT,		OBJ_PIT			},
	{ V0_ITEM_CHQUEST,	OBJ_CHQUEST		},
	{ V0_ITEM_NOFIND,	OBJ_NOFIND		},
	{ 0, 0 }
};

#define	V0_TO_IMMUNE	-2
#define	V0_TO_RESIST	-3
#define V0_TO_VULN	-4

/*
 * Snarf an obj section. new style
 */
DBLOAD_FUN(load_objects)
{
	bool done;
	OBJ_INDEX_DATA *pObjIndex;

	if (!area_current) {
		log(LOG_ERROR, "load_objects: no #AREA seen yet.");
		return;
	}

	for (;;) {
		int vnum;
		char letter;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#') {
			log(LOG_ERROR, "load_objects: # not found.");
			return;
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		if (get_obj_index(vnum)) {
			log(LOG_ERROR, "load_objects: vnum %d duplicated.", vnum);
			return;
		}

		pObjIndex                       = new_obj_index();

		pObjIndex->vnum                 = vnum;
		pObjIndex->reset_num		= 0;
		pObjIndex->name                 = fread_string(fp);
		mlstr_fread(fp, &pObjIndex->short_descr);

		mlstr_fread(fp, &pObjIndex->description);
		if (mlstr_stripnl(&pObjIndex->description))
			TOUCH_AREA(area_current);

		free_string(pObjIndex->material);
		pObjIndex->material		= fread_string(fp);

		if (IS_NULLSTR(pObjIndex->material)) {
			free_string(pObjIndex->material);
			pObjIndex->material = str_dup("unknown"); // notrans
		}

		if (!material_lookup(pObjIndex->material))
			log(LOG_INFO, "Obj %d: unknown material '%s'", vnum, pObjIndex->material);

		pObjIndex->item_type		= fread_fword(item_types, fp);
		if (area_current->ver > 0) {
			pObjIndex->stat_flags	= fread_flags(fp);
			pObjIndex->obj_flags	= fread_flags(fp);
		} else {
			int64_t f = fread_flags64(fp);
			pObjIndex->stat_flags	= subst_flag(v0_subst_stat, f);
			pObjIndex->obj_flags	= subst_flag(v0_subst_obj, f);
		}

		pObjIndex->wear_flags           = fread_flags(fp);
		if (area_current->ver == 0) {
			if (IS_SET(pObjIndex->wear_flags, V0_ITEM_NO_SAC)) {
				REMOVE_BIT(pObjIndex->wear_flags, V0_ITEM_NO_SAC);
				SET_BIT(pObjIndex->obj_flags, OBJ_NOSAC);
			}
		}

		fread_objval(pObjIndex->item_type, pObjIndex->value, fp);
		pObjIndex->level		= fread_number(fp);
		pObjIndex->weight               = fread_number(fp);
		pObjIndex->cost                 = fread_number(fp); 
		pObjIndex->limit                = -1;

		/* condition */
		letter = fread_letter(fp);
		switch (letter) {
		default:
		case 'P':
			pObjIndex->condition = 100;
			break;
		case 'G':
			pObjIndex->condition = 90;
			break;
		case 'A':
			pObjIndex->condition = 75;
			break;
		case 'W':
			pObjIndex->condition = 50;
			break;
		case 'D':
			pObjIndex->condition = 25;
			break;
		case 'B':
			pObjIndex->condition = 10;
			break;
		case 'R':
			pObjIndex->condition = 0;
			break;
		}

		for (done = FALSE; !done;) {
			AFFECT_DATA *paf;
			int where;
			vo_t location;
			int modifier;
			int16_t resists[MAX_RESIST];
			int64_t f;

			switch (letter = fread_letter(fp)) {
			case 'a':
				if (area_current->ver < 6) {
					paf = aff_fread_v5(fp);

					if (area_current->ver < 4
					&&  IS_APPLY_AFFECT(paf)
					&&  INT(paf->location) >= 27) {
						AFFECT_DATA *paf2 = aff_new(
						    TO_RESISTS, str_empty);

						INT(paf2->location) = damtbl[
						    UMIN(INT(paf->location)-27,
							 (int) DAMTBL_SZ - 1)];
						paf2->modifier = paf->modifier;

						paf2->duration = -1;
						paf2->level = pObjIndex->level;
						SLIST_ADD(AFFECT_DATA,
						    pObjIndex->affected, paf2);

						INT(paf->location) = APPLY_NONE;
						paf->modifier = 0;
					}
				} else {
					paf = aff_fread(fp, AFF_X_NOLD);

					if (area_current->ver == 6) {
						fread_number(fp);
						fread_number(fp);
					}
				}

				paf->level = pObjIndex->level;
				paf->duration = -1;
				SLIST_ADD(AFFECT_DATA,
				    pObjIndex->affected, paf);
				break;

			case 'f':
				paf = aff_fread(fp, AFF_X_NOTYPE | AFF_X_NOLD);

				if (area_current->ver == 6) {
					fread_number(fp);
					fread_number(fp);
				}

				paf->level = pObjIndex->level;
				paf->duration = -1;
				SLIST_ADD(AFFECT_DATA,
				    pObjIndex->affected, paf);
				break;

			case 'A':
				paf = aff_new(TO_OBJECT, str_empty);
				INT(paf->location)	= fread_number(fp);
				paf->modifier           = fread_number(fp);

				if (area_current->ver < 4
				&&  INT(paf->location) >= 27) {
					paf->where = TO_RESISTS;
					INT(paf->location) = damtbl[
					    UMIN(INT(paf->location)-27,
						 (int) DAMTBL_SZ-1)];
				}

				paf->level              = pObjIndex->level;
				paf->duration           = -1;
				SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
				break;

			case 'E':
				ed_fread(fp, &pObjIndex->ed);
				break;

			case 'F':
				letter = fread_letter(fp);

				INT(location)	= fread_number(fp);
				modifier	= fread_number(fp);
				f		= fread_flags64(fp);

				switch (letter) {
				case 'A':
				case 'D':
					where = TO_AFFECTS;
					break;
				case 'I':
					where = V0_TO_IMMUNE;
					init_resistances(resists);
					set_percent_resistances((flag_t)f, 0, 0, resists);
					break;
				case 'R':
					where = V0_TO_RESIST;
					init_resistances(resists);
					set_percent_resistances(0, (flag_t)f, 0, resists);
					break;
				case 'V':
					where = V0_TO_VULN;
					init_resistances(resists);
					set_percent_resistances(0, 0, (flag_t)f, resists);
					break;
				case 'i':
					where = TO_INVIS;
					break;
				case 'd':
					where = TO_DETECTS;
					break;
				default:
					log(LOG_ERROR, "load_objects: vnum %d: '%c': bad where on flag.", pObjIndex->vnum, letter);
					return;
				}

				if (area_current->ver == 0
				&&  (where == V0_TO_IMMUNE ||
				     where == V0_TO_RESIST ||
				     where == V0_TO_VULN)) {
					int i;

					for (i = 0; i < MAX_RESIST; i++) {
						if (resists[i] == RES_UNDEF)
							continue;

						paf = aff_new(
						    TO_RESISTS, str_empty);
						paf->level = pObjIndex->level;
						paf->duration = -1;
						INT(paf->location) = i;
						paf->modifier = resists[i];
						paf->bitvector = 0;
						SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
					}

					if (modifier) {
						paf = aff_new(
						    TO_AFFECTS, str_empty);
						paf->level = pObjIndex->level;
						paf->duration = -1;
						paf->location = location;
						paf->bitvector = 0;
						paf->modifier = modifier;
						SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
					}
					break;
				}

				if (where == TO_AFFECTS
				&&  area_current->ver == 0) {
					flag_t f2;

					/*
					 * TO_AFFECTS
					 */
					f2 = subst_flag(v0_subst_aff, f);
					if (f2 != 0) {
						paf = aff_new(
						    TO_AFFECTS, str_empty);
						paf->duration = -1;
						paf->location = location;
						paf->modifier = modifier;
						paf->bitvector = f2;
						SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
						INT(location) = APPLY_NONE;
						modifier = 0;
					}

					/*
					 * TO_INVIS
					 */
					f2 = subst_flag(v0_subst_invis, f);
					if (f2 != 0) {
						paf = aff_new(
						    TO_INVIS, str_empty);
						paf->level = pObjIndex->level;
						paf->duration = -1;
						paf->location = location;
						paf->modifier = modifier;
						paf->bitvector = f2;
						SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
						INT(location) = APPLY_NONE;
						modifier = 0;
					}

					/*
					 * TO_DETECTS
					 */
					f2 = subst_flag(v0_subst_detect, f);
					if (f2 != 0) {
						paf = aff_new(
						    TO_DETECTS, str_empty);
						paf->level = pObjIndex->level;
						paf->duration = -1;
						paf->location = location;
						paf->modifier = modifier;
						paf->bitvector = f2;
						SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
						INT(location) = APPLY_NONE;
						modifier = 0;
					}

					if (!modifier)
						break;

					where = TO_AFFECTS;
					f = 0;
				}

				if (where == TO_AFFECTS
				&&  INT(location) >= 27
				&&  area_current->ver < 4) {
					paf = aff_new(TO_RESISTS, str_empty);
					paf->level = pObjIndex->level;
					paf->duration = -1;
					INT(paf->location) = damtbl[
					    UMIN(INT(location)-27,
						 (int) DAMTBL_SZ-1)];
					paf->modifier = modifier;
					paf->bitvector = 0;
					SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
					INT(location) = APPLY_NONE;
					modifier = 0;
				}

				paf		= aff_new(where, str_empty);
				paf->level	= pObjIndex->level;
				paf->duration	= -1;
				paf->location	= location;
				paf->modifier	= modifier;
				paf->bitvector	= f;

				SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
				break;

			case 'g':
				mlstr_fread(fp, &pObjIndex->gender);
				break;

			case 'G':
				fread_word(fp);
				mlstr_destroy(&pObjIndex->gender);
				mlstr_init2(&pObjIndex->gender, rfile_tok(fp));
				break;

#if 0
			XXX
			case 'R':
				fread_cc_vexpr(&pObjIndex->restrictions,
					       "obj_wear", fp);	// notrans
				break;
#endif

			case 'S':
				paf = aff_new(TO_SKILLS, str_empty);
				paf->level = pObjIndex->level;
				paf->duration = -1;
				paf->location.s = fread_strkey(
				    fp, &skills, "load_objects"); // notrans
				paf->modifier = fread_number(fp);
				paf->bitvector = fread_flags(fp);
				SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
				break;

			default:
				xungetc(fp);
				done = TRUE;
				break;
			}
		}

		iHash                   = vnum % MAX_KEY_HASH;
		pObjIndex->next         = obj_index_hash[iHash];
		obj_index_hash[iHash]   = pObjIndex;
		vnum_check(area_current, vnum);
		if (vnum > top_vnum_obj)
			top_vnum_obj = vnum;
	}
}

/*
 * Sets vnum range for area using OLC protection features.
 */
static void
vnum_check(AREA_DATA *area, int vnum)
{
	if (area->min_vnum == 0 || area->max_vnum == 0) {
		log(LOG_INFO, "%s: min_vnum or max_vnum not assigned",
			   area->file_name);
	}

	if (vnum < area->min_vnum || vnum > area->max_vnum) {
		log(LOG_INFO, "%s: %d not in area vnum range",
			   area->file_name, vnum);
	}
}

