/*
 * $Id: save.c,v 1.108 1999-03-17 15:27:37 kostik Exp $
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

#ifndef BSD44
#include <malloc.h>
#endif

#include "merc.h"
#include "quest.h"
#include "db/db.h"

DECLARE_DO_FUN(do_quit_count);

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

char DEFAULT_PROMPT[] = "%hhp %mm %vmv Opp:%o {c%e{x# ";
char OLD_DEFAULT_PROMPT[] = "<%n: {M%h{xhp {C%m{xm {W%v{xmv Opp:%o> ";

/*
 * Local functions.
 */
void fwrite_char (CHAR_DATA * ch, FILE * fp, bool reboot);
void fwrite_obj (CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest);
void fwrite_pet (CHAR_DATA * pet, FILE * fp);
void fwrite_affect(AFFECT_DATA *paf, FILE *fp);
void fread_char (CHAR_DATA * ch, FILE * fp);
void fread_pet  (CHAR_DATA * ch, FILE * fp);
void fread_obj  (CHAR_DATA * ch, FILE * fp);

/*
 * delete_player -- delete player, update clan lists if necessary
 *		    if !msg then the player is delete due to
 *		    low con or high number of deaths. this msg is logged
 */
void delete_player(CHAR_DATA *victim, char* msg)
{
	CLAN_DATA *clan;
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
	do_quit_count(victim, str_empty);
	dunlink(PLAYER_PATH, name);
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj(CHAR_DATA * ch, bool reboot)
{
	FILE           *fp;
	const char 	*name;
	if (IS_NPC(ch) || ch->level < 2)
		return;

	if (ch->desc != NULL && ch->desc->original != NULL)
		ch = ch->desc->original;

	name = capitalize(ch->name);

	/* create god log */
	if (IS_IMMORTAL(ch)) {
		if ((fp = dfopen(GODS_PATH, name, "w")) == NULL)
			return;
		fprintf(fp, "Lev %2d %s%s\n",
		      ch->level, ch->name, ch->pcdata->title);
		fclose(fp);
	}

	if ((fp = dfopen(PLAYER_PATH, TMP_FILE, "w")) == NULL)
		return;

	act_puts("Saving.", ch, NULL, NULL, TO_CHAR, POS_DEAD);
	fwrite_char(ch, fp, reboot);
	if (ch->carrying)
		fwrite_obj(ch, ch->carrying, fp, 0);

	/* save the pets */
	if (ch->pet && ch->pet->in_room == ch->in_room)
		fwrite_pet(ch->pet, fp);
	fprintf(fp, "#END\n");
	fclose(fp);
	d2rename(PLAYER_PATH, TMP_FILE, PLAYER_PATH, name);
}

/*
 * Write the char.
 */
void 
fwrite_char(CHAR_DATA * ch, FILE * fp, bool reboot)
{
	AFFECT_DATA    *paf;
	int		pos;

	fprintf(fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER");

	fwrite_string(fp, "Name", ch->name);
	mlstr_fwrite(fp, "ShD", ch->short_descr);
	fprintf(fp, "Id   %d\n", ch->id);
	fprintf(fp, "LogO %ld\n", current_time);
	fprintf(fp, "Vers %d\n", 6);
	fprintf(fp, "Ethos %s\n", flag_string(ethos_table, ch->ethos));
	fwrite_string(fp, "Hometown", hometown_name(ch->hometown));

	if (ch->clan) {
		fwrite_string(fp, "Clan", clan_name(ch->clan));
		if (!IS_NPC(ch))
			fprintf(fp, "ClanStatus %d\n", ch->pcdata->clan_status);
	}

	fwrite_string(fp, "Desc", mlstr_mval(ch->description));
	if (str_cmp(ch->prompt, DEFAULT_PROMPT)
	&&  str_cmp(ch->prompt, OLD_DEFAULT_PROMPT))
		fwrite_string(fp, "Prom", ch->prompt);
	fwrite_string(fp, "Race", race_name(ch->race));
	fprintf(fp, "Sex  %d\n", ch->sex);
	fwrite_string(fp, "Class", class_name(ch));
	fprintf(fp, "Levl %d\n", ch->level);
	fprintf(fp, "Scro %d\n", ch->lines);
	fprintf(fp, "Room %d\n",
		(ch->in_room == get_room_index(ROOM_VNUM_LIMBO)
		 && ch->was_in_room != NULL)
		? ch->was_in_room->vnum
		: ch->in_room == NULL ? 3001 : ch->in_room->vnum);

	fprintf(fp, "HMV  %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
	if (ch->gold > 0)
		fprintf(fp, "Gold %d\n", ch->gold);
	else
		fprintf(fp, "Gold %d\n", 0);
	if (ch->silver > 0)
		fprintf(fp, "Silv %d\n", ch->silver);
	else
		fprintf(fp, "Silv %d\n", 0);
	fprintf(fp, "Exp %d\n", ch->exp);
	fprintf(fp, "ExpTL %d\n", ch->exp_tl);
	fprintf(fp, "Drain_level %d\n", ch->drain_level);
	if (ch->plr_flags)
		fprintf(fp, "Act %s\n", format_flags(ch->plr_flags));
	if (ch->affected_by)
		fprintf(fp, "AfBy %s\n", format_flags(ch->affected_by));
	if (ch->comm)
		fprintf(fp, "Comm %s\n", format_flags(ch->comm));
	if (ch->invis_level)
		fprintf(fp, "Invi %d\n", ch->invis_level);
	if (ch->incog_level)
		fprintf(fp, "Inco %d\n", ch->incog_level);
	fprintf(fp, "Pos  %d\n",
		ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
	if (ch->practice != 0)
		fprintf(fp, "Prac %d\n", ch->practice);
	if (ch->train != 0)
		fprintf(fp, "Trai %d\n", ch->train);
	if (ch->saving_throw != 0)
		fprintf(fp, "Save  %d\n", ch->saving_throw);
	fprintf(fp, "Alig  %d\n", ch->alignment);
	if (ch->hitroll != 0)
		fprintf(fp, "Hit   %d\n", ch->hitroll);
	if (ch->damroll != 0)
		fprintf(fp, "Dam   %d\n", ch->damroll);
	fprintf(fp, "ACs %d %d %d %d\n",
		ch->armor[0], ch->armor[1], ch->armor[2], ch->armor[3]);
	if (ch->wimpy != 0)
		fprintf(fp, "Wimp  %d\n", ch->wimpy);
	fprintf(fp, "Attr %d %d %d %d %d %d\n",
		ch->perm_stat[STAT_STR],
		ch->perm_stat[STAT_INT],
		ch->perm_stat[STAT_WIS],
		ch->perm_stat[STAT_DEX],
		ch->perm_stat[STAT_CON],
		ch->perm_stat[STAT_CHA]);

	fprintf(fp, "AMod %d %d %d %d %d %d\n",
		ch->mod_stat[STAT_STR],
		ch->mod_stat[STAT_INT],
		ch->mod_stat[STAT_WIS],
		ch->mod_stat[STAT_DEX],
		ch->mod_stat[STAT_CON],
		ch->mod_stat[STAT_CHA]);

	fprintf(fp, "Relig %d\n", ch->religion);

	if (IS_NPC(ch)) {
		fprintf(fp, "Vnum %d\n", ch->pIndexData->vnum);
	} else {
		QTROUBLE_DATA  *qt;
		PC_DATA *pcdata = ch->pcdata;
		int i;

		if (pcdata->wiznet)
			fprintf(fp, "Wizn %s\n", format_flags(pcdata->wiznet));

		if (pcdata->trust)
			fprintf(fp, "Trust %s\n", format_flags(pcdata->trust));

		for (qt = pcdata->qtrouble; qt; qt = qt->next)
			fprintf(fp, "Qtrouble %d %d\n", qt->vnum, qt->count);

		if (pcdata->race != ch->race)
			fwrite_string(fp, "OrgRace",
				      race_name(pcdata->race));
		if (pcdata->plevels > 0)
			fprintf(fp, "PLev %d\n", pcdata->plevels);
		fprintf(fp, "Plyd %d\n",
			pcdata->played + (int) (current_time - ch->logon));
		fprintf(fp, "Not  %ld %ld %ld %ld %ld\n",
			pcdata->last_note, pcdata->last_idea,
			pcdata->last_penalty, pcdata->last_news,
			pcdata->last_changes);

		fprintf(fp, "Dead %d\n", pcdata->death);
		if (pcdata->homepoint)
			fprintf(fp, "Homepoint %d\n", pcdata->homepoint->vnum);

		if (pcdata->bank_s)
			fprintf(fp, "Banks %d\n", pcdata->bank_s);
		if (pcdata->bank_g)
			fprintf(fp, "Bankg %d\n", pcdata->bank_g);
		if (pcdata->security)
			fprintf(fp, "Sec %d\n", pcdata->security);
		fwrite_string(fp, "Pass", pcdata->pwd);
		fwrite_string(fp, "Bin", pcdata->bamfin);
		fwrite_string(fp, "Bout", pcdata->bamfout);
		fwrite_string(fp, "Titl", pcdata->title);
		fprintf(fp, "Pnts %d\n", pcdata->points);
		fprintf(fp, "TSex %d\n", pcdata->true_sex);
		fprintf(fp, "LLev %d\n", pcdata->last_level);
		fprintf(fp, "HMVP %d %d %d\n", pcdata->perm_hit,
			pcdata->perm_mana,
			pcdata->perm_move);
		fprintf(fp, "CndC  %d %d %d %d %d %d\n",
			pcdata->condition[0],
			pcdata->condition[1],
			pcdata->condition[2],
			pcdata->condition[3],
			pcdata->condition[4],
			pcdata->condition[5]);

		/* write lang */
		fprintf(fp, "Lang %d\n", ch->lang);

		/* write pc_killed */
		fprintf(fp, "PC_Killed %d\n", pcdata->pc_killed);

		/* write alias */
		for (pos = 0; pos < MAX_ALIAS; pos++) {
			if (pcdata->alias[pos] == NULL
			||  pcdata->alias_sub[pos] == NULL)
				break;

			fprintf(fp, "Alias %s %s~\n",
				pcdata->alias[pos],
				fix_string(pcdata->alias_sub[pos]));
		}

		for (i = 0; i < pcdata->learned.nused; i++) {
			PC_SKILL *ps = VARR_GET(&pcdata->learned, i);

			if (ps->percent == 0)
				continue;

			fprintf(fp, "Sk %d '%s'\n",
				ps->percent, skill_name(ps->sn));
		}

		if (pcdata->questpoints != 0)
			fprintf(fp, "QuestPnts %d\n", pcdata->questpoints);
		if (pcdata->questtime != 0)
			fprintf(fp, "QuestTime %d\n",
				reboot ? -abs(pcdata->questtime) :
					 pcdata->questtime);
		fprintf(fp, "Haskilled %d\n", pcdata->has_killed);
		fprintf(fp, "Antkilled %d\n", pcdata->anti_killed);
		fwrite_string(fp, "Twitlist", pcdata->twitlist);
		fwrite_string(fp, "Granted", pcdata->granted);
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
fwrite_pet(CHAR_DATA * pet, FILE * fp)
{
	AFFECT_DATA    *paf;
	fprintf(fp, "#PET\n");

	fprintf(fp, "Vnum %d\n", pet->pIndexData->vnum);
	fwrite_string(fp, "Name", pet->name);
	fprintf(fp, "LogO %ld\n", current_time);
	if (pet->clan)
		fwrite_string(fp, "Clan", clan_name(pet->clan));
	if (mlstr_cmp(pet->short_descr, pet->pIndexData->short_descr) != 0)
		mlstr_fwrite(fp, "ShD", pet->short_descr);
	if (mlstr_cmp(pet->long_descr, pet->pIndexData->long_descr) != 0)
		mlstr_fwrite(fp, "LnD", pet->short_descr);
	if (mlstr_cmp(pet->description, pet->pIndexData->description) != 0)
		mlstr_fwrite(fp, "Desc", pet->short_descr);
	if (pet->race != pet->pIndexData->race)	/* serdar ORG_RACE */
		fwrite_string(fp, "Race", race_name(pet->race));
	fprintf(fp, "Sex  %d\n", pet->sex);
	if (pet->level != pet->pIndexData->level)
		fprintf(fp, "Levl %d\n", pet->level);
	fprintf(fp, "HMV  %d %d %d %d %d %d\n",
		pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
	if (pet->gold)
		fprintf(fp, "Gold %d\n", pet->gold);
	if (pet->silver)
		fprintf(fp, "Silv %d\n", pet->silver);
	if (pet->exp)
		fprintf(fp, "Exp  %d\n", pet->exp);
	if (pet->affected_by != pet->pIndexData->affected_by)
		fprintf(fp, "AfBy %s\n", format_flags(pet->affected_by));
	if (pet->comm != 0)
		fprintf(fp, "Comm %s\n", format_flags(pet->comm));
	fprintf(fp, "Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
	if (pet->saving_throw)
		fprintf(fp, "Save %d\n", pet->saving_throw);
	if (pet->alignment != pet->pIndexData->alignment)
		fprintf(fp, "Alig %d\n", pet->alignment);
	if (pet->hitroll != pet->pIndexData->hitroll)
		fprintf(fp, "Hit  %d\n", pet->hitroll);
	if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
		fprintf(fp, "Dam  %d\n", pet->damroll);
	fprintf(fp, "ACs  %d %d %d %d\n",
		pet->armor[0], pet->armor[1], pet->armor[2], pet->armor[3]);
	fprintf(fp, "Attr %d %d %d %d %d %d\n",
		pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
		pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
		pet->perm_stat[STAT_CON], pet->perm_stat[STAT_CHA]);
	fprintf(fp, "AMod %d %d %d %d %d %d\n",
		pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
		pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
		pet->mod_stat[STAT_CON], pet->mod_stat[STAT_CHA]);

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
	 */
	if (!IS_IMMORTAL(ch)) {
		if ((get_wear_level(ch, obj) < obj->level &&
		     obj->pIndexData->item_type != ITEM_CONTAINER)
		||  (ch->level > obj->level + 20 &&
		     obj->pIndexData->limit > 0)
		||  (IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST) &&
		     ch->level < obj->pIndexData->level)) {
			extract_obj_raw(obj, X_F_NORECURSE);
			return;
		}
	}

/* do not save named quest rewards if ch is not owner */
	if (!IS_IMMORTAL(ch)
	&&  IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST)
	&&  !IS_OWNER(ch, obj)) {
		log_printf("%s: '%s' of %s",
			   ch->name, obj->name,
			   mlstr_mval(obj->owner));
		act("$p vanishes!", ch, obj, NULL, TO_CHAR);
		extract_obj(obj);
		return;
	}
	
	if (IS_SET(obj->extra_flags, ITEM_CLAN) 
	    || IS_SET(obj->pIndexData->extra_flags, ITEM_CLAN)
	    || IS_SET(obj->pIndexData->extra_flags, ITEM_QUIT_DROP))
		return;

	fprintf(fp, "#O\n");
	fprintf(fp, "Vnum %d\n", obj->pIndexData->vnum);
	fprintf(fp, "Cond %d\n", obj->condition);

	fprintf(fp, "Nest %d\n", iNest);
	mlstr_fwrite(fp, "Owner", obj->owner);

	if (obj->pIndexData->limit < 0) {
		if (str_cmp(obj->name, obj->pIndexData->name))
			fwrite_string(fp, "Name", obj->name);
		if (mlstr_cmp(obj->short_descr, obj->pIndexData->short_descr))
			mlstr_fwrite(fp, "ShD", obj->short_descr);
		if (mlstr_cmp(obj->description, obj->pIndexData->description))
			mlstr_fwrite(fp, "Desc", obj->description);
	}

	if (obj->extra_flags != obj->pIndexData->extra_flags)
		fprintf(fp, "ExtF %d\n", obj->extra_flags);

	fprintf(fp, "Wear %d\n", obj->wear_loc);
	if (obj->level != obj->pIndexData->level)
		fprintf(fp, "Lev  %d\n", obj->level);
	if (obj->timer != 0)
		fprintf(fp, "Time %d\n", obj->timer);
	fprintf(fp, "Cost %d\n", obj->cost);
	if (obj->value[0] != obj->pIndexData->value[0]
	    || obj->value[1] != obj->pIndexData->value[1]
	    || obj->value[2] != obj->pIndexData->value[2]
	    || obj->value[3] != obj->pIndexData->value[3]
	    || obj->value[4] != obj->pIndexData->value[4])
		fprintf(fp, "Val  %d %d %d %d %d\n",
		  obj->value[0], obj->value[1], obj->value[2], obj->value[3],
			obj->value[4]);

	switch (obj->pIndexData->item_type) {
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
		mlstr_fwrite(fp, NULL, ed->description);
	}

	fprintf(fp, "End\n\n");

	if (obj->contains != NULL)
		fwrite_obj(ch, obj->contains, fp, iNest + 1);
}

/*
 * Load a char and inventory into a new ch structure.
 */
void load_char_obj(DESCRIPTOR_DATA * d, const char *name)
{
	CHAR_DATA      *ch;
	FILE           *fp;
	bool            found;
	ch = new_char();
	ch->pcdata = new_pcdata();

	d->character = ch;
	ch->desc = d;
	ch->name = str_dup(capitalize(name));
	ch->short_descr = mlstr_new(ch->name);
	ch->id = get_pc_id();
	ch->race = rn_lookup("human");
	ch->plr_flags = PLR_NOSUMMON | PLR_NOCANCEL;
	ch->comm = COMM_COMBINE | COMM_PROMPT;
	ch->prompt = str_dup(DEFAULT_PROMPT);

	ch->pcdata->race = ch->race;
	ch->pcdata->clan_status = CLAN_COMMONER;
	ch->pcdata->condition[COND_THIRST] = 48;
	ch->pcdata->condition[COND_FULL] = 48;
	ch->pcdata->condition[COND_HUNGER] = 48;
	ch->pcdata->condition[COND_BLOODLUST] = 48;
	ch->pcdata->condition[COND_DESIRE] = 48;

	found = FALSE;

	name = capitalize(name);
	snprintf(filename, sizeof(filename), "%s.gz", name);
	if (dfexist(PLAYER_PATH, filename)) {
		char buf[PATH_MAX * 2];
		snprintf(buf, sizeof(buf), "gzip -dfq %s%c%s",
			 PLAYER_PATH, PATH_SEPARATOR, filename);
		system(buf);
	}
	if (dfexist(PLAYER_PATH, name)
	&&  (fp = dfopen(PLAYER_PATH, name, "r")) != NULL) {
		int             iNest;
		for (iNest = 0; iNest < MAX_NEST; iNest++)
			rgObjNest[iNest] = NULL;

		found = TRUE;
		for (;;) {
			char	letter;
			char *	word;
			letter = fread_letter(fp);
			if (letter == '*') {
				fread_to_eol(fp);
				continue;
			}
			if (letter != '#') {
				bug("Load_char_obj: # not found.", 0);
				break;
			}
			word = fread_word(fp);
			if (!str_cmp(word, "PLAYER"))
				fread_char(ch, fp);
			else if (!str_cmp(word, "OBJECT"))
				fread_obj(ch, fp);
			else if (!str_cmp(word, "O"))
				fread_obj(ch, fp);
			else if (!str_cmp(word, "PET"))
				fread_pet(ch, fp);
			else if (!str_cmp(word, "END"))
				break;
			else {
				bug("Load_char_obj: bad section.", 0);
				break;
			}
		}
		fclose(fp);
	}

	/* initialize race */
	if (found) {
		RACE_DATA *r;

		if (ORG_RACE(ch) == 0)
			SET_ORG_RACE(ch, rn_lookup("human"));
		if (ch->race == 0)
			ch->race = rn_lookup("human");

		r = RACE(ch->race);

		if (!IS_NPC(ch))
			ch->size = r->pcdata->size;

		ch->dam_type = 17;	/* punch */
		ch->affected_by = ch->affected_by | r->aff;
		ch->imm_flags = ch->imm_flags | r->imm;
		ch->res_flags = ch->res_flags | r->res;
		ch->vuln_flags = ch->vuln_flags | r->vuln;
		ch->form = r->form;
		ch->parts = r->parts;
		affect_check(ch, -1, -1);

		if (ch->pcdata->condition[COND_BLOODLUST] < 48
		&&  !HAS_SKILL(ch, gsn_vampire))
			ch->pcdata->condition[COND_BLOODLUST] = 48;
	}

	if (!found)
		ch->plr_flags |= PLR_NEW;
}

/*
 * Read in a char.
 */
void 
fread_char(CHAR_DATA * ch, FILE * fp)
{
	char           *word = "End";
	bool            fMatch;
	int             count = 0;
	int             lastlogoff = current_time;
	int             percent;

	log_printf("Loading %s.", ch->name);
	ch->pcdata->bank_s = 0;
	ch->pcdata->bank_g = 0;

	for (;;) {
		word = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0])) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			KEY("Act", ch->plr_flags, fread_flags(fp) &
					    ~(PLR_GHOST | PLR_CONFIRM_DELETE |
					      PLR_NOEXP | PLR_NEW | PLR_PUMPED |
					      PLR_PRACTICER));
			KEY("AffectedBy", ch->affected_by, fread_flags(fp));
			KEY("AfBy", ch->affected_by, fread_flags(fp) &
						     ~AFF_CHARM);
			KEY("Alignment", ch->alignment, fread_number(fp));
			KEY("Alig", ch->alignment, fread_number(fp));
			KEY("AntKilled", ch->pcdata->anti_killed, fread_number(fp));

			if (!str_cmp(word, "Alia")) {
				if (count >= MAX_ALIAS) {
					fread_to_eol(fp);
					fMatch = TRUE;
					break;
				}
				ch->pcdata->alias[count] = str_dup(fread_word(fp));
				ch->pcdata->alias_sub[count] = str_dup(fread_word(fp));
				count++;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Alias")) {
				if (count >= MAX_ALIAS) {
					fread_to_eol(fp);
					fMatch = TRUE;
					break;
				}
				ch->pcdata->alias[count] = str_dup(fread_word(fp));
				ch->pcdata->alias_sub[count] = fread_string(fp);
				count++;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "AC") || !str_cmp(word, "Armor")) {
				fread_to_eol(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "ACs")) {
				int             i;
				for (i = 0; i < 4; i++)
					ch->armor[i] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "AffD")) {
				AFFECT_DATA    *paf;
				int             sn;
				paf = aff_new();

				sn = sn_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_char: unknown skill.", 0);
				else
					paf->type = sn;

				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_flags(fp);
				paf->next = ch->affected;
				ch->affected = paf;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA    *paf;
				int             sn;
				paf = aff_new();

				sn = sn_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_char: unknown skill.", 0);
				else
					paf->type = sn;

				paf->where = fread_number(fp);
				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_flags(fp);
				paf->next = ch->affected;
				ch->affected = paf;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "AttrMod") || !str_cmp(word, "AMod")) {
				int             stat;
				for (stat = 0; stat < MAX_STATS; stat++)
					ch->mod_stat[stat] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "AttrPerm") || !str_cmp(word, "Attr")) {
				int             stat;
				for (stat = 0; stat < MAX_STATS; stat++)
					ch->perm_stat[stat] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'B':
			SKEY("Bamfin", ch->pcdata->bamfin);
			KEY("Banks", ch->pcdata->bank_s, fread_number(fp));
			KEY("Bankg", ch->pcdata->bank_g, fread_number(fp));
			SKEY("Bamfout", ch->pcdata->bamfout);
			SKEY("Bin", ch->pcdata->bamfin);
			SKEY("Bout", ch->pcdata->bamfout);
			break;

		case 'C':
			if (!str_cmp(word, "Class")) {
				const char *cl = fread_string(fp);
				ch->class = cln_lookup(cl);
				free_string(cl);
				fMatch = TRUE;
				break;
			}
			KEY("Cla", ch->class, fread_number(fp));
			KEY("Clan", ch->clan, fread_clan(fp));
			KEY("ClanStatus", ch->pcdata->clan_status,
			    fread_number(fp));
			if (!str_cmp(word, "Condition")
			|| !str_cmp(word, "Cond")) {
				ch->pcdata->condition[0] = fread_number(fp);
				ch->pcdata->condition[1] = fread_number(fp);
				ch->pcdata->condition[2] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "CndC")) {
				ch->pcdata->condition[0] = fread_number(fp);
				ch->pcdata->condition[1] = fread_number(fp);
				ch->pcdata->condition[2] = fread_number(fp);
				ch->pcdata->condition[3] = fread_number(fp);
				ch->pcdata->condition[4] = fread_number(fp);
				ch->pcdata->condition[5] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Cnd")) {
				ch->pcdata->condition[0] = fread_number(fp);
				ch->pcdata->condition[1] = fread_number(fp);
				ch->pcdata->condition[2] = fread_number(fp);
				ch->pcdata->condition[3] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			KEY("Comm", ch->comm, fread_flags(fp) & ~COMM_AFK);

			break;

		case 'D':
			KEY("Damroll", ch->damroll, fread_number(fp));
			KEY("Dam", ch->damroll, fread_number(fp));
			KEY("Drain_level", ch->drain_level, fread_number(fp));
			MLSKEY("Desc", ch->description);
			KEY("Dead", ch->pcdata->death, fread_number(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				CLAN_DATA *clan;
				const char **nl = NULL;
				bool touched = FALSE;

				/*
				 * adjust hp mana move up  -- here for speed's
				 * sake
				 */
				percent = (current_time - lastlogoff)
					  * 25 / (2 * 60 * 60);

				percent = UMIN(percent, 100);

				if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON)
				    && !IS_AFFECTED(ch, AFF_PLAGUE)) {
					ch->hit += (ch->max_hit - ch->hit)
						   * percent / 100;
					ch->mana += (ch->max_mana - ch->mana)
						    * percent / 100;
					ch->move += (ch->max_move - ch->move)
						    * percent / 100;
					if (!IS_NPC(ch))
						ch->pcdata->questtime =
						  -abs(ch->pcdata->questtime *
						  (100 - UMIN(5 * percent, 100))
							/ 100);
				}
				ch->played = ch->pcdata->played;
				if (ch->lines < SCROLL_MIN-2)
					ch->lines = SCROLL_MAX-2;

				/* XXX update clan lists */
				if (!ch->clan
				||  (clan = clan_lookup(ch->clan)) == NULL)
					return;
				
				touched = !name_add(&clan->member_list, ch->name, NULL, NULL);
				switch (ch->pcdata->clan_status) {
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
			KEY("Exp", ch->exp, fread_number(fp));
			KEY("ExpTL", ch->exp_tl, fread_number(fp));
			KEY("Etho", ch->ethos, (1 << (fread_number(fp)-1)));
			KEY("Ethos", ch->ethos, fread_fword(ethos_table, fp));
			break;

		case 'G':
			KEY("Gold", ch->gold, fread_number(fp));
			SKEY("Granted", ch->pcdata->granted);
			if (!str_cmp(word, "Group") || !str_cmp(word, "Gr")) {
				fread_word(fp);
				fMatch = TRUE;
			}
			break;

		case 'H':
			if (!str_cmp(word, "Hometown")) {
				const char *s = fread_string(fp);
				ch->hometown = htn_lookup(s);
				free_string(s);
				fMatch = TRUE;
			}

			KEY("Hitroll", ch->hitroll, fread_number(fp));
			KEY("Hit", ch->hitroll, fread_number(fp));
			KEY("Home", ch->hometown, fread_number(fp));
			KEY("Haskilled", ch->pcdata->has_killed,
			    fread_number(fp));
			KEY("Homepoint", ch->pcdata->homepoint,
			    get_room_index(fread_number(fp)));

			if (!str_cmp(word, "HpManaMove") || !str_cmp(word, "HMV")) {
				ch->hit = fread_number(fp);
				ch->max_hit = fread_number(fp);
				ch->mana = fread_number(fp);
				ch->max_mana = fread_number(fp);
				ch->move = fread_number(fp);
				ch->max_move = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "HpManaMovePerm") || !str_cmp(word, "HMVP")) {
				ch->pcdata->perm_hit = fread_number(fp);
				ch->pcdata->perm_mana = fread_number(fp);
				ch->pcdata->perm_move = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'I':
			KEY("Id", ch->id, fread_number(fp));
			KEY("InvisLevel", ch->invis_level, fread_number(fp));
			KEY("Inco", ch->incog_level, fread_number(fp));
			KEY("Invi", ch->invis_level, fread_number(fp));
			KEY("I_Lang", ch->lang, fread_number(fp));
			break;

		case 'L':
			KEY("LastLevel", ch->pcdata->last_level, fread_number(fp));
			KEY("LLev", ch->pcdata->last_level, fread_number(fp));
			KEY("Level", ch->level, fread_number(fp));
			KEY("Lev", ch->level, fread_number(fp));
			KEY("Levl", ch->level, fread_number(fp));
			KEY("LogO", lastlogoff, fread_number(fp));
			KEY("Lang", ch->lang, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", ch->name);
			KEY("Note", ch->pcdata->last_note, fread_number(fp));
			if (!str_cmp(word, "Not")) {
				ch->pcdata->last_note = fread_number(fp);
				ch->pcdata->last_idea = fread_number(fp);
				ch->pcdata->last_penalty = fread_number(fp);
				ch->pcdata->last_news = fread_number(fp);
				ch->pcdata->last_changes = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'O':
			if (!str_cmp(word, "OrgRace")) {
				const char *race = fread_string(fp);
				ch->pcdata->race = rn_lookup(race);
				free_string(race);
				fMatch = TRUE;
				break;
			}
			break;
		case 'P':
			KEY("PLev", ch->pcdata->plevels, fread_number(fp));
			SKEY("Password", ch->pcdata->pwd);
			SKEY("Pass", ch->pcdata->pwd);
			KEY("PC_Killed", ch->pcdata->pc_killed, fread_number(fp));
			KEY("Played", ch->pcdata->played, fread_number(fp));
			KEY("Plyd", ch->pcdata->played, fread_number(fp));
			KEY("Points", ch->pcdata->points, fread_number(fp));
			KEY("Pnts", ch->pcdata->points, fread_number(fp));
			KEY("Position", ch->position, fread_number(fp));
			KEY("Pos", ch->position, fread_number(fp));
			KEY("Practice", ch->practice, fread_number(fp));
			KEY("Prac", ch->practice, fread_number(fp));
			SKEY("Prompt", ch->prompt);
			SKEY("Prom", ch->prompt);
			break;

		case 'Q':
			KEY("QuestTime", ch->pcdata->questtime, fread_number(fp));
			KEY("QuestPnts", ch->pcdata->questpoints, fread_number(fp));

			if (str_cmp(word, "Qtrouble") == 0) {
				int             vnum;
				int             count;
				vnum = fread_number(fp);
				count = fread_number(fp);
				qtrouble_set(ch, vnum, count);
				fMatch = TRUE;
			}
			break;

		case 'R':
			KEY("Relig", ch->religion, fread_number(fp));
			if (!str_cmp(word, "Race")) {
				const char *race = fread_string(fp);
				ch->race = rn_lookup(race);
				ch->pcdata->race = ch->race;
				free_string(race);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Room")) {
				ch->in_room = get_room_index(fread_number(fp));
				if (ch->in_room == NULL)
					ch->in_room = get_room_index(ROOM_VNUM_LIMBO);
				fMatch = TRUE;
				break;
			}
			break;

		case 'S':
			KEY("SavingThrow", ch->saving_throw, fread_number(fp));
			KEY("Save", ch->saving_throw, fread_number(fp));
			KEY("Scro", ch->lines, fread_number(fp));
			KEY("Sex", ch->sex, fread_number(fp));
			MLSKEY("ShortDescr", ch->short_descr);
			MLSKEY("ShD", ch->short_descr);
			KEY("Sec", ch->pcdata->security, fread_number(fp));
			KEY("Silv", ch->silver, fread_number(fp));

			if (!str_cmp(word, "Skill") || !str_cmp(word, "Sk")) {
				int             sn;
				int             value;
				char           *temp;
				value = fread_number(fp);
				temp = fread_word(fp);
				sn = sn_lookup(temp);
				if (sn < 0) {
					fprintf(stderr, "%s", temp);
					bug("Fread_char: unknown skill. ", 0);
				}
				else
					set_skill(ch, sn, value);
				fMatch = TRUE;
			}
			break;

		case 'T':
			KEY("TrueSex", ch->pcdata->true_sex, fread_number(fp));
			KEY("TSex", ch->pcdata->true_sex, fread_number(fp));
			KEY("Trai", ch->train, fread_number(fp));
			KEY("Trust", ch->pcdata->trust, fread_flags(fp));
			SKEY("Twitlist", ch->pcdata->twitlist);
			if (!str_cmp(word, "Title") || !str_cmp(word, "Titl")) {
				const char *p = fread_string(fp);
				set_title(ch, p);
				free_string(p);
				fMatch = TRUE;
				break;
			}
			break;

		case 'V':
			KEY("Version", ch->version, fread_number(fp));
			KEY("Vers", ch->version, fread_number(fp));
			if (!str_cmp(word, "Vnum")) {
				ch->pIndexData = get_mob_index(fread_number(fp));
				fMatch = TRUE;
				break;
			}
			break;

		case 'W':
			KEY("Wimpy", ch->wimpy, fread_number(fp));
			KEY("Wimp", ch->wimpy, fread_number(fp));
			KEY("Wizn", ch->pcdata->wiznet, fread_flags(fp));
			break;
		}

		if (!fMatch) {
			log_printf("fread_char: %s: no match", word);
			fread_to_eol(fp);
		}
	}
}

/* load a pet from the forgotten reaches */
void 
fread_pet(CHAR_DATA * ch, FILE * fp)
{
	char           *word;
	CHAR_DATA      *pet;
	bool            fMatch;
	int             lastlogoff = current_time;
	int             percent;
	/* first entry had BETTER be the vnum or we barf */
	word = feof(fp) ? "END" : fread_word(fp);
	if (!str_cmp(word, "Vnum")) {
		int             vnum;
		vnum = fread_number(fp);
		if (get_mob_index(vnum) == NULL) {
			bug("Fread_pet: bad vnum %d.", vnum);
			pet = create_mob(get_mob_index(MOB_VNUM_FIDO));
		} else
			pet = create_mob(get_mob_index(vnum));
	} else {
		bug("Fread_pet: no vnum in file.", 0);
		pet = create_mob(get_mob_index(MOB_VNUM_FIDO));
	}

	for (;;) {
		word = feof(fp) ? "END" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0])) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			KEY("AfBy", pet->affected_by, fread_flags(fp));
			KEY("Alig", pet->alignment, fread_number(fp));

			if (!str_cmp(word, "ACs")) {
				int             i;
				for (i = 0; i < 4; i++)
					pet->armor[i] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "AffD")) {
				AFFECT_DATA    *paf;
				int             sn;
				paf = aff_new();

				sn = sn_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_char: unknown skill.", 0);
				else
					paf->type = sn;

				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_flags(fp);
				paf->next = pet->affected;
				pet->affected = paf;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA    *paf;
				int             sn;
				paf = aff_new();

				sn = sn_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_char: unknown skill.", 0);
				else
					paf->type = sn;

				paf->where = fread_number(fp);
				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_flags(fp);
				paf->next = pet->affected;
				pet->affected = paf;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "AMod")) {
				int             stat;
				for (stat = 0; stat < MAX_STATS; stat++)
					pet->mod_stat[stat] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Attr")) {
				int             stat;
				for (stat = 0; stat < MAX_STATS; stat++)
					pet->perm_stat[stat] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'C':
			KEY("Clan", pet->clan, fread_clan(fp));
			KEY("Comm", pet->comm, fread_flags(fp));
			break;

		case 'D':
			KEY("Dam", pet->damroll, fread_number(fp));
			MLSKEY("Desc", pet->description);
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				pet->leader = ch;
				pet->master = ch;
				ch->pet = pet;
				/*
				 * adjust hp mana move up  -- here for speed's
				 * sake
				 */
				percent = (current_time - lastlogoff) * 25 / (2 * 60 * 60);

				if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON)
				    && !IS_AFFECTED(ch, AFF_PLAGUE)) {
					percent = UMIN(percent, 100);
					pet->hit += (pet->max_hit - pet->hit) * percent / 100;
					pet->mana += (pet->max_mana - pet->mana) * percent / 100;
					pet->move += (pet->max_move - pet->move) * percent / 100;
				}
				return;
			}
			KEY("Exp", pet->exp, fread_number(fp));
			break;

		case 'G':
			KEY("Gold", pet->gold, fread_number(fp));
			break;

		case 'H':
			KEY("Hit", pet->hitroll, fread_number(fp));

			if (!str_cmp(word, "HMV")) {
				pet->hit = fread_number(fp);
				pet->max_hit = fread_number(fp);
				pet->mana = fread_number(fp);
				pet->max_mana = fread_number(fp);
				pet->move = fread_number(fp);
				pet->max_move = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'L':
			MLSKEY("LnD",  pet->description);
			KEY("Levl", pet->level, fread_number(fp));
			KEY("LogO", lastlogoff, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", pet->name);
			break;

		case 'P':
			KEY("Pos", pet->position, fread_number(fp));
			break;

		case 'R':
			if (!str_cmp(word, "Race")) {
				const char *race = fread_string(fp);
				pet->race = rn_lookup(race);
				free_string(race);
				fMatch = TRUE;
				break;
			}
			break;

		case 'S':
			KEY("Save", pet->saving_throw, fread_number(fp));
			KEY("Sex", pet->sex, fread_number(fp));
			MLSKEY("ShD", pet->short_descr);
			KEY("Silv", pet->silver, fread_number(fp));
			break;

			if (!fMatch) {
				bug("Fread_pet: no match.", 0);
				fread_to_eol(fp);
			}
		}
	}
}

extern	OBJ_DATA	*obj_free;

void 
fread_obj(CHAR_DATA * ch, FILE * fp)
{
	OBJ_DATA       *obj;
	char           *word;
	int             iNest;
	bool            fMatch;
	bool            fNest;
	bool            fVnum;
	bool            first;
	bool		enchanted = FALSE;
	fVnum = FALSE;
	obj = NULL;
	first = TRUE;		/* used to counter fp offset */

	word = feof(fp) ? "End" : fread_word(fp);
	if (!str_cmp(word, "Vnum")) {
		int             vnum;
		first = FALSE;	/* fp will be in right place */

		vnum = fread_number(fp);
		if (get_obj_index(vnum) == NULL)
			bug("Fread_obj: bad vnum %d.", vnum);
		else 
			obj = create_obj_nocount(get_obj_index(vnum), -1);
	}
	if (obj == NULL) {	/* either not found or old style */
		obj = new_obj();
		obj->name = str_dup(str_empty);
	}
	fNest = FALSE;
	fVnum = TRUE;
	iNest = 0;

	for (;;) {
		if (first)
			first = FALSE;
		else
			word = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0])) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			if (!str_cmp(word, "AffD")) {
				AFFECT_DATA    *paf;
				int             sn;
				paf = aff_new();

				sn = sn_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_obj: unknown skill.", 0);
				else
					paf->type = sn;

				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_flags(fp);
				paf->next = obj->affected;
				obj->affected = paf;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA    *paf;
				int             sn;
				paf = aff_new();

				sn = sn_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_obj: unknown skill.", 0);
				else
					paf->type = sn;

				paf->where = fread_number(fp);
				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_flags(fp);
				paf->next = obj->affected;
				obj->affected = paf;
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
			if (!str_cmp(word, "Enchanted")) {
				enchanted = TRUE;
				fMatch = TRUE;
				break;
			}

			KEY("ExtraFlags", obj->extra_flags, fread_number(fp));
			KEY("ExtF", obj->extra_flags, fread_number(fp));

			if (!str_cmp(word, "ExtraDescr") || !str_cmp(word, "ExDe")) {
				ed_fread(fp, &obj->ed);
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "End")) {
				if (enchanted)
					SET_BIT(obj->extra_flags,
						ITEM_ENCHANTED);
				if (!fNest
				||  (fVnum && obj->pIndexData == NULL)) {
					bug("Fread_obj: incomplete object.", 0);
					free_obj(obj);
					return;
				}

				if (!fVnum) {
					free_obj(obj);
					obj = create_obj(get_obj_index(OBJ_VNUM_DUMMY), 0);
				}

				if (IS_SET(obj->pIndexData->extra_flags,
					   ITEM_QUEST)
				&&  IS_NULLSTR(obj->owner)) {
					mlstr_free(obj->owner);
					obj->owner = mlstr_dup(ch->short_descr);
				}

				if (iNest == 0 || rgObjNest[iNest] == NULL)
					obj_to_char(obj, ch);
				else
					obj_to_obj(obj, rgObjNest[iNest - 1]);
				return;
			}
			break;

		case 'L':
			KEY("Level", obj->level, fread_number(fp));
			KEY("Lev", obj->level, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", obj->name);

			if (!str_cmp(word, "Nest")) {
				iNest = fread_number(fp);
				if (iNest < 0 || iNest >= MAX_NEST) {
					bug("Fread_obj: bad nest %d.", iNest);
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

		case 'Q':
			KEY("Quality", obj->condition, fread_number(fp));
			break;

		case 'S':
			MLSKEY("ShortDescr", obj->short_descr);
			MLSKEY("ShD", obj->short_descr);

			if (!str_cmp(word, "Spell")) {
				int             iValue;
				int             sn;
				iValue = fread_number(fp);
				sn = sn_lookup(fread_word(fp));
				if (iValue < 0 || iValue > 3) {
					bug("Fread_obj: bad iValue %d.", iValue);
				} else if (sn < 0) {
					bug("Fread_obj: unknown skill.", 0);
				} else {
					obj->value[iValue] = sn;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'T':
			KEY("Timer", obj->timer, fread_number(fp));
			KEY("Time", obj->timer, fread_number(fp));
			break;

		case 'V':
			if (!str_cmp(word, "Values") || !str_cmp(word, "Vals")) {
				obj->value[0] = fread_number(fp);
				obj->value[1] = fread_number(fp);
				obj->value[2] = fread_number(fp);
				obj->value[3] = fread_number(fp);
				if (obj->pIndexData->item_type == ITEM_WEAPON && obj->value[0] == 0)
					obj->value[0] = obj->pIndexData->value[0];
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Val")) {
				obj->value[0] = fread_number(fp);
				obj->value[1] = fread_number(fp);
				obj->value[2] = fread_number(fp);
				obj->value[3] = fread_number(fp);
				obj->value[4] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Vnum")) {
				int             vnum;
				vnum = fread_number(fp);
				if ((obj->pIndexData = get_obj_index(vnum)) == NULL)
					bug("Fread_obj: bad vnum %d.", vnum);
				else
					fVnum = TRUE;
				fMatch = TRUE;
				break;
			}
			break;

		case 'W':
			KEY("WearLoc", obj->wear_loc, fread_number(fp));
			KEY("Wear", obj->wear_loc, fread_number(fp));
			break;

		}

		if (!fMatch) {
			bug("Fread_obj: no match.", 0);
			fread_to_eol(fp);
		}
	}
}

void fwrite_affect(AFFECT_DATA *paf, FILE *fp)
{
	SKILL_DATA *sk;

	if (paf->type == gsn_doppelganger
	||  (sk = skill_lookup(paf->type)) == NULL)
		return;

	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %s\n",
		sk->name,
		paf->where, paf->level, paf->duration, paf->modifier,
		paf->location, format_flags(paf->bitvector));
}

