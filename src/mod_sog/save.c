/*
 * $Id: save.c,v 1.130 1999-10-25 12:05:24 fjoe Exp $
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
#include "db.h"
#include "rfile.h"

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

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
void fread_char (CHAR_DATA * ch, rfile_t * fp, int flags);
void fread_pet  (CHAR_DATA * ch, rfile_t * fp, int flags);
void fread_obj  (CHAR_DATA * ch, rfile_t * fp, int flags);

/*
 * move_pfile - shifts vnum in range minvnum..maxvnum by delta)
 */
#define IN_RANGE(i, l, u)	((l) <= (i) && (i) <= (u))
#define MOVE(i)		if (IN_RANGE(i, minv, maxv)) i += del

void move_pfile(const char *name, int minvnum, int maxvnum, int delta)
{
	CHAR_DATA *ch;
	minv = minvnum; maxv = maxvnum; del = delta;
	ch = char_load(name, LOAD_F_MOVE);
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
	if ((clan = clan_lookup(victim->clan))) {
		clan_update_lists(clan, victim, TRUE);
		clan_save(clan);
	}

	RESET_FIGHT_TIME(victim);
	name = capitalize(victim->name);
	quit_char(victim, XC_F_COUNT);
	dunlink(PLAYER_PATH, name);
}

void char_nuke(CHAR_DATA *ch)
{
	PC_DATA *pc = PC(ch);

	if (pc->pet) {
		extract_char(pc->pet, 0);
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
	&&  (IS_SET(flags, SAVE_F_PSCAN) || pet->in_room == ch->in_room))
		fwrite_pet(pet, fp, flags);
	fprintf(fp, "#END\n");
	fclose(fp);
	d2rename(PLAYER_PATH, TMP_FILE, PLAYER_PATH, name);
}

static void *
pc_skill_save_cb(void *p, void *d)
{
	pc_skill_t *pc_sk = (pc_skill_t *) p;

	if (pc_sk->percent == 0)
		return NULL;

	fprintf((FILE *) d, "Sk %d '%s'\n", pc_sk->percent, pc_sk->sn);
	return NULL;
}

static void *
spn_save_cb(void *p, void *d)
{
	const char *spn = *(const char **) p;

	if (IS_NULLSTR(spn))
		return NULL;

	fprintf((FILE *) d, "Spec '%s'\n", spn);
	return NULL;
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

	fwrite_word(fp, "Clan", ch->clan);

	fwrite_string(fp, "Desc", mlstr_mval(&ch->description));

	fwrite_word(fp, "Race", ch->race);
	fprintf(fp, "Sex  %d\n", ch->sex);
	fwrite_word(fp, "Class", ch->class);
	fprintf(fp, "Levl %d\n", ch->level);
	fprintf(fp, "Room %d\n",
		(ch->in_room == get_room_index(ROOM_VNUM_LIMBO) &&
		 !IS_NPC(ch) && PC(ch)->was_in_room != NULL) ?
			PC(ch)->was_in_room->vnum : ch->in_room == NULL ?
			3001 : ch->in_room->vnum);

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
	fprintf(fp, "Drain_level %d\n", ch->drain_level);
	if (PC(ch)->plr_flags)
		fprintf(fp, "Act %s\n", format_flags(PC(ch)->plr_flags));
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

		if (!IS_NULLSTR(ch->clan))
			fprintf(fp, "ClanStatus %d\n", pc->clan_status);
		else 
			fwrite_word(fp, "Peti", pc->petition);

		fprintf(fp, "Relig %d\n", pc->religion);
		if (pc->practice != 0)
			fprintf(fp, "Prac %d\n", pc->practice);
		if (pc->train != 0)
			fprintf(fp, "Trai %d\n", pc->train);
		fprintf(fp, "Exp %d\n", pc->exp);
		fprintf(fp, "ExpTL %d\n", pc->exp_tl);
		fwrite_string(fp, "Hometown", hometown_name(pc->hometown));
		fprintf(fp, "LogO %ld\n",
			IS_SET(flags, SAVE_F_PSCAN) ?
				pc->logoff : current_time);
		if (pc->wiznet)
			fprintf(fp, "Wizn %s\n", format_flags(pc->wiznet));

		if (pc->trust)
			fprintf(fp, "Trust %s\n", format_flags(pc->trust));

		if (!IS_RACE(pc->race, ch->race))
			fprintf(fp, "OrgRace '%s'\n", pc->race);
		if (pc->plevels > 0)
			fprintf(fp, "PLev %d\n", pc->plevels);
		fprintf(fp, "Plyd %d\n",
			pc->played + (int) (current_time - pc->logon));
		fprintf(fp, "Not  %ld %ld %ld %ld %ld\n",
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
		if (pc->form_name)
			fwrite_string(fp, "FormName", pc->form_name);
		fprintf(fp, "Pnts %d\n", pc->points);
		fprintf(fp, "TSex %d\n", pc->true_sex);
		fprintf(fp, "HMVP %d %d %d\n", pc->perm_hit,
			pc->perm_mana,
			pc->perm_move);
		fprintf(fp, "CndC  %d %d %d %d %d %d\n",
			pc->condition[0],
			pc->condition[1],
			pc->condition[2],
			pc->condition[3],
			pc->condition[4],
			pc->condition[5]);

		/* write lang */
		fprintf(fp, "Lang %d\n", GET_LANG(ch));

		/* write pc_killed */
		fprintf(fp, "PC_Killed %d\n", pc->pc_killed);

		varr_foreach(&pc->specs, spn_save_cb, fp);
		varr_foreach(&pc->learned, pc_skill_save_cb, fp);

		if (pc->questpoints != 0)
			fprintf(fp, "QuestPnts %d\n", pc->questpoints);
		if (pc->questtime != 0)
			fprintf(fp, "QuestTime %d\n",
				IS_SET(flags, SAVE_F_REBOOT) ?
					-abs(pc->questtime) :
					 pc->questtime);
		fprintf(fp, "Haskilled %d\n", pc->has_killed);
		fprintf(fp, "Antkilled %d\n", pc->anti_killed);
		fwrite_string(fp, "Twitlist", pc->twitlist);
		fwrite_string(fp, "Granted", pc->granted);
	}

	for (paf = ch->affected; paf != NULL; paf = paf->next) {
		if (!IS_NPC(ch) && paf->where == TO_AFFECTS
		&&  IS_SET(paf->bitvector, AFF_CHARM))
			continue;

		fprintf(fp, "Affc ");
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
	fwrite_word(fp, "Clan", pet->clan);
	if (mlstr_cmp(&pet->short_descr, &pet->pMobIndex->short_descr) != 0)
		mlstr_fwrite(fp, "ShD", &pet->short_descr);
	if (mlstr_cmp(&pet->long_descr, &pet->pMobIndex->long_descr) != 0)
		mlstr_fwrite(fp, "LnD", &pet->short_descr);
	if (mlstr_cmp(&pet->description, &pet->pMobIndex->description) != 0)
		mlstr_fwrite(fp, "Desc", &pet->short_descr);
	if (!IS_RACE(pet->race, pet->pMobIndex->race))	/* serdar ORG_RACE */
		fprintf(fp, "Race '%s'\n", pet->race);
	fprintf(fp, "Sex  %d\n", pet->sex);
	if (pet->level != pet->pMobIndex->level)
		fprintf(fp, "Levl %d\n", pet->level);
	fprintf(fp, "HMV  %d %d %d %d %d %d\n",
		pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
	if (pet->gold)
		fprintf(fp, "Gold %d\n", pet->gold);
	if (pet->silver)
		fprintf(fp, "Silv %d\n", pet->silver);
	if (pet->affected_by != pet->pMobIndex->affected_by)
		fprintf(fp, "AfBy %s\n", format_flags(pet->affected_by));
	if (pet->comm != 0)
		fprintf(fp, "Comm %s\n", format_flags(pet->comm));
	fprintf(fp, "Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
	if (pet->saving_throw)
		fprintf(fp, "Save %d\n", pet->saving_throw);
	if (pet->alignment != pet->pMobIndex->alignment)
		fprintf(fp, "Alig %d\n", pet->alignment);
	if (pet->hitroll != pet->pMobIndex->hitroll)
		fprintf(fp, "Hit  %d\n", pet->hitroll);
	if (pet->damroll != pet->pMobIndex->damage[DICE_BONUS])
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

	for (paf = pet->affected; paf != NULL; paf = paf->next) {
		fprintf(fp, "Affc ");
		fwrite_affect(paf, fp);
	}

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
		fprintf(fp, "ExtF %d\n", obj->extra_flags);

	fprintf(fp, "Wear %d\n", obj->wear_loc);
	if (obj->level != obj->pObjIndex->level)
		fprintf(fp, "Lev  %d\n", obj->level);
	if (obj->timer != 0)
		fprintf(fp, "Time %d\n", obj->timer);
	fprintf(fp, "Cost %d\n", obj->cost);

	if (INT_VAL(obj->value[0]) != INT_VAL(obj->pObjIndex->value[0])
	||  INT_VAL(obj->value[1]) != INT_VAL(obj->pObjIndex->value[1])
	||  INT_VAL(obj->value[2]) != INT_VAL(obj->pObjIndex->value[2])
	||  INT_VAL(obj->value[3]) != INT_VAL(obj->pObjIndex->value[3])
	||  INT_VAL(obj->value[4]) != INT_VAL(obj->pObjIndex->value[4])) {
		fprintf(fp, "Values ");
		fwrite_objval(obj->pObjIndex->item_type, obj->value, fp);
	}

	for (paf = obj->affected; paf != NULL; paf = paf->next) {
		fprintf(fp, "Affc ");
		fwrite_affect(paf, fp);
	}

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

#ifdef RESET_CHAR
void reset_obj_affects(CHAR_DATA *ch, OBJ_DATA *obj, AFFECT_DATA *af)
{
	for (; af != NULL; af = af->next) {
		int mod = af->modifier;

		switch(af->location) {
		case APPLY_MANA:
			ch->max_mana	-= mod;
			break;
		case APPLY_HIT:
			ch->max_hit	-= mod;
			break;
		case APPLY_MOVE:
			ch->max_move	-= mod;
			break;
		}

	}
}

void add_affects_back(CHAR_DATA *ch, AFFECT_DATA *af)
{
	for (; af; af = af->next) {
		int i;
		int mod = af->modifier;

		if (af->where == TO_SKILLS) {
			affect_to_char(ch, af);
			continue;
		}

		switch(af->location) {
		case APPLY_STR:
			ch->mod_stat[STAT_STR] += mod;
			break;
		case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod; break;
		case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod; break;
		case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod; break;
		case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod; break;
 		case APPLY_CHA:		ch->mod_stat[STAT_CHA]	+= mod; break;
		case APPLY_MANA:	ch->max_mana		+= mod; break;
		case APPLY_HIT:		ch->max_hit		+= mod; break;
		case APPLY_MOVE:	ch->max_move		+= mod; break;
		case APPLY_AGE:		
			if (!IS_NPC(ch))
				PC(ch)->played += age_to_num(mod);
			break;
		case APPLY_AC:		
			for (i = 0; i < 4; i ++)
				ch->armor[i] += mod; 
			break;
		case APPLY_HITROLL:
			ch->hitroll += mod;
			break;
		case APPLY_DAMROLL:
			ch->damroll += mod;
			break;
		case APPLY_SIZE:
			ch->size += mod;
			break;
		case APPLY_LEVEL:
			ch->drain_level += mod;
			break;
		case APPLY_SAVES:
		case APPLY_SAVING_ROD:
		case APPLY_SAVING_PETRI:
		case APPLY_SAVING_BREATH:
		case APPLY_SAVING_SPELL:
			ch->saving_throw += mod;
			break;
		}
	}
}

/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
	int loc, stat;
	OBJ_DATA *obj;
	int i;

	if (IS_NPC(ch))
		return;

	if (PC(ch)->perm_hit == 0 
	||  PC(ch)->perm_mana == 0
	||  PC(ch)->perm_move == 0) {
		/* do a FULL reset */
		log("reset_char: %s: full reset", ch->name);

		for (loc = 0; loc < MAX_WEAR; loc++) {
			obj = get_eq_char(ch,loc);
			if (obj == NULL)
				continue;
			if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
				reset_obj_affects(ch, obj,
						  obj->pObjIndex->affected);
			reset_obj_affects(ch, obj, obj->affected);
	        }

		/* now reset the permanent stats */
		PC(ch)->perm_hit 	= ch->max_hit;
		PC(ch)->perm_mana 	= ch->max_mana;
		PC(ch)->perm_move	= ch->max_move;
		if (PC(ch)->true_sex < 0 || PC(ch)->true_sex > 2)
			if (ch->sex > 0 && ch->sex < 3)
		    	    PC(ch)->true_sex	= ch->sex;
			else
			    PC(ch)->true_sex 	= 0;
	}

	/* now restore the character to his/her true condition */
	for (stat = 0; stat < MAX_STATS; stat++)
		ch->mod_stat[stat] = 0;

	if (PC(ch)->true_sex < 0 || PC(ch)->true_sex > 2)
		PC(ch)->true_sex = 0; 
	ch->sex		= PC(ch)->true_sex;
	ch->max_hit 	= PC(ch)->perm_hit;
	ch->max_mana	= PC(ch)->perm_mana;
	ch->max_move	= PC(ch)->perm_move;
   
	for (i = 0; i < 4; i++)
		ch->armor[i]	= 100;

	ch->hitroll		= 0;
	ch->damroll		= 0;
	ch->saving_throw	= 0;
	ch->drain_level		= 0;

	/* now start adding back the effects */
	for (loc = 0; loc < MAX_WEAR; loc++) {
		obj = get_eq_char(ch, loc);
		if (obj == NULL)
			continue;

		for (i = 0; i < 4; i++)
			ch->armor[i] -= apply_ac(obj, loc, i);

		if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
			add_affects_back(ch, obj->pObjIndex->affected);
		add_affects_back(ch, obj->affected);
	}
  
	/* now add back spell affects */
	add_affects_back(ch, ch->affected);

	/* make sure sex is RIGHT! */
	if (ch->sex < 0 || ch->sex > 2)
		ch->sex = PC(ch)->true_sex;
}
#endif

/*
 * Load a char and inventory into a new ch structure.
 */
CHAR_DATA *char_load(const char *name, int flags)
{
	CHAR_DATA      *ch;
	rfile_t           *fp = NULL;
	bool		found;

	int             iNest;
	race_t *	r;

	name = capitalize(name);
	snprintf(filename, sizeof(filename), "%s.gz", name);
	if (dfexist(PLAYER_PATH, filename)) {
		char buf[PATH_MAX * 2];
		snprintf(buf, sizeof(buf), "gzip -dfq %s%c%s",
			 PLAYER_PATH, PATH_SEPARATOR, filename);
		system(buf);
	}

	found = (dfexist(PLAYER_PATH, name) &&
		 (fp = rfile_open(PLAYER_PATH, name)) != NULL);

	if (!found && IS_SET(flags, LOAD_F_NOCREATE))
		return NULL;

	ch = char_new(NULL);

	ch->name = str_dup(capitalize(name));
	mlstr_init(&ch->short_descr, ch->name);

	PC(ch)->plr_flags = PLR_NOSUMMON | PLR_NOCANCEL;
	ch->comm = COMM_COMBINE | COMM_PROMPT;

	ch->race = str_dup("human");
	PC(ch)->race = str_qdup(ch->race);
	PC(ch)->clan_status = CLAN_COMMONER;
	PC(ch)->condition[COND_THIRST] = 48;
	PC(ch)->condition[COND_FULL] = 48;
	PC(ch)->condition[COND_HUNGER] = 48;
	PC(ch)->condition[COND_BLOODLUST] = 48;
	PC(ch)->condition[COND_DESIRE] = 48;

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
		if (!str_cmp(word, "PLAYER"))
			fread_char(ch, fp, flags);
		else if (!str_cmp(word, "OBJECT"))
			fread_obj(ch, fp, flags);
		else if (!str_cmp(word, "O"))
			fread_obj(ch, fp, flags);
		else if (!str_cmp(word, "PET"))
			fread_pet(ch, fp, flags);
		else if (!str_cmp(word, "END"))
			break;
		else {
			log("char_load: %s: %s: bad section.", 
				   ch->name, word);
			break;
		}
	}
	rfile_close(fp);

	/* initialize race */
	if (IS_NULLSTR(ORG_RACE(ch)))
		SET_ORG_RACE(ch, "human");
	if (IS_NULLSTR(ch->race))
		ch->race = str_dup("human");

	if ((r = race_lookup(ch->race)) != NULL) {
		if (!IS_NPC(ch))
			ch->size = r->race_pcdata->size;

		ch->damtype = str_dup("punch");
		ch->affected_by = ch->affected_by | r->aff;
		ch->imm_flags = ch->imm_flags | r->imm;
		ch->res_flags = ch->res_flags | r->res;
		ch->vuln_flags = ch->vuln_flags | r->vuln;
		ch->form = r->form;
		ch->parts = r->parts;
	}
	affect_check(ch, -1, -1);

	if (PC(ch)->condition[COND_BLOODLUST] < 48
	&&  !IS_VAMPIRE(ch))
		PC(ch)->condition[COND_BLOODLUST] = 48;

#ifdef RESET_CHAR
	reset_char(ch);
#endif
	return ch;
}

/*
 * Read in a char.
 */
void 
fread_char(CHAR_DATA * ch, rfile_t * fp, int flags)
{
	char           *word = "End";
	bool            fMatch;
	int             count = 0;
	int             percent;
	int		foo;

	PC(ch)->bank_s = 0;
	PC(ch)->bank_g = 0;

	for (;;) {
		word = rfile_feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

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
			KEY("AffectedBy", ch->affected_by, fread_flags(fp));
			KEY("AfBy", ch->affected_by, fread_flags(fp) &
						     ~AFF_CHARM);
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
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA *paf = fread_affect(fp);
				SLIST_ADD(AFFECT_DATA, ch->affected, paf);
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
			SKEY("Bamfin", PC(ch)->bamfin, fread_string(fp));
			KEY("Banks", PC(ch)->bank_s, fread_number(fp));
			KEY("Bankg", PC(ch)->bank_g, fread_number(fp));
			SKEY("Bamfout", PC(ch)->bamfout, fread_string(fp));
			SKEY("Bin", PC(ch)->bamfin, fread_string(fp));
			SKEY("Bout", PC(ch)->bamfout, fread_string(fp));
			break;

		case 'C':
			KEY("Class", ch->class,
			    fread_strkey(fp, &classes, "fread_char"));
			KEY("ClassW", ch->class,
			    fread_strkey(fp, &classes, "fread_char"));
			KEY("Clan", ch->clan,
			    fread_strkey(fp, &clans, "fread_char"));
			KEY("ClanW", ch->clan,
			    fread_strkey(fp, &clans, "fread_char"));
			KEY("ClanStatus", PC(ch)->clan_status,
			    fread_number(fp));
			if (!str_cmp(word, "Condition")
			|| !str_cmp(word, "Cond")) {
				PC(ch)->condition[0] = fread_number(fp);
				PC(ch)->condition[1] = fread_number(fp);
				PC(ch)->condition[2] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
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
			if (!str_cmp(word, "Cnd")) {
				PC(ch)->condition[0] = fread_number(fp);
				PC(ch)->condition[1] = fread_number(fp);
				PC(ch)->condition[2] = fread_number(fp);
				PC(ch)->condition[3] = fread_number(fp);
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
			KEY("Dead", PC(ch)->death, fread_number(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				clan_t *clan;
				const char **nl = NULL;
				bool touched = FALSE;
				int logoff = PC(ch)->logoff;

				if (!logoff)
					logoff = current_time;

				/*
				 * adjust hp mana move up  -- here for speed's
				 * sake
				 */
				percent = (current_time - logoff)
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
						PC(ch)->questtime =
						  -abs(PC(ch)->questtime *
						  (100 - UMIN(5 * percent, 100))
							/ 100);
				}

				if (PC(ch)->dvdata->pagelen < MIN_PAGELEN-2)
					PC(ch)->dvdata->pagelen = MAX_PAGELEN-2;

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
					clan_save(clan);
				return;
			}
			KEY("Exp", PC(ch)->exp, fread_number(fp));
			KEY("ExpTL", PC(ch)->exp_tl, fread_number(fp));
			KEY("Etho", ch->ethos, (1 << (fread_number(fp)-1)));
			KEY("Ethos", ch->ethos, fread_fword(ethos_table, fp));
			break;
		case 'F':
			SKEY("FormName", PC(ch)->form_name, fread_string(fp));
			break;

		case 'G':
			KEY("Gold", ch->gold, fread_number(fp));
			SKEY("Granted", PC(ch)->granted, fread_string(fp));
			if (!str_cmp(word, "Group") || !str_cmp(word, "Gr")) {
				fread_word(fp);
				fMatch = TRUE;
			}
			break;

		case 'H':
			if (!str_cmp(word, "Hometown")) {
				const char *s = fread_string(fp);
				PC(ch)->hometown = htn_lookup(s);
				free_string(s);
				fMatch = TRUE;
			}

			KEY("Hitroll", ch->hitroll, fread_number(fp));
			KEY("Hit", ch->hitroll, fread_number(fp));
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
				PC(ch)->perm_hit = fread_number(fp);
				PC(ch)->perm_mana = fread_number(fp);
				PC(ch)->perm_move = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'I':
			KEY("InvisLevel", ch->invis_level, fread_number(fp));
			KEY("Inco", ch->incog_level, fread_number(fp));
			KEY("Invi", ch->invis_level, fread_number(fp));
			KEY("Id", foo, fread_number(fp));
			break;

		case 'L':
			KEY("Level", ch->level, fread_number(fp));
			KEY("Lev", ch->level, fread_number(fp));
			KEY("Levl", ch->level, fread_number(fp));
			KEY("LogO", PC(ch)->logoff, fread_number(fp));
			KEY("Lang", PC(ch)->dvdata->lang, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", ch->name, fread_string(fp));
			KEY("Note", PC(ch)->last_note, fread_number(fp));
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
			SKEY("OrgRace", PC(ch)->race,
			     fread_strkey(fp, &races, "fread_char"));
			SKEY("OrgRaceW", PC(ch)->race,
			     fread_strkey(fp, &races, "fread_char"));
			break;
		case 'P':
			KEY("Peti", PC(ch)->petition,
			    fread_strkey(fp, &clans, "fread_char"));
			KEY("PetiW", PC(ch)->petition,
			    fread_strkey(fp, &clans, "fread_char"));
			KEY("PLev", PC(ch)->plevels, fread_number(fp));
			SKEY("Password", PC(ch)->pwd, fread_string(fp));
			SKEY("Pass", PC(ch)->pwd, fread_string(fp));
			KEY("PC_Killed", PC(ch)->pc_killed, fread_number(fp));
			KEY("Played", PC(ch)->played, fread_number(fp));
			KEY("Plyd", PC(ch)->played, fread_number(fp));
			KEY("Points", PC(ch)->points, fread_number(fp));
			KEY("Pnts", PC(ch)->points, fread_number(fp));
			KEY("Position", ch->position, fread_number(fp));
			KEY("Pos", ch->position, fread_number(fp));
			KEY("Practice", PC(ch)->practice, fread_number(fp));
			KEY("Prac", PC(ch)->practice, fread_number(fp));
			SKEY("Prompt", PC(ch)->dvdata->prompt, fread_string(fp));
			SKEY("Prom", PC(ch)->dvdata->prompt, fread_string(fp));
			break;

		case 'Q':
			KEY("QuestTime", PC(ch)->questtime, fread_number(fp));
			KEY("QuestPnts", PC(ch)->questpoints, fread_number(fp));
			break;

		case 'R':
			KEY("Relig", PC(ch)->religion, fread_number(fp));
			if (!str_cmp(word, "Race")
			||  !str_cmp(word, "RaceW")) {
				free_string(ch->race);
				ch->race = fread_strkey(fp, &races, "fread_char");
				SET_ORG_RACE(ch, ch->race);
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
			KEY("SavingThrow", ch->saving_throw, fread_number(fp));
			KEY("Save", ch->saving_throw, fread_number(fp));
			KEY("Scro", PC(ch)->dvdata->pagelen, fread_number(fp));
			KEY("Sex", ch->sex, fread_number(fp));
			MLSKEY("ShortDescr", ch->short_descr);
			MLSKEY("ShD", ch->short_descr);
			KEY("Sec", PC(ch)->security, fread_number(fp));
			KEY("Silv", ch->silver, fread_number(fp));
			if (!str_cmp(word, "Spec")) {
				const char **pspn = varr_enew(&PC(ch)->specs);
				*pspn = fread_strkey(fp, &specs, "fread_char");
				fMatch = TRUE;
			}
			if (!str_cmp(word, "Skill") || !str_cmp(word, "Sk")) {
				int value = fread_number(fp);
				const char *sn = fread_strkey(fp, &skills,
							    "fread_char");
				set_skill(ch, sn, value);
				free_string(sn);
				fMatch = TRUE;
			}
			break;

		case 'T':
			KEY("TrueSex", PC(ch)->true_sex, fread_number(fp));
			KEY("TSex", PC(ch)->true_sex, fread_number(fp));
			KEY("Trai", PC(ch)->train, fread_number(fp));
			KEY("Trust", PC(ch)->trust, fread_flags(fp));
			SKEY("Twitlist", PC(ch)->twitlist, fread_string(fp));
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
			if (!str_cmp(word, "Vnum")) {
				ch->pMobIndex = get_mob_index(fread_number(fp));
				fMatch = TRUE;
				break;
			}
			break;

		case 'W':
			KEY("Wimpy", ch->wimpy, fread_number(fp));
			KEY("Wimp", ch->wimpy, fread_number(fp));
			KEY("Wizn", PC(ch)->wiznet, fread_flags(fp));
			SKEY("WantedBy", PC(ch)->wanted_by, fread_string(fp));
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
fread_pet(CHAR_DATA * ch, rfile_t * fp, int flags)
{
	char           *word;
	CHAR_DATA      *pet;
	bool            fMatch;
	int             percent;
	/* first entry had BETTER be the vnum or we barf */
	word = rfile_feof(fp) ? "END" : fread_word(fp);
	if (!str_cmp(word, "Vnum")) {
		int             vnum;
		vnum = fread_number(fp);
		if (IS_SET(flags, LOAD_F_MOVE))
			MOVE(vnum);
		
		if (get_mob_index(vnum) == NULL) {
			log("fread_pet: %s: %d: bad vnum",
				   ch->name, vnum);
			pet = create_mob(get_mob_index(MOB_VNUM_KITTEN));
		} else
			pet = create_mob(get_mob_index(vnum));
	} else {
		log("fread_pet: %s: no vnum in file", ch->name);
		pet = create_mob(get_mob_index(MOB_VNUM_KITTEN));
	}

	for (;;) {
		word = rfile_feof(fp) ? "END" : fread_word(fp);
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
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA *paf = fread_affect(fp);
				SLIST_ADD(AFFECT_DATA, pet->affected, paf);
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
			KEY("Clan", pet->clan,
			    fread_strkey(fp, &clans, "fread_pet"));
			KEY("ClanW", pet->clan,
			    fread_strkey(fp, &clans, "fread_pet"));
			KEY("Comm", pet->comm, fread_flags(fp));
			break;

		case 'D':
			KEY("Dam", pet->damroll, fread_number(fp));
			MLSKEY("Desc", pet->description);
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				int logoff = PC(ch)->logoff;
				if (!logoff)
					logoff = current_time;

				pet->leader = ch;
				pet->master = ch;
				PC(ch)->pet = pet;

				/*
				 * adjust hp mana move up  -- here for speed's
				 * sake
				 */
				percent = (current_time - logoff) * 25 / (2 * 60 * 60);

				if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON)
				    && !IS_AFFECTED(ch, AFF_PLAGUE)) {
					percent = UMIN(percent, 100);
					pet->hit += (pet->max_hit - pet->hit) * percent / 100;
					pet->mana += (pet->max_mana - pet->mana) * percent / 100;
					pet->move += (pet->max_move - pet->move) * percent / 100;
				}
				return;
			}
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
			break;

		case 'N':
			SKEY("Name", pet->name, fread_string(fp));
			break;

		case 'P':
			KEY("Pos", pet->position, fread_number(fp));
			break;

		case 'R':
			SKEY("Race", pet->race,
			     fread_strkey(fp, &races, "fread_pet"));
			SKEY("RaceW", pet->race,
			     fread_strkey(fp, &races, "fread_pet"));
			break;

		case 'S':
			KEY("Save", pet->saving_throw, fread_number(fp));
			KEY("Sex", pet->sex, fread_number(fp));
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

extern	OBJ_DATA	*obj_free;

void 
fread_obj(CHAR_DATA * ch, rfile_t * fp, int flags)
{
	OBJ_DATA       *obj;
	char           *word;
	int             iNest;
	int		wl;
	bool            fMatch;
	bool            fNest;
	bool            fVnum;
	bool            first;
	bool		enchanted = FALSE;
	fVnum = FALSE;
	obj = NULL;
	first = TRUE;		/* used to counter fp offset */

	word = rfile_feof(fp) ? "End" : fread_word(fp);
	if (!str_cmp(word, "Vnum")) {
		int             vnum;
		first = FALSE;	/* fp will be in right place */

		vnum = fread_number(fp);
		if (IS_SET(flags, LOAD_F_MOVE))
			MOVE(vnum);

		if (get_obj_index(vnum) == NULL)
			log("fread_obj: %s: %d: bad vnum",
				   ch->name, vnum);
		else 
			obj = create_obj(get_obj_index(vnum), CO_F_NOCOUNT);
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
			word = rfile_feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0])) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA *paf = fread_affect(fp);
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
				||  (fVnum && obj->pObjIndex == NULL)) {
					log("fread_obj: %s: incomplete object", ch->name);
					free_obj(obj);
					return;
				}

				if (!fVnum) {
					free_obj(obj);
					obj = create_obj(get_obj_index(OBJ_VNUM_DUMMY), 0);
				}

				if (IS_SET(obj->pObjIndex->extra_flags,
					   ITEM_QUEST)
				&&  mlstr_null(&obj->owner)) {
					mlstr_cpy(&obj->owner, &ch->short_descr);
				}

				if (iNest == 0 || rgObjNest[iNest] == NULL)
					obj_to_char(obj, ch);
				else
					obj_to_obj(obj, rgObjNest[iNest - 1]);
				if (obj->wear_loc != WEAR_NONE) {
					wl = obj->wear_loc;
				}
				return;
			}
			break;

		case 'L':
			KEY("Level", obj->level, fread_number(fp));
			KEY("Lev", obj->level, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", obj->name, fread_string(fp));

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

		case 'Q':
			KEY("Quality", obj->condition, fread_number(fp));
			break;

		case 'S':
			MLSKEY("ShortDescr", obj->short_descr);
			MLSKEY("ShD", obj->short_descr);

			if (!str_cmp(word, "Spell")) {
				int iValue = fread_number(fp);
				const char *sn = fread_strkey(fp, &skills, "fread_obj");

				if (iValue < 0 || iValue > 3) {
					log("fread_obj: %s: %d: bad iValue", ch->name, iValue);
					free_string(sn);
				} else
					STR_VAL_ASSIGN(obj->value[iValue], sn);
				fMatch = TRUE;
				break;
			}
			break;

		case 'T':
			KEY("Timer", obj->timer, fread_number(fp));
			KEY("Time", obj->timer, fread_number(fp));
			break;

		case 'V':
			if (!str_cmp(word, "Values")) {
				fread_objval(obj->pObjIndex->item_type,
					     obj->value, fp);
				if (obj->pObjIndex->item_type == ITEM_WEAPON
				&&  INT_VAL(obj->value[0]) == 0)
					obj->value[0] = obj->pObjIndex->value[0];
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Val")) {
				switch(obj->pObjIndex->item_type) {
				default:
					obj->value[0] = fread_number(fp);
					obj->value[1] = fread_number(fp);
					obj->value[2] = fread_number(fp);
					obj->value[3] = fread_number(fp);
					obj->value[4] = fread_number(fp);
					break;
				case ITEM_PILL:
				case ITEM_POTION:
				case ITEM_SCROLL:
					obj->value[0] = fread_number(fp);
							fread_number(fp);
							fread_number(fp);
							fread_number(fp);
							fread_number(fp);
					break;
				case ITEM_WEAPON:
					obj->value[0] = fread_number(fp);
					obj->value[1] = fread_number(fp);
					obj->value[2] = fread_number(fp);
					STR_VAL_ASSIGN(obj->value[3],
						damtype_slot_lookup(fread_number(fp)));
					obj->value[4] = fread_number(fp);
					break;
				case ITEM_STAFF:
				case ITEM_WAND:
					obj->value[0] = fread_number(fp);
					obj->value[1] = fread_number(fp);
					obj->value[2] = fread_number(fp);
							fread_number(fp);
					obj->value[4] = fread_number(fp);
					break;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Vnum")) {
				int             vnum;
				vnum = fread_number(fp);
				if ((obj->pObjIndex = get_obj_index(vnum)) == NULL)
					log("fread_obj: %s: bad vnum %d",
						   ch->name, vnum);
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
			log("fread_obj: %s: %s: no match",
				   ch->name, word);
			fread_to_eol(fp);
		}
	}
}

