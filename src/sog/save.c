/*
 * $Id: save.c,v 1.46 1998-08-14 05:45:17 fjoe Exp $
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

#ifdef BSD44
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "db.h"
#include "comm.h"
#include "const.h"
#include "act_info.h"
#include "resource.h"
#include "hometown.h"
#include "magic.h"
#include "quest.h"
#include "util.h"
#include "log.h"
#include "mlstring.h"

extern int _filbuf args((FILE *));

int             rename(const char *oldfname, const char *newfname);


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];



/*
 * Local functions.
 */
void fwrite_char (CHAR_DATA * ch, FILE * fp, bool reboot);
void fwrite_obj (CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest);
void fwrite_pet (CHAR_DATA * pet, FILE * fp);
void fread_char (CHAR_DATA * ch, FILE * fp);
void fread_pet  (CHAR_DATA * ch, FILE * fp);
void fread_obj  (CHAR_DATA * ch, FILE * fp);



/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */

void save_char_obj(CHAR_DATA * ch, bool reboot)
{
	char            strsave[PATH_MAX];
	FILE           *fp;
	if (IS_NPC(ch) || ch->level < 2)
		return;

	if (ch->desc != NULL && ch->desc->original != NULL)
		ch = ch->desc->original;

	/* create god log */
	if (IS_IMMORTAL(ch)) {
		fclose(fpReserve);
		snprintf(strsave, sizeof(strsave),
			 "%s%s", GOD_DIR, capitalize(ch->name));
		if ((fp = fopen(strsave, "w")) == NULL) {
			bug("Save_char_obj: fopen", 0);
			perror(strsave);
		}
		fprintf(fp, "Lev %2d Trust %2d  %s%s\n",
		      ch->level, get_trust(ch), ch->name, ch->pcdata->title);
		fclose(fp);
		fpReserve = fopen(NULL_FILE, "r");
		if (fpReserve == NULL)
			bug("save_char_obj: Can't open null file.", 0);

	}
	fclose(fpReserve);
	snprintf(strsave, sizeof(strsave),
		 "%s%s", PLAYER_DIR, capitalize(ch->name));
	if ((fp = fopen(TEMP_FILE, "w")) == NULL) {
		bug("Save_char_obj: fopen", 0);
		perror(strsave);
	} else {
		send_to_char("Saving.\n\r", ch);
		fwrite_char(ch, fp, reboot);
		if (ch->carrying != NULL)
			fwrite_obj(ch, ch->carrying, fp, 0);
		/* save the pets */
		if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
			fwrite_pet(ch->pet, fp);
		fprintf(fp, "#END\n");
	}
	fclose(fp);
	rename(TEMP_FILE, strsave);
	fpReserve = fopen(NULL_FILE, "r");
	if (fpReserve == NULL)
		bug("save_char_obj: Can't open null file.", 0);
}



/*
 * Write the char.
 */
void 
fwrite_char(CHAR_DATA * ch, FILE * fp, bool reboot)
{
	AFFECT_DATA    *paf;
	int             sn, pos;
	fprintf(fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER");

	fprintf(fp, "Name %s~\n", ch->name);
	fprintf(fp, "Id   %d\n", ch->id);
	fprintf(fp, "LogO %ld\n", current_time);
	fprintf(fp, "Vers %d\n", 6);
	fprintf(fp, "Etho %d\n", ch->ethos);
	fprintf(fp, "Home %d\n", ch->hometown);

	if (ch->clan) {
		fprintf(fp, "Clan %s~\n", clan_table[ch->clan].short_name);
		if (!IS_NPC(ch))
			fprintf(fp, "ClanStatus %d\n", ch->pcdata->clan_status);
	}

	if (!IS_NULLSTR(mlstr_mval(ch->description)))
		fprintf(fp, "Desc %s~\n", mlstr_mval(ch->description));
	if (ch->prompt != NULL || !str_cmp(ch->prompt, DEFAULT_PROMPT))
		fprintf(fp, "Prom %s~\n", ch->prompt);
	fprintf(fp, "Race %s~\n", pc_race_table[ORG_RACE(ch)].name);
	fprintf(fp, "Sex  %d\n", ch->sex);
	fprintf(fp, "Cla  %d\n", ch->class);
	fprintf(fp, "Levl %d\n", ch->level);
	if (ch->trust != 0)
		fprintf(fp, "Tru  %d\n", ch->trust);
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
	fprintf(fp, "Exp  %d\n", ch->exp);
	if (ch->act != 0)
		if (IS_NPC(ch))
			fprintf(fp, "Act  %s\n", format_flags(ch->act));
		else
			fprintf(fp, "Act  %s\n", 
				format_flags(ch->act & ~(PLR_NOEXP |
							 PLR_CHANGED_AFF |
							 PLR_GHOST)));
	if (ch->affected_by != 0) {
		if (IS_NPC(ch))
			fprintf(fp, "AfBy %s\n", format_flags(ch->affected_by));
		else
			fprintf(fp, "AfBy %s\n",
			     format_flags((ch->affected_by & (~AFF_CHARM))));
	}
	if (ch->detection != 0)
		fprintf(fp, "Detect %s\n", format_flags(ch->detection));
	fprintf(fp, "Comm %s\n", format_flags(ch->comm & ~(COMM_AFK)));
	if (ch->wiznet)
		fprintf(fp, "Wizn %s\n", format_flags(ch->wiznet));
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

		for (qt = ch->pcdata->qtrouble; qt; qt = qt->next)
			fprintf(fp, "Qtrouble %d %d\n", qt->vnum, qt->count);

		fprintf(fp, "Plyd %d\n",
			ch->pcdata->played + (int) (current_time - ch->logon));
		fprintf(fp, "Not  %ld %ld %ld %ld %ld\n",
			ch->pcdata->last_note, ch->pcdata->last_idea,
			ch->pcdata->last_penalty, ch->pcdata->last_news,
			ch->pcdata->last_changes);

		fprintf(fp, "Dead %d\n", ch->pcdata->death);

		if (ch->pcdata->bank_s > 0)
			fprintf(fp, "Banks %d\n", ch->pcdata->bank_s);
		else
			fprintf(fp, "Banks %d\n", ch->pcdata->bank_s);
		if (ch->pcdata->bank_g > 0)
			fprintf(fp, "Bankg %d\n", ch->pcdata->bank_g);
		else
			fprintf(fp, "Bankg %d\n", ch->pcdata->bank_g);

		fprintf( fp, "Sec  %d\n", ch->pcdata->security); /* OLC */
		fprintf(fp, "Pass %s~\n", ch->pcdata->pwd);
		if (ch->pcdata->bamfin[0] != '\0')
			fprintf(fp, "Bin  %s~\n", ch->pcdata->bamfin);
		if (ch->pcdata->bamfout[0] != '\0')
			fprintf(fp, "Bout %s~\n", ch->pcdata->bamfout);
		fprintf(fp, "Titl %s~\n", ch->pcdata->title);
		fprintf(fp, "Pnts %d\n", ch->pcdata->points);
		fprintf(fp, "TSex %d\n", ch->pcdata->true_sex);
		fprintf(fp, "LLev %d\n", ch->pcdata->last_level);
		fprintf(fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit,
			ch->pcdata->perm_mana,
			ch->pcdata->perm_move);
		fprintf(fp, "CndC  %d %d %d %d %d %d\n",
			ch->pcdata->condition[0],
			ch->pcdata->condition[1],
			ch->pcdata->condition[2],
			ch->pcdata->condition[3],
			ch->pcdata->condition[4],
			ch->pcdata->condition[5]);
		/* write lang */
		fprintf(fp, "Lang %d\n", ch->lang);

		/* write pc_killed */
		fprintf(fp, "PC_Killed %d\n", ch->pcdata->pc_killed);

		/* write alias */
		for (pos = 0; pos < MAX_ALIAS; pos++) {
			if (ch->pcdata->alias[pos] == NULL
			    || ch->pcdata->alias_sub[pos] == NULL)
				break;

			fprintf(fp, "Alias %s %s~\n", ch->pcdata->alias[pos],
				ch->pcdata->alias_sub[pos]);
		}

		for (sn = 0; sn < MAX_SKILL; sn++) {
			if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0) {
				fprintf(fp, "Sk %d '%s'\n",
					ch->pcdata->learned[sn], skill_table[sn].name);
			}
		}

		if (ch->pcdata->questpoints != 0)
			fprintf(fp, "QuestPnts %d\n", ch->pcdata->questpoints);
		if (!reboot) {
			if (ch->pcdata->questtime != 0)
				fprintf(fp, "QuestTime %d\n", ch->pcdata->questtime);
			if (IS_ON_QUEST(ch)) {
				fprintf(fp, "QuestMob %d\n", ch->pcdata->questmob);
				fprintf(fp, "QuestObj %d\n", ch->pcdata->questobj);
				fprintf(fp, "QuestGiv %d\n", ch->pcdata->questgiver);
			}
		}
		fprintf(fp, "Haskilled %d\n", ch->pcdata->has_killed);
		fprintf(fp, "Antkilled %d\n", ch->pcdata->anti_killed);
	}

	for (paf = ch->affected; paf != NULL; paf = paf->next) {
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;

		if (!IS_NPC(ch) && paf->bitvector == AFF_CHARM)
			continue;

		fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
			skill_table[paf->type].name,
			paf->where,
			paf->level,
			paf->duration,
			paf->modifier,
			paf->location,
			paf->bitvector
			);
	}

	fprintf(fp, "End\n\n");
	return;
}
/* write a pet */
void 
fwrite_pet(CHAR_DATA * pet, FILE * fp)
{
	AFFECT_DATA    *paf;
	fprintf(fp, "#PET\n");

	fprintf(fp, "Vnum %d\n", pet->pIndexData->vnum);

	fprintf(fp, "Name %s~\n", pet->name);
	fprintf(fp, "LogO %ld\n", current_time);
	if (pet->clan)
		fprintf(fp, "Clan %s~\n", clan_table[pet->clan].short_name);
	if (mlstr_cmp(pet->short_descr, pet->pIndexData->short_descr) != 0)
		mlstr_fwrite(fp, "ShD", pet->short_descr);
	if (mlstr_cmp(pet->long_descr, pet->pIndexData->long_descr) != 0)
		mlstr_fwrite(fp, "LnD", pet->short_descr);
	if (mlstr_cmp(pet->description, pet->pIndexData->description) != 0)
		mlstr_fwrite(fp, "Desc", pet->short_descr);
	if (RACE(pet) != pet->pIndexData->race)	/* serdar ORG_RACE */
		fprintf(fp, "Race %s~\n", race_table[ORG_RACE(pet)].name);
	fprintf(fp, "Sex  %d\n", pet->sex);
	if (pet->level != pet->pIndexData->level)
		fprintf(fp, "Levl %d\n", pet->level);
	fprintf(fp, "HMV  %d %d %d %d %d %d\n",
		pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
	if (pet->gold > 0)
		fprintf(fp, "Gold %d\n", pet->gold);
	if (pet->silver > 0)
		fprintf(fp, "Silv %d\n", pet->silver);
	if (pet->exp > 0)
		fprintf(fp, "Exp  %d\n", pet->exp);
	if (pet->act != pet->pIndexData->act)
		fprintf(fp, "Act  %s\n", format_flags(pet->act));
	if (pet->affected_by != pet->pIndexData->affected_by)
		fprintf(fp, "AfBy %s\n", format_flags(pet->affected_by));
	if (pet->detection != pet->pIndexData->detection)
		fprintf(fp, "Detect %s\n", format_flags(pet->detection));
	if (pet->comm != 0)
		fprintf(fp, "Comm %s\n", format_flags(pet->comm));
	fprintf(fp, "Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
	if (pet->saving_throw != 0)
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

	for (paf = pet->affected; paf != NULL; paf = paf->next) {
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;

		fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
			skill_table[paf->type].name,
			paf->where, paf->level, paf->duration, paf->modifier, paf->location,
			paf->bitvector);
	}

	fprintf(fp, "End\n");
	return;
}
/*
 * Write an object and its contents.
 */
void 
fwrite_obj(CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest)
{
	ED_DATA *ed;
	AFFECT_DATA    *paf;
	int             i;
	/*
	 * Slick recursion to write lists backwards, so loading them will load
	 * in forwards order.
	 */
	if (obj->next_content != NULL)
		fwrite_obj(ch, obj->next_content, fp, iNest);

	for (i = 1; clan_table[i].long_name != NULL; i++)
		if (obj->pIndexData->vnum == clan_table[i].obj_vnum)
			return;

	/*
	 * Castrate storage characters.
	 */
	if (((ch->level < 10) && (obj->pIndexData->limit != -1))
	    || (obj->item_type == ITEM_KEY && obj->value[0] == 0)
	    || (obj->item_type == ITEM_MAP && !obj->value[0])
	    || ((ch->level < obj->level - 3) && (obj->item_type != ITEM_CONTAINER))
	|| ((ch->level > obj->level + 35) && (obj->pIndexData->limit > 1))) {
		extract_obj(obj);
		return;
	}
/* FIXX ME
	if (obj->pIndexData->vnum == QUEST_ITEM1
		|| obj->pIndexData->vnum == QUEST_ITEM2
		|| obj->pIndexData->vnum == QUEST_ITEM3
		|| obj->pIndexData->vnum == OBJ_VNUM_EYED_SWORD)
	if (strstr(mlstr_mval(obj->short_descr), ch->name) == NULL) {
		act("$p vanishes!",ch,obj,NULL,TO_CHAR);
		extract_obj(obj);
		return;
	}
*/

	fprintf(fp, "#O\n");
	fprintf(fp, "Vnum %d\n", obj->pIndexData->vnum);
	fprintf(fp, "Cond %d\n", obj->condition);

	if (!obj->pIndexData->new_format)
		fprintf(fp, "Oldstyle\n");
	if (obj->enchanted)
		fprintf(fp, "Enchanted\n");
	fprintf(fp, "Nest %d\n", iNest);

	/* these data are only used if they do not match the defaults */

	if (obj->name != obj->pIndexData->name)
		fprintf(fp, "Name %s~\n", obj->name);
	if (mlstr_cmp(obj->short_descr, obj->pIndexData->short_descr) != 0)
		mlstr_fwrite(fp, "ShD", obj->short_descr);
	if (mlstr_cmp(obj->description, obj->pIndexData->description) != 0)
		mlstr_fwrite(fp, "Desc", obj->short_descr);
	if (obj->extra_flags != obj->pIndexData->extra_flags)
		fprintf(fp, "ExtF %d\n", obj->extra_flags);
	if (obj->wear_flags != obj->pIndexData->wear_flags)
		fprintf(fp, "WeaF %d\n", obj->wear_flags);
	if (obj->item_type != obj->pIndexData->item_type)
		fprintf(fp, "Ityp %d\n", obj->item_type);
	if (obj->weight != obj->pIndexData->weight)
		fprintf(fp, "Wt   %d\n", obj->weight);

	/* variable data */

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

	switch (obj->item_type) {
	case ITEM_POTION:
	case ITEM_SCROLL:
		if (obj->value[1] > 0) {
			fprintf(fp, "Spell 1 '%s'\n",
				skill_table[obj->value[1]].name);
		}
		if (obj->value[2] > 0) {
			fprintf(fp, "Spell 2 '%s'\n",
				skill_table[obj->value[2]].name);
		}
		if (obj->value[3] > 0) {
			fprintf(fp, "Spell 3 '%s'\n",
				skill_table[obj->value[3]].name);
		}
		break;

	case ITEM_PILL:
	case ITEM_STAFF:
	case ITEM_WAND:
		if (obj->value[3] > 0) {
			fprintf(fp, "Spell 3 '%s'\n",
				skill_table[obj->value[3]].name);
		}
		break;
	}

	for (paf = obj->affected; paf != NULL; paf = paf->next) {
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;
		fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
			skill_table[paf->type].name,
			paf->where,
			paf->level,
			paf->duration,
			paf->modifier,
			paf->location,
			paf->bitvector
			);
	}

	for (ed = obj->ed; ed != NULL; ed = ed->next) {
		fprintf(fp, "ExDe %s~ ", ed->keyword);
		mlstr_fwrite(fp, NULL, ed->description);
	}

	fprintf(fp, "End\n\n");

	if (obj->contains != NULL)
		fwrite_obj(ch, obj->contains, fp, iNest + 1);
}


void add_race_skills(CHAR_DATA * ch, int race);

/*
 * Load a char and inventory into a new ch structure.
 */
bool 
load_char_obj(DESCRIPTOR_DATA * d, const char *name)
{
	char            strsave[PATH_MAX];
	CHAR_DATA      *ch;
	FILE           *fp;
	bool            found;
	int		sn;
	ch = new_char();
	ch->pcdata = new_pcdata();

	d->character = ch;
	ch->desc = d;
	ch->name = str_dup(name);
	ch->name[0] = UPPER(ch->name[0]);
	ch->id = get_pc_id();
	ch->race = race_lookup("human");
	ch->act = PLR_NOSUMMON;
	ch->comm = COMM_COMBINE | COMM_PROMPT;
	ch->prompt = str_dup(DEFAULT_PROMPT);

	ch->pcdata->race = ch->race;
	ch->pcdata->clan_status = CLAN_COMMON;
	ch->pcdata->points = 0;
	ch->pcdata->confirm_delete = FALSE;
	ch->pcdata->pwd = str_dup("");
	ch->pcdata->bamfin = str_dup("");
	ch->pcdata->bamfout = str_dup("");
	ch->pcdata->title = str_dup("");
	for (sn = 0; sn < MAX_SKILL; sn++)
		ch->pcdata->learned[sn] = 0;
	ch->pcdata->condition[COND_THIRST] = 48;
	ch->pcdata->condition[COND_FULL] = 48;
	ch->pcdata->condition[COND_HUNGER] = 48;
	ch->pcdata->condition[COND_BLOODLUST] = 48;
	ch->pcdata->condition[COND_DESIRE] = 48;
	ch->pcdata->security		= 0;	/* OLC */

	ch->pcdata->pc_killed = 0;
	ch->pcdata->petition = 0;
	ch->pcdata->questpoints = 0;
	ch->pcdata->questgiver = 0;
	ch->pcdata->questtime = 0;
	ch->pcdata->questobj = 0;
	ch->pcdata->questmob = 0;
	ch->pcdata->has_killed = 0;
	ch->pcdata->anti_killed = 0;

	found = FALSE;
	fclose(fpReserve);

	/* decompress if .gz file exists */
	snprintf(strsave, sizeof(strsave),
		 "%s%s%s", PLAYER_DIR, capitalize(name), ".gz");
	if ((fp = fopen(strsave, "r")) != NULL) {
		char buf[PATH_MAX * 2];
		fclose(fp);
		snprintf(buf, sizeof(buf), "gzip -dfq %s", strsave);
		system(buf);
	}
	snprintf(strsave, sizeof(strsave),
		 "%s%s", PLAYER_DIR, capitalize(name));
	if ((fp = fopen(strsave, "r")) != NULL) {
		int             iNest;
		for (iNest = 0; iNest < MAX_NEST; iNest++)
			rgObjNest[iNest] = NULL;

		found = TRUE;
		for (;;) {
			char            letter;
			char           *word;
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
			if (!str_cmp(word, "PLAYER")) {
				fread_char(ch, fp);
				add_race_skills(ch, ch->pcdata->race);
			} else if (!str_cmp(word, "OBJECT"))
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
	fpReserve = fopen(NULL_FILE, "r");
	if (fpReserve == NULL)
		bug("load_char: Can't open null file.", 0);


	/* initialize race */
	if (found) {
		if (ORG_RACE(ch) == 0)
			ORG_RACE(ch) = race_lookup("human");
		if (RACE(ch) == 0)
			RACE(ch) = race_lookup("human");

		ch->size = pc_race_table[ORG_RACE(ch)].size;
		ch->dam_type = 17;	/* punch */

		ch->detection = ch->detection | race_table[RACE(ch)].det;
		ch->affected_by = ch->affected_by | race_table[RACE(ch)].aff;
		ch->imm_flags = ch->imm_flags | race_table[RACE(ch)].imm;
		ch->res_flags = ch->res_flags | race_table[RACE(ch)].res;
		ch->vuln_flags = ch->vuln_flags | race_table[RACE(ch)].vuln;
		ch->form = race_table[RACE(ch)].form;
		ch->parts = race_table[RACE(ch)].parts;
	}
	if (ch->pcdata->condition[COND_BLOODLUST] < 48
	    && ch->class != CLASS_VAMPIRE)
		ch->pcdata->condition[COND_BLOODLUST] = 48;

	if (found && ch->version < 6)
		ch->pcdata->learned[gsn_spell_craft] = 1;
	return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY(literal, field, value)		\
	if (str_cmp(word, literal) == 0) { 	\
		field  = value;			\
		fMatch = TRUE;			\
		break;				\
	}

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
			KEY("Act", ch->act, fread_flags(fp) & ~PLR_GHOST);
			KEY("AffectedBy", ch->affected_by, fread_flags(fp));
			KEY("AfBy", ch->affected_by, fread_flags(fp));
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
				paf = new_affect();

				sn = skill_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_char: unknown skill.", 0);
				else
					paf->type = sn;

				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_number(fp);
				paf->next = ch->affected;
				ch->affected = paf;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA    *paf;
				int             sn;
				paf = new_affect();

				sn = skill_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_char: unknown skill.", 0);
				else
					paf->type = sn;

				paf->where = fread_number(fp);
				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_number(fp);
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
			KEY("Bamfin", ch->pcdata->bamfin, fread_string(fp));
			KEY("Banks", ch->pcdata->bank_s, fread_number(fp));
			KEY("Bankg", ch->pcdata->bank_g, fread_number(fp));
			KEY("Bamfout", ch->pcdata->bamfout, fread_string(fp));
			KEY("Bin", ch->pcdata->bamfin, fread_string(fp));
			KEY("Bout", ch->pcdata->bamfout, fread_string(fp));
			break;

		case 'C':
			KEY("Class", ch->class, fread_number(fp));
			KEY("Cla", ch->class, fread_number(fp));
			KEY("ClanStatus", ch->pcdata->clan_status,
			    fread_number(fp));

			if (!str_cmp(word, "Clan")) {
				char *p = fread_string(fp);
				int clan = clan_lookup(p);

				if (clan < 0) 
					log_printf("fread_char: unknown clan:"
						   " `%s'", p);
				else
					ch->clan = clan;
				fMatch = TRUE;
				break;
			}
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
			KEY("Comm", ch->comm, fread_flags(fp));

			break;

		case 'D':
			KEY("Damroll", ch->damroll, fread_number(fp));
			KEY("Dam", ch->damroll, fread_number(fp));
			KEY("Desc", ch->description, mlstr_fread(fp));
			KEY("Dead", ch->pcdata->death, fread_number(fp));
			KEY("Detect", ch->detection, fread_flags(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
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
				return;
			}
			KEY("Exp", ch->exp, fread_number(fp));
			KEY("Etho", ch->ethos, fread_number(fp));
			break;

		case 'G':
			KEY("Gold", ch->gold, fread_number(fp));
			if (!str_cmp(word, "Group") || !str_cmp(word, "Gr")) {
				fread_word(fp);
				fMatch = TRUE;
			}
			break;

		case 'H':
			KEY("Hitroll", ch->hitroll, fread_number(fp));
			KEY("Hit", ch->hitroll, fread_number(fp));
			KEY("Home", ch->hometown, fread_number(fp));
			KEY("Haskilled", ch->pcdata->has_killed, fread_number(fp));

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
			KEY("Name", ch->name, fread_string(fp));
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

		case 'P':
			KEY("Password", ch->pcdata->pwd, fread_string(fp));
			KEY("Pass", ch->pcdata->pwd, fread_string(fp));
			KEY("PC_Killed", ch->pcdata->pc_killed, fread_number(fp));
			KEY("Played", ch->pcdata->played, fread_number(fp));
			KEY("Plyd", ch->pcdata->played, fread_number(fp));
			KEY("Points", ch->pcdata->points, fread_number(fp));
			KEY("Pnts", ch->pcdata->points, fread_number(fp));
			KEY("Position", ch->position, fread_number(fp));
			KEY("Pos", ch->position, fread_number(fp));
			KEY("Practice", ch->practice, fread_number(fp));
			KEY("Prac", ch->practice, fread_number(fp));
			KEY("Prompt", ch->prompt, fread_string(fp));
			KEY("Prom", ch->prompt, fread_string(fp));
			break;

		case 'Q':
			KEY("QuestTime", ch->pcdata->questtime, fread_number(fp));
			KEY("QuestMob", ch->pcdata->questmob, fread_number(fp));
			KEY("QuestObj", ch->pcdata->questobj, fread_number(fp));
			KEY("QuestGiv", ch->pcdata->questgiver, fread_number(fp));
			KEY("QuestPnts", ch->pcdata->questpoints, fread_number(fp));

			/* handle old quest trouble */
			if (strcmp(word, "Ques") == 0) {

#define QUEST_ITEM1 94
#define QUEST_ITEM2 95
#define QUEST_ITEM3 96
#define QUEST_WEAPON		(C)
#define QUEST_GIRTH		(D)
#define QUEST_RING		(E)
#define QUEST_WEAPON2		(F)
#define QUEST_GIRTH2		(G)
#define QUEST_RING2		(H)
#define QUEST_WEAPON3		(I)
#define QUEST_GIRTH3		(J)
#define QUEST_RING3		(K)

				int             old_quest;
				old_quest = fread_flags(fp);
				fMatch = TRUE;
				if (old_quest & QUEST_GIRTH)
					qtrouble_set(ch, QUEST_ITEM1, 1);
				if (old_quest & QUEST_GIRTH2)
					qtrouble_set(ch, QUEST_ITEM1, 2);
				if (old_quest & QUEST_GIRTH3)
					qtrouble_set(ch, QUEST_ITEM1, 3);
				if (old_quest & QUEST_RING)
					qtrouble_set(ch, QUEST_ITEM2, 1);
				if (old_quest & QUEST_RING2)
					qtrouble_set(ch, QUEST_ITEM2, 2);
				if (old_quest & QUEST_RING3)
					qtrouble_set(ch, QUEST_ITEM2, 3);
				if (old_quest & QUEST_WEAPON)
					qtrouble_set(ch, QUEST_ITEM3, 1);
				if (old_quest & QUEST_WEAPON2)
					qtrouble_set(ch, QUEST_ITEM3, 2);
				if (old_quest & QUEST_WEAPON3)
					qtrouble_set(ch, QUEST_ITEM3, 3);
			}
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

/*		KEY("Race",        ch->race,  race_lookup(fread_string(fp))); */
			if (!str_cmp(word, "Race")) {
				RACE(ch) = race_lookup(fread_string(fp));
				ORG_RACE(ch) = RACE(ch);
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
			KEY("ShortDescr", ch->short_descr, mlstr_fread(fp));
			KEY("ShD", ch->short_descr, mlstr_fread(fp));
			KEY("Sec", ch->pcdata->security, fread_number(fp));
			KEY("Silv", ch->silver, fread_number(fp));

			if (!str_cmp(word, "Skill") || !str_cmp(word, "Sk")) {
				int             sn;
				int             value;
				char           *temp;
				value = fread_number(fp);
				temp = fread_word(fp);
				sn = skill_lookup(temp);
				if (sn < 0) {
					fprintf(stderr, "%s", temp);
					bug("Fread_char: unknown skill. ", 0);
				} else
					ch->pcdata->learned[sn] = value;
				fMatch = TRUE;
			}
			break;

		case 'T':
			KEY("TrueSex", ch->pcdata->true_sex, fread_number(fp));
			KEY("TSex", ch->pcdata->true_sex, fread_number(fp));
			KEY("Trai", ch->train, fread_number(fp));
			KEY("Trust", ch->trust, fread_number(fp));
			KEY("Tru", ch->trust, fread_number(fp));

			if (!str_cmp(word, "Title") || !str_cmp(word, "Titl")) {
				char           *p = fread_string(fp);
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
			KEY("Wizn", ch->wiznet, fread_flags(fp));
			break;
		}

		if (!fMatch) {
			bug("Fread_char: no match.", 0);
			fread_to_eol(fp);
		}
	}
	return;
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
			KEY("Act", pet->act, fread_flags(fp));
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
				paf = new_affect();

				sn = skill_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_char: unknown skill.", 0);
				else
					paf->type = sn;

				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_number(fp);
				paf->next = pet->affected;
				pet->affected = paf;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA    *paf;
				int             sn;
				paf = new_affect();

				sn = skill_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_char: unknown skill.", 0);
				else
					paf->type = sn;

				paf->where = fread_number(fp);
				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_number(fp);
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
			if (!str_cmp(word, "Clan")) {
				char *p = fread_string(fp);
				int clan = clan_lookup(p);

				if (clan < 0) 
					log_printf("fread_pet: unknown clan:"
						   " `%s'", p);
				else
					pet->clan = clan;
				fMatch = TRUE;
				break;
			}
			KEY("Comm", pet->comm, fread_flags(fp));
			break;

		case 'D':
			KEY("Dam", pet->damroll, fread_number(fp));
			KEY("Desc", pet->description, mlstr_fread(fp));
			KEY("Detect", pet->detection, fread_flags(fp));
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
			KEY("LnD",  pet->description, mlstr_fread(fp));
			KEY("Levl", pet->level, fread_number(fp));
			KEY("LogO", lastlogoff, fread_number(fp));
			break;

		case 'N':
			KEY("Name", pet->name, fread_string(fp));
			break;

		case 'P':
			KEY("Pos", pet->position, fread_number(fp));
			break;

		case 'R':
			if (!str_cmp(word, "Race")) {
				RACE(pet) = race_lookup(fread_string(fp));
				ORG_RACE(pet) = RACE(pet);
				fMatch = TRUE;
				break;
			}
			break;

		case 'S':
			KEY("Save", pet->saving_throw, fread_number(fp));
			KEY("Sex", pet->sex, fread_number(fp));
			KEY("ShD", pet->short_descr, mlstr_fread(fp));
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
	bool            new_format;	/* to prevent errors */
	bool            make_new;	/* update object */
	fVnum = FALSE;
	obj = NULL;
	first = TRUE;		/* used to counter fp offset */
	new_format = FALSE;
	make_new = FALSE;

	word = feof(fp) ? "End" : fread_word(fp);
	if (!str_cmp(word, "Vnum")) {
		int             vnum;
		first = FALSE;	/* fp will be in right place */

		vnum = fread_number(fp);
		if (get_obj_index(vnum) == NULL) {
			bug("Fread_obj: bad vnum %d.", vnum);
		} else {
			obj = create_obj_nocount(get_obj_index(vnum), -1);
			new_format = TRUE;
		}

	}
	if (obj == NULL) {	/* either not found or old style */
		obj = new_obj();
		obj->name = str_dup("");
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
				paf = new_affect();

				sn = skill_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_obj: unknown skill.", 0);
				else
					paf->type = sn;

				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_number(fp);
				paf->next = obj->affected;
				obj->affected = paf;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA    *paf;
				int             sn;
				paf = new_affect();

				sn = skill_lookup(fread_word(fp));
				if (sn < 0)
					bug("Fread_obj: unknown skill.", 0);
				else
					paf->type = sn;

				paf->where = fread_number(fp);
				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_number(fp);
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
			KEY("Description", obj->description, mlstr_fread(fp));
			KEY("Desc", obj->description, mlstr_fread(fp));
			break;

		case 'E':

			if (!str_cmp(word, "Enchanted")) {
				obj->enchanted = TRUE;
				fMatch = TRUE;
				break;
			}
			KEY("ExtraFlags", obj->extra_flags, fread_number(fp));
			KEY("ExtF", obj->extra_flags, fread_number(fp));

			if (!str_cmp(word, "ExtraDescr") || !str_cmp(word, "ExDe")) {
				ed_fread(fp, &obj->ed);
				fMatch = TRUE;
			}
			if (!str_cmp(word, "End")) {
				if (!fNest
				||  (fVnum && obj->pIndexData == NULL)) {
					bug("Fread_obj: incomplete object.", 0);
					free_obj(obj);
					return;
				} else {
					if (!fVnum) {
						free_obj(obj);
						obj = create_obj(get_obj_index(OBJ_VNUM_DUMMY), 0);
					}

					if (!new_format) {
						obj->next = object_list;
						object_list = obj;
						obj->pIndexData->count++;
					}

					if (!obj->pIndexData->new_format
					    && obj->item_type == ITEM_ARMOR
					    && obj->value[1] == 0) {
						obj->value[1] = obj->value[0];
						obj->value[2] = obj->value[0];
					}

					if (make_new) {
						int             wear;
						wear = obj->wear_loc;
						extract_obj(obj);

						obj = create_obj(obj->pIndexData, 0);
						obj->wear_loc = wear;
					}

					if (iNest == 0 || rgObjNest[iNest] == NULL)
						obj_to_char(obj, ch);
					else
						obj_to_obj(obj, rgObjNest[iNest - 1]);
					return;
				}
			}
			break;

		case 'I':
			KEY("ItemType", obj->item_type, fread_number(fp));
			KEY("Ityp", obj->item_type, fread_number(fp));
			break;

		case 'L':
			KEY("Level", obj->level, fread_number(fp));
			KEY("Lev", obj->level, fread_number(fp));
			break;

		case 'N':
			KEY("Name", obj->name, fread_string(fp));

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
			if (!str_cmp(word, "Oldstyle")) {
				if (obj->pIndexData != NULL && obj->pIndexData->new_format)
					make_new = TRUE;
				fMatch = TRUE;
			}
			break;


		case 'Q':
			KEY("Quality", obj->condition, fread_number(fp));
			break;

		case 'S':
			KEY("ShortDescr", obj->short_descr, mlstr_fread(fp));
			KEY("ShD", obj->short_descr, mlstr_fread(fp));

			if (!str_cmp(word, "Spell")) {
				int             iValue;
				int             sn;
				iValue = fread_number(fp);
				sn = skill_lookup(fread_word(fp));
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
				if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
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
			KEY("WearFlags", obj->wear_flags, fread_number(fp));
			KEY("WeaF", obj->wear_flags, fread_number(fp));
			KEY("WearLoc", obj->wear_loc, fread_number(fp));
			KEY("Wear", obj->wear_loc, fread_number(fp));
			KEY("Weight", obj->weight, fread_number(fp));
			KEY("Wt", obj->weight, fread_number(fp));
			break;

		}

		if (!fMatch) {
			bug("Fread_obj: no match.", 0);
			fread_to_eol(fp);
		}
	}
}
