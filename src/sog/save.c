/*
 * $Id: save.c,v 1.126.2.20 2003-09-11 10:39:48 tatyana Exp $
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
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>

#ifndef BSD44
#include <malloc.h>
#endif

#include "merc.h"
#include "quest.h"
#include "db.h"
#include "bm.h"

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];
CHAR_DATA *obj_to;

/*
 * global vars for areaed_move()
 */
int minv, maxv, del;

/*
 * Local functions.
 */
void fwrite_char (CHAR_DATA * ch, FILE * fp, int flags);
void fwrite_obj (CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest);
void fwrite_pet (CHAR_DATA * pet, FILE * fp, int flags);
void fwrite_affect(AFFECT_DATA *paf, FILE *fp);
void fread_char (CHAR_DATA * ch, FILE * fp, int flags);
void fread_pet  (CHAR_DATA * ch, FILE * fp, int flags);
void fread_obj  (CHAR_DATA * ch, FILE * fp, int flags);

/*
 * move_pfile - shifts vnum in range minvnum..maxvnum by delta)
 */
#define IN_RANGE(i, l, u)	((l) <= (i) && (i) <= (u))
#define MOVE(i)		if (IN_RANGE(i, minv, maxv)) i += del

void move_pfile(const char *name, int minvnum, int maxvnum, int delta)
{
	CHAR_DATA *ch;
	minv = minvnum; maxv = maxvnum; del = delta;
	ch = char_load(name, LOAD_F_MOVE | LOAD_F_NOCREATE);
	char_save(ch, SAVE_F_PSCAN);
	char_nuke(ch);
}

/*
 * delete_player -- delete player, update clan lists if necessary
 *		    if !msg then the player is deleted due to
 *		    low con or high number of deaths. this msg is logged
 *		    victim is assumed to be !IS_NPC
 */
void delete_player(CHAR_DATA *victim, char* msg)
{
	clan_t *clan;
	char *name;

	if (msg) {
		char_puts("You became a ghost permanently "
			  "and leave the earth realm.\n", victim);
		act("$n is dead, and will not rise again.\n",
		    victim, NULL, NULL, TO_ROOM);
		victim->hit = 1;
		victim->position = POS_STANDING;
		wiznet("$N is deleted due to $t.", victim, msg, 0, 0, 0);
	}

	/*
	 * remove char from clan lists
	 */
	if (victim->clan && (clan = clan_lookup(victim->clan))) {
		clan_update_lists(clan, victim, TRUE);
		clan_save(clan);
	}

	RESET_FIGHT_TIME(victim);
	name = capitalize(victim->name);
	quit_char(victim, 0);
	dunlink(PLAYER_PATH, name);
}

void char_nuke(CHAR_DATA *ch)
{
	PC_DATA *pc = PC(ch);

	if (pc->pet) {
		char_free(pc->pet); 
		pc->pet = NULL;
	}

	char_free(ch);
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void char_save(CHAR_DATA *ch, int flags)
{
	FILE           *fp;
	const char 	*name;
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

	if (IS_SET(flags, SAVE_F_PSCAN)) {
		/*
		 * restore atime/mtime
		 */
		struct stat s;
		struct timeval tv[2];
		char fname[PATH_MAX];

		if (dstat(PLAYER_PATH, name, &s) < 0) {
			log("char_save: %s%c%s: stat: %s",
			    PLAYER_PATH, PATH_SEPARATOR, name, strerror(errno));
			goto err;
		}

		tv[0].tv_sec = s.st_atime;
		tv[0].tv_usec = 0;
		tv[1].tv_sec = s.st_mtime;
		tv[1].tv_usec = 0;

		snprintf(fname, sizeof(fname), "%s%c%s",
			 PLAYER_PATH, PATH_SEPARATOR, TMP_FILE);
		if (utimes(fname, tv) < 0) {
			log("char_save: %s: utimes: %s",
			    fname, strerror(errno));
			goto err;
		}
	}

err:
	d2rename(PLAYER_PATH, TMP_FILE, PLAYER_PATH, name);
}

/*
 * Write the char.
 */
void 
fwrite_char(CHAR_DATA *ch, FILE *fp, int flags)
{
	AFFECT_DATA    *paf;

	fprintf(fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER");

	fprintf(fp, "Vers %d\n", PFILE_VERSION);
	fwrite_string(fp, "Name", ch->name);
	mlstr_fwrite(fp, "ShD", &ch->short_descr);
	fprintf(fp, "Ethos %s\n", flag_string(ethos_table, ch->ethos));

	if (ch->clan) {
		fwrite_string(fp, "Clan", clan_name(ch->clan));
	}

	fwrite_string(fp, "Desc", mlstr_mval(&ch->description));

	fprintf(fp, "Sex %d\n", ch->sex);
	fprintf(fp, "Class '%s'\n", class_name(ch));
	fprintf(fp, "Levl %d\n", ch->level);
	fprintf(fp, "Room %d\n",
		(ch->in_room == get_room_index(ROOM_VNUM_LIMBO) &&
		 !IS_NPC(ch) && PC(ch)->was_in_room != NULL) ?
			PC(ch)->was_in_room->vnum : ch->in_room == NULL ?
			3001 : ch->in_room->vnum);

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
	if (PC(ch)->plr_flags)
		fprintf(fp, "Act %s\n", format_flags(PC(ch)->plr_flags));
	if (ch->comm)
		fprintf(fp, "Comm %s\n", format_flags(ch->comm));
	if (ch->invis_level)
		fprintf(fp, "Invi %d\n", ch->invis_level);
	if (ch->incog_level)
		fprintf(fp, "Inco %d\n", ch->incog_level);
	fprintf(fp, "Pos %d\n",
		ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
	fprintf(fp, "Alig  %d\n", ch->alignment);
	if (ch->wimpy != 0)
		fprintf(fp, "Wimp %d\n", ch->wimpy);
	fprintf(fp, "Attr %d %d %d %d %d %d\n",
		ch->perm_stat[STAT_STR],
		ch->perm_stat[STAT_INT],
		ch->perm_stat[STAT_WIS],
		ch->perm_stat[STAT_DEX],
		ch->perm_stat[STAT_CON],
		ch->perm_stat[STAT_CHA]);

	if (IS_NPC(ch)) {
		fprintf(fp, "Vnum %d\n", ch->pMobIndex->vnum);
	} else {
		PC_DATA *pc = PC(ch);
		int i;

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

		if (ch->clan)
			fprintf(fp, "ClanStatus %d\n", pc->clan_status);
		else if (pc->petition)
			fwrite_string(fp, "Peti",
				      clan_name(pc->petition));

		fprintf(fp, "Race '%s'\n", race_name(pc->race));
		fprintf(fp, "Relig %d\n", pc->religion);
		if (pc->practice != 0)
			fprintf(fp, "Prac %d\n", pc->practice);
		if (pc->train != 0)
			fprintf(fp, "Trai %d\n", pc->train);
		fprintf(fp, "Exp %d\n", PC(ch)->exp);
		fprintf(fp, "ExpTL %d\n", PC(ch)->exp_tl);
		fwrite_string(fp, "Hometown", hometown_name(pc->hometown));
		fprintf(fp, "LogO %ld\n",
			IS_SET(flags, SAVE_F_PSCAN) ?
				pc->logoff : current_time);
		if (pc->wiznet)
			fprintf(fp, "Wizn %s\n", format_flags(pc->wiznet));

		if (pc->trust)
			fprintf(fp, "Trust %s\n", format_flags(pc->trust));

		if (pc->plevels > 0)
			fprintf(fp, "PLev %d\n", pc->plevels);
		fprintf(fp, "Plyd %d\n",
			pc->played + (int) (current_time - pc->logon));
		fprintf(fp, "Not %ld %ld %ld %ld %ld\n",
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

		for (i = 0; i < pc->learned.nused; i++) {
			pcskill_t *ps = VARR_GET(&pc->learned, i);

			if (ps->percent == 0)
				continue;

			fprintf(fp, "Sk %d '%s'\n",
				ps->percent, skill_name(ps->sn));
		}

		if (pc->questpoints != 0)
			fprintf(fp, "QuestPnts %d\n", pc->questpoints);
		if (pc->questtime != 0 && !IS_SET(flags, SAVE_F_REBOOT))
			fprintf(fp, "QuestTime %d\n", number_range(12, 15));
		fprintf(fp, "Haskilled %d\n", pc->has_killed);
		fprintf(fp, "Antkilled %d\n", pc->anti_killed);
		fwrite_string(fp, "Twitlist", pc->twitlist);
		fwrite_string(fp, "Granted", pc->granted);
		fwrite_string(fp, "LLHost", pc->ll_host);
		fwrite_string(fp, "LLIp", pc->ll_ip);
		fprintf(fp, "LLTime %ld\n", pc->ll_time);
	}

	for (paf = ch->affected; paf != NULL; paf = paf->next) {
		if (!IS_NPC(ch) && paf->where == TO_AFFECTS
		&&  IS_SET(paf->bitvector, AFF_CHARM))
			continue;

		fwrite_affect(paf, fp);
	}

	fprintf(fp, "End\n\n");
}

/* write a pet */
void 
fwrite_pet(CHAR_DATA * pet, FILE * fp, int flags)
{
	AFFECT_DATA    *paf;
	fprintf(fp, "#PET\n");

	fprintf(fp, "Vnum %d\n", pet->pMobIndex->vnum);
	fwrite_string(fp, "Name", pet->name);
	if (pet->clan)
		fwrite_string(fp, "Clan", clan_name(pet->clan));
	if (mlstr_cmp(&pet->short_descr, &pet->pMobIndex->short_descr) != 0)
		mlstr_fwrite(fp, "ShD", &pet->short_descr);
	if (mlstr_cmp(&pet->long_descr, &pet->pMobIndex->long_descr) != 0)
		mlstr_fwrite(fp, "LnD", &pet->short_descr);
	if (mlstr_cmp(&pet->description, &pet->pMobIndex->description) != 0)
		mlstr_fwrite(fp, "Desc", &pet->short_descr);
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
	fprintf(fp, "Pos %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
	for (paf = pet->affected; paf != NULL; paf = paf->next)
		fwrite_affect(paf, fp);

	fprintf(fp, "End\n\n");
}

/*
 * Write an object and its contents.
 */
void 
fwrite_obj(CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest)
{
	ED_DATA *ed;
	AFFECT_DATA    *paf;

	/*
	 * Slick recursion to write lists backwards, so loading them will load
	 * in forwards order.
	 */
	if (obj->next_content != NULL)
		fwrite_obj(ch, obj->next_content, fp, iNest);

	/*
	 * Castrate storage characters.
	 * Leave stuck-in objects, containers even if their level is high
	 */
	if (!IS_IMMORTAL(ch) && obj->wear_loc != WEAR_STUCK_IN) {
		if ((get_wear_level(ch, obj) < obj->level &&
		     obj->pObjIndex->item_type != ITEM_CONTAINER)
		||  (IS_SET(obj->pObjIndex->extra_flags, ITEM_QUEST) &&
		     ch->level < obj->pObjIndex->level)) {
			extract_obj(obj, XO_F_NORECURSE);
			return;
		}
	}

/* do not save named quest rewards if ch is not owner */
	if (!IS_IMMORTAL(ch)
	&&  IS_SET(obj->pObjIndex->extra_flags, ITEM_QUEST)
	&&  !IS_OWNER(ch, obj)) {
		log("fwrite_obj: %s: '%s' of %s",
			   ch->name, obj->name,
			   mlstr_mval(&obj->owner));
		act("$p vanishes!", ch, obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
		return;
	}
	
	if (IS_SET(obj->pObjIndex->extra_flags,
		   ITEM_CLAN | ITEM_QUIT_DROP | ITEM_CHQUEST))
		return;

	fprintf(fp, "#O\n");
	fprintf(fp, "Vnum %d\n", obj->pObjIndex->vnum);
	fprintf(fp, "Cond %d\n", obj->condition);

	fprintf(fp, "Nest %d\n", iNest);
	mlstr_fwrite(fp, "Owner", &obj->owner);

	if (obj->pObjIndex->limit < 0) {
		if (str_cmp(obj->name, obj->pObjIndex->name))
			fwrite_string(fp, "Name", obj->name);
		if (mlstr_cmp(&obj->short_descr, &obj->pObjIndex->short_descr))
			mlstr_fwrite(fp, "ShD", &obj->short_descr);
		if (mlstr_cmp(&obj->description, &obj->pObjIndex->description))
			mlstr_fwrite(fp, "Desc", &obj->description);
	}

	if (obj->extra_flags != obj->pObjIndex->extra_flags)
		fprintf(fp, "ExtF %s\n", format_flags(obj->extra_flags));

	fprintf(fp, "Wear %d\n", obj->wear_loc);
	if (obj->level != obj->pObjIndex->level)
		fprintf(fp, "Lev %d\n", obj->level);
	if (obj->timer != 0)
		fprintf(fp, "Time %d\n", obj->timer);
	fprintf(fp, "Cost %d\n", obj->cost);
	if (obj->value[0] != obj->pObjIndex->value[0]
	    || obj->value[1] != obj->pObjIndex->value[1]
	    || obj->value[2] != obj->pObjIndex->value[2]
	    || obj->value[3] != obj->pObjIndex->value[3]
	    || obj->value[4] != obj->pObjIndex->value[4])
		fprintf(fp, "Val %d %d %d %d %d\n",
		  obj->value[0], obj->value[1], obj->value[2], obj->value[3],
			obj->value[4]);

	switch (obj->pObjIndex->item_type) {
	case ITEM_POTION:
	case ITEM_SCROLL:
		if (obj->value[1] > 0) {
			fprintf(fp, "Spell 1 '%s'\n",
				skill_name(obj->value[1]));
		}
		if (obj->value[2] > 0) {
			fprintf(fp, "Spell 2 '%s'\n",
				skill_name(obj->value[2]));
		}
		if (obj->value[3] > 0) {
			fprintf(fp, "Spell 3 '%s'\n",
				skill_name(obj->value[3]));
		}
		break;

	case ITEM_PILL:
	case ITEM_STAFF:
	case ITEM_WAND:
		if (obj->value[3] > 0) {
			fprintf(fp, "Spell 3 '%s'\n",
				skill_name(obj->value[3]));
		}
		break;
	}

	for (paf = obj->affected; paf != NULL; paf = paf->next)
		fwrite_affect(paf, fp);

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
CHAR_DATA *char_load(const char *name, int flags)
{
	CHAR_DATA      *ch;
	FILE           *fp = NULL;
	bool		found;

	int             iNest;

	name = capitalize(name);
	snprintf(filename, sizeof(filename), "%s.gz", name);
	if (dfexist(PLAYER_PATH, filename)) {
		char buf[PATH_MAX * 2];
		snprintf(buf, sizeof(buf), "gzip -dfq %s%c%s",
			 PLAYER_PATH, PATH_SEPARATOR, filename);
		system(buf);
	}

	found = (dfexist(PLAYER_PATH, name) &&
		 (fp = dfopen(PLAYER_PATH, name, "r")) != NULL);

	if (!found && IS_SET(flags, LOAD_F_NOCREATE))
		return NULL;

	ch = char_new(NULL);

	ch->name = str_dup(capitalize(name));
	mlstr_init(&ch->short_descr, ch->name);

	if (!found) {
		PC(ch)->plr_flags |= PLR_NEW;
		return ch;
	}

	for (iNest = 0; iNest < MAX_NEST; iNest++)
		rgObjNest[iNest] = NULL;

	for (;;) {
		char	letter;
		char *	word;
		letter = fread_letter(fp);
		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}
		if (letter != '#') {
			log("char_load: %s: # not found.", ch->name);
			break;
		}
		word = fread_word(fp);
		if (!str_cmp(word, "PLAYER")) {
			fread_char(ch, fp, flags);
			obj_to = ch;
		}
		else if (!str_cmp(word, "OBJECT"))
			fread_obj(ch, fp, flags);
		else if (!str_cmp(word, "O"))
			fread_obj(ch, fp, flags);
		else if (!str_cmp(word, "PET")) {
			fread_pet(ch, fp, flags);
			if (GET_PET(ch))
				obj_to = GET_PET(ch);
		}
		else if (!str_cmp(word, "END"))
			break;
		else {
			log("char_load: %s: %s: bad section.", 
				   ch->name, word);
			break;
		}
	}
	fclose(fp);

	return ch;
}

/*
 * Read in a char.
 */
void 
fread_char(CHAR_DATA * ch, FILE * fp, int flags)
{
	int count = 0;

	for (;;) {
		bool fMatch = FALSE;
		const char *word = feof(fp) ? "End" : fread_word(fp);

		switch (UPPER(word[0])) {
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

			if (!str_cmp(word, "Alia")
			||  !str_cmp(word, "Alias")) {
				if (count >= MAX_ALIAS) {
					fread_to_eol(fp);
					fMatch = TRUE;
					break;
				}
				PC(ch)->dvdata->alias[count] =
						str_dup(fread_word(fp));
				PC(ch)->dvdata->alias_sub[count] =
						fread_string(fp);
				count++;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA af;
				const char *skname = fread_word(fp);
				int sn = sn_lookup(skname);
				if (sn < 0)
					log("fread_char: %s: %s: unknown skill", ch->name, skname);

				af.type = sn;
				af.where = fread_number(fp);
				af.level = fread_number(fp);
				af.duration = fread_number(fp);
				af.modifier = fread_number(fp);
				af.location = fread_number(fp);
				af.bitvector = fread_flags(fp);

				affect_to_char(ch, &af);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Attr")) {
				int             stat;
				for (stat = 0; stat < MAX_STATS; stat++)
					ch->perm_stat[stat] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'B':
			KEY("Banks", PC(ch)->bank_s, fread_number(fp));
			KEY("Bankg", PC(ch)->bank_g, fread_number(fp));
			SKEY("Bin", PC(ch)->bamfin);
			SKEY("Bout", PC(ch)->bamfout);
			break;

		case 'C':
			KEY("ClassW", ch->class, cn_lookup(fread_word(fp)));
			KEY("Class", ch->class, cn_lookup(fread_word(fp)));
			KEY("Clan", ch->clan, fread_clan(fp));
			KEY("ClanStatus", PC(ch)->clan_status,
			    fread_number(fp));
			if (!str_cmp(word, "CndC")) {
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

			break;

		case 'D':
			MLSKEY("Desc", ch->description);
			KEY("Dead", PC(ch)->death, fread_number(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				clan_t *clan;
				const char **nl = NULL;
				bool touched = FALSE;

				if (PC(ch)->condition[COND_BLOODLUST] < 48
				&&  !IS_VAMPIRE(ch))
					PC(ch)->condition[COND_BLOODLUST] = 48;

				if (PC(ch)->dvdata->pagelen < MIN_PAGELEN-2)
					PC(ch)->dvdata->pagelen = MAX_PAGELEN-2;

				/* XXX update clan lists */
				if (!ch->clan
				||  (clan = clan_lookup(ch->clan)) == NULL)
					return;

				switch (PC(ch)->clan_status) {
				case CLAN_INACTIVE:
					touched = !name_add(&clan->inactive_list, ch->name,
							    NULL, NULL);
					break;
				default:
					touched = !name_add(&clan->member_list, ch->name,
							    NULL, NULL);
					break;
				}

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
					clan_save(clan);
				return;
			}
			KEY("Exp", PC(ch)->exp, fread_number(fp));
			KEY("ExpTL", PC(ch)->exp_tl, fread_number(fp));
			KEY("Ethos", ch->ethos, fread_fword(ethos_table, fp));
			break;

		case 'F':
			SKEY("FormName", PC(ch)->form_name);
			break;

		case 'G':
			KEY("Gold", ch->gold, fread_number(fp));
			SKEY("Granted", PC(ch)->granted);
			break;

		case 'H':
			if (!str_cmp(word, "Hometown")) {
				const char *s = fread_string(fp);
				PC(ch)->hometown = htn_lookup(s);
				free_string(s);
				fMatch = TRUE;
			}

			KEY("Home", PC(ch)->hometown, fread_number(fp));
			KEY("Haskilled", PC(ch)->has_killed,
			    fread_number(fp));
			if (!str_cmp(word, "Homepoint")) {
				int room = fread_number(fp);

				if (IS_SET(flags, LOAD_F_MOVE))
					MOVE(room);
				PC(ch)->homepoint = get_room_index(room);
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "HMV")) {
				ch->hit = fread_number(fp);
				ch->max_hit = ch->perm_hit = fread_number(fp);
				ch->mana = fread_number(fp);
				ch->max_mana = ch->perm_mana = fread_number(fp);
				ch->move = fread_number(fp);
				ch->max_move = ch->perm_move = fread_number(fp);
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "HMVP")) {
				ch->max_hit = ch->perm_hit = fread_number(fp);
				ch->max_mana = ch->perm_mana = fread_number(fp);
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
			SKEY("LLHost", PC(ch)->ll_host);
			SKEY("LLIp", PC(ch)->ll_ip);
			KEY("LLTime", PC(ch)->ll_time,
			    fread_number(fp));
			break;

		case 'N':
			SKEY("Name", ch->name);
			if (!str_cmp(word, "Not")) {
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
			if (!str_cmp(word, "OrgRaceW")) {
				/*
				 * old pfiles compatibility
				 */
				ch->race = rn_lookup(fread_word(fp));
				PC(ch)->race = ch->race;
				race_setstats(ch, ch->race);
				fMatch = TRUE;
			}
			break;
		case 'P':
			KEY("Peti", PC(ch)->petition, fread_clan(fp));
			KEY("PLev", PC(ch)->plevels, fread_number(fp));
			SKEY("Password", PC(ch)->pwd);
			SKEY("Pass", PC(ch)->pwd);
			KEY("PC_Killed", PC(ch)->pc_killed, fread_number(fp));
			KEY("Plyd", PC(ch)->played, fread_number(fp));
			KEY("Pos", ch->position, fread_number(fp));
			KEY("Prac", PC(ch)->practice, fread_number(fp));
			SKEY("Prom", PC(ch)->dvdata->prompt);
			break;

		case 'Q':
			KEY("QuestTime", PC(ch)->questtime, fread_number(fp));
			KEY("QuestPnts", PC(ch)->questpoints, fread_number(fp));
			break;

		case 'R':
			KEY("Relig", PC(ch)->religion, fread_number(fp));
			if (!str_cmp(word, "RaceW")
			||  !str_cmp(word, "Race")) {
				ch->race = rn_lookup(fread_word(fp));
				PC(ch)->race = ch->race;
				race_setstats(ch, ch->race);
				fMatch = TRUE;
			}
			if (!str_cmp(word, "Room")) {
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
			KEY("Sex", ch->sex, fread_number(fp));
			MLSKEY("ShD", ch->short_descr);
			KEY("Sec", PC(ch)->security, fread_number(fp));
			KEY("Silv", ch->silver, fread_number(fp));

			if (!str_cmp(word, "Sk")) {
				int value = fread_number(fp);
				char *skname = fread_word(fp);
				int sn = sn_lookup(skname);
				if (sn < 0)
					log("fread_char: %s: %s: unknown skill", ch->name, skname);
				else
					set_skill(ch, sn, value);
				fMatch = TRUE;
			}
			break;

		case 'T':
			KEY("Trai", PC(ch)->train, fread_number(fp));
			KEY("Trust", PC(ch)->trust, fread_flags(fp));
			SKEY("Twitlist", PC(ch)->twitlist);
			if (!str_cmp(word, "Title") || !str_cmp(word, "Titl")) {
				const char *p = fread_string(fp);
				set_title(ch, p);
				free_string(p);
				fMatch = TRUE;
				break;
			}
			break;

		case 'V':
			KEY("Vers", PC(ch)->version, fread_number(fp));
			break;

		case 'W':
			KEY("Wimp", ch->wimpy, fread_number(fp));
			KEY("Wizn", PC(ch)->wiznet, fread_flags(fp));
			SKEY("WantedBy", PC(ch)->wanted_by);
			break;
		}

		if (!fMatch) {
			log("fread_char: %s: %s: no match",
				   ch->name, word);
			fread_to_eol(fp);
		}
	}
}

/* load a pet from the forgotten reaches */
void 
fread_pet(CHAR_DATA * ch, FILE * fp, int flags)
{
	char           *word;
	CHAR_DATA      *pet = NULL;

	/* first entry had BETTER be the vnum or we barf */
	word = feof(fp) ? "END" : fread_word(fp);
	if (!str_cmp(word, "Vnum")) {
		MOB_INDEX_DATA *pMobIndex;
		int vnum = fread_number(fp);

		if (IS_SET(flags, LOAD_F_MOVE))
			MOVE(vnum);
		
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
				pet = create_mob(pMobIndex, CM_F_NOLIST);
	}

	if (pet == NULL) {
		log("fread_pet: %s: no pet vnum or bad vnum in file", ch->name);
		fread_to_end(fp);
		return;
	}

	for (;;) {
		bool fMatch = FALSE;
		word = feof(fp) ? "END" : fread_word(fp);

		switch (UPPER(word[0])) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':

			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA af;
				const char *skname = fread_word(fp);
				int sn = sn_lookup(skname);
				if (sn < 0)
					log("fread_pet: %s: %s: unknown skill", ch->name, skname);

				af.type = sn;
				af.where = fread_number(fp);
				af.level = fread_number(fp);
				af.duration = fread_number(fp);
				af.modifier = fread_number(fp);
				af.location = fread_number(fp);
				af.bitvector = fread_flags(fp);

				affect_to_char(pet, &af);
				fMatch = TRUE;
				break;
			}
			break;

		case 'C':
			KEY("Clan", pet->clan, fread_clan(fp));
			KEY("Comm", pet->comm, fread_flags(fp));
			break;

		case 'D':
			MLSKEY("Desc", pet->description);
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
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
			if (!str_cmp(word, "HMV")) {
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
			break;

		case 'N':
			SKEY("Name", pet->name);
			break;

		case 'P':
			KEY("Pos", pet->position, fread_number(fp));
			break;

		case 'S':
			MLSKEY("ShD", pet->short_descr);
			KEY("Silv", pet->silver, fread_number(fp));
			break;
		}

		if (!fMatch) {
			log("fread_pet: %s: %s: no match",
				   ch->name, word);
			fread_to_eol(fp);
		}
	}
}

void 
fread_obj(CHAR_DATA * ch, FILE * fp, int flags)
{
	OBJ_DATA       *obj = NULL;
	char           *word;
	int             iNest;
	bool            fNest;

	word = feof(fp) ? "End" : fread_word(fp);
	if (!str_cmp(word, "Vnum")) {
		OBJ_INDEX_DATA *pObjIndex;
		int vnum = fread_number(fp);

		if (IS_SET(flags, LOAD_F_MOVE))
			MOVE(vnum);

		if ((pObjIndex = get_obj_index(vnum)) != NULL)
			obj = create_obj(pObjIndex, CO_F_NOCOUNT);
	}

	if (obj == NULL) {
		log("fread_obj: %s: no obj vnum or bad vnum in file", ch->name);
		fread_to_end(fp);
		return;
	}

	fNest = FALSE;
	iNest = 0;

	for (;;) {
		bool fMatch = FALSE;
		word = feof(fp) ? "End" : fread_word(fp);

		switch (UPPER(word[0])) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA *paf = aff_new();
				const char *skname = fread_word(fp);
				int sn = sn_lookup(skname);
				if (sn < 0)
					log("fread_obj: %s: %s: unknown skill", ch->name, skname);

				paf->type = sn;
				paf->where = fread_number(fp);
				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_flags(fp);

				if (sn < 0)
					aff_free(paf);
				else 
					SLIST_ADD(AFFECT_DATA,
						  obj->affected, paf);

				fMatch = TRUE;
				break;
			}
			break;

		case 'C':
			KEY("Cond", obj->condition, fread_number(fp));
			KEY("Cost", obj->cost, fread_number(fp));
			break;

		case 'D':
			MLSKEY("Description", obj->description);
			MLSKEY("Desc", obj->description);
			break;

		case 'E':
			KEY("ExtF", obj->extra_flags, fread_flags(fp));

			if (!str_cmp(word, "ExDe")) {
				ed_fread(fp, &obj->ed);
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "End")) {
				if (IS_SET(obj->pObjIndex->extra_flags,
					   ITEM_QUEST)
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

		case 'L':
			KEY("Lev", obj->level, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", obj->name);

			if (!str_cmp(word, "Nest")) {
				iNest = fread_number(fp);
				if (iNest < 0 || iNest >= MAX_NEST) {
					log("fread_obj: %s: bad nest %d",
						   ch->name, iNest);
				} else {
					rgObjNest[iNest] = obj;
					fNest = TRUE;
				}
				fMatch = TRUE;
			}
			break;

		case 'O':
			MLSKEY("Owner", obj->owner);
			break;

		case 'S':
			MLSKEY("ShD", obj->short_descr);

			if (!str_cmp(word, "Spell")) {
				int iValue = fread_number(fp);
				const char *skname = fread_word(fp);
				int sn = sn_lookup(skname);

				if (iValue < 0 || iValue > 3)
					log("fread_obj: %s: %d: bad iValue", ch->name, iValue);
				else if (sn < 0)
					log("fread_obj: %s: %s: unknown skill", ch->name, skname);
				else
					obj->value[iValue] = sn;

				fMatch = TRUE;
				break;
			}
			break;

		case 'T':
			KEY("Time", obj->timer, fread_number(fp));
			break;

		case 'V':
			if (!str_cmp(word, "Val")) {
				obj->value[0] = fread_number(fp);
				obj->value[1] = fread_number(fp);
				obj->value[2] = fread_number(fp);
				obj->value[3] = fread_number(fp);
				obj->value[4] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'W':
			KEY("Wear", obj->wear_loc, fread_number(fp));
			break;
		}

		if (!fMatch) {
			log("fread_obj: %s: %s: no match",
				   ch->name, word);
			fread_to_eol(fp);
		}
	}
}

void fwrite_affect(AFFECT_DATA *paf, FILE *fp)
{
	skill_t *sk;

	if (paf->type == gsn_doppelganger
	||  (sk = skill_lookup(paf->type)) == NULL)
		return;

	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %s\n",
		sk->name,
		paf->where, paf->level, paf->duration, paf->modifier,
		paf->location, format_flags(paf->bitvector));
}

void save_black_market()
{
	FILE		*fp;
	bmitem_t	*item;
	ED_DATA		*ed;
	AFFECT_DATA	*paf;

	if ((fp = dfopen(ETC_PATH, BLACK_MARKET_CONF, "w")) == NULL)
		return;

	for (item = bmitem_list; item != NULL && item->obj != NULL; item = item->next) {
		fprintf(fp, "#ITEM\n");
		fprintf(fp, "Vnum %d\n", item->obj->pObjIndex->vnum);
		fprintf(fp, "Cond %d\n", item->obj->condition);
		mlstr_fwrite(fp, "Owner", &item->obj->owner);
		if (item->obj->extra_flags != item->obj->pObjIndex->extra_flags)
			fprintf(fp, "ExtF %s\n", format_flags(item->obj->extra_flags));
		if (item->obj->level != item->obj->pObjIndex->level)
			fprintf(fp, "Lev %d\n", item->obj->level);
		fprintf(fp, "Cost %d\n", item->obj->cost);
		if (item->obj->value[0] != item->obj->pObjIndex->value[0]
		||  item->obj->value[1] != item->obj->pObjIndex->value[1]
		||  item->obj->value[2] != item->obj->pObjIndex->value[2]
	        ||  item->obj->value[3] != item->obj->pObjIndex->value[3]
	        ||  item->obj->value[4] != item->obj->pObjIndex->value[4])
			fprintf(fp, "Val %d %d %d %d %d\n",
				item->obj->value[0], item->obj->value[1],
				item->obj->value[2], item->obj->value[3],
				item->obj->value[4]);

		switch (item->obj->pObjIndex->item_type) {
		case ITEM_POTION:
		case ITEM_SCROLL:
			if (item->obj->value[1] > 0) {
				fprintf(fp, "Spell 1 '%s'\n",
					skill_name(item->obj->value[1]));
			}
			if (item->obj->value[2] > 0) {
				fprintf(fp, "Spell 2 '%s'\n",
					skill_name(item->obj->value[2]));
			}
			if (item->obj->value[3] > 0) {
				fprintf(fp, "Spell 3 '%s'\n",
					skill_name(item->obj->value[3]));
			}
			break;

		case ITEM_PILL:
		case ITEM_STAFF:
		case ITEM_WAND:
			if (item->obj->value[3] > 0) {
				fprintf(fp, "Spell 3 '%s'\n",
					skill_name(item->obj->value[3]));
			}
			break;
		}

		for (paf = item->obj->affected; paf != NULL; paf = paf->next)
			fwrite_affect(paf, fp);

		for (ed = item->obj->ed; ed != NULL; ed = ed->next) {
			if (IS_NULLSTR(ed->keyword))
				continue;
			fwrite_string(fp, "ExDe", ed->keyword);
			mlstr_fwrite(fp, NULL, &ed->description);
		}
		fwrite_string(fp, "Seller", item->seller);
		fwrite_string(fp, "Buyer", item->buyer);
		fprintf(fp, "Bet %d\n", item->bet);
		fprintf(fp, "ItemTimer %d\n", item->timer);
		fprintf(fp, "End\n\n");
	}
	fprintf(fp, "#$\n");
	fclose(fp);
}
