/*
 * $Id: save.c,v 1.191 2001-08-20 17:57:28 fjoe Exp $
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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>

#if defined(__FreeBSD__)
#include <osreldate.h>
#endif

#include <merc.h>
#include <db.h>
#include <rwfile.h>

#include <sog.h>

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

/*
 * global vars for areaed_move()
 */
static int minv, maxv, del;

/*
 * Local functions.
 */
static void fwrite_char(CHAR_DATA *ch, FILE *fp, int flags);
static void fwrite_obj(CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest);
static void fwrite_pet(CHAR_DATA *pet, FILE *fp, int flags);
static void fread_char(CHAR_DATA *ch, rfile_t *fp, int flags);
static void fread_pet(CHAR_DATA *ch, rfile_t *fp, int flags);
static void fread_obj(CHAR_DATA *ch, CHAR_DATA *obj_to, rfile_t *fp, int flags);
static void move_pfile(const char *name, int minvnum, int maxvnum, int delta);

/*
 * move_pfile - shifts vnum in range minvnum..maxvnum by delta)
 */
#define IN_RANGE(i, l, u)	((l) <= (i) && (i) <= (u))
#define MOVE(i)					\
	do {					\
		if (IN_RANGE(i, minv, maxv))	\
			i += del;		\
	} while (0)

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void
char_save(CHAR_DATA *ch, int flags)
{
	FILE           *fp;
	const char	*name;
	CHAR_DATA *	pet;

	if (IS_NPC(ch) || ch->level < 2)
		return;

	name = capitalize(ch->name);

	/* create god log */
	if (IS_IMMORTAL(ch)) {
		if ((fp = dfopen(GODS_PATH, name, "w")) == NULL)
			return;
		fprintf(fp, "Lev %2d %s%s\n",
		      ch->level, ch->name, PC(ch)->title);
		fclose(fp);
	}

	if ((fp = dfopen(PLAYER_PATH, TMP_FILE, "w")) == NULL)
		return;

	act_puts("Saving.", ch, NULL, NULL, TO_CHAR, POS_DEAD);
	fwrite_char(ch, fp, flags);
	if (ch->carrying)
		fwrite_obj(ch, ch->carrying, fp, 0);

	/* save the pets */
	if ((pet = GET_PET(ch))
	&&  (IS_SET(flags, SAVE_F_PSCAN) || pet->in_room == ch->in_room)) {
		fwrite_pet(pet, fp, flags);
		if (pet->carrying)
			fwrite_obj(ch, pet->carrying, fp, 0);
	}
	fprintf(fp, "#END\n");
	fclose(fp);
	d2rename(PLAYER_PATH, TMP_FILE, PLAYER_PATH, name);
}

static
FOREACH_CB_FUN(pc_skill_save_cb, p, ap)
{
	pc_skill_t *pc_sk = (pc_skill_t *) p;

	FILE *fp = va_arg(ap, FILE *);

	if (pc_sk->percent == 0)
		return NULL;

	fprintf(fp, "Sk %d '%s'\n", pc_sk->percent, pc_sk->sn);
	return NULL;
}

static
FOREACH_CB_FUN(spn_save_cb, p, ap)
{
	const char *spn = *(const char **) p;

	FILE *fp = va_arg(ap, FILE *);

	if (IS_NULLSTR(spn))
		return NULL;

	fprintf(fp, "Spec '%s'\n", spn);
	return NULL;
}

/*
 * Write the char.
 */
static void
fwrite_char(CHAR_DATA *ch, FILE *fp, int flags)
{
	AFFECT_DATA    *paf;

	fprintf(fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER");

	fprintf(fp, "Vers %d\n", PFILE_VERSION);
	fwrite_string(fp, "Name", ch->name);
	mlstr_fwrite(fp, "ShD", &ch->short_descr);
	fprintf(fp, "Ethos %s\n", flag_string(ethos_table, ch->ethos));
	fwrite_word(fp, "Clan", ch->clan);
	fwrite_string(fp, "Desc", mlstr_mval(&ch->description));
	mlstr_fwrite(fp, "SSex", &ch->gender);
	fwrite_word(fp, "Class", ch->class);
	fprintf(fp, "Levl %d\n", ch->level);
	fprintf(fp, "Room %d\n",
		(ch->in_room == get_room_index(ROOM_VNUM_LIMBO) &&
		 !IS_NPC(ch) && PC(ch)->was_in_vnum) ?
			PC(ch)->was_in_vnum :
		 ch->in_room == NULL ?
			ROOM_VNUM_TEMPLE : ch->in_room->vnum);

	fprintf(fp, "HMV %d %d %d %d %d %d\n",
		ch->hit, ch->perm_hit,
		ch->mana, ch->perm_mana,
		ch->move, ch->perm_move);
	if (ch->gold > 0)
		fprintf(fp, "Gold %d\n", ch->gold);
	else
		fprintf(fp, "Gold %d\n", 0);
	if (ch->silver > 0)
		fprintf(fp, "Silv %d\n", ch->silver);
	else
		fprintf(fp, "Silv %d\n", 0);
	if (ch->comm)
		fprintf(fp, "Comm %s\n", format_flags(ch->comm));
	if (ch->chan)
		fprintf(fp, "Chan %s\n", format_flags(ch->chan));
	if (ch->invis_level)
		fprintf(fp, "Invi %d\n", ch->invis_level);
	if (ch->incog_level)
		fprintf(fp, "Inco %d\n", ch->incog_level);
	fprintf(fp, "Pos %d\n",
		ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
	fprintf(fp, "Alig %d\n", ch->alignment);
	if (ch->wimpy != 0)
		fprintf(fp, "Wimp %d\n", ch->wimpy);
	fprintf(fp, "Attr %d %d %d %d %d %d\n",
		ch->perm_stat[STAT_STR],
		ch->perm_stat[STAT_INT],
		ch->perm_stat[STAT_WIS],
		ch->perm_stat[STAT_DEX],
		ch->perm_stat[STAT_CON],
		ch->perm_stat[STAT_CHA]);

	fprintf(fp, "Luck %d", ch->luck);
	if (ch->luck_mod)
		fprintf(fp, "LuckMod %d", ch->luck_mod);

	if (IS_NPC(ch)) {
		fprintf(fp, "Vnum %d\n", ch->pMobIndex->vnum);
	} else {
		PC_DATA *pc = PC(ch);
		int i;

		if (pc->plr_flags) {
			fprintf(fp, "Act %s\n",
				format_flags(PC(ch)->plr_flags));
		}
		if (pc->www_show_flags) {
			fprintf(fp, "WWW %s\n",
				format_flags(PC(ch)->www_show_flags));
		}
		if (str_cmp(pc->dvdata->prompt, DEFAULT_PROMPT))
			fwrite_string(fp, "Prom", pc->dvdata->prompt);
		fprintf(fp, "Scro %d\n", pc->dvdata->pagelen);

		/* write aliases */
		for (i = 0; i < MAX_ALIAS; i++) {
			if (pc->dvdata->alias[i] == NULL
			||  pc->dvdata->alias_sub[i] == NULL)
				break;

			fprintf(fp, "Alias %s %s~\n",
				pc->dvdata->alias[i],
				fix_string(pc->dvdata->alias_sub[i]));
		}

		if (!IS_NULLSTR(ch->clan))
			fprintf(fp, "ClanStatus %d\n", pc->clan_status);
		else
			fwrite_word(fp, "Peti", pc->petition);

		if (pc->practice != 0)
			fprintf(fp, "Prac %d\n", pc->practice);
		if (pc->train != 0)
			fprintf(fp, "Trai %d\n", pc->train);
		fprintf(fp, "Exp %d\n", pc->exp);
		fwrite_string(fp, "Hometown", hometown_name(pc->hometown));
#if defined(__FreeBSD__) && __FreeBSD_version >= 500000
		fprintf(fp, "LogO %d\n",
#else
		fprintf(fp, "LogO %ld\n",
#endif
			IS_SET(flags, SAVE_F_PSCAN) ?
				pc->logoff : current_time);
		if (pc->wiznet)
			fprintf(fp, "Wizn %s\n", format_flags(pc->wiznet));

		if (pc->trust)
			fprintf(fp, "Trust %s\n", format_flags(pc->trust));

		fprintf(fp, "Race '%s'\n", pc->race);
		if (pc->plevels > 0)
			fprintf(fp, "PLev %d\n", pc->plevels);
		fprintf(fp, "Plyd %d\n",
			pc->played + (int) (current_time - pc->logon));
#if defined(__FreeBSD__) && __FreeBSD_version >= 500000
		fprintf(fp, "Not %d %d %d %d %d\n",
#else
		fprintf(fp, "Not %ld %ld %ld %ld %ld\n",
#endif
			pc->last_note, pc->last_idea,
			pc->last_penalty, pc->last_news,
			pc->last_changes);

		fprintf(fp, "Dead %d\n", pc->death);
		if (pc->homepoint)
			fprintf(fp, "Homepoint %d\n", pc->homepoint->vnum);

		if (pc->bank_s)
			fprintf(fp, "Banks %d\n", pc->bank_s);
		if (pc->bank_g)
			fprintf(fp, "Bankg %d\n", pc->bank_g);
		if (pc->security)
			fprintf(fp, "Sec %d\n", pc->security);
		fwrite_string(fp, "Pass", pc->pwd);
		fwrite_string(fp, "Bin", pc->bamfin);
		fwrite_string(fp, "Bout", pc->bamfout);
		fwrite_string(fp, "Titl", pc->title);
		fwrite_string(fp, "WantedBy", pc->wanted_by);
		fwrite_string(fp, "FormName", pc->form_name);
		fprintf(fp, "CndC %d %d %d %d %d %d\n",
			pc->condition[0],
			pc->condition[1],
			pc->condition[2],
			pc->condition[3],
			pc->condition[4],
			pc->condition[5]);

		/* write lang */
		fprintf(fp, "Lang %d\n", pc->dvdata->lang);

		/* write pc_killed */
		fprintf(fp, "PC_Killed %d\n", pc->pc_killed);

		varr_foreach(&pc->specs, spn_save_cb, fp);
		varr_foreach(&pc->learned, pc_skill_save_cb, fp);

		if (pc->questpoints != 0)
			fprintf(fp, "QuestPnts %d\n", pc->questpoints);
		if (pc->questtime != 0)
			fprintf(fp, "QuestTime %d\n", number_range(12, 15));
		fprintf(fp, "Haskilled %d\n", pc->has_killed);
		fprintf(fp, "Antkilled %d\n", pc->anti_killed);
		fwrite_string(fp, "Twitlist", pc->twitlist);
		fwrite_string(fp, "Granted", pc->granted);
		if (pc->hints_level != HINT_ALL) {
			fprintf(fp, "HintsLevel %s\n",
				flag_string(hint_levels, pc->hints_level));
		}
		if (pc->dvdata->olc_flags) {
			fprintf(fp, "OLC %s\n",
				format_flags(pc->dvdata->olc_flags));
		}
	}

	for (paf = ch->affected; paf != NULL; paf = paf->next) {
		if (!IS_NPC(ch) && paf->where == TO_AFFECTS
		&&  IS_SET(paf->bitvector, AFF_CHARM))
			continue;

		fprintf(fp, "Affc ");
		aff_fwrite(paf, fp, TRUE);
	}

	fprintf(fp, "End\n\n");
}

/* write a pet */
/* flags do not affect pet saving, reserved for future use */
static void
fwrite_pet(CHAR_DATA *pet, FILE *fp, int flags __attribute__((unused)))
{
	fprintf(fp, "#PET\n");

	fprintf(fp, "Vnum %d\n", pet->pMobIndex->vnum);
	fwrite_string(fp, "Name", pet->name);
	fwrite_word(fp, "Clan", pet->clan);
	if (mlstr_cmp(&pet->short_descr, &pet->pMobIndex->short_descr) != 0)
		mlstr_fwrite(fp, "ShD", &pet->short_descr);
	if (mlstr_cmp(&pet->long_descr, &pet->pMobIndex->long_descr) != 0)
		mlstr_fwrite(fp, "LnD", &pet->short_descr);
	if (mlstr_cmp(&pet->description, &pet->pMobIndex->description) != 0)
		mlstr_fwrite(fp, "Desc", &pet->short_descr);
	if (mlstr_cmp(&pet->gender, &pet->pMobIndex->gender) != 0)
		mlstr_fwrite(fp, "SSex", &pet->gender);
	if (pet->level != pet->pMobIndex->level)
		fprintf(fp, "Levl %d\n", pet->level);
	fprintf(fp, "HMV %d %d %d %d %d %d\n",
		pet->hit, pet->perm_hit,
		pet->mana, pet->perm_mana,
		pet->move, pet->perm_move);
	if (pet->gold)
		fprintf(fp, "Gold %d\n", pet->gold);
	if (pet->silver)
		fprintf(fp, "Silv %d\n", pet->silver);
	if (pet->comm != 0)
		fprintf(fp, "Comm %s\n", format_flags(pet->comm));
	if (pet->chan != 0)
		fprintf(fp, "Chan %s\n", format_flags(pet->chan));
	fprintf(fp, "Pos %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
	if (pet->alignment != pet->pMobIndex->alignment)
		fprintf(fp, "Alig %d\n", pet->alignment);
	if (pet->damroll != pet->pMobIndex->damage[DICE_BONUS])
		fprintf(fp, "Damr %d\n", pet->damroll);
	if (NPC(pet)->dam.dice_number != pet->pMobIndex->damage[DICE_NUMBER] ||
	    NPC(pet)->dam.dice_type != pet->pMobIndex->damage[DICE_TYPE])
		fprintf(fp, "Damd %d %d\n",
			NPC(pet)->dam.dice_number,
			NPC(pet)->dam.dice_type);
	fprintf(fp, "Attr %d %d %d %d %d %d\n",
		pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
		pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
		pet->perm_stat[STAT_CON], pet->perm_stat[STAT_CHA]);
	fprintf(fp, "AC %d %d %d %d\n",
		pet->armor[0], pet->armor[1], pet->armor[2], pet->armor[3]);

	aff_fwrite_list("Affc", NULL, pet->affected, fp);
	fprintf(fp, "End\n\n");
}

/*
 * Write an object and its contents.
 */
static void
fwrite_obj(CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest)
{
	ED_DATA *ed;

	/*
	 * Slick recursion to write lists backwards, so loading them will load
	 * in forwards order.
	 */
	if (obj->next_content != NULL)
		fwrite_obj(ch, obj->next_content, fp, iNest);

/* do not save named quest rewards if ch is not owner */
	if (!IS_IMMORTAL(ch)
	&&  OBJ_IS(obj, OBJ_QUEST)
	&&  !IS_OWNER(ch, obj)) {
		log(LOG_INFO, "fwrite_obj: %s: '%s' of %s",
			   ch->name, obj->pObjIndex->name,
			   mlstr_mval(&obj->owner));
		act("$p vanishes!", ch, obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
		return;
	}

	if (OBJ_IS(obj, OBJ_CLAN | OBJ_QUIT_DROP | OBJ_CHQUEST))
		return;

/* Do not save limited eq if player is not in PK */
	if (ch->level < LEVEL_PK && obj->pObjIndex->limit != -1)
		return;

	fprintf(fp, "#O\n");
	fprintf(fp, "Vnum %d\n", obj->pObjIndex->vnum);
	fprintf(fp, "Cond %d\n", obj->condition);

	fprintf(fp, "Nest %d\n", iNest);
	mlstr_fwrite(fp, "Owner", &obj->owner);

	if (obj->pObjIndex->limit < 0) {
		if (mlstr_cmp(&obj->short_descr, &obj->pObjIndex->short_descr))
			mlstr_fwrite(fp, "ShD", &obj->short_descr);
		if (mlstr_cmp(&obj->description, &obj->pObjIndex->description))
			mlstr_fwrite(fp, "Desc", &obj->description);
	}

	if (!IS_NULLSTR(obj->label)) {
		if (obj->label[0] != ' ') {
			const char *p = obj->label;
			obj->label = str_printf(" %s", obj->label);
			free_string(p);
		}
		fwrite_string(fp, "Label", obj->label);
	}

	if (obj->stat_flags != obj->pObjIndex->stat_flags)
		fprintf(fp, "StatF %d\n", obj->stat_flags);

	fprintf(fp, "Wear %d\n", obj->wear_loc);
	if (obj->level != obj->pObjIndex->level)
		fprintf(fp, "Lev %d\n", obj->level);
	if (obj->timer != 0)
		fprintf(fp, "Time %d\n", obj->timer);
	fprintf(fp, "Cost %d\n", obj->cost);

	if (obj->item_type != obj->pObjIndex->item_type) {
		fprintf(fp, "ItemType %s\n",
			flag_string(item_types, obj->item_type));
	}

	if (INT(obj->value[0]) != INT(obj->pObjIndex->value[0])
	||  INT(obj->value[1]) != INT(obj->pObjIndex->value[1])
	||  INT(obj->value[2]) != INT(obj->pObjIndex->value[2])
	||  INT(obj->value[3]) != INT(obj->pObjIndex->value[3])
	||  INT(obj->value[4]) != INT(obj->pObjIndex->value[4])) {
		fprintf(fp, "Values ");
		fwrite_objval(obj->item_type, obj->value, fp);
	}

	aff_fwrite_list("Affc", NULL, obj->affected, fp);

	for (ed = obj->ed; ed != NULL; ed = ed->next) {
		if (IS_NULLSTR(ed->keyword))
			continue;
		fwrite_string(fp, "ExDe", ed->keyword);
		mlstr_fwrite(fp, NULL, &ed->description);
	}

	fprintf(fp, "End\n\n");

	if (obj->contains != NULL)
		fwrite_obj(ch, obj->contains, fp, iNest + 1);
}

/*
 * Load a char and inventory into a new ch structure.
 */
CHAR_DATA *
char_load(const char *name, int flags)
{
	CHAR_DATA *ch;
	rfile_t	*fp = NULL;
	bool found;

	CHAR_DATA *obj_to = NULL;
	int iNest;

	name = capitalize(name);
	snprintf(bootdb_filename, sizeof(bootdb_filename),
	    "%s.gz", name);					// notrans
	if (dfexist(PLAYER_PATH, bootdb_filename)) {
		char buf[PATH_MAX];
		snprintf(buf, sizeof(buf), "gzip -dfq %s%c%s",	// notrans
			 PLAYER_PATH, PATH_SEPARATOR, bootdb_filename);
		system(buf);
	}

	found = (dfexist(PLAYER_PATH, name) &&
		 (fp = rfile_open(PLAYER_PATH, name)) != NULL);

	if (!found && IS_SET(flags, LOAD_F_NOCREATE))
		return NULL;

	ch = char_new(NULL);

	ch->name = str_dup(capitalize(name));
	mlstr_init2(&ch->short_descr, ch->name);

	if (!found) {
		PC(ch)->plr_flags |= PLR_NEW;
		return ch;
	}

	for (iNest = 0; iNest < MAX_NEST; iNest++)
		rgObjNest[iNest] = NULL;

	for (;;) {
		char	letter;
		letter = fread_letter(fp);
		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}
		if (letter != '#') {
			log(LOG_INFO, "char_load: %s: # not found.", ch->name);
			break;
		}

		fread_word(fp);
		if (IS_TOKEN(fp, "PLAYER")) {
			fread_char(ch, fp, flags);
			obj_to = ch;
		} else if (IS_TOKEN(fp, "OBJECT"))
			fread_obj(ch, obj_to, fp, flags);
		else if (IS_TOKEN(fp, "O"))
			fread_obj(ch, obj_to, fp, flags);
		else if (IS_TOKEN(fp, "PET")) {
			fread_pet(ch, fp, flags);
			if (GET_PET(ch))
				obj_to = GET_PET(ch);
		} else if (IS_TOKEN(fp, "END"))
			break;
		else {
			log(LOG_INFO, "char_load: %s: %s: bad section.",
			    ch->name, rfile_tok(fp));
			break;
		}
	}
	rfile_close(fp);

	return ch;
}

/*
 * Read in a char.
 */
static void
fread_char(CHAR_DATA *ch, rfile_t *fp, int flags)
{
	int count = 0;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			KEY("Act", PC(ch)->plr_flags, fread_flags(fp) &
					    ~(PLR_GHOST | PLR_CONFIRM_DELETE |
					      PLR_NOEXP | PLR_NEW | PLR_PUMPED |
					      PLR_PRACTICER));
			KEY("Alignment", ch->alignment, fread_number(fp));
			KEY("Alig", ch->alignment, fread_number(fp));
			KEY("AntKilled", PC(ch)->anti_killed, fread_number(fp));

			if (IS_TOKEN(fp, "Alias")) {
				if (count >= MAX_ALIAS) {
					fread_to_eol(fp);
					fMatch = TRUE;
					break;
				}
				PC(ch)->dvdata->alias[count] =
						fread_sword(fp);
				PC(ch)->dvdata->alias_sub[count] =
						fread_string(fp);
				count++;
				fMatch = TRUE;
				break;
			}
			if (IS_TOKEN(fp, "Affc")) {
				AFFECT_DATA *paf = aff_fread(fp, TRUE);
				affect_to_char(ch, paf);
				aff_free(paf);
				fMatch = TRUE;
				break;
			}
			if (IS_TOKEN(fp, "Attr")) {
				int stat;
				for (stat = 0; stat < MAX_STAT; stat++)
					ch->perm_stat[stat] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'B':
			KEY("Banks", PC(ch)->bank_s, fread_number(fp));
			KEY("Bankg", PC(ch)->bank_g, fread_number(fp));
			SKEY("Bin", PC(ch)->bamfin, fread_string(fp));
			SKEY("Bout", PC(ch)->bamfout, fread_string(fp));
			break;

		case 'C':
			KEY("Class", ch->class, fread_strkey(
			    fp, &classes, "fread_char"));	// notrans
			KEY("Clan", ch->clan, fread_strkey(
			    fp, &clans, "fread_char"));		// notrans
			KEY("ClanStatus", PC(ch)->clan_status,
			    fread_number(fp));
			if (IS_TOKEN(fp, "CndC")) {
				PC(ch)->condition[0] = fread_number(fp);
				PC(ch)->condition[1] = fread_number(fp);
				PC(ch)->condition[2] = fread_number(fp);
				PC(ch)->condition[3] = fread_number(fp);
				PC(ch)->condition[4] = fread_number(fp);
				PC(ch)->condition[5] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			KEY("Comm", ch->comm, fread_flags(fp) & ~COMM_AFK);
			KEY("Chan", ch->chan, fread_flags(fp));

			break;

		case 'D':
			MLSKEY("Desc", ch->description);
			KEY("Dead", PC(ch)->death, fread_number(fp));
			break;

		case 'E':
			if (IS_TOKEN(fp, "End")) {
				clan_t *clan;
				const char **nl = NULL;
				bool touched = FALSE;

				if (PC(ch)->condition[COND_BLOODLUST] < 48
				&&  !IS_VAMPIRE(ch))
					PC(ch)->condition[COND_BLOODLUST] = 48;

				if (PC(ch)->dvdata->pagelen < MIN_PAGELEN-2)
					PC(ch)->dvdata->pagelen = MAX_PAGELEN-2;

				varr_qsort(&PC(ch)->specs, cmpstr);
				spec_update(ch);

				/* XXX update clan lists */
				if ((clan = clan_lookup(ch->clan)) == NULL)
					return;

				touched = !name_add(&clan->member_list, ch->name, NULL, NULL);
				switch (PC(ch)->clan_status) {
				case CLAN_LEADER:
					nl = &clan->leader_list;
					break;
				case CLAN_SECOND:
					nl = &clan->second_list;
					break;
				}
				if (nl)
					touched = !name_add(nl, ch->name, NULL, NULL) || touched;
				if (touched)
					clan_save(clan->name);
				return;
			}
			KEY("Exp", PC(ch)->exp, fread_number(fp));
			KEY("Ethos", ch->ethos, fread_fword(ethos_table, fp));
			break;

		case 'F':
			SKEY("FormName", PC(ch)->form_name, fread_string(fp));
			break;

		case 'G':
			KEY("Gold", ch->gold, fread_number(fp));
			SKEY("Granted", PC(ch)->granted, fread_string(fp));
			break;

		case 'H':
			KEY("HintsLevel", PC(ch)->hints_level,
			    fread_fword(hint_levels, fp));
			if (IS_TOKEN(fp, "Hometown")) {
				const char *s = fread_string(fp);
				PC(ch)->hometown = htn_lookup(s);
				free_string(s);
				fMatch = TRUE;
			}

			KEY("Haskilled", PC(ch)->has_killed,
			    fread_number(fp));
			if (IS_TOKEN(fp, "Homepoint")) {
				int room = fread_number(fp);

				if (IS_SET(flags, LOAD_F_MOVE))
					MOVE(room);
				PC(ch)->homepoint = get_room_index(room);
				fMatch = TRUE;
				break;
			}

			if (IS_TOKEN(fp, "HMV")) {
				ch->hit = fread_number(fp);
				ch->max_hit = ch->perm_hit = fread_number(fp);
				ch->mana = fread_number(fp);
				ch->max_mana = ch->perm_mana = fread_number(fp);
				ch->move = fread_number(fp);
				ch->max_move = ch->perm_move = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'I':
			KEY("Inco", ch->incog_level, fread_number(fp));
			KEY("Invi", ch->invis_level, fread_number(fp));
			break;

		case 'L':
			KEY("Levl", ch->level, fread_number(fp));
			KEY("LogO", PC(ch)->logoff, fread_number(fp));
			KEY("Lang", PC(ch)->dvdata->lang, fread_number(fp));
			KEY("Luck", ch->luck, fread_number(fp));
			KEY("LuckMod", ch->luck_mod, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", ch->name, fread_string(fp));
			if (IS_TOKEN(fp, "Not")) {
				PC(ch)->last_note = fread_number(fp);
				PC(ch)->last_idea = fread_number(fp);
				PC(ch)->last_penalty = fread_number(fp);
				PC(ch)->last_news = fread_number(fp);
				PC(ch)->last_changes = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'O':
			KEY("OLC", PC(ch)->dvdata->olc_flags, fread_flags(fp));
			break;

		case 'P':
			KEY("Peti", PC(ch)->petition, fread_strkey(
			    fp, &clans, "fread_char"));		// notrans
			KEY("PLev", PC(ch)->plevels, fread_number(fp));
			SKEY("Pass", PC(ch)->pwd, fread_string(fp));
			KEY("PC_Killed", PC(ch)->pc_killed, fread_number(fp));
			KEY("Plyd", PC(ch)->played, fread_number(fp));
			KEY("Pos", ch->position, fread_number(fp));
			KEY("Prac", PC(ch)->practice, fread_number(fp));
			SKEY("Prom", PC(ch)->dvdata->prompt, fread_string(fp));
			break;

		case 'Q':
			KEY("QuestTime", PC(ch)->questtime, fread_number(fp));
			KEY("QuestPnts", PC(ch)->questpoints, fread_number(fp));
			break;

		case 'R':
			if (IS_TOKEN(fp, "Race")) {
				free_string(ch->race);
				ch->race = fread_strkey(
				    fp, &races, "fread_char");	// notrans
				PC(ch)->race = str_qdup(ch->race);
				race_resetstats(ch);
				fMatch = TRUE;
			}
			if (IS_TOKEN(fp, "Room")) {
				int room = fread_number(fp);

				if (IS_SET(flags, LOAD_F_MOVE))
					MOVE(room);
				ch->in_room = get_room_index(room);
				if (ch->in_room == NULL)
					ch->in_room = get_room_index(ROOM_VNUM_LIMBO);
				fMatch = TRUE;
				break;
			}
			break;

		case 'S':
			KEY("Scro", PC(ch)->dvdata->pagelen, fread_number(fp));
			if (IS_TOKEN(fp, "Sex")) {
				mlstr_destroy(&ch->gender);
				mlstr_init2(&ch->gender,
					   flag_string(gender_table, fread_number(fp)));
				fMatch = TRUE;
			}
			MLSKEY("SSex", ch->gender);
			MLSKEY("ShD", ch->short_descr);
			KEY("Sec", PC(ch)->security, fread_number(fp));
			KEY("Silv", ch->silver, fread_number(fp));
			if (IS_TOKEN(fp, "Spec")) {
				const char **pspn = varr_enew(&PC(ch)->specs);
				*pspn = fread_strkey(
				    fp, &specs, "fread_char");	// notrans
				fMatch = TRUE;
			}
			if (IS_TOKEN(fp, "Sk")) {
				int value = fread_number(fp);
				const char *sn = fread_strkey(
				    fp, &skills, "fread_char");	// notrans
				set_skill(ch, sn, value);
				free_string(sn);
				fMatch = TRUE;
			}
			break;

		case 'T':
			KEY("Trai", PC(ch)->train, fread_number(fp));
			KEY("Trust", PC(ch)->trust, fread_flags(fp));
			SKEY("Twitlist", PC(ch)->twitlist, fread_string(fp));
			SKEY("Titl", PC(ch)->title, fread_string(fp));
			break;

		case 'V':
			KEY("Vers", PC(ch)->version, fread_number(fp));
			break;

		case 'W':
			KEY("Wimp", ch->wimpy, fread_number(fp));
			KEY("Wizn", PC(ch)->wiznet, fread_flags(fp));
			SKEY("WantedBy", PC(ch)->wanted_by, fread_string(fp));
			KEY("WWW", PC(ch)->www_show_flags, fread_flags(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_INFO, "fread_char: %s: %s: no match",
			    ch->name, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

/* load a pet from the forgotten reaches */
static void
fread_pet(CHAR_DATA *ch, rfile_t *fp, int flags)
{
	CHAR_DATA *pet = NULL;

	/* first entry had BETTER be the vnum or we barf */
	fread_keyword(fp);
	if (IS_TOKEN(fp, "Vnum")) {
		int vnum = fread_number(fp);

		if (IS_SET(flags, LOAD_F_MOVE))
			MOVE(vnum);

		pet = create_mob(vnum, CM_F_NOLIST);
	}

	if (pet == NULL) {
		log(LOG_INFO, "fread_pet: %s: no pet vnum or bad vnum in file", ch->name);
		fread_to_end(fp);
		return;
	}

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			KEY("Alig", pet->alignment, fread_number(fp));

			if (IS_TOKEN(fp, "AC")) {
				int i;

				for (i = 0; i < 4; i++)
					pet->armor[i] = fread_number(fp);
				fMatch = TRUE;
				break;
			}

			if (IS_TOKEN(fp, "Affc")) {
				AFFECT_DATA *paf = aff_fread(fp, TRUE);
				affect_to_char(pet, paf);
				aff_free(paf);
				fMatch = TRUE;
				break;
			}
			if (IS_TOKEN(fp, "Attr")) {
				int stat;
				for (stat = 0; stat < MAX_STAT; stat++)
					pet->perm_stat[stat] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'C':
			KEY("Clan", pet->clan, fread_strkey(
			    fp, &clans, "fread_pet"));		// notrans
			KEY("Comm", pet->comm, fread_flags(fp));
			KEY("Chan", pet->chan, fread_flags(fp));
			break;

		case 'D':
			MLSKEY("Desc", pet->description);
			if (IS_TOKEN(fp, "Damr")) {
				pet->damroll = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (IS_TOKEN(fp, "Damd")) {
				NPC(pet)->dam.dice_number = fread_number(fp);
				NPC(pet)->dam.dice_type = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'E':
			if (IS_TOKEN(fp, "End")) {
				pet->leader = ch;
				pet->master = ch;
				SET_BIT(pet->affected_by, AFF_CHARM);
				PC(ch)->pet = pet;
				return;
			}
			break;

		case 'G':
			KEY("Gold", pet->gold, fread_number(fp));
			break;

		case 'H':
			if (IS_TOKEN(fp, "HMV")) {
				pet->hit = fread_number(fp);
				pet->max_hit = pet->perm_hit = fread_number(fp);
				pet->mana = fread_number(fp);
				pet->max_mana = pet->perm_mana = fread_number(fp);
				pet->move = fread_number(fp);
				pet->max_move = pet->perm_move = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'L':
			MLSKEY("LnD",  pet->description);
			KEY("Levl", pet->level, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", pet->name, fread_string(fp));
			break;

		case 'P':
			KEY("Pos", pet->position, fread_number(fp));
			break;

		case 'S':
			if (IS_TOKEN(fp, "Sex")) {
				mlstr_destroy(&pet->gender);
				mlstr_init2(&pet->gender,
					   flag_string(gender_table, fread_number(fp)));
				fMatch = TRUE;
			}
			MLSKEY("SSex", pet->gender);
			MLSKEY("ShD", pet->short_descr);
			KEY("Silv", pet->silver, fread_number(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_INFO, "fread_pet: %s: %s: no match",
			    ch->name, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

static void
fread_obj(CHAR_DATA *ch, CHAR_DATA *obj_to, rfile_t *fp, int flags)
{
	OBJ_DATA       *obj = NULL;
	int             iNest;

	if (obj_to == NULL) {
		log(LOG_INFO, "fread_obj: %s: obj_to == NULL", ch->name);
		fread_to_end(fp);
		return;
	}

	fread_keyword(fp);
	if (IS_TOKEN(fp, "Vnum")) {
		int vnum;

		vnum = fread_number(fp);
		if (IS_SET(flags, LOAD_F_MOVE))
			MOVE(vnum);

		if ((obj = create_obj(vnum, CO_F_NOCOUNT)) == NULL) {
			log(LOG_INFO, "fread_obj: %s: vnum %d: no such object",
			    ch->name, vnum);
			fread_to_end(fp);
			return;
		}
	} else {
		log(LOG_INFO, "fread_obj: %s: no obj vnum in file", ch->name);
		fread_to_end(fp);
		return;
	}

	iNest = 0;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			if (IS_TOKEN(fp, "Affc")) {
				AFFECT_DATA *paf = aff_fread(fp, TRUE);
				SLIST_ADD(AFFECT_DATA, obj->affected, paf);
				fMatch = TRUE;
				break;
			}
			break;

		case 'C':
			KEY("Cond", obj->condition, fread_number(fp));
			KEY("Cost", obj->cost, fread_number(fp));
			break;

		case 'D':
			MLSKEY("Desc", obj->description);
			break;

		case 'E':
			KEY("ExtF", obj->stat_flags, fread_number(fp));

			if (IS_TOKEN(fp, "ExDe")) {
				ed_fread(fp, &obj->ed);
				fMatch = TRUE;
				break;
			}

			if (IS_TOKEN(fp, "End")) {
				if (OBJ_IS(obj, OBJ_QUEST)
				&&  mlstr_null(&obj->owner)) {
					mlstr_cpy(&obj->owner, &ch->short_descr);
				}

				if (iNest == 0 || rgObjNest[iNest] == NULL) {
					obj_to_char(obj, obj_to);
					if (obj->wear_loc != WEAR_NONE)
						_equip_char(obj_to, obj);
				} else
					obj_to_obj(obj, rgObjNest[iNest - 1]);
				return;
			}
			break;

		case 'I':
			KEY("ItemType", obj->item_type,
			    fread_fword(item_types, fp));
			break;

		case 'L':
			KEY("Lev", obj->level, fread_number(fp));
			SKEY("Label", obj->label, fread_string(fp));
			break;

		case 'N':
			if (IS_TOKEN(fp, "Nest")) {
				iNest = fread_number(fp);
				if (iNest < 0 || iNest >= MAX_NEST) {
					log(LOG_INFO, "fread_obj: %s: bad nest %d",
						   ch->name, iNest);
				} else {
					rgObjNest[iNest] = obj;
				}
				fMatch = TRUE;
			}
			break;

		case 'O':
			MLSKEY("Owner", obj->owner);
			break;

		case 'S':
			KEY("StatF", obj->stat_flags, fread_number(fp));
			MLSKEY("ShD", obj->short_descr);

			if (IS_TOKEN(fp, "Spell")) {
				int iValue = fread_number(fp);
				const char *sn = fread_strkey(
				    fp, &skills, "fread_obj");	// notrans

				if (iValue < 0 || iValue > 3) {
					log(LOG_INFO, "fread_obj: %s: %d: bad iValue", ch->name, iValue);
					free_string(sn);
				} else
					STR_ASSIGN(obj->value[iValue], sn);
				fMatch = TRUE;
				break;
			}
			break;

		case 'T':
			KEY("Time", obj->timer, fread_number(fp));
			break;

		case 'V':
			if (IS_TOKEN(fp, "Values")) {
				fread_objval(obj->item_type, obj->value, fp);
				if (obj->item_type == ITEM_WEAPON
				&&  INT(obj->value[0]) == 0)
					obj->value[0] = obj->pObjIndex->value[0];
				fMatch = TRUE;
			}
			break;

		case 'W':
			KEY("Wear", obj->wear_loc, fread_number(fp));
			break;

		}

		if (!fMatch) {
			log(LOG_INFO, "fread_obj: %s: %s: no match",
			    ch->name, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

void
char_nuke(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	PC_DATA *pc;

	if (!IS_NPC(ch) && (pc = PC(ch))->pet != NULL) {
		char_nuke(pc->pet);
		pc->pet = NULL;
	}

	for (obj = ch->carrying; obj; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj, XO_F_NOCOUNT);
	}

	char_free(ch);
}

/*
 * char_delete -- delete player, update clan lists if necessary
 *		    if !msg then the player is deleted due to
 *		    low con or high number of deaths. this msg is logged
 *		    victim is assumed to be !IS_NPC
 */
void
char_delete(CHAR_DATA *victim, const char* msg)
{
	bool touched;
	AREA_DATA *pArea;
	char *name;
	clan_t *clan;

	if (msg) {
		act_char("You became a ghost permanently and leave the earth realm.", victim);
		act("$n is dead, and will not rise again.",
		    victim, NULL, NULL, TO_ROOM);
		victim->hit = 1;
		victim->position = POS_STANDING;
		wiznet("$N is deleted due to $t.", victim, msg, 0, 0, 0);
	}

	/*
	 * remove char from clan lists
	 */
	if ((clan = clan_lookup(victim->clan)) != NULL) {
		clan_update_lists(clan->name, victim, TRUE);
		clan_save(clan->name);
	}

	/*
	 * remove char from builder's lists
	 */
	touched = FALSE;
	for (pArea = area_first; pArea != NULL; pArea = pArea->next) {
		if (!is_name_strict(victim->name, pArea->builders))
			continue;
		name_delete(&pArea->builders, victim->name, NULL, NULL);
		SET_BIT(pArea->area_flags, AREA_CHANGED);
		touched = TRUE;
	}

	if (touched)
		dofun("asave", NULL, "changed");		// notrans

	RESET_FIGHT_TIME(victim);
	name = capitalize(victim->name);
	quit_char(victim, 0);
	dunlink(PLAYER_PATH, name);
}

void
move_pfiles(int minvnum, int maxvnum, int delta)
{
	struct dirent *dp;
	DIR *dirp;

	if ((dirp = opendir(PLAYER_PATH)) == NULL) {
		log(LOG_ERROR, "move_pfiles: unable to open player directory");
		return;
	}

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

#if defined (LINUX) || defined (WIN32)
		if (strlen(dp->d_name) < 3)
			continue;
#else
		if (dp->d_namlen < 3 || dp->d_type != DT_REG)
			continue;
#endif

		if (strchr(dp->d_name, '.'))
			continue;

		move_pfile(dp->d_name, minvnum, maxvnum, delta);
	}
	closedir(dirp);

}

static void
move_pfile(const char *name, int minvnum, int maxvnum, int delta)
{
	CHAR_DATA *ch;
	minv = minvnum; maxv = maxvnum; del = delta;
	ch = char_load(name, LOAD_F_MOVE | LOAD_F_NOCREATE);
	char_save(ch, SAVE_F_PSCAN);
	char_nuke(ch);
}
