/*
 * $Id: handler.c,v 1.182.2.16 2000-03-28 11:03:02 fjoe Exp $
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

/**************************************************************************r
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
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "obj_prog.h"
#include "fight.h"
#include "quest.h"
#include "chquest.h"
#include "db.h"
#include "lang.h"
#include "mob_prog.h"
#include "auction.h"

/*
 * Local functions.
 */
void	affect_modify	(CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd);
void	strip_raff_owner(CHAR_DATA *ch);

/*
 * Room record:
 * For less than 5 people in room create a new record.
 * Else use the oldest one.
 */

void room_record(const char *name,ROOM_INDEX_DATA *room,int door)
{
  ROOM_HISTORY_DATA *rec;
  int i;

  for (i=0,rec = room->history;i < 5 && rec != NULL;
	   i++,rec = rec->next);

  if (i < 5) {
	rec = calloc(1, sizeof(*rec)); 
	rec->next = room->history;
	if (rec->next != NULL)
	  rec->next->prev = rec; 
	room->history = rec; 
	rec->name = NULL;
  }
  else { 
	rec = room->history->next->next->next->next; 
	rec->prev->next = NULL; 
	rec->next = room->history; 
	rec->next->prev = rec; 
	room->history = rec; 
  }
  rec->prev = NULL;

  if(rec->name) {
	 free_string(rec->name);
  }


  rec->name = str_dup(name);
  rec->went = door;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
	CHAR_DATA *fch;
	int count = 0;

	if (obj->in_room == NULL)
		return 0;

	for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
		if (fch->on == obj)
		    count++;

	return count;
}

/*
 * Check the material
 */
bool check_material(OBJ_DATA *obj, char *material)
{
	return strstr(obj->material, material) ? TRUE : FALSE;
}

bool is_metal(OBJ_DATA *obj)
{

  if (check_material(obj, "silver") ||
	   check_material(obj, "gold") ||
	   check_material(obj, "iron") ||
	   check_material(obj, "mithril") ||
	   check_material(obj, "adamantite") ||
	   check_material(obj, "steel") ||
	   check_material(obj, "lead") ||
	   check_material(obj, "bronze") ||
	   check_material(obj, "copper") ||
	   check_material(obj, "brass") ||
	   check_material(obj, "platinium") ||
	   check_material(obj, "titanium") ||
	   check_material(obj, "aliminum"))
	return TRUE;

  return FALSE;

}

bool may_float(OBJ_DATA *obj)
{

	if (check_material(obj, "wood")  ||
	     check_material(obj, "ebony")  ||
	     check_material(obj, "ice")  ||
	     check_material(obj, "energy")  ||
	     check_material(obj, "hardwood")  ||
	     check_material(obj, "softwood")  ||
	     check_material(obj, "flesh")  ||
	     check_material(obj, "silk")  ||
	     check_material(obj, "wool")  ||
	     check_material(obj, "cloth")  ||
	     check_material(obj, "fur")  ||
	     check_material(obj, "water")  ||
	     check_material(obj, "ice")  ||
	     check_material(obj, "oak"))
	   return TRUE;

	if (obj->pObjIndex->item_type == ITEM_BOAT) 
		return TRUE;

	return FALSE;
}


bool cant_float(OBJ_DATA *obj)
{
	if (check_material(obj, "steel") ||
	     check_material(obj, "iron") ||
	     check_material(obj, "brass") ||
	     check_material(obj, "silver") ||
	     check_material(obj, "gold") ||
	     check_material(obj, "ivory") ||
	     check_material(obj, "copper") ||
	     check_material(obj, "diamond") ||
	     check_material(obj, "pearl") ||
	     check_material(obj, "gem") ||
	     check_material(obj, "platinium") ||
	     check_material(obj, "ruby") ||
	     check_material(obj, "bronze") ||
	     check_material(obj, "titanium") ||
	     check_material(obj, "mithril") ||
	     check_material(obj, "obsidian") ||
	     check_material(obj, "lead"))
	   return TRUE;

	return FALSE;
}

int floating_time(OBJ_DATA *obj)
{
 int  ftime;

 ftime = 0;
 switch(obj->pObjIndex->item_type)  
 {
	default: break;
	case ITEM_KEY 	: ftime = 1;	break;
	case ITEM_ARMOR 	: ftime = 2;	break;
	case ITEM_TREASURE 	: ftime = 2;	break;
	case ITEM_PILL 	: ftime = 2;	break;
	case ITEM_POTION 	: ftime = 3;	break;
	case ITEM_TRASH 	: ftime = 3;	break;
	case ITEM_FOOD 	: ftime = 4;	break;
	case ITEM_CONTAINER	: ftime = 5;	break;
	case ITEM_CORPSE_NPC: ftime = 10;	break;
	case ITEM_CORPSE_PC	: ftime = 10;	break;
 }
 ftime = number_fuzzy(ftime) ;

 return (ftime < 0 ? 0 : ftime);
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
	int immune, def;
	int bit;

	immune = -1;
	def = IS_NORMAL;

	if (dam_type == DAM_NONE)
		return immune;

	if (dam_type <= 3)
	{
		if (IS_SET(ch->imm_flags,IMM_WEAPON))
		    def = IS_IMMUNE;
		else if (IS_SET(ch->res_flags,RES_WEAPON))
		    def = IS_RESISTANT;
		else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
		    def = IS_VULNERABLE;
	}
	else /* magical attack */
	{	
		if (IS_SET(ch->imm_flags,IMM_MAGIC))
		    def = IS_IMMUNE;
		else if (IS_SET(ch->res_flags,RES_MAGIC))
		    def = IS_RESISTANT;
		else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
		    def = IS_VULNERABLE;
	}

	/* set bits to check -- VULN etc. must ALL be the same or this will fail */
	switch (dam_type)
	{
		case(DAM_BASH):		bit = IMM_BASH;		break;
		case(DAM_PIERCE):	bit = IMM_PIERCE;	break;
		case(DAM_SLASH):	bit = IMM_SLASH;	break;
		case(DAM_FIRE):		bit = IMM_FIRE;		break;
		case(DAM_COLD):		bit = IMM_COLD;		break;
		case(DAM_LIGHTNING):	bit = IMM_LIGHTNING;	break;
		case(DAM_ACID):		bit = IMM_ACID;		break;
		case(DAM_POISON):	bit = IMM_POISON;	break;
		case(DAM_NEGATIVE):	bit = IMM_NEGATIVE;	break;
		case(DAM_HOLY):		bit = IMM_HOLY;		break;
		case(DAM_ENERGY):	bit = IMM_ENERGY;	break;
		case(DAM_MENTAL):
			if (IS_IMMORTAL(ch))
				return IS_IMMUNE;
			bit = IMM_MENTAL;
			break;
		case(DAM_DISEASE):	bit = IMM_DISEASE;	break;
		case(DAM_DROWNING):	bit = IMM_DROWNING;	break;
		case(DAM_LIGHT):	bit = IMM_LIGHT;	break;
		case(DAM_CHARM):
			if (IS_IMMORTAL(ch))
				return IS_IMMUNE;
			bit = IMM_CHARM;
			break;
		case(DAM_SOUND):	bit = IMM_SOUND;	break;
		default:		return def;
	}

	if (IS_SET(ch->imm_flags, bit))
		immune = IS_IMMUNE;
	else if (IS_SET(ch->res_flags, bit) && immune != IS_IMMUNE)
		immune = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags, bit)) {
		if (immune == IS_IMMUNE)
		    immune = IS_RESISTANT;
		else if (immune == IS_RESISTANT)
		    immune = IS_NORMAL;
		else
		    immune = IS_VULNERABLE;
	}

	if (!IS_NPC(ch) && get_curr_stat(ch, STAT_CHA) < 18 
			&& dam_type == DAM_CHARM)
		immune = IS_VULNERABLE;

	if (immune == -1)
		return def;
	else
	  	return immune;
}

int age_to_num(int age)
{
	return  age * 72000;
}

DECLARE_SPEC_FUN(spec_janitor);

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return -1;

	if (IS_NPC(ch)) {
		if (IS_SET(ch->pMobIndex->act, ACT_PET))
			return 0;
		if (ch->pMobIndex->spec_fun == spec_janitor)
			return -1;
	}

	return MAX_WEAR + get_curr_stat(ch,STAT_DEX) - 10 + ch->size;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return -1;

	if (IS_NPC(ch)) {
		if (IS_SET(ch->pMobIndex->act, ACT_PET))
			return 0;
		if (ch->pMobIndex->spec_fun == spec_janitor)
			return -1;
		if (IS_SET(ch->pMobIndex->act, ACT_CHANGER))
			return -1;
	}

	return str_app[get_curr_stat(ch,STAT_STR)].carry * 10 + ch->level * 25;
}

/*---------------------------------------------------------------------------
 * name list stuff
 *
 * name list is simply string of names
 * separated by spaces. if name contains spaces itself it is enclosed
 * in single quotes
 *
 */

/*
 * See if a string is one of the names of an object.
 */
bool is_name_raw(const char *str, const char *namelist,
		 int (*cmpfun)(const char*, const char*))
{
	char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
	const char *list, *string;
	
	if (IS_NULLSTR(namelist) || IS_NULLSTR(str))
		return FALSE;

	if (!str_cmp(namelist, "all"))
		return TRUE;

	string = str;
	/* we need ALL parts of string to match part of namelist */
	for (; ;) { /* start parsing string */
		str = one_argument(str, part, sizeof(part));

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;
		for (; ;) { /* start parsing namelist */
			list = one_argument(list, name, sizeof(name));
			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!cmpfun(string, name))
				return TRUE; /* full pattern match */

			if (!cmpfun(part, name))
				break;
		}
	}
}

bool is_name(const char *str, const char *namelist)
{
	return is_name_raw(str, namelist, str_prefix);
}

void cat_name(char *buf, const char *name, size_t len)
{
	bool have_spaces = strpbrk(name, " \t") != NULL;

	if (buf[0])
		strnzcat(buf, len, " ");
	if (have_spaces)
		strnzcat(buf, len, "'");
	strnzcat(buf, len, name);
	if (have_spaces)
		strnzcat(buf, len, "'");
}

/* 
 * name_edit flags
 */
#define NE_F_DELETE	(A)	/* delete name if found		*/
#define NE_F_ADD	(B)	/* add name if not found	*/

/*
 * name_edit - edit 'name' according to 'flags' in name list pointed by 'nl'
 *             if ch == NULL name_edit will be silent
 *	       (and 'editor_name' is not used)
 * Return values: TRUE  - name was found in namelist
 *		  FALSE - name was not found
 *
 */
bool name_edit(const char **nl, const char *name, int flags,
	       CHAR_DATA *ch, const char *editor_name)
{
	bool found = FALSE;
	const char *p = *nl;
	char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	for (;;) {
		char arg[MAX_STRING_LENGTH];

		p = first_arg(p, arg, sizeof(arg), FALSE);

		if (arg[0] == '\0')
			break;

		if (!str_cmp(name, arg)) {
			found = TRUE;
			if (IS_SET(flags, NE_F_DELETE))
				continue;
		}

		cat_name(buf, arg, sizeof(buf));
	}

	if (!found) {
		if (!IS_SET(flags, NE_F_ADD))
			return found;

		if (strlen(buf) + strlen(name) + 4 > MAX_STRING_LENGTH) {
			if (ch)
				char_printf(ch, "%s: name list too long\n",
					    editor_name);
			return found;
		}
		cat_name(buf, name, sizeof(buf));
		if (ch)
			char_printf(ch, "%s: %s: name added.\n",
				    editor_name, name);
	}
	else {
		if (!IS_SET(flags, NE_F_DELETE))
			return found;

		if (ch)
			char_printf(ch, "%s: %s: name removed.\n",
				    editor_name, name);
	}

	free_string(*nl);
	*nl = str_dup(buf);
	return found;
}

bool name_add(const char **nl, const char *name,
	      CHAR_DATA *ch, const char *editor_name)
{
	return name_edit(nl, name, NE_F_ADD, ch, editor_name);
}

bool name_delete(const char **nl, const char *name,
		 CHAR_DATA *ch, const char *editor_name)
{
	return name_edit(nl, name, NE_F_DELETE, ch, editor_name);
}

bool name_toggle(const char **nl, const char *name,
		 CHAR_DATA *ch, const char *editor_name)
{
	if (!str_cmp(name, "all")) {
		free_string(*nl);
		*nl = str_dup(name);
		if (ch)
			char_printf(ch, "%s: name list set to ALL.\n",
				    editor_name);
		return TRUE;
	}

	if (!str_cmp(name, "none")) {
		free_string(*nl);
		*nl = str_empty;
		if (ch)
			char_printf(ch, "%s: name list reset.\n", editor_name);
		return TRUE;
	}

	if (!str_cmp(*nl, "all")) {
		free_string(*nl);
		*nl = str_empty;
	}

	return name_edit(nl, name, NE_F_ADD | NE_F_DELETE, ch, editor_name);
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
	/* okay, move all the old flags into new vectors if we have to */
	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED)) {
	    AFFECT_DATA *paf, *af_new;
	    SET_BIT(obj->extra_flags, ITEM_ENCHANTED);

	    for (paf = obj->pObjIndex->affected;
	         paf != NULL; paf = paf->next)
	    {
		    af_new = aff_new();

	        af_new->next = obj->affected;
	        obj->affected = af_new;
 
		    af_new->where	= paf->where;
	        af_new->type        = UMAX(0,paf->type);
	        af_new->level       = paf->level;
	        af_new->duration    = paf->duration;
	        af_new->location    = paf->location;
	        af_new->modifier    = paf->modifier;
	        af_new->bitvector   = paf->bitvector;
	    }
	}
}
	       

/*
 * Apply or remove an affect to a character.
 */
void affect_modify(CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd)
{
	OBJ_DATA *wield, *obj2;
	int mod, i;

	if (paf->where == TO_SKILLS) {
		if (fAdd) {
			saff_t *sa = varr_enew(&ch->sk_affected);
			sa->sn = -paf->location;
			sa->type = paf->type;
			sa->mod = paf->modifier;
			sa->bit =  paf->bitvector;
			varr_qsort(&ch->sk_affected, cmpint);
		} else {
			int i;

			for (i = 0; i < ch->sk_affected.nused;) {
				saff_t *sa = VARR_GET(&ch->sk_affected, i);

				if (sa->sn != -paf->location
				||  sa->type != paf->type
				||  sa->mod != paf->modifier
				||  sa->bit != paf->bitvector) {
					i++;
					continue;
				}

				varr_del(&ch->sk_affected, sa);
			}
		}
  		return;
	}

	mod = paf->modifier;
	if (fAdd) {
		switch (paf->where) {
		case TO_AFFECTS:
			SET_BIT(ch->affected_by, paf->bitvector);
			break;
		case TO_IMMUNE:
			SET_BIT(ch->imm_flags, paf->bitvector);
			break;
		case TO_RESIST:
			SET_BIT(ch->res_flags, paf->bitvector);
			break;
		case TO_VULN:
			SET_BIT(ch->vuln_flags, paf->bitvector);
			break;
		}
	}
	else {
		switch (paf->where) {
		case TO_AFFECTS:
			REMOVE_BIT(ch->affected_by, paf->bitvector);
			break;
		case TO_IMMUNE:
			REMOVE_BIT(ch->imm_flags, paf->bitvector);
			break;
		case TO_RESIST:
			REMOVE_BIT(ch->res_flags, paf->bitvector);
			break;
		case TO_VULN:
	        	REMOVE_BIT(ch->vuln_flags, paf->bitvector);
	        	break;
		}
		mod = 0 - mod;
	}

	switch (paf->location) {
	case APPLY_NONE:
	case APPLY_CLASS:
	case APPLY_HEIGHT:
	case APPLY_WEIGHT:
	case APPLY_GOLD:
	case APPLY_EXP:
	case APPLY_SEX:
	case APPLY_SPELL_AFFECT:
		break;

	case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod; break;
	case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod;	break;
	case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod;	break;
	case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod;	break;
	case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod;	break;
	case APPLY_CHA:		ch->mod_stat[STAT_CHA]	+= mod; break;

	case APPLY_MANA:	ch->max_mana		+= mod;	break;
	case APPLY_HIT:		ch->max_hit		+= mod;	break;
	case APPLY_MOVE:	ch->max_move		+= mod;	break;

	case APPLY_HITROLL:	ch->hitroll		+= mod;	break;
	case APPLY_DAMROLL:	ch->damroll		+= mod;	break;
	case APPLY_LEVEL:	ch->add_level		+= mod; break;

	case APPLY_SIZE:	ch->size	+= mod;			break;
	case APPLY_AGE:	
		if (!IS_NPC(ch))
			PC(ch)->add_age += age_to_num(mod);
		break;

	case APPLY_AC:
		for (i = 0; i < 4; i ++)
			ch->armor[i] += mod;
		break;

	case APPLY_SAVES:		ch->saving_throw	+= mod;	break;
	case APPLY_SAVING_ROD:		ch->saving_throw	+= mod;	break;
	case APPLY_SAVING_PETRI:	ch->saving_throw	+= mod;	break;
	case APPLY_SAVING_BREATH:	ch->saving_throw	+= mod;	break;
	case APPLY_SAVING_SPELL:	ch->saving_throw	+= mod;	break;

	case APPLY_RACE: {
		int from;
		int to;
		race_t *rto;
		race_t *rfrom;

		if (fAdd) {
			from = ORG_RACE(ch);
			to = ch->race = paf->modifier;
		}
		else {
			from = ch->race;
			to = ch->race = ORG_RACE(ch);
		}

		rfrom = race_lookup(from);
		rto = race_lookup(to);
		if (!rfrom || !rto || !rfrom->race_pcdata || !rto->race_pcdata)
			return;

		REMOVE_BIT(ch->affected_by, rfrom->aff);
		SET_BIT(ch->affected_by, rto->aff);
		affect_check(ch, TO_AFFECTS, rfrom->aff);

		REMOVE_BIT(ch->imm_flags, rfrom->imm);
		SET_BIT(ch->imm_flags, rto->imm);
		affect_check(ch, TO_IMMUNE, rfrom->imm);

		REMOVE_BIT(ch->res_flags, rfrom->res);
		SET_BIT(ch->res_flags, rto->res);
		affect_check(ch, TO_RESIST, rfrom->res);

		REMOVE_BIT(ch->vuln_flags, rfrom->vuln);
		SET_BIT(ch->vuln_flags, rto->vuln);
		affect_check(ch, TO_VULN, rfrom->vuln);

		ch->form = rto->form;
		ch->parts = rto->parts;
		ch->size = rto->race_pcdata->size;
		update_skills(ch);
		break;
	}
	default:
		if (IS_NPC(ch)) {
			log("affect_modify: vnum %d: in room %d: "
				   "unknown location %d, where: %d",
				   ch->pMobIndex->vnum,
				   ch->in_room ? ch->in_room->vnum : -1,
				   paf->location, paf->where);
		}
		else {
			log("affect_modify: %s: unknown location %d"
				"where: %d", ch->name, paf->location,
				paf->where);
		}
		return;

	}

	/*
	 * Check for weapon wielding.
	 * Guard against recursion (for weapons with affects).
	 * May be called from char_load (ch->in_room will be NULL)
	 */
	if (!IS_NPC(ch)
	&&  ch->in_room != NULL
	&&  (wield = get_eq_char(ch, WEAR_WIELD)) != NULL
	&&  get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10))
	{
		static int depth;

		if (depth == 0) {
		    depth++;
		    act("You drop $p.", ch, wield, NULL, TO_CHAR);
		    act("$n drops $p.", ch, wield, NULL, TO_ROOM);
		    obj_from_char(wield);
		    obj_to_room(wield, ch->in_room);

		if ((obj2 = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL) {
			act("You wield his second weapon as your first!",  ch, NULL,NULL,TO_CHAR);
			act("$n wields his second weapon as first!",  ch, NULL,NULL,TO_ROOM);
		unequip_char(ch, obj2);
		equip_char(ch, obj2 , WEAR_WIELD);
		}
		    depth--;
		}
	}
}


/* find an effect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
	AFFECT_DATA *paf_find;
	
	for (paf_find = paf; paf_find != NULL; paf_find = paf_find->next)
	{
	    if (paf_find->type == sn)
		return paf_find;
	}

	return NULL;
}

void affect_check_list(CHAR_DATA *ch, AFFECT_DATA *paf,
		       int where, flag64_t vector)
{
	for (; paf; paf = paf->next)
		if ((where < 0 || paf->where == where)
		&&  (paf->bitvector & vector))
			switch (paf->where) {
			case TO_AFFECTS:
				SET_BIT(ch->affected_by, paf->bitvector);
				break;
			case TO_IMMUNE:
				SET_BIT(ch->imm_flags, paf->bitvector);   
				break;
			case TO_RESIST:
				SET_BIT(ch->res_flags, paf->bitvector);
				break;
			case TO_VULN:
				SET_BIT(ch->vuln_flags, paf->bitvector);
				break;
			}
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch, int where, flag64_t vector)
{
	OBJ_DATA *obj;

	if (where == TO_OBJECT
	||  where == TO_WEAPON
	||  where == TO_SKILLS
	||  vector == 0)
		return;

	affect_check_list(ch, ch->affected, where, vector);
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == -1 || obj->wear_loc == WEAR_STUCK_IN)
			continue;
		affect_check_list(ch, obj->affected, where, vector);

		if (IS_SET(obj->extra_flags, ITEM_ENCHANTED))
			continue;

		affect_check_list(ch, obj->pObjIndex->affected, where, vector);
	}
}

/*
 * Give an affect to a char.
 */
void affect_to_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new;

	paf_new = aff_new();

	*paf_new	= *paf;
	paf_new->next	= ch->affected;
	ch->affected	= paf_new;

	affect_modify(ch, paf_new, TRUE);
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new;

	paf_new = aff_new();
	*paf_new	= *paf;
	paf_new->next	= obj->affected;
	obj->affected	= paf_new;

	/* apply any affect vectors to the object's extra_flags */
	if (paf->bitvector)
		switch (paf->where) {
		case TO_OBJECT:
			SET_BIT(obj->extra_flags,paf->bitvector);
			break;
		case TO_WEAPON:
			if (obj->pObjIndex->item_type == ITEM_WEAPON)
		        	SET_BIT(obj->value[4],paf->bitvector);
			break;
		}
}

/*
 * Remove an affect from a char.
 */
void affect_remove(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	int where;
	int vector;

	if (ch->affected == NULL) {
		bug("Affect_remove: no affect.", 0);
		return;
	}

	affect_modify(ch, paf, FALSE);
	where = paf->where;
	vector = paf->bitvector;

	if (paf == ch->affected)
		ch->affected	= paf->next;
	else {
		AFFECT_DATA *prev;

		for (prev = ch->affected; prev; prev = prev->next) {
			if (prev->next == paf) {
				prev->next = paf->next;
				break;
			}
		}

		if (prev == NULL) {
			bug("Affect_remove: cannot find paf.", 0);
			return;
		}
	}

	aff_free(paf);
	affect_check(ch, where, vector);
}

void affect_remove_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	int where, vector;

	if (obj->affected == NULL)
		return;

	if (obj->carried_by != NULL && obj->wear_loc != -1)
		affect_modify(obj->carried_by, paf, FALSE);

	where = paf->where;
	vector = paf->bitvector;

	/* remove flags from the object if needed */
	if (paf->bitvector)
		switch(paf->where) {
		case TO_OBJECT:
			REMOVE_BIT(obj->extra_flags,paf->bitvector);
			break;
		case TO_WEAPON:
			if (obj->pObjIndex->item_type == ITEM_WEAPON)
				REMOVE_BIT(obj->value[4],paf->bitvector);
			break;
		}

	if (paf == obj->affected)
	    obj->affected    = paf->next;
	else
	{
	    AFFECT_DATA *prev;

	    for (prev = obj->affected; prev != NULL; prev = prev->next)
	    {
	        if (prev->next == paf)
	        {
	            prev->next = paf->next;
	            break;
	        }
	    }

	    if (prev == NULL)
	    {
	        bug("Affect_remove_object: cannot find paf.", 0);
	        return;
	    }
	}

	aff_free(paf);

	if (obj->carried_by != NULL && obj->wear_loc != -1)
		affect_check(obj->carried_by,where,vector);
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip(CHAR_DATA *ch, int sn)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		if (paf->type == sn)
			affect_remove(ch, paf);
	}
}

/*
 * strip all affects which affect given bitvector
 */
void affect_bit_strip(CHAR_DATA *ch, int where, flag64_t bits)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		if (paf->where == where && (paf->bitvector & bits))
			affect_remove(ch, paf);
	}
}

/*
 * Return true if a char is affected by a spell.
 */
bool is_affected(CHAR_DATA *ch, int sn)
{
	AFFECT_DATA *paf;

	for (paf = ch->affected; paf; paf = paf->next)
		if (paf->type == sn)
			return TRUE;

	return FALSE;
}

bool is_bit_affected(CHAR_DATA *ch, int where, flag64_t bits)
{
	AFFECT_DATA *paf;

	for (paf = ch->affected; paf; paf = paf->next)
		if (paf->where == where && (paf->bitvector & bits))
			return TRUE;

	return FALSE;
}

bool has_obj_affect(CHAR_DATA *ch, int vector)
{
	OBJ_DATA *obj;

	for (obj = ch->carrying; obj; obj = obj->next_content) {
		AFFECT_DATA *paf;

		if (obj->wear_loc == -1 || obj->wear_loc == WEAR_STUCK_IN)
			continue;

		for (paf = obj->affected; paf; paf = paf->next)
	        	if (paf->bitvector & vector)
				return TRUE;

		if (IS_SET(obj->extra_flags, ITEM_ENCHANTED))
			continue;

		for (paf = obj->pObjIndex->affected; paf; paf = paf->next)
			if (paf->bitvector & vector)
				return TRUE;
	}
	return FALSE;
}

/*
 * Add or enhance an affect.
 */
void affect_join(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_old;
	bool found;

	found = FALSE;
	for (paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next)
	{
		if (paf_old->type == paf->type)
		{
		    paf->level = (paf->level += paf_old->level) / 2;
		    paf->duration += paf_old->duration;
		    paf->modifier += paf_old->modifier;
		    affect_remove(ch, paf_old);
		    break;
		}
	}

	affect_to_char(ch, paf);
	return;
}

/*
 * Move a char out of a room.
 */
void char_from_room(CHAR_DATA *ch)
{
	OBJ_DATA *obj;

	if (ch->in_room == NULL) {
		bug("Char_from_room: NULL.", 0);
		return;
	}

	if (ch->in_room->affected)
		  check_room_affects(ch, ch->in_room, EVENT_LEAVE);

	if (!IS_NPC(ch))
		--ch->in_room->area->nplayer;

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
	&&   obj->pObjIndex->item_type == ITEM_LIGHT
	&&   obj->value[2] != 0
	&&   ch->in_room->light > 0)
		--ch->in_room->light;

	if (ch == ch->in_room->people)
		ch->in_room->people = ch->next_in_room;
	else
	{
		CHAR_DATA *prev;

		for (prev = ch->in_room->people; prev; prev = prev->next_in_room)
		{
		    if (prev->next_in_room == ch)
		    {
			prev->next_in_room = ch->next_in_room;
			break;
		    }
		}

		if (prev == NULL)
		    bug("Char_from_room: ch not found.", 0);
	}

	ch->in_room      = NULL;
	ch->next_in_room = NULL;
	ch->on 	     = NULL;  /* sanity check! */

	if (MOUNTED(ch))
	{
	 ch->mount->riding	= FALSE;
	 ch->riding		= FALSE;
	}

	if (RIDDEN(ch))
	{
	 ch->mount->riding	= FALSE;
	 ch->riding		= FALSE;
	}

	return;
}

/*
 * Move a char into a room.
 */
void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
{
	olced_t *olced;
	OBJ_DATA *obj;

	if (pRoomIndex == NULL) {
		ROOM_INDEX_DATA *room;

		bug("Char_to_room: NULL.", 0);
		
		if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
			char_to_room(ch, room);
		
		return;
	}

	ch->in_room		= pRoomIndex;
	ch->next_in_room	= pRoomIndex->people;
	pRoomIndex->people	= ch;

	if (!IS_NPC(ch))
	{
		if (ch->in_room->area->empty)
		{
		    ch->in_room->area->empty = FALSE;
		    ch->in_room->area->age = 0;
		}
		++ch->in_room->area->nplayer;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
	&&   obj->pObjIndex->item_type == ITEM_LIGHT
	&&   obj->value[2] != 0)
		++ch->in_room->light;
		
	while (IS_AFFECTED(ch,AFF_PLAGUE))
	{
	    AFFECT_DATA *af, plague;
	    CHAR_DATA *vch;
	    
	    for (af = ch->affected; af != NULL; af = af->next)
	    {
	        if (af->type == gsn_plague)
	            break;
	    }
	    
	    if (af == NULL)
	    {
	        REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
	        break;
	    }
	    
	    if (af->level == 1)
	        break;
	    
		plague.where		= TO_AFFECTS;
	    plague.type 		= gsn_plague;
	    plague.level 		= af->level - 1; 
	    plague.duration 	= number_range(1,2 * plague.level);
	    plague.location		= APPLY_STR;
	    plague.modifier 	= -5;
	    plague.bitvector 	= AFF_PLAGUE;
	    
	    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	    {
	        if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
		    &&  !IS_IMMORTAL(vch) &&
	        	!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
	        {
	        	char_puts("You feel hot and feverish.\n",vch);
	        	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
	        	affect_join(vch,&plague);
	        }
	    }
		break;
	}

	if (pRoomIndex->affected) {
		if (IS_IMMORTAL(ch))
			dofun("raffects", ch, str_empty);
		else {
			check_room_affects(ch, ch->in_room, EVENT_ENTER);
			if (IS_EXTRACTED(ch))
				return;
		}
	}

	if (ch->desc != NULL
	&&  (olced = OLCED(ch)) != NULL
	&&  !str_cmp(olced->id, "rooms"))
		dofun("edit", ch, "rooms dropout");
}

/*
 * Give an obj to a char.
 */
void obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch)
{
	obj->next_content	= ch->carrying;
	ch->carrying		= obj;
	obj->carried_by		= ch;
	obj->in_room		= NULL;
	obj->in_obj		= NULL;
	ch->carry_number	+= get_obj_number(obj);
	ch->carry_weight	+= get_obj_weight(obj);
}

/*
 * Take an obj from its character.
 */
void obj_from_char(OBJ_DATA *obj)
{
	CHAR_DATA *ch;

	if ((ch = obj->carried_by) == NULL) {
		bug("Obj_from_char: null ch.", 0);
		log("Name %s", obj->name);
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(ch, obj);

	if (ch->carrying == obj)
		ch->carrying = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = ch->carrying; prev; prev = prev->next_content) {
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == NULL)
			bug("Obj_from_char: obj not in list.", 0);
	}

	obj->carried_by		= NULL;
	obj->next_content	= NULL;
	ch->carry_number	-= get_obj_number(obj);
	ch->carry_weight	-= get_obj_weight(obj);
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac(OBJ_DATA *obj, int iWear, int type)
{
	if (obj->pObjIndex->item_type != ITEM_ARMOR)
		return 0;

	switch (iWear) {
	case WEAR_BODY:		return 3 * obj->value[type];
	case WEAR_HEAD:		return 2 * obj->value[type];
	case WEAR_LEGS:		return 2 * obj->value[type];
	case WEAR_FEET:		return	obj->value[type];
	case WEAR_HANDS:	return	obj->value[type];
	case WEAR_ARMS:		return	obj->value[type];
	case WEAR_SHIELD:	return	obj->value[type];
	case WEAR_FINGER_L:	return	0;
	case WEAR_FINGER_R:	return	obj->value[type];
	case WEAR_NECK_1:	return	obj->value[type];
	case WEAR_NECK_2:	return	obj->value[type];
	case WEAR_ABOUT:	return 2 * obj->value[type];
	case WEAR_WAIST:	return	obj->value[type];
	case WEAR_WRIST_L:	return	obj->value[type];
	case WEAR_WRIST_R:	return	obj->value[type];
	case WEAR_HOLD:		return	obj->value[type];
	}

	return 0;
}

/* find stuck-in objects of certain weapon type */
OBJ_DATA *get_stuck_eq(CHAR_DATA *ch, int wtype)
{
	OBJ_DATA *obj;

	if (!ch)
		return NULL;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc == WEAR_STUCK_IN
		&&  obj->pObjIndex->item_type == ITEM_WEAPON
		&&  obj->value[0] == wtype)
			return obj;

	return NULL;
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char(CHAR_DATA *ch, int iWear)
{
	OBJ_DATA *obj;

	if (ch == NULL)
		return NULL;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (obj->wear_loc == iWear)
			return obj;

	return NULL;
}

void _equip_char(CHAR_DATA *ch, OBJ_DATA *obj)
{
	int i;
	AFFECT_DATA *paf;

	if (obj->wear_loc == WEAR_STUCK_IN)
		return;

	for (i = 0; i < 4; i++)
		ch->armor[i] -= apply_ac(obj, obj->wear_loc, i);

	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		for (paf = obj->pObjIndex->affected; paf; paf = paf->next)
			affect_modify(ch, paf, TRUE);

	for (paf = obj->affected; paf; paf = paf->next)
		affect_modify(ch, paf, TRUE);
}

/*
 * Equip a char with an obj. Return obj on success. Otherwise returns NULL.
 */
OBJ_DATA * equip_char(CHAR_DATA *ch, OBJ_DATA *obj, int iWear)
{
	if (iWear == WEAR_STUCK_IN) {
		obj->wear_loc = iWear;
		return obj;
	}

	if (get_eq_char(ch, iWear)) {
		if (IS_NPC(ch)) {
			log("equip_char: vnum %d: in_room %d: "
				   "obj vnum %d: location %s: "
				   "already equipped.",
				   ch->pMobIndex->vnum,
				   ch->in_room ? ch->in_room->vnum : -1,
				   obj->pObjIndex->vnum,
				   flag_string(wear_loc_flags, iWear));
		} else {
			log("equip_char: %s: location %s: "
				   "already equipped.",
				   ch->name,
				   flag_string(wear_loc_flags, iWear));
		}
		return NULL;
	}

	if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)   )
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)   )
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
		/*
		 * Thanks to Morgenes for the bug fix here!
		 */
		act("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
		act("$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		return NULL;
	}

	obj->wear_loc = iWear;
	_equip_char(ch, obj);

	if (obj->pObjIndex->item_type == ITEM_LIGHT
	&&  obj->value[2] != 0
	&&  ch->in_room != NULL)
		++ch->in_room->light;

	oprog_call(OPROG_WEAR, obj, ch, NULL);
	return obj;
}

void strip_obj_affects(CHAR_DATA *ch, OBJ_DATA *obj, AFFECT_DATA *paf)
{
	for (; paf != NULL; paf = paf->next) {
		affect_modify(ch, paf, FALSE);
		affect_check(ch, paf->where, paf->bitvector);
	}
}

/*
 * Unequip a char with an obj.
 */
void unequip_char(CHAR_DATA *ch, OBJ_DATA *obj)
{
	int i;

	if (obj->wear_loc == WEAR_NONE) {
		bug("Unequip_char: already unequipped.", 0);
		return;
	}

	if (obj->wear_loc == WEAR_STUCK_IN) {
		obj->wear_loc = WEAR_NONE;
		return;
	}

	for (i = 0; i < 4; i++)
		ch->armor[i]	+= apply_ac(obj, obj->wear_loc,i);
	obj->wear_loc	 = -1;

	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		strip_obj_affects(ch, obj, obj->pObjIndex->affected);
	strip_obj_affects(ch, obj, obj->affected);

	if (obj->pObjIndex->item_type == ITEM_LIGHT
	&&   obj->value[2] != 0
	&&   ch->in_room != NULL
	&&   ch->in_room->light > 0)
		--ch->in_room->light;

	oprog_call(OPROG_REMOVE, obj, ch, NULL);
}

/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list(OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list)
{
	OBJ_DATA *obj;
	int nMatch;

	nMatch = 0;
	for (obj = list; obj != NULL; obj = obj->next_content) {
		if (obj->pObjIndex == pObjIndex)
		    nMatch++;
	}

	return nMatch;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room(OBJ_DATA *obj)
{
	ROOM_INDEX_DATA *in_room;
	CHAR_DATA *ch;

	if ((in_room = obj->in_room) == NULL) {
		log("obj_from_room: NULL obj->in_room (vnum %d)",
			   obj->pObjIndex->vnum);
		return;
	}

	for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
		if (ch->on == obj)
			ch->on = NULL;

	if (obj == in_room->contents)
		in_room->contents = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = in_room->contents; prev; prev = prev->next_content) {
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == NULL) {
			bug("Obj_from_room: obj not found.", 0);
			return;
		}
	}

	obj->in_room      = NULL;
	obj->next_content = NULL;
	return;
}

/*
 * Move an obj into a room.
 */
void obj_to_room(OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex)
{
	obj->next_content	= pRoomIndex->contents;
	pRoomIndex->contents	= obj;
	obj->in_room		= pRoomIndex;
	obj->carried_by		= NULL;
	obj->in_obj		= NULL;

	if (IS_WATER(pRoomIndex)) {
		if (may_float(obj))
			obj->water_float = -1;
		else
			obj->water_float = floating_time(obj);
	}
}

/*
 * Move an object into an object.
 */
void obj_to_obj(OBJ_DATA *obj, OBJ_DATA *obj_to)
{
	if (obj == obj_to) {
		log("obj_to_obj: obj == obj_to (vnum %d)",
			   obj->pObjIndex->vnum);
		return;
	}

	obj->next_content	= obj_to->contains;
	obj_to->contains	= obj;
	obj->in_obj		= obj_to;
	obj->in_room		= NULL;
	obj->carried_by		= NULL;
	if (IS_SET(obj_to->pObjIndex->extra_flags, ITEM_PIT))
		obj->cost = 0; 

	for (; obj_to != NULL; obj_to = obj_to->in_obj) {
		if (obj_to->carried_by != NULL)
		{
/*	    obj_to->carried_by->carry_number += get_obj_number(obj); */
		    obj_to->carried_by->carry_weight += get_obj_weight(obj)
			* WEIGHT_MULT(obj_to) / 100;
		}
	}
}

/*
 * Move an object out of an object.
 */
void obj_from_obj(OBJ_DATA *obj)
{
	OBJ_DATA *obj_from;

	if ((obj_from = obj->in_obj) == NULL) {
		bug("Obj_from_obj: null obj_from.", 0);
		return;
	}

	if (obj == obj_from->contains)
		obj_from->contains = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = obj_from->contains; prev; prev = prev->next_content) {
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == NULL) {
			bug("Obj_from_obj: obj not found.", 0);
			return;
		}
	}

	obj->next_content = NULL;
	obj->in_obj       = NULL;

	for (; obj_from != NULL; obj_from = obj_from->in_obj) {
		if (obj_from->carried_by != NULL)
/*	    obj_from->carried_by->carry_number -= get_obj_number(obj); */
			obj_from->carried_by->carry_weight -= get_obj_weight(obj) * WEIGHT_MULT(obj_from) / 100;
	}
}

/*
 * Extract an obj from the world.
 */
void extract_obj(OBJ_DATA *obj, int flags)
{
	OBJ_DATA *obj_content;
	OBJ_DATA *obj_next;

	if (!mem_is(obj, MT_OBJ)) {
		bug("extract_obj: obj is not MT_OBJ");
		return;
	}

	if (IS_SET(obj->pObjIndex->extra_flags, ITEM_CLAN))
		return;

	if (IS_SET(obj->pObjIndex->extra_flags, ITEM_CHQUEST)) {
		if (!IS_SET(flags, XO_F_NOCHQUEST))
			chquest_extract(obj);
		flags |= XO_F_NORECURSE;
	}

	for (obj_content = obj->contains; obj_content; obj_content = obj_next) {
		obj_next = obj_content->next_content;

		if (!IS_SET(flags, XO_F_NORECURSE)
		||  IS_SET(flags, XO_F_NUKE)) {
			extract_obj(obj_content, flags);
			continue;
		}

		obj_from_obj(obj_content);
		if (obj->in_room)
			obj_to_room(obj_content, obj->in_room);
		else if (obj->carried_by)
			obj_to_char(obj_content, obj->carried_by);
		else if (obj->in_obj)
			obj_to_obj(obj_content, obj->in_obj);
		else
			extract_obj(obj_content, 0);
	}

	if (!IS_SET(flags, XO_F_NUKE)) {
		if (obj->in_room)
			obj_from_room(obj);
		else if (obj->carried_by)
			obj_from_char(obj);
		else if (obj->in_obj)
			obj_from_obj(obj);
	}

	if (obj->pObjIndex->vnum == OBJ_VNUM_MAGIC_JAR) {
		 CHAR_DATA *wch;
		 
		 for (wch = char_list; wch && !IS_NPC(wch); wch = wch->next) {
		 	if (!mlstr_cmp(&obj->owner, &wch->short_descr)) {
				REMOVE_BIT(PC(wch)->plr_flags, PLR_NOEXP);
				char_puts("Now you catch your spirit.\n", wch);
				break;
			}
		}
	}

	if (object_list == obj)
		object_list = obj->next;
	else {
		OBJ_DATA *prev;

		for (prev = object_list; prev != NULL; prev = prev->next) {
			if (prev->next == obj) {
				prev->next = obj->next;
				break;
			}
		}

		if (prev == NULL) {
			bug("extract_obj: obj %d not found.",
			    obj->pObjIndex->vnum);
			return;
		}
	}

	if (!IS_SET(flags, XO_F_NOCOUNT))
		--obj->pObjIndex->count;
	free_obj(obj);
}

void strip_raff_owner(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *room, *room_next;
	ROOM_AFFECT_DATA *raf, *raf_next;

	for (room = top_affected_room; room; room = room_next) {
		room_next = room->aff_next;

		for (raf = room->affected; raf; raf = raf_next) {
			raf_next = raf->next;
			if (raf->owner == ch) affect_remove_room(room, raf);
		}
	}
}

/*
 * Extract a char from the world.
 */
void extract_char(CHAR_DATA *ch, int flags)
{
	CHAR_DATA *wch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	OBJ_DATA *wield;
	int extract_obj_flags;

	if (!mem_is(ch, MT_CHAR)) {
		bug("extract_char: ch is not MT_CHAR");
		return;
	}
	
	strip_raff_owner(ch);
	if (!IS_NPC(ch))
		nuke_pets(ch);

	if (!IS_SET(flags, XC_F_INCOMPLETE))
		die_follower(ch);
	
	stop_fighting(ch, TRUE);

	if ((wield = get_eq_char(ch, WEAR_WIELD)) != NULL)
		unequip_char(ch, wield); 

	extract_obj_flags = (IS_SET(flags, XC_F_NOCOUNT) ? XO_F_NOCOUNT : 0);
	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj, extract_obj_flags);
	}
	
	if (ch->in_room)
		char_from_room(ch);

	if (IS_SET(flags, XC_F_INCOMPLETE)) {
		char_to_room(ch, get_altar(ch)->room);
		return;
	}

	if (IS_NPC(ch))
		--ch->pMobIndex->count;

	if (ch->desc != NULL && ch->desc->original != NULL) {
		dofun("return", ch, str_empty);
		ch->desc = NULL;
	}

	for (wch = char_list; wch && !IS_NPC(wch); wch = wch->next) {
		if (PC(wch)->reply == ch)
			PC(wch)->reply = NULL;
	}

	for (wch = npc_list; wch; wch = wch->next) {
		if (NPC(wch)->mprog_target == ch)
			NPC(wch)->mprog_target = NULL;
	}

	if (ch == char_list) {
		char_list = ch->next;
		if (ch == char_list_lastpc)
			char_list_lastpc = NULL;
	}
	else {
		CHAR_DATA *prev;

		for (prev = char_list; prev; prev = prev->next) {
			if (prev->next == ch)
				break;
		}

		if (!prev) {
			bug("Extract_char: char not found.", 0);
			return;
		}

		prev->next = ch->next;
		if (ch == char_list_lastpc)
			char_list_lastpc = prev;
	}

	if (ch->desc)
		ch->desc->character = NULL;

	char_free(ch);
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room_raw(CHAR_DATA *ch, const char *name, uint *number,
			     ROOM_INDEX_DATA *room)
{
	CHAR_DATA *rch;
	bool ugly;

	if (!str_cmp(name, "self"))
		return ch;

	ugly = !str_cmp(name, "ugly");
	for (rch = room->people; rch; rch = rch->next_in_room) {
		CHAR_DATA *vch;

		if (!can_see(ch, rch))
			continue;

		if (ugly
		&&  *number == 1
		&&  is_affected(rch, gsn_vampire))
			return rch;

		vch = (is_affected(rch, gsn_doppelganger) &&
		       (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))) ?
					rch->doppel : rch;
		if (name[0] && !is_name(name, vch->name))
			continue;

		if (!--(*number))
			return rch;
	}

	return NULL;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	return get_char_room_raw(ch, arg, &number, ch->in_room);
}

CHAR_DATA *get_char_area(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ach;
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	if ((ach = get_char_room_raw(ch, arg, &number, ch->in_room)))
		return ach;

	if (arg[0] == '\0')
		return NULL;

	for (ach = char_list; ach; ach = ach->next) { 
		if (!ach->in_room
		||  ach->in_room == ch->in_room)
			continue;

		if (ach->in_room->area != ch->in_room->area
		||  !can_see(ch, ach)
		||  !is_name(arg, ach->name))
			continue;

		if (!--number)
			return ach;
	}
	return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *wch;
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	if ((wch = get_char_room_raw(ch, arg, &number, ch->in_room)))
		return wch;

	if (arg[0] == '\0')
		return NULL;

	for (wch = char_list; wch; wch = wch->next) {
		if (!wch->in_room
		||  wch->in_room == ch->in_room
		||  !can_see(ch, wch) 
		||  !is_name(arg, wch->name))
			continue;

		if (!--number)
			return wch;
	}

	return NULL;
}

int opposite_door(int door)
{
	int opdoor;

	switch (door) {
	case 0: opdoor=2;	break;
	case 1: opdoor=3;	break;
	case 2: opdoor=0;	break;
	case 3: opdoor=1;	break;
	case 4: opdoor=5;	break;
	case 5: opdoor=4;	break;
	default: opdoor=-1;	break;
	}

	return opdoor;
}

CHAR_DATA *find_char(CHAR_DATA *ch, const char *argument, int door, int range) 
{
	EXIT_DATA *pExit, *bExit;
	ROOM_INDEX_DATA *dest_room = ch->in_room;
	ROOM_INDEX_DATA *back_room;
	CHAR_DATA *target;
	uint number;
	int opdoor;
	char arg[MAX_INPUT_LENGTH];

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	if ((target = get_char_room_raw(ch, arg, &number, dest_room)))
		return target;

	if ((opdoor = opposite_door(door)) == -1) {
		bug("In find_char wrong door: %d", door);
		char_puts("You don't see that there.\n", ch);
		return NULL;
	}

	while (range > 0) {
		range--;

		/* find target room */
		back_room = dest_room;
		if ((pExit = dest_room->exit[door]) == NULL
		||  (dest_room = pExit->to_room.r) == NULL
		||  IS_SET(pExit->exit_info, EX_CLOSED))
			break;

		if ((bExit = dest_room->exit[opdoor]) == NULL
		||  bExit->to_room.r != back_room) {
			char_puts("The path you choose prevents your power "
				  "to pass.\n",ch);
			return NULL;
		}
		if ((target = get_char_room_raw(ch, arg, &number, dest_room))) 
			return target;
	}

	char_puts("You don't see that there.\n", ch);
	return NULL;
}

int check_exit(const char *arg)
{
	int door = -1;

	     if (!str_cmp(arg, "n") || !str_cmp(arg, "north")) door = 0;
	else if (!str_cmp(arg, "e") || !str_cmp(arg, "east" )) door = 1;
	else if (!str_cmp(arg, "s") || !str_cmp(arg, "south")) door = 2;
	else if (!str_cmp(arg, "w") || !str_cmp(arg, "west" )) door = 3;
	else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"   )) door = 4;
	else if (!str_cmp(arg, "d") || !str_cmp(arg, "down" )) door = 5;

	return door;
}

/*
 * Find a char for range casting.
 * argument must specify target in form '[d.][n.]name' where
 * 'd' - direction
 * 'n' - number
 */
CHAR_DATA *get_char_spell(CHAR_DATA *ch, const char *argument,
			  int *door, int range)
{
	char buf[MAX_INPUT_LENGTH];
	char *p;

	p = strchr(argument, '.');
	if (!p) {
		*door = -1;
		return get_char_room(ch, argument);
	}

	strnzncpy(buf, sizeof(buf), argument, p-argument);
	if ((*door = check_exit(buf)) < 0)
		return get_char_room(ch, argument);

	return find_char(ch, p+1, *door, range);
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type(OBJ_INDEX_DATA *pObjIndex)
{
	OBJ_DATA *obj;

	for (obj = object_list; obj; obj = obj->next)
		if (obj->pObjIndex == pObjIndex)
			return obj;

	return NULL;
}

/*
 * flags for get_obj_list_raw
 */
enum {
	GETOBJ_F_WEAR_ANY,	/* any obj->wear_loc			     */
	GETOBJ_F_WEAR_NONE,	/* obj->wear_loc == WEAR_NONE (in inventory) */
	GETOBJ_F_WEAR,		/* obj->wear_loc != WEAR_NONE (worn)	     */
};

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list_raw(CHAR_DATA *ch, const char *name, uint *number,
			   OBJ_DATA *list, int flags)
{
	OBJ_DATA *obj;

	for (obj = list; obj; obj = obj->next_content) {
		if (!can_see_obj(ch, obj)
		||  !is_name(name, obj->name))
			continue;

		switch (flags) {
		case GETOBJ_F_WEAR_NONE:
			if (obj->wear_loc != WEAR_NONE)
				continue;
			break;

		case GETOBJ_F_WEAR:
			if (obj->wear_loc == WEAR_NONE)
				continue;
			break;
		}

		if (!--(*number))
			return obj;
	}

	return NULL;
}

/*
 * Find an obj in the room or in eq/inventory.
 */
OBJ_DATA *get_obj_here_raw(CHAR_DATA *ch, const char *name, uint *number)
{
	OBJ_DATA *obj;

/* search in player's inventory */
	obj = get_obj_list_raw(ch, name, number, ch->carrying,
			       GETOBJ_F_WEAR_NONE);
	if (obj)
		return obj;

/* search in player's eq */
	obj = get_obj_list_raw(ch, name, number, ch->carrying, GETOBJ_F_WEAR);
	if (obj)
		return obj;

/* search in room contents */
	obj = get_obj_list_raw(ch, name, number, ch->in_room->contents,
			       GETOBJ_F_WEAR_ANY);
	if (obj)
		return obj;

	return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list(CHAR_DATA *ch, const char *argument, OBJ_DATA *list)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(ch, arg, &number, list, GETOBJ_F_WEAR_ANY);
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(ch, arg, &number, ch->carrying,
				GETOBJ_F_WEAR_NONE);
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(ch, arg, &number, ch->carrying, GETOBJ_F_WEAR);
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_here_raw(ch, arg, &number);
}

OBJ_DATA *get_obj_room(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *vch;
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	if ((obj = get_obj_here_raw(ch, arg, &number)))
		return obj;

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		/*
		 * search in the vch's inventory
		 */
		obj = get_obj_list_raw(ch, arg, &number, vch->carrying,
				       GETOBJ_F_WEAR_NONE);
		if (obj)
			return obj;

		/*
		 * search in the vch's eq
		 */
		obj = get_obj_list_raw(ch, arg, &number, vch->carrying,
				       GETOBJ_F_WEAR);
		if (obj)
			return obj;
	}

	return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	if ((obj = get_obj_here_raw(ch, arg, &number)))
		return obj;

	for (obj = object_list; obj; obj = obj->next)
		if (can_see_obj(ch, obj)
		&&  obj->carried_by != ch
		&&  is_name(arg, obj->name)
		&&  !--number)
			return obj;

	return NULL;
}

/*
 * deduct cost from a character
 */
void deduct_cost(CHAR_DATA *ch, uint cost)
{
	/*
	 * price in silver. MUST BE signed for proper exchange operations
	 */
	int silver = UMIN(ch->silver, cost); 
	int gold = 0;

	if (silver < cost) {
		gold = (cost - silver + 99) / 100;
		silver = cost - 100 * gold;
	}

	if (ch->gold < gold) {
		log("deduct cost: %s: ch->gold (%d) < gold (%d)",
			   ch->name, ch->gold, gold);
		ch->gold = gold;
	}

	if (ch->silver < silver) {
		log("deduct cost: %s: ch->silver (%d) < silver (%d)",
			   ch->name, ch->silver, silver);
		ch->silver = silver;
	}

	ch->gold -= gold;
	ch->silver -= silver;
} 

static inline void
money_form(int lang, char *buf, size_t len, int num, const char *name)
{
	char tmp[MAX_STRING_LENGTH];

	if (num < 0)
		return;

	strnzcpy(tmp, sizeof(tmp),
		 word_form(GETMSG(name, lang), 1, lang, RULES_CASE));
	strnzcpy(buf, len, word_form(tmp, num, lang, RULES_QTY));
}

struct _data {
	int num1;
	const char *name1;
	int num2;
	const char *name2;
};

static void
money_cb(int lang, const char **p, void *arg)
{
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	const char *q;
	struct _data *d = (struct _data *) arg;

	if (IS_NULLSTR(*p))
		return;

	money_form(lang, buf1, sizeof(buf1), d->num1, d->name1);
	money_form(lang, buf2, sizeof(buf2), d->num2, d->name2);

	q = str_printf(*p, d->num1, buf1, d->num2, buf2);
	free_string(*p);
	*p = q;
}

static void
money_descr(mlstring *descr,
	    int num1, const char *name1,
	    int num2, const char *name2)
{
	struct _data data;

	data.num1 = num1;
	data.num2 = num2;
	data.name1 = name1;
	data.name2 = name2;

	mlstr_foreach(descr, &data, money_cb);
}

/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money(int gold, int silver)
{
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;

	if (gold < 0 || silver < 0 || (gold == 0 && silver == 0)) {
		log("create_money: gold %d, silver %d",
			   gold, silver);
		gold = UMAX(1, gold);
		silver = UMAX(1, silver);
	}

	if (gold == 0 && silver == 1)
		obj = create_obj(get_obj_index(OBJ_VNUM_SILVER_ONE), 0);
	else if (gold == 1 && silver == 0)
		obj = create_obj(get_obj_index(OBJ_VNUM_GOLD_ONE), 0);
	else if (silver == 0) {
		pObjIndex = get_obj_index(OBJ_VNUM_GOLD_SOME);
		obj = create_obj(pObjIndex, 0);
		money_descr(&obj->short_descr, gold, "gold coins", -1, NULL);
		obj->value[1]	= gold;
		obj->cost	= 100*gold;
		obj->weight	= gold/5;
	}
	else if (gold == 0) {
		pObjIndex = get_obj_index(OBJ_VNUM_SILVER_SOME);
		obj = create_obj(pObjIndex, 0);
		money_descr(&obj->short_descr,
			    silver, "silver coins", -1, NULL);
		obj->value[0]	= silver;
		obj->cost	= silver;
		obj->weight	= silver/20;
	}
	else {
		pObjIndex = get_obj_index(OBJ_VNUM_COINS);
		obj = create_obj(pObjIndex, 0);
		money_descr(&obj->short_descr,
			    silver, "silver coins", gold, "gold coins");
		obj->value[0]	= silver;
		obj->value[1]	= gold;
		obj->cost	= 100*gold + silver;
		obj->weight	= gold/5 + silver/20;
	}

	return obj;
}


/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number(OBJ_DATA *obj)
{
	int number;
/* 
	if (obj->pObjIndex->item_type == ITEM_CONTAINER || obj->pObjIndex->item_type == ITEM_MONEY
	||  obj->pObjIndex->item_type == ITEM_GEM || obj->pObjIndex->item_type == ITEM_JEWELRY)
	    number = 0;
*/
	if (obj->pObjIndex->item_type == ITEM_MONEY)
		number = 0;
	else
	    number = 1;

/* 
	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
	    number += get_obj_number(obj);
*/ 
	return number;
}

int get_obj_realnumber(OBJ_DATA *obj)
{
	int number = 1;

	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
	    number += get_obj_number(obj);

	return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight(OBJ_DATA *obj)
{
	int weight;
	OBJ_DATA *tobj;

	weight = obj->weight;
	for (tobj = obj->contains; tobj != NULL; tobj = tobj->next_content)
		weight += get_obj_weight(tobj) * WEIGHT_MULT(obj) / 100;

	return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
	int weight;
 
	weight = obj->weight;
	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
	    weight += get_obj_weight(obj);
 
	return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA * pRoomIndex = ch->in_room;

	if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))
		return FALSE;

	if (is_affected(ch, gsn_vampire))
		return FALSE;
		
	if (pRoomIndex->light > 0)
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_DARK))
		return TRUE;

	if (pRoomIndex->sector_type == SECT_INSIDE
	||   pRoomIndex->sector_type == SECT_CITY)
		return FALSE;

	if (weather_info.sunlight == SUN_SET
	||  weather_info.sunlight == SUN_DARK)
		return TRUE;

	return FALSE;
}

bool room_dark(ROOM_INDEX_DATA *pRoomIndex)
{
	if (pRoomIndex->light > 0)
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_DARK))
		return TRUE;

	if (pRoomIndex->sector_type == SECT_INSIDE
	||   pRoomIndex->sector_type == SECT_CITY)
		return FALSE;

	if (weather_info.sunlight == SUN_SET
		   || weather_info.sunlight == SUN_DARK)
		return TRUE;

	return FALSE;
}


/*
 * True if room is private.
 */
bool room_is_private(ROOM_INDEX_DATA *pRoomIndex)
{
	CHAR_DATA *rch;
	int count;

/*
	if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
		return TRUE;
*/
	count = 0;
	for (rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room)
		count++;

	if (IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2)
		return TRUE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1)
		return TRUE;
	
	if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY))
		return TRUE;

	return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
{
	if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
	&&  !IS_TRUSTED(ch, LEVEL_IMP))
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
	&&  !IS_TRUSTED(ch, LEVEL_GOD))
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
	&&  !IS_TRUSTED(ch, LEVEL_HERO))
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_NEWBIES_ONLY)
	&&  ch->level > LEVEL_NEWBIE && !IS_IMMORTAL(ch))
		return FALSE;

	return TRUE;
}

/*
 * True if char can see victim.
 */
bool can_see(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *vch;

	if (ch == NULL || victim == NULL) {
		bug("can_see: ch = %p, victim = %p", ch, victim);
		return FALSE;
	}
	
	vch = GET_ORIGINAL(ch);
	if (ch == victim || vch == victim)
		return TRUE;

	if (IS_CLAN_GUARD(ch))
		return TRUE;

	/*
	 * wizi check for PC victim
	 */
	if (!IS_NPC(victim) && !IS_TRUSTED(ch, victim->invis_level))
		return FALSE;

	if (!IS_TRUSTED(ch, victim->incog_level)
	&&  ch->in_room != victim->in_room)
		return FALSE;

	if (!IS_NPC(vch) && IS_SET(PC(vch)->plr_flags, PLR_HOLYLIGHT))
		return TRUE;

	/*
	 * wizi check for mob victim
	 */
	if (IS_NPC(victim) && !IS_TRUSTED(ch, victim->invis_level)) {
		AREA_DATA *pArea = area_vnum_lookup(victim->pMobIndex->vnum);
		if (pArea == NULL
		||  !IS_BUILDER(vch, pArea))
			return FALSE;
	}

	if (IS_AFFECTED(ch, AFF_BLIND))
		return FALSE;

	if (room_is_dark(ch) && !IS_AFFECTED(ch, AFF_INFRARED))
		return FALSE;

	if (IS_AFFECTED(victim, AFF_INVIS)
	&&  !IS_AFFECTED(ch, AFF_DETECT_INVIS))
		return FALSE;

	if (IS_AFFECTED(victim, AFF_IMP_INVIS)
	&&  !IS_AFFECTED(ch, AFF_DETECT_IMP_INVIS))
		return FALSE;

	if (IS_AFFECTED(victim, AFF_CAMOUFLAGE)
	&&  !IS_AFFECTED(ch, AFF_ACUTE_VISION))
		return FALSE;

	if (IS_AFFECTED(victim, AFF_BLEND)
	&&  !IS_AFFECTED(ch, AFF_AWARENESS))
		return FALSE;

	if (IS_AFFECTED(victim, AFF_HIDE)
	&&  !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
	&&  victim->fighting == NULL)
		return FALSE;

	if (IS_AFFECTED(victim, AFF_FADE)
	&&  !IS_AFFECTED(ch, AFF_DETECT_FADE)
	&&  victim->fighting == NULL)
		return FALSE;

	return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))
		return TRUE;

	if (IS_SET(obj->extra_flags, ITEM_VIS_DEATH))
		return FALSE;

	if (IS_AFFECTED(ch, AFF_BLIND) && obj->pObjIndex->item_type != ITEM_POTION)
		return FALSE;

	if (obj->pObjIndex->item_type == ITEM_LIGHT && obj->value[2] != 0)
		return TRUE;

	if (IS_SET(obj->extra_flags, ITEM_INVIS)
	&&   !IS_AFFECTED(ch, AFF_DETECT_INVIS))
	    return FALSE;

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		return TRUE;

	if (room_is_dark(ch) && !IS_AFFECTED(ch, AFF_INFRARED))
		return FALSE;

	return TRUE;
}

/*
 * True if char can drop obj.
 */
bool can_drop_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (!IS_SET(obj->extra_flags, ITEM_NODROP))
		return TRUE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	return FALSE;
}

int isn_dark_safe(CHAR_DATA *ch)
{
	CHAR_DATA *rch;
	OBJ_DATA *light;
	int light_exist;

	if (!is_affected(ch, gsn_vampire)
	||  IS_SET(ch->in_room->room_flags, ROOM_DARK))
		return 0;

	if (weather_info.sunlight == SUN_LIGHT
	||  weather_info.sunlight == SUN_RISE)
		return 2;

	light_exist = 0;
	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if ((light = get_eq_char(rch, WEAR_LIGHT))
		&&  light->value[2] != 0) {
			light_exist = 1;
			break;
		}
	}

	return light_exist;
}

int count_charmed(CHAR_DATA *ch)	
{
  CHAR_DATA *gch;
  int count = 0;

  for (gch = char_list; gch != NULL; gch = gch->next)
	{
	  if (IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch)
		  count++;
	}

  if (count >= MAX_CHARM(ch))
   {
	char_puts("You are already controlling as many charmed mobs as you can!\n",ch);
	return count;
   }
  return 0;
}

/*
 * add_mind - remember 'str' in mind buffer of 'ch'
 *	      remember the place to return in mind buffer if it is empty
 *	      ch is assumed to be IS_NPC
 */
void add_mind(CHAR_DATA *ch, const char *str)
{
	NPC_DATA *npc = NPC(ch);

	if (npc->in_mind == NULL) {
		/* remember a place to return */
		npc->in_mind = str_printf("%d", ch->in_room->vnum);
	}

	name_add(&npc->in_mind, str, NULL, NULL);
}

/*
 * remove_mind - remove 'str' from mind buffer of 'ch'
 *		 if it was the last revenge - return home
 *		 ch is assumed to be IS_NPC
 */
void remove_mind(CHAR_DATA *ch, const char *str)
{
	NPC_DATA *npc = NPC(ch);

	if (!name_delete(&npc->in_mind, str, NULL, NULL))
		return;

	if (IS_NULLSTR(npc->in_mind) || is_number(npc->in_mind)) {
		dofun("say", ch, "At last, I took my revenge!"); 
		back_home(ch);
		if (!IS_EXTRACTED(ch)) {
			free_string(npc->in_mind);
			npc->in_mind = NULL;
		}
	}
}

/*
 * ch is assumed to be IS_NPC
 */
void back_home(CHAR_DATA *ch)
{
	NPC_DATA *npc = NPC(ch);
	ROOM_INDEX_DATA *location;
	char arg[MAX_INPUT_LENGTH];

	if (npc->in_mind == NULL
	&&  ch->pMobIndex->vnum < 100) {
		act("$n wanders on home.", ch, NULL, NULL, TO_ROOM);
		extract_char(ch, 0);
	}

	one_argument(npc->in_mind, arg, sizeof(arg));
	if ((location = find_location(ch, arg)) == NULL) {
		act("$n wanders on home.", ch, NULL, NULL, TO_ROOM);
		extract_char(ch, 0);
		return;
	}

	if (ch->fighting == NULL && location != ch->in_room) {
		act("$n prays for transportation.", ch, NULL, NULL, TO_ROOM);
		recall(ch, location);
	}
}

void path_to_track(CHAR_DATA *ch, CHAR_DATA *victim, int door)
{
	ROOM_INDEX_DATA *temp;
	EXIT_DATA *pExit;
	int opdoor;
	int range = 0;

	SET_FIGHT_TIME(ch);
  	if (!IS_NPC(victim)) {
		SET_FIGHT_TIME(victim);
		return;
	}

	if (victim->position == POS_DEAD)
		return;

	NPC(victim)->last_fought = ch;

	if ((opdoor = opposite_door(door)) == -1) {
		bug("In path_to_track wrong door: %d",door);
		return;
	}

	temp = ch->in_room;
	while (1) {
		range++;
		if (victim->in_room == temp)
			break;
		if ((pExit = temp->exit[ door ]) == NULL
		||  (temp = pExit->to_room.r) == NULL) {
			bug("path_to_track: couldn't calculate range %d",
			    range);
			return;
		}

		if (range > 100) {
			bug("In path_to_track: range exceeded 100",0);
			return;
		}
	}

	temp = victim->in_room;
	while (--range > 0) {
		room_record(ch->name,temp, opdoor);
		if ((pExit = temp->exit[opdoor]) == NULL
		||  (temp = pExit->to_room.r) == NULL) {
			bug("path_to_track: Range: %d Room: %d opdoor:%d",
			     range, temp->vnum, opdoor); 
			return;
		}
	}
	add_mind(victim, ch->name);
	dofun("track", victim, str_empty);
}

int pk_range(int level)
{
	return UMAX(4, level/10 + 2);
}

bool in_PK(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (IS_NPC(ch) || IS_NPC(victim))
		return TRUE;

	if (victim->level < LEVEL_PK || ch->level < LEVEL_PK)
		return FALSE;

	/* level adjustment */
	if (ch != victim && !IS_IMMORTAL(ch)
	&&  (ch->level >= (victim->level + pk_range(ch->level)) ||
	     ch->level <= (victim->level - pk_range(ch->level)))
	&&  (victim->level >= (ch->level + pk_range(victim->level)) ||
	     victim->level <= (ch->level - pk_range(victim->level))))
		return FALSE;

	return TRUE;
}

bool can_gate(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (victim == ch
	||  ch->fighting != NULL
	||  victim->in_room == NULL
	||  !can_see_room(ch, victim->in_room)
	||  IS_SET(ch->in_room->room_flags, ROOM_NORECALL |
					    ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(victim->in_room->room_flags, ROOM_SAFE | ROOM_NORECALL |
						ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(ch->in_room->area->area_flags, AREA_CLOSED)
	||  room_is_private(victim->in_room)
	||  IS_SET(victim->imm_flags, IMM_SUMMON))
		return FALSE;

	if (IS_NPC(victim)) {
		if (NPC(victim)->hunter)
			return FALSE;
		return TRUE;
	}

	if (((!in_PK(ch, victim) ||
	      ch->in_room->area != victim->in_room->area) &&
	     IS_SET(PC(victim)->plr_flags, PLR_NOSUMMON))
	||  victim->level >= LEVEL_HERO
	||  !guild_ok(ch, victim->in_room))
		return FALSE;

	return TRUE;
}

void transfer_char(CHAR_DATA *ch, CHAR_DATA *vch, ROOM_INDEX_DATA *to_room,
		   const char *msg_out,
		   const char *msg_travel,
		   const char *msg_in)
{
	ROOM_INDEX_DATA *was_in = ch->in_room;

	if (ch != vch)
		act_puts(msg_travel, vch, NULL, ch, TO_VICT, POS_DEAD);

	char_from_room(ch);

	act(msg_out, was_in->people, NULL, ch, TO_ALL);
	act(msg_in, to_room->people, NULL, ch, TO_ALL);

	char_to_room(ch, to_room);

	if (!IS_EXTRACTED(ch))
		dofun("look", ch, "auto");
}

void
recall(CHAR_DATA *ch, ROOM_INDEX_DATA *location)
{
	transfer_char(ch, NULL, location,
		      "$N disappears.", NULL, "$N appears in the room.");
}

void look_at(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	ROOM_INDEX_DATA *was_in = ch->in_room;
	OBJ_DATA *obj;
	bool adjust_light = FALSE;

	if ((obj = get_eq_char(ch, WEAR_LIGHT))
	&&  obj->pObjIndex->item_type == ITEM_LIGHT
	&&  obj->value[2]) {
		adjust_light = TRUE;
		room->light++;
	}
		
	ch->in_room = room;
	dofun("look", ch, str_empty);
	ch->in_room = was_in;

	if (adjust_light)
		room->light--;
}

/* random room generation procedure */
ROOM_INDEX_DATA  *get_random_room(CHAR_DATA *ch, AREA_DATA *area)
{
	int min_vnum;
	int max_vnum;
	ROOM_INDEX_DATA *room;

	if (!area) {
		min_vnum = 1;
		max_vnum = top_vnum_room;
	}
	else {
		min_vnum = area->min_vnum;
		max_vnum = area->max_vnum;
	}

	for (; ;) {
		room = get_room_index(number_range(min_vnum, max_vnum));
		if (!room)
			continue;

		if (ch) {
			if (!can_see_room(ch, room)
			||  (IS_NPC(ch) &&
			     IS_SET(ch->pMobIndex->act, ACT_AGGRESSIVE) &&
			     IS_SET(room->room_flags, ROOM_LAW)))
				continue;
		}

		if (!room_is_private(room)
		&&  !IS_SET(room->room_flags, ROOM_SAFE | ROOM_PEACE) 
		&&  (area || !IS_SET(room->area->area_flags, AREA_CLOSED)))
			break;
	}

	return room;
}

void format_obj(BUFFER *output, OBJ_DATA *obj)
{
	buf_printf(output,
		"Object '%s' is type %s, extra flags %s.\n"
		"Weight is %d, value is %d, level is %d.\n",
		obj->name,
		flag_string(item_types, obj->pObjIndex->item_type),
		flag_string(extra_flags, obj->extra_flags & ~ITEM_ENCHANTED),
		obj->weight / 10,
		obj->cost,
		obj->level);

	if (obj->pObjIndex->limit != -1)
		buf_printf(output,
			   "This equipment has been LIMITED by number %d \n",
			   obj->pObjIndex->limit);

	switch (obj->pObjIndex->item_type) {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		buf_printf(output, "Level %d spells of:", obj->value[0]);

		if (obj->value[1] >= 0)
			buf_printf(output, " '%s'", skill_name(obj->value[1]));

		if (obj->value[2] >= 0)
			buf_printf(output, " '%s'", skill_name(obj->value[2]));

		if (obj->value[3] >= 0)
			buf_printf(output, " '%s'", skill_name(obj->value[3]));

		if (obj->value[4] >= 0)
			buf_printf(output, " '%s'", skill_name(obj->value[4]));

		buf_add(output, ".\n");
		break;

	case ITEM_WAND: 
	case ITEM_STAFF: 
		buf_printf(output, "Has %d charges of level %d",
			   obj->value[2], obj->value[0]);
	  
		if (obj->value[3] >= 0)
			buf_printf(output, " '%s'", skill_name(obj->value[3]));

		buf_add(output, ".\n");
		break;

	case ITEM_DRINK_CON:
		buf_printf(output, "It holds %s-colored %s.\n",
			   liq_table[obj->value[2]].liq_color,
	        	   liq_table[obj->value[2]].liq_name);
		break;

	case ITEM_CONTAINER:
		buf_printf(output,
			   "Capacity: %d#  Maximum weight: %d#  flags: %s\n",
			   obj->value[0], obj->value[3],
			   flag_string(cont_flags, obj->value[1]));
		if (obj->value[4] != 100)
			buf_printf(output, "Weight multiplier: %d%%\n",
				   obj->value[4]);
		break;
			
	case ITEM_WEAPON:
		buf_printf(output, "Weapon type is %s.\n",
			   flag_string(weapon_class, obj->value[0]));
		buf_printf(output, "Damage is %dd%d (average %d).\n",
			   obj->value[1],obj->value[2],
			   (1 + obj->value[2]) * obj->value[1] / 2);
		if (obj->value[4])
	        	buf_printf(output, "Weapons flags: %s\n",
				   flag_string(weapon_type2, obj->value[4]));
		break;

	case ITEM_ARMOR:
		buf_printf(output, "Armor class is %d pierce, "
				   "%d bash, %d slash, and %d vs. magic.\n", 
			   obj->value[0], obj->value[1],
			   obj->value[2], obj->value[3]);
		break;
	}
}

void format_obj_affects(BUFFER *output, AFFECT_DATA *paf, int flags)
{
	for (; paf; paf = paf->next) {
		where_t *w;

		if (paf->where != TO_SKILLS
		    && paf->location != APPLY_NONE
		    && paf->modifier) { 
			buf_printf(output, "Affects %s by %d",
				   flag_string(apply_flags, paf->location),
				   paf->modifier);
			if (!IS_SET(flags, FOA_F_NODURATION)
			&&  paf->duration > -1)
				buf_printf(output, " for %d hours",
					   paf->duration);
			buf_add(output, ".\n");
		}

		if (IS_SET(flags, FOA_F_NOAFFECTS))
			continue;

		if (paf->where == TO_SKILLS) {
			w = where_lookup(TO_SKILLS);
			buf_add(output, "Affects ");
			buf_printf(output, w->format,
				skill_name(-(paf->location)),
				paf->modifier,
				flag_string(w->table, paf->bitvector));
			buf_add(output, ".\n");
		}

		if (paf->where != TO_SKILLS
		    && (w = where_lookup(paf->where))
		    && paf->bitvector) {
			buf_add(output, "Adds ");
			buf_printf(output, w->format,
				   flag_string(w->table, paf->bitvector));
			buf_add(output, ".\n");
		}
	}
}

int get_wear_level(CHAR_DATA *ch, OBJ_DATA *obj)
{
	int wear_level = ch->level;
	class_t *cl;

	if ((cl = class_lookup(ch->class)) == NULL)
		return wear_level;

	switch (obj->pObjIndex->item_type) {
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_WAND:
	case ITEM_STAFF:
	case ITEM_SCROLL:
		return wear_level;
	}

	if (!IS_SET(obj->pObjIndex->extra_flags, ITEM_QUEST)
	&&  (obj->pObjIndex->limit < 0 || obj->pObjIndex->limit > 1))
		wear_level += pk_range(wear_level);

	if (IS_SET(cl->class_flags, CLASS_MAGIC)) {
		if (obj->pObjIndex->item_type == ITEM_ARMOR)
			wear_level += 3;
	}
	else if (obj->pObjIndex->item_type == ITEM_WEAPON)
		wear_level += 3;
	return wear_level;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell(int level, CHAR_DATA *victim, int dam_type)
{
	class_t *vcl;
	int save;

	save = 40 + (LEVEL(victim) - level) * 4 - 
		(victim->saving_throw * 90) / UMAX(45, victim->level);

	if (IS_AFFECTED(victim, AFF_BERSERK))
		save += victim->level / 5;

	switch(check_immune(victim, dam_type)) {
	case IS_IMMUNE:
		return TRUE;
	case IS_RESISTANT:
		save += victim->level / 5;
		break;
	case IS_VULNERABLE:
		save -= victim->level / 5;
		break;
	}

	if (!IS_NPC(victim) && (vcl = class_lookup(victim->class))
	&&  IS_SET(vcl->class_flags, CLASS_MAGIC))
		save = 9 * save / 10;
	save = URANGE(5, save, 95);
	return number_percent() < save;
}

/* RT configuration smashed */

bool saves_dispel(int dis_level, int spell_level, int duration)
{
	int save;
	
	  /* impossible to dispel permanent effects */
	if (duration == -2) return 1;
	if (duration == -1) spell_level += 5;

	save = 50 + (spell_level - dis_level) * 5;
	save = URANGE(5, save, 95);
	return number_percent() < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel(int dis_level, CHAR_DATA *victim, int sn)
{
	AFFECT_DATA *af;

	if (is_affected(victim, sn)) {
	    for (af = victim->affected; af != NULL; af = af->next) {
	        if (af->type == sn) {
	            if (!saves_dispel(dis_level,af->level,af->duration)) {
			skill_t *sk;

	                affect_strip(victim,sn);
			if ((sk = skill_lookup(sn))
			&&  !IS_NULLSTR(sk->msg_off))
				char_printf(victim, "%s\n", sk->msg_off);
			return TRUE;
		    } else
			af->level--;
	        }
	    }
	}
	return FALSE;
}

bool check_blind_raw(CHAR_DATA *ch)
{
	if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))
		return TRUE;

	if (IS_AFFECTED(ch, AFF_BLIND))
		return FALSE;

	return TRUE;
}

bool check_blind(CHAR_DATA *ch)
{
	bool can_see = check_blind_raw(ch);

	if (!can_see)
		char_puts("You can't see a thing!\n", ch);

	return can_see;
}

/*----------------------------------------------------------------------------
 * show affects stuff
 */

void show_name(CHAR_DATA *ch, BUFFER *output,
	       AFFECT_DATA *paf, AFFECT_DATA *paf_last)
{
	if (paf_last && paf->type == paf_last->type)
		if (ch && ch->level < 20)
			return;
		else
			buf_add(output, "                      ");
	else
		buf_printf(output, "Spell: {c%-15s{x", skill_name(paf->type));
}

void show_duration(BUFFER *output, AFFECT_DATA *paf)
{
	if (paf->duration < 0)
		buf_add(output, "permanently.\n");
	else 
		buf_act(output, "for {c$j{x $qj{hours}.", NULL,
			(const void*) paf->duration, NULL, NULL, ACT_NOUCASE);
}

void show_loc_affect(CHAR_DATA *ch, BUFFER *output,
		 AFFECT_DATA *paf, AFFECT_DATA **ppaf)
{
	if (paf->where == TO_SKILLS)
		return;

	if (paf->location == APPLY_NONE
	&&  paf->bitvector)
		return;

	show_name(ch, output, paf, *ppaf);
	if (paf->location > 0)
		buf_printf(output, ": modifies {c%s{x by {c%d{x ",
			   flag_string(apply_flags, paf->location),
			   paf->modifier);
	else
		buf_add(output, ": ");
	show_duration(output, paf);
	*ppaf = paf;
}

void show_bit_affect(BUFFER *output, AFFECT_DATA *paf, AFFECT_DATA **ppaf)
{
	char buf[MAX_STRING_LENGTH];
	where_t *w;

	if (paf->where == TO_SKILLS
	||  (w = where_lookup(paf->where)) == NULL
	||  !paf->bitvector)
		return;

	show_name(NULL, output, paf, *ppaf);
	snprintf(buf, sizeof(buf), ": adds %s ", w->format);
	buf_printf(output, buf, flag_string(w->table, paf->bitvector));
	show_duration(output, paf);
	*ppaf = paf;
}

void show_obj_affects(CHAR_DATA *ch, BUFFER *output, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_last = NULL;

	for (; paf; paf = paf->next)
		if (paf->location != APPLY_SPELL_AFFECT)
			show_bit_affect(output, paf, &paf_last);
}

void show_affects(CHAR_DATA *ch, BUFFER *output)
{
	OBJ_DATA *obj;
	AFFECT_DATA *paf, *paf_last = NULL;

	buf_add(output, "You are affected by the following spells:\n");
	for (paf = ch->affected; paf; paf = paf->next) {
		if (ch->level < 20) {
			show_name(ch, output, paf, paf_last);
			if (paf_last && paf_last->type == paf->type)
				continue;
			buf_add(output, "\n");
			paf_last = paf;
			continue;
		}
		show_loc_affect(ch, output, paf, &paf_last);
		show_bit_affect(output, paf, &paf_last);
	}

	if (ch->level < 20)
		return;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc != WEAR_NONE) {
			if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
				show_obj_affects(ch, output,
						 obj->pObjIndex->affected);
			show_obj_affects(ch, output, obj->affected);
		}
}

/*
 * Parse a name for acceptability.
 */
bool pc_name_ok(const char *name)
{
	const unsigned char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;
	int i;

	/*
	 * Reserved words.
	 */
	if (is_name(name, "chronos all auto immortals self someone something"
			  "the you demise balance circle loner honor "
			  "none clan"))
		return FALSE;
	
	/*
	 * Length restrictions.
	 */
	 
	if (strlen(name) < 2)
		return FALSE;

	if (strlen(name) > MAX_CHAR_NAME)
		return FALSE;

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	fIll = TRUE;
	for (pc = name; *pc != '\0'; pc++) {
		if (IS_SET(mud_options, OPT_ASCII_ONLY_NAMES) && !isascii(*pc))
			return FALSE;

		if (!isalpha(*pc))
			return FALSE;

		if (isupper(*pc)) { /* ugly anti-caps hack */
			if (adjcaps)
				cleancaps = TRUE;
			total_caps++;
			adjcaps = TRUE;
		}
		else
			adjcaps = FALSE;

		if (LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
			fIll = FALSE;
	}

	if (fIll)
		return FALSE;

	if (total_caps > strlen(name) / 2)
		return FALSE;

	/*
	 * Prevent players from naming themselves after mobs.
	 */
	{
		MOB_INDEX_DATA *pMobIndex;
		int iHash;

		for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
			for (pMobIndex  = mob_index_hash[iHash];
			     pMobIndex != NULL; pMobIndex  = pMobIndex->next) 
				if (is_name(name, pMobIndex->name))
					return FALSE;
		}
	}

	for (i = 0; i < clans.nused; i++) {
		class_t *clan = VARR_GET(&clans, i);
		if (!str_cmp(name, clan->name))
			return FALSE;
	}

	return TRUE;
}

const char *stat_aliases[MAX_STATS][6] =
{
	{ "Titanic", "Herculian", "Strong", "Average", "Poor", "Weak"	},
	{ "Genious", "Clever", "Good", "Average", "Poor", "Hopeless"	},
	{ "Excellent", "Wise", "Good", "Average", "Dim", "Fool"		}, 	
	{ "Fast", "Quick", "Dextrous", "Average", "Clumsy", "Slow"	},
	{ "Iron", "Hearty", "Healthy", "Average", "Poor", "Fragile"	},
	{ "Charismatic", "Familier", "Good", "Average", "Poor", "Mongol"}
};

const char *get_stat_alias(CHAR_DATA *ch, int stat)
{
	int val;
	int i;

	if (stat >= MAX_STATS)
		return "Unknown";

	val = get_curr_stat(ch, stat);
	     if (val >  22)	i = 0;
	else if (val >= 20)	i = 1;
	else if (val >= 18)	i = 2;
	else if (val >= 14)	i = 3;
	else if (val >= 10)	i = 4;
	else			i = 5;
	return stat_aliases[stat][i];
}

/*
 * returns TRUE if lch is one of the leaders of ch
 */
bool is_leader(CHAR_DATA *ch, CHAR_DATA *lch)
{
	for (ch = ch->leader; ch; ch = ch->leader) {
		if (ch == lch)
			return TRUE;
	}

	return FALSE;
}

void set_leader(CHAR_DATA *ch, CHAR_DATA *lch)
{
	/*
	 * This code must prevent ->leader cycling
	 */
	if (is_leader(lch, ch)) {
		CHAR_DATA *tch;
		CHAR_DATA *tch_next;

		for (tch = lch; tch && tch != ch; tch = tch_next) {
			tch_next = tch->leader;
			log("set_leader: removing cycle: %s", tch->name);
			tch->leader = NULL;
			stop_follower(tch);
		}
	}
	ch->leader = lch;
}

void set_title(CHAR_DATA *ch, const char *title)
{
	char buf[MAX_TITLE_LENGTH];
	static char nospace[] = "-.,!?':";

	buf[0] = '\0';

	if (title) {
		if (strchr(nospace, *cstrfirst(title)) == NULL) {
			buf[0] = ' ';
			buf[1] = '\0';
		}

		strnzcat(buf, sizeof(buf), title);
	}

	free_string(PC(ch)->title);
	PC(ch)->title = str_dup(buf);
}

const char *garble(CHAR_DATA *ch, const char *i)
{
	static char not_garbled[] = "?!()[]{},.:;'\" ";
	static char buf[MAX_STRING_LENGTH];
	char *o;

	if (!is_affected(ch, gsn_garble))
		return i;

	for (o = buf; *i && o-buf < sizeof(buf)-1; i++, o++) {
		if (strchr(not_garbled, *i))
			*o = *i;
		else
			*o = number_range(' ', 254);
	}
	*o = '\0';
	return buf;
}

void do_tell_raw(CHAR_DATA *ch, CHAR_DATA *victim, const char *msg)
{
	if (ch == victim) {
		char_puts("Talking to yourself, eh?\n", ch);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOTELL)) {
		char_puts("Your message didn't get through.\n", ch);
		return;
	}

	if (victim == NULL 
	|| (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_SET(victim->comm, (COMM_QUIET | COMM_DEAF))
	&&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
		act_puts("$E is not receiving tells.", ch, 0, victim,
			 TO_CHAR, POS_DEAD);
		return;
	}

	msg = garble(ch, msg);
	act_puts("You tell $N '{G$t{x'",
		 ch, msg, victim, TO_CHAR | ACT_SPEECH(ch), POS_DEAD);
	act_puts("$n tells you '{G$t{x'",
		 ch, msg, victim,
		 TO_VICT | ACT_TOBUF | ACT_NOTWIT | ACT_SPEECH(ch),
		 POS_SLEEPING);

	if (IS_NPC(ch))
		return;

	if (IS_NPC(victim)) {
		if (HAS_TRIGGER(victim, TRIG_SPEECH))
			mp_act_trigger(msg, victim, ch, NULL, NULL, TRIG_SPEECH);
	}
	else {
		if (!IS_IMMORTAL(victim)
		&&  !IS_IMMORTAL(ch)
		&&  is_name(ch->name, PC(victim)->twitlist))
			return;

		if (victim->desc == NULL)
			act_puts("$N seems to have misplaced $S link but "
				 "your tell will go through if $E returns.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		else if (IS_SET(victim->comm, COMM_AFK))
			act_puts("$E is AFK, but your tell will go through "
				 "when $E returns.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		PC(victim)->reply = ch;
	}
}

void yell(CHAR_DATA *victim, CHAR_DATA* ch, const char* text)
{
	if (IS_NPC(victim)
	||  IS_IMMORTAL(victim)
	||  victim->in_room == NULL
	||  victim->position <= POS_SLEEPING
	||  IS_EXTRACTED(victim)
	||  IS_SET(PC(victim)->plr_flags, PLR_GHOST))
		return;

	act_puts("You yell '{M$t{x'", victim,
		 act_speech(victim, victim, text, ch), NULL,
		 TO_CHAR | ACT_SPEECH(ch), POS_DEAD);
	act_yell(victim, text, ch, "$n yells in panic '{M$t{x'");
}

static void drop_objs(CHAR_DATA *ch, OBJ_DATA *obj_list)
{
	OBJ_DATA *obj, *obj_next;

	/*
	 * drop ITEM_QUIT_DROP/ITEM_CHQUEST/ITEM_CLAN items
	 */
	for (obj = obj_list; obj != NULL; obj = obj_next) {
		int cn;
		obj_next = obj->next_content;

		if (obj->contains)
			drop_objs(ch, obj->contains);

		if (!IS_SET(obj->pObjIndex->extra_flags,
			    ITEM_CLAN | ITEM_QUIT_DROP | ITEM_CHQUEST))
			continue;

		if (obj->carried_by)
			obj_from_char(obj);
		else if (obj->in_obj)
			obj_from_obj(obj);
		else {
			extract_obj(obj, 0);
			continue;
		}

		if (!IS_SET(obj->pObjIndex->extra_flags, ITEM_CLAN)) {
			if (ch->in_room != NULL)
				obj_to_room(obj, ch->in_room);
			else
				extract_obj(obj, 0);
			continue;
		}

		for (cn = 0; cn < clans.nused; cn++) {
			if (obj == CLAN(cn)->obj_ptr) {
				obj_to_room(obj,
					get_room_index(CLAN(cn)->altar_vnum));
				continue;
			}
		}
		extract_obj(obj, 0);
	}
}

/*
 * ch is assumed to be !IS_NPC
 */
void quit_char(CHAR_DATA *ch, int flags)
{
	DESCRIPTOR_DATA *d, *d_next;
	CHAR_DATA *vch, *vch_next;
	const char *name;

	if (ch->position == POS_FIGHTING) {
		char_puts("No way! You are fighting.\n", ch);
		return;
	}

	if (ch->position < POS_STUNNED ) {
		char_puts("You're not DEAD yet.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		char_puts("You don't want to leave your master.\n", ch);
		return;
	}

	if (IS_SET(PC(ch)->plr_flags, PLR_NOEXP)) {
		char_puts("You don't want to lose your spirit.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP)) {
		char_puts("You cannot quit, you are in deep sleep.\n", ch);
		return;
	}

	for (vch=npc_list; vch; vch=vch->next) {
		if (IS_AFFECTED(vch, AFF_CHARM)
		&& IS_NPC(vch)
		&& IS_SET(vch->pMobIndex->act, ACT_FAMILIAR)
		&& vch->master == ch
		&& vch->in_room != ch->in_room) {
			act("You cannot quit and leave your $N alone.\n", 
				ch, NULL, vch, TO_CHAR);
			return;
		}
	}

	if (auction.item != NULL
	&&  ((ch == auction.buyer) || (ch == auction.seller))) {
		char_puts("Wait till you have sold/bought the item "
			  "on auction.\n",ch);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		if (IS_PUMPED(ch)) {
			char_puts("Your adrenalin is gushing! You can't quit yet.\n", ch);
			return;
		}

		if (is_affected(ch, gsn_witch_curse)) {
			char_puts("You are cursed. Wait till you DIE!\n", ch);
			return;
		}

		if (ch->in_room->area->clan
		&&  ch->in_room->area->clan != ch->clan) {
			char_puts("You can't quit here.\n", ch);
			return;
		}

		if (ch->in_room && IS_RAFFECTED(ch->in_room, RAFF_ESPIRIT)) {
			char_puts("Evil spirits in the area prevents you from leaving.\n", ch);
			return;
		}

		if (!get_skill(ch, gsn_evil_spirit)
		&&  is_affected(ch, gsn_evil_spirit)) {
			char_puts("Evil spirits in you prevents you from leaving.\n", ch);
			return;
		}
	}

	char_puts("Alas, all good things must come to an end.\n", ch);
	char_puts("You hit reality hard. Reality truth does unspeakable things to you.\n", ch);
	act_puts("$n has left the game.", ch, NULL, NULL, TO_ROOM, POS_RESTING);
	log("%s has quit.", ch->name);
	wiznet("{W$N{x rejoins the real world.",
		ch, NULL, WIZ_LOGINS, 0, ch->level);

	drop_objs(ch, ch->carrying);

	for (vch = char_list; vch; vch = vch_next) {
		NPC_DATA *vnpc;

		vch_next = vch->next;
		if (is_affected(vch, gsn_doppelganger)
		&&  vch->doppel == ch) {
			char_puts("You shift to your true form as your victim leaves.\n",
				  vch);
			affect_strip(vch, gsn_doppelganger);
		}

		if (vch->hunting == ch)
			vch->hunting = NULL;

		if (!IS_NPC(vch))
			continue;

		vnpc = NPC(vch);
		if (vnpc->hunter == ch)
			vnpc->hunter = NULL;

		if (vnpc->target == ch) {
			if (vch->pMobIndex->vnum == MOB_VNUM_SHADOW) {
				act("$n slowly fades away.",
				    vch, NULL, NULL, TO_ROOM);
				extract_char(vch, 0);
				continue;
			}

			if (vch->pMobIndex->vnum == MOB_VNUM_STALKER) {
				act_clan(vch, "$i has left the realm, I have to leave too.", ch);
				act("$n slowly fades away.",
				    vch, NULL, NULL, TO_ROOM);
				extract_char(vch, 0);
				continue;
			}
		}

		if (vnpc->last_fought == ch)
			vnpc->last_fought = NULL;
	}

	if (!IS_NPC(ch)) {
		if ((vch = PC(ch)->guarding) != NULL) {
			PC(ch)->guarding = NULL;
			PC(vch)->guarded_by = NULL;
			act("You stop guarding $N.",
			    ch, NULL, vch, TO_CHAR);
			act("$n stops guarding you.",
			    ch, NULL, vch, TO_VICT);
			act("$n stops guarding $N.",
			    ch, NULL, vch, TO_NOTVICT);
			if (ch->in_room != vch->in_room) {
				act("$N stops guarding $n.",
				    vch, NULL, ch, TO_NOTVICT);
			}
		}

		if ((vch = PC(ch)->guarded_by) != NULL) {
			PC(vch)->guarding = NULL;
			PC(ch)->guarded_by = NULL;
			act("You stop guarding $N.",
			    vch, NULL, ch, TO_CHAR);
			act("$n stops guarding you.",
			    vch, NULL, ch, TO_VICT);
			act("$n stops guarding $N.",
			    vch, NULL, ch, TO_NOTVICT);
			if (ch->in_room != vch->in_room) {
				act("$N stops guarding $n.",
				    ch, NULL, vch, TO_NOTVICT);
			}
		}
	}

	/*
	 * After extract_char the ch is no longer valid!
	 */
	char_save(ch, 0);
	name = str_qdup(ch->name);
	d = ch->desc;
	extract_char(ch, flags);

	if (d)
		close_descriptor(d, SAVE_F_NONE);

	/*
	 * toast evil cheating bastards 
	 *
	 * Workaround against clone cheat --
	 * Log in once, connect a second time and enter only name,
	 * drop all and quit with first character, finish login
	 * with second. This clones the player's inventory.
	 */
	for (d = descriptor_list; d; d = d_next) {
		CHAR_DATA *tch;

		d_next = d->next;
		tch = d->original ? d->original : d->character;
		if (tch && !str_cmp(name, tch->name)) {
			if (d->connected == CON_PLAYING)
				extract_char(tch, 0);
			close_descriptor(d, SAVE_F_NONE);
		} 
	}

	free_string(name);
}

void add_follower(CHAR_DATA *ch, CHAR_DATA *master)
{
	if (ch->master)
		stop_follower(ch);
	ch->master = master;
	ch->leader = NULL;

	if (can_see(master, ch))
		act_puts("$n now follows you.", ch, NULL, master, 
			 TO_VICT, POS_RESTING);
	act_puts("You now follow $N.", ch, NULL, master, 
		 TO_CHAR, POS_RESTING);
}

void stop_follower(CHAR_DATA *ch)
{
	if (ch->master == NULL) {
		bug("Stop_follower: null master.", 0);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		REMOVE_BIT(ch->affected_by, AFF_CHARM);
		affect_bit_strip(ch, TO_AFFECTS, AFF_CHARM);
	}

	if (can_see(ch->master, ch) && ch->in_room != NULL) {
		act_puts("$n stops following you.",ch, NULL, ch->master, 
			 TO_VICT, POS_RESTING);
		act_puts("You stop following $N.", ch, NULL, ch->master, 
			 TO_CHAR, POS_RESTING);
	}

	if (!IS_NPC(ch->master)) {
		PC_DATA *pc = PC(ch->master);
		if (pc->pet == ch)
			pc->pet = NULL;
	}

	if (ch->leader && ch->fighting == ch->leader->fighting)
		stop_fighting(ch, TRUE);        
	ch->master = NULL;
	ch->leader = NULL;
}

/*
 * nuke pet
 * ch is assumed to be !IS_NPC
 */
void nuke_pets(CHAR_DATA *ch)
{    
	CHAR_DATA *pet;

	if ((pet = PC(ch)->pet) != NULL) {
		stop_follower(pet);
		if (pet->in_room)
			act("$n slowly fades away.", pet, NULL, NULL, TO_ROOM);
		extract_char(pet, 0);
		PC(ch)->pet = NULL;
	}
}

void die_follower(CHAR_DATA *ch)
{
	CHAR_DATA *fch;
	CHAR_DATA *fch_next;

	if (ch->master != NULL)
		stop_follower(ch);

	ch->leader = NULL;

	for (fch = char_list; fch != NULL; fch = fch_next) {
		fch_next = fch->next;
		if (IS_NPC(fch) && IS_SET(fch->pMobIndex->act, ACT_FAMILIAR))
			continue;
		if (fch->master == ch)
			stop_follower(fch);
		if (fch->leader == ch)
			fch->leader = NULL;
	}
}

CHAR_DATA* leader_lookup(CHAR_DATA* ch)
{
	CHAR_DATA* res;
	for (res = ch; res->leader != NULL; res = res->leader)
		;
	return res;
}

void do_who_raw(CHAR_DATA* ch, CHAR_DATA *wch, BUFFER* output)
{
	clan_t *clan;
	class_t *cl;
	race_t *r;

	if ((cl = class_lookup(wch->class)) == NULL
	||  (r = race_lookup(wch->race)) == NULL
	||  !r->race_pcdata)
		return;

	buf_add(output, "{x[");
	if ((ch && (IS_IMMORTAL(ch) || ch == wch))
	||  wch->level >= LEVEL_HERO || get_curr_stat(wch, STAT_CHA) < 18)
		buf_printf(output, "%3d ", wch->level);
	else
		buf_add(output, "    ");

	if (wch->level >= LEVEL_HERO) {
		if (ch && IS_IMMORTAL(ch))
			buf_add(output, "  ");
		buf_add(output, "{G");
		switch (wch->level) {
		case LEVEL_IMP:		buf_add(output, " IMP "); break;
		case LEVEL_CRE:		buf_add(output, " CRE "); break;
		case LEVEL_SUP:		buf_add(output, " SUP "); break;
		case LEVEL_DEI:		buf_add(output, " DEI "); break;
		case LEVEL_GOD:		buf_add(output, " GOD "); break;
		case LEVEL_IMM:		buf_add(output, " IMM "); break;
		case LEVEL_DEM:		buf_add(output, " DEM "); break;
		case LEVEL_ANG:		buf_add(output, " ANG "); break;
		case LEVEL_AVA:		buf_add(output, " AVA "); break;
		case LEVEL_HERO:	buf_add(output, "HERO "); break;
		}
		buf_add(output, "{x");
		if (ch && IS_IMMORTAL(ch))
			buf_add(output, "  ");
	}
	else {
		buf_printf(output, "%5.5s", r->race_pcdata->who_name);

		if (ch && IS_IMMORTAL(ch))
			buf_printf(output, " %3.3s", cl->who_name);
	}
	buf_add(output, "] ");

	if (wch->clan
	&&  (clan = clan_lookup(wch->clan))
	&&  (!IS_SET(clan->clan_flags, CLAN_HIDDEN) ||
	     (ch && (wch->clan == ch->clan || IS_IMMORTAL(ch)))))
		buf_printf(output, "[{c%s{x] ", clan->name);

	if (IS_SET(wch->comm, COMM_AFK))
		buf_add(output, "{c[AFK]{x ");

	if (wch->invis_level >= LEVEL_HERO)
		buf_add(output, "[{WWizi{x] ");
	if (wch->incog_level >= LEVEL_HERO)
		buf_add(output, "[{DIncog{x] ");

	if (ch && in_PK(ch, wch) && !IS_IMMORTAL(ch) && !IS_IMMORTAL(wch))
		buf_add(output, "{r[{RPK{r]{x ");

	if (IS_WANTED(wch))
		buf_add(output, "{R(WANTED){x ");

	if (IS_IMMORTAL(wch))
		buf_printf(output, "{W%s{x", wch->name);
	else
		buf_add(output, wch->name);

	buf_add(output, PC(wch)->title);

	buf_add(output, "\n");
}

static int movement_loss[MAX_SECT+1] =
{
	1, 2, 2, 3, 4, 6, 4, 1, 12, 10, 6
};

int mount_success(CHAR_DATA *ch, CHAR_DATA *mount, int canattack)
{
	int	percent;
	int	success;
	int	chance;

	if ((chance = get_skill(ch, gsn_riding)) == 0)
		return FALSE;

	percent = number_percent() + (ch->level < mount->level ? 
		  (mount->level - ch->level) * 3 : 
		  (mount->level - ch->level) * 2);

	if (!ch->fighting)
		percent -= 25;

	if (!IS_NPC(ch) && IS_DRUNK(ch)) {
		percent += chance / 2;
		char_puts("Due to your being under the influence, riding seems "
			  "a bit harder...\n", ch);
	}

	success = percent - chance;

	if (success <= 0) { /* Success */
		check_improve(ch, gsn_riding, TRUE, 1);
		return TRUE;
	}

	check_improve(ch, gsn_riding, FALSE, 1);
	if (success >= 10 && MOUNTED(ch) == mount) {
		act_puts("You lose control and fall off of $N.",
			 ch, NULL, mount, TO_CHAR, POS_DEAD);
		act("$n loses control and falls off of $N.",
		    ch, NULL, mount, TO_NOTVICT);
		act_puts("$n loses control and falls off of you.",
			 ch, NULL, mount, TO_VICT, POS_SLEEPING);

		ch->riding = FALSE;
		mount->riding = FALSE;
		if (ch->position > POS_STUNNED) 
			ch->position=POS_SITTING;
	
		ch->hit -= 5;
		update_pos(ch);
	}
	if (success >= 40 && canattack) {
		act_puts("$N doesn't like the way you've been treating $M.",
			 ch, NULL, mount, TO_CHAR, POS_DEAD);
		act("$N doesn't like the way $n has been treating $M.",
		    ch, NULL, mount, TO_NOTVICT);
		act_puts("You don't like the way $n has been treating you.",
			 ch, NULL, mount, TO_VICT, POS_SLEEPING);

		act_puts("$N snarls and attacks you!",
			 mount, NULL, ch, TO_VICT, POS_DEAD);
		act("$N snarls and attacks $n!",
		    mount, NULL, ch, TO_NOTVICT);
		act_puts("You snarl and attack $n!",
			 mount, NULL, ch, TO_CHAR, POS_SLEEPING);

		damage(mount, ch, number_range(1, mount->level),
			gsn_kick, DAM_BASH, DAMF_SHOW);
	}
	return FALSE;
}

void move_char(CHAR_DATA *ch, int door, bool follow)
{
	move_char_org(ch, door, follow, FALSE);
}

bool has_boat(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	bool found;

	found = FALSE;

	if (IS_IMMORTAL(ch)) return TRUE;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (obj->pObjIndex->item_type == ITEM_BOAT) {
			found = TRUE;
			break;
		}
	return found;
}

bool move_char_org(CHAR_DATA *ch, int door, bool follow, bool is_charge)
{
	CHAR_DATA *fch;
	CHAR_DATA *fch_next;
	CHAR_DATA *mount;
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	bool room_has_pc;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	int act_flags;

	if (RIDDEN(ch) && !IS_NPC(ch->mount)) 
		return move_char_org(ch->mount,door,follow,is_charge);

	if (IS_AFFECTED(ch, AFF_DETECT_WEB) 
	|| (MOUNTED(ch) && IS_AFFECTED(ch->mount, AFF_DETECT_WEB))) {
		WAIT_STATE(ch, PULSE_VIOLENCE);
		if (number_percent() < str_app[IS_NPC(ch) ?
			20 : get_curr_stat(ch,STAT_STR)].tohit * 5) {
		 	affect_strip(ch, gsn_web);
		 	REMOVE_BIT(ch->affected_by, AFF_DETECT_WEB);
		 	act_puts("When you attempt to leave the room, you "
				 "break the webs holding you tight.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		 	act_puts("$n struggles against the webs which hold $m "
				 "in place, and break it.",
				 ch, NULL, NULL, TO_ROOM, POS_RESTING);
		}
		else {
			act_puts("You attempt to leave the room, but the webs "
				 "hold you tight.",
				 ch, NULL, NULL, TO_ROOM, POS_DEAD);
			act("$n struggles vainly against the webs which "
			    "hold $m in place.",
			    ch, NULL, NULL, TO_ROOM);
			return FALSE; 
		}
	}

	for (fch = ch->in_room->people; fch; fch = fch->next_in_room) {
		if (IS_NPC(fch)
		&&  NPC(fch)->target == ch
		&&  fch->pMobIndex->vnum == MOB_VNUM_SHADOW) {
			act_puts("You attempt to leave your shadow alone,"
				 " but fail.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}
	}

	if (door < 0 || door >= MAX_DIR) {
		bug("move_char_org: bad door %d.", door);
		return FALSE;
	}

	if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE)
	&&  !IS_AFFECTED(ch, AFF_SNEAK)) {
		REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
		act_puts("You step out of shadows.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act_puts("$n steps out of shadows.",
			 ch, NULL, NULL, TO_ROOM, POS_RESTING);
	}

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE))  {
		int chance;

		if ((chance = get_skill(ch, gsn_camouflage_move)) == 0) {
			REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
			act_puts("You step out from your cover.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n steps out from $m's cover.",
			    ch, NULL, NULL, TO_ROOM);
		}	    
		else if (number_percent() < chance)
			check_improve(ch, gsn_camouflage_move, TRUE, 5);
		else {
			REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
			act_puts("You step out from your cover.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n steps out from $m's cover.",
			    ch, NULL, NULL, TO_ROOM);
			check_improve(ch, gsn_camouflage_move, FALSE, 5);
		}	    
	}
	if (IS_AFFECTED(ch, AFF_BLEND)) {
		REMOVE_BIT(ch->affected_by, AFF_BLEND);
		affect_bit_strip(ch, TO_AFFECTS, AFF_BLEND);
		act_puts("You step out from your cover.",
			ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n steps out from $m's cover.",
			ch, NULL, NULL, TO_ROOM);
	}

	/*
	 * Exit trigger, if activated, bail out. Only PCs are triggered.
	 */
	if (!IS_NPC(ch) && mp_exit_trigger(ch, door))
		return FALSE;

	in_room = ch->in_room;
	if ((pexit = in_room->exit[door]) == NULL
	||  (to_room = pexit->to_room.r) == NULL 
	||  !can_see_room(ch, pexit->to_room.r)) {
		char_puts("Alas, you cannot go that way.\n", ch);
		return FALSE;
	}

	if (IS_ROOM_AFFECTED(in_room, RAFF_RANDOMIZER) && !is_charge) {
		int d0;
		while (1) {
			d0 = number_range(0, MAX_DIR-1);
			if ((pexit = in_room->exit[d0]) == NULL
			||  (to_room = pexit->to_room.r) == NULL 
			||  !can_see_room(ch, pexit->to_room.r))
				continue;	  
			door = d0;
			break;
		}
	}

	if (IS_SET(pexit->exit_info, EX_CLOSED) 
	&&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) ||
	     IS_SET(pexit->exit_info, EX_NOPASS))
	&&  !IS_TRUSTED(ch, LEVEL_ANG)) {
		if (IS_AFFECTED(ch, AFF_PASS_DOOR)
		&&  IS_SET(pexit->exit_info, EX_NOPASS)) {
  			act_puts("You failed to pass through the $d.",
				 ch, NULL, pexit->keyword, TO_CHAR, POS_DEAD);
			act("$n tries to pass through the $d, but $e fails.",
			    ch, NULL, pexit->keyword, TO_ROOM);
		}
		else {
			act_puts("The $d is closed.",
				 ch, NULL, pexit->keyword, TO_CHAR, POS_DEAD);
		}
		return FALSE;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	&&  ch->master != NULL
	&&  in_room == ch->master->in_room) {
		char_puts("What? And leave your beloved master?\n", ch);
		return FALSE;
	}

	if (room_is_private(to_room)) {
		char_puts("That room is private right now.\n", ch);
		return FALSE;
	}

	if (MOUNTED(ch)) {
		if (MOUNTED(ch)->position < POS_FIGHTING) {
			char_puts("Your mount must be standing.\n", ch);
			return FALSE; 
		}
		if (!mount_success(ch, MOUNTED(ch), FALSE)) {
			char_puts("Your mount subbornly refuses to go that way.\n", ch);
			return FALSE;
		}
	}

	if (!IS_NPC(ch)) {
		int move;

		if (!IS_IMMORTAL(ch)) {
			if (IS_SET(to_room->room_flags, ROOM_GUILD)
			&&  !guild_ok(ch, to_room)) {
				char_puts("You aren't allowed there.\n", ch);
				return FALSE;
			}

			if (IS_PUMPED(ch)
			&&  IS_SET(to_room->room_flags, ROOM_PEACE | ROOM_GUILD)
			&&  !IS_SET(in_room->room_flags,
				    ROOM_PEACE | ROOM_GUILD)) {
				act_puts("You feel too bloody to go in there now.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return FALSE;
			}
		}

		if (in_room->sector_type == SECT_AIR
		||  to_room->sector_type == SECT_AIR) {
			if (MOUNTED(ch)) {
		        	if(!IS_AFFECTED(MOUNTED(ch), AFF_FLYING)) {
		        		char_puts("You mount can't fly.\n", ch);
						   return FALSE;
				}
			} 
			else if (!IS_AFFECTED(ch, AFF_FLYING)
			&& !IS_IMMORTAL(ch)) {
				act_puts("You can't fly.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return FALSE;
			} 
		}

		if ((in_room->sector_type == SECT_WATER_SWIM
		||  to_room->sector_type == SECT_WATER_SWIM)
		&& MOUNTED(ch)
		&& !IS_AFFECTED(MOUNTED(ch), AFF_FLYING | AFF_SWIM)) {
			act_puts("Your mount can neither fly nor swim.",
				ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}

		if ((in_room->sector_type == SECT_WATER_SWIM ||
		     to_room->sector_type == SECT_WATER_SWIM)
		&& !MOUNTED(ch) 
		&& !IS_AFFECTED(ch, AFF_FLYING | AFF_SWIM) 
		&& !has_boat(ch)) {
			char_puts("Learn to swim or buy a boat.\n", ch);
			return FALSE;
		}

		if ((in_room->sector_type == SECT_WATER_NOSWIM ||
		     to_room->sector_type == SECT_WATER_NOSWIM)
		&& MOUNTED(ch)
		&& !IS_AFFECTED(MOUNTED(ch),AFF_FLYING)) {
			act_puts("You can't take your mount there.\n",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}  

		if ((in_room->sector_type == SECT_WATER_NOSWIM ||
		     to_room->sector_type == SECT_WATER_NOSWIM)
		&& !MOUNTED(ch) 
		&& !IS_AFFECTED(ch, AFF_FLYING) 
		&& !has_boat(ch)) {
			char_puts("You need a boat to go there.\n", ch);
			return FALSE;
		}

		if (in_room->sector_type == SECT_UNDERWATER
		||  to_room->sector_type == SECT_UNDERWATER) {
			if (MOUNTED(ch)) {
				act_puts("Your mount refuses to dive.",
					ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return FALSE;
			}
			if (!IS_AFFECTED(ch, AFF_SWIM))	{
				act_puts("You can't swim.",
					ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return FALSE;
			}
			if (!IS_AFFECTED(ch, AFF_WATER_BREATHING)
			&& in_room->sector_type != SECT_UNDERWATER
			&& to_room->sector_type == SECT_UNDERWATER)
				act_puts("Take a deep breath...",
					ch, NULL, NULL, TO_CHAR, POS_DEAD);
		}

		move = (movement_loss[URANGE(0, in_room->sector_type, MAX_SECT)]
		  + movement_loss[URANGE(0, to_room->sector_type, MAX_SECT)])/2;

		if (is_affected(ch, gsn_thumbling))
			move *= 2;
		else {
			if (IS_AFFECTED(ch,AFF_FLYING)
			|| IS_AFFECTED(ch,AFF_HASTE))
				move /= 2;

			if (IS_AFFECTED(ch,AFF_SLOW))
				move *= 2;
		}

		if (!MOUNTED(ch)) {
			int wait;

			if (ch->move < move) {
				act_puts("You are too exhausted.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return FALSE;
			}

			ch->move -= move;

			if (ch->in_room->sector_type == SECT_DESERT
			||  IS_WATER(ch->in_room))
				wait = 2;
			else
				wait = 1;

			if (IS_AFFECTED(ch, AFF_SLOW)) wait *= 2;
			WAIT_STATE(ch, wait);
		}
	}

	if (!IS_AFFECTED(ch, AFF_SNEAK)
	&&  !IS_AFFECTED(ch, AFF_CAMOUFLAGE)
	&&  ch->invis_level < LEVEL_HERO) 
		act_flags = TO_ROOM;
	else
		act_flags = TO_ROOM | ACT_NOMORTAL;

	if (!IS_NPC(ch)
	&&  ch->in_room->sector_type != SECT_INSIDE
	&&  ch->in_room->sector_type != SECT_CITY
	&&  number_percent() < get_skill(ch, gsn_quiet_movement) 
	&&  !is_charge) {
		act(MOUNTED(ch) ? "$n leaves, riding on $N." : "$n leaves.",
		    ch, NULL, MOUNTED(ch), act_flags);
		check_improve(ch,gsn_quiet_movement,TRUE,1);
	}
	else if (is_charge) {
		act("$n spurs $s $N, leaving $t.",
		    ch, dir_name[door], ch->mount,  TO_ROOM);
	}
	else {
		act(MOUNTED(ch) ? "$n leaves $t, riding on $N." :
				  "$n leaves $t.",
		    ch, dir_name[door], MOUNTED(ch), act_flags);
	}

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE)
	&&  to_room->sector_type != SECT_FOREST
	&&  to_room->sector_type != SECT_MOUNTAIN
	&&  to_room->sector_type != SECT_HILLS) {
		REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
		act_puts("You step out from your cover.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n steps out from $m's cover.",
		    ch, NULL, NULL, TO_ROOM);
	}

	/* room record for tracking */
	if (!IS_NPC(ch) && !is_affected(ch, gsn_notrack))
		room_record(ch->name, in_room, door);

	/*
	 * now, after all the checks are done we should
	 * - take the char from the room
	 * - print the message to chars in to_room about ch arrival
	 * - put the char to to_room
	 * - CHECK THAT CHAR IS NOT DEAD after char_to_room
	 * - move all the followers and pull all the triggers
	 */
	mount = MOUNTED(ch);
	char_from_room(ch);

	if (!IS_AFFECTED(ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO) 
		act_flags = TO_ALL;
	else
		act_flags = TO_ALL | ACT_NOMORTAL;

	if (!is_charge) 
		act(mount ? "$i has arrived, riding $N." : "$i has arrived.",
	    	    to_room->people, ch, mount, act_flags);

	char_to_room(ch, to_room);

	if (mount) {
		char_from_room(mount);
		char_to_room(mount, to_room);
  		ch->riding = TRUE;
  		mount->riding = TRUE;
	}

	if (!IS_EXTRACTED(ch))
		dofun("look", ch, "auto");

	if (in_room == to_room) /* no circular follows */
		return TRUE;

	/*
	 * move all the followers
	 */
	for (fch = in_room->people; fch; fch = fch_next) {
		fch_next = fch->next_in_room;

		if (fch->master != ch || fch->position != POS_STANDING
		||  !can_see_room(fch, to_room))
			continue;

		if (IS_SET(to_room->room_flags, ROOM_LAW)
		&&  IS_NPC(fch)
		&&  IS_SET(fch->pMobIndex->act, ACT_AGGRESSIVE)) {
			act_puts("You can't bring $N into the city.",
				 ch, NULL, fch, TO_CHAR, POS_DEAD);
			act("You aren't allowed in the city.",
			    fch, NULL, NULL, TO_CHAR);
			continue;
		}

		act_puts("You follow $N.", fch, NULL, ch, TO_CHAR, POS_DEAD);
		move_char(fch, door, TRUE);
	}

	if (IS_EXTRACTED(ch))
		return TRUE;

	room_has_pc = FALSE;
	for (fch = to_room->people; fch != NULL; fch = fch_next) {
		fch_next = fch->next_in_room;
		if (!IS_NPC(fch)) {
			room_has_pc = TRUE;
			break;
		}
	}

	if (!room_has_pc)
		return TRUE;

	/*
	 * pull GREET and ENTRY triggers
	 *
	 * if someone is following the char, these triggers get activated
	 * for the followers before the char, but it's safer this way...
	 */
	for (fch = to_room->people; fch; fch = fch_next) {
		fch_next = fch->next_in_room;

		/* greet progs for items carried by people in room */
		for (obj = fch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;
			oprog_call(OPROG_GREET, obj, ch, NULL);
		}
	}

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		oprog_call(OPROG_GREET, obj, ch, NULL);
	}

	if (!IS_NPC(ch))
    		mp_greet_trigger(ch);

	for (obj = ch->carrying; obj; obj = obj_next) {
		obj_next = obj->next_content;
		oprog_call(OPROG_ENTRY, obj, NULL, NULL);
	}

	if (IS_NPC(ch) && HAS_TRIGGER(ch, TRIG_ENTRY))
		mp_percent_trigger(ch, NULL, NULL, NULL, TRIG_ENTRY);

	return TRUE;
}

int find_door(CHAR_DATA *ch, char *arg)
{
	EXIT_DATA *pexit;
	int door;

	     if (!str_cmp(arg, "n") || !str_cmp(arg, "north")) door = 0;
	else if (!str_cmp(arg, "e") || !str_cmp(arg, "east" )) door = 1;
	else if (!str_cmp(arg, "s") || !str_cmp(arg, "south")) door = 2;
	else if (!str_cmp(arg, "w") || !str_cmp(arg, "west" )) door = 3;
	else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"   )) door = 4;
	else if (!str_cmp(arg, "d") || !str_cmp(arg, "down" )) door = 5;
	else {
		for (door = 0; door <= 5; door++) {
		    if ((pexit = ch->in_room->exit[door]) != NULL
		    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
		    &&   pexit->keyword != NULL
		    &&   is_name(arg, pexit->keyword))
			return door;
		}
		act_puts("I see no $T here.", ch, NULL, arg, TO_CHAR, POS_DEAD);
		return -1;
	}

	if ((pexit = ch->in_room->exit[door]) == NULL) {
		act_puts("I see no door $T here.",
			 ch, NULL, arg, TO_CHAR, POS_DEAD);
		return -1;
	}

	if (!IS_SET(pexit->exit_info, EX_ISDOOR)) {
		char_puts("You can't do that.\n", ch);
		return -1;
	}

	return door;
}

/* RT part of the corpse looting code */
bool can_loot(CHAR_DATA * ch, OBJ_DATA * obj)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	/*
	 * PC corpses in the ROOM_BATTLE_ARENA rooms can be looted
	 * only by owners
	 */
	if (obj->in_room
	&&  IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA)
	&&  !IS_OWNER(ch, obj))
		return FALSE;

	return TRUE;
}

void get_obj(CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container,
	     const char *msg_others)
{
	/* variables for AUTOSPLIT */
	CHAR_DATA      *gch;
	int             members;
	int		carry_w, carry_n;

	if (!CAN_WEAR(obj, ITEM_TAKE)
	||  (obj->pObjIndex->item_type == ITEM_CORPSE_PC &&
	     obj->in_room &&
	     IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA) &&
	     !IS_OWNER(ch, obj))) {
		char_puts("You can't take that.\n", ch);
		return;
	}

	/* can't even get limited eq which does not match alignment */
	if (obj->pObjIndex->limit != -1) {
		if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
			act_puts("You are zapped by $p and drop it.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n is zapped by $p and drops it.",
			    ch, obj, NULL, TO_ROOM);
			return;
		}
	}

	if ((carry_n = can_carry_n(ch)) >= 0
	&&  ch->carry_number + get_obj_number(obj) > carry_n) {
		act_puts("$P: you can't carry that many items.",
			 ch, NULL, obj, TO_CHAR, POS_DEAD);
		return;
	}

	if ((carry_w = can_carry_w(ch)) >= 0
	&&  get_carry_weight(ch) + get_obj_weight(obj) > carry_w) {
		act_puts("$P: you can't carry that much weight.",
			 ch, NULL, obj, TO_CHAR, POS_DEAD);
		return;
	}

	if (obj->in_room != NULL) {
		for (gch = obj->in_room->people; gch != NULL;
		     gch = gch->next_in_room)
			if (gch->on == obj) {
				act_puts("$N appears to be using $p.",
					 ch, obj, gch, TO_CHAR, POS_DEAD);
				return;
			}
	}

	if (obj->pObjIndex->item_type == ITEM_MONEY) {
		if (carry_w >= 0
		&&  get_carry_weight(ch) + MONEY_WEIGHT(obj) > carry_w) {
			act_puts("$d: you can't carry that much weight.",
				 ch, NULL, obj->name, TO_CHAR, POS_DEAD);
			return;
		}
	}

	if (container) {
		if (IS_SET(container->pObjIndex->extra_flags, ITEM_PIT)
		&&  !IS_OBJ_STAT(obj, ITEM_HAD_TIMER))
			obj->timer = 0;
		REMOVE_BIT(obj->extra_flags, ITEM_HAD_TIMER);

		act_puts("You get $p from $P.",
			 ch, obj, container, TO_CHAR, POS_DEAD);
		act(msg_others == NULL ? "$n gets $p from $P." : msg_others,
		    ch, obj, container,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? ACT_NOMORTAL : 0));

		obj_from_obj(obj);
	}
	else {
		act_puts("You get $p.", ch, obj, container, TO_CHAR, POS_DEAD);
		act(msg_others == NULL ? "$n gets $p." : msg_others,
		    ch, obj, container,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? ACT_NOMORTAL : 0));

		obj_from_room(obj);
	}

	if (obj->pObjIndex->item_type == ITEM_MONEY) {
		ch->silver += obj->value[0];
		ch->gold += obj->value[1];
		if (!IS_NPC(ch)
		&&  IS_SET(PC(ch)->plr_flags, PLR_AUTOSPLIT)) {
			/* AUTOSPLIT code */
			members = 0;
			for (gch = ch->in_room->people; gch != NULL;
			     gch = gch->next_in_room) {
				if (!IS_AFFECTED(gch, AFF_CHARM)
				    && is_same_group(gch, ch))
					members++;
			}

			if (members > 1 && (obj->value[0] > 1
					    || obj->value[1]))
				dofun("split", ch, "%d %d", obj->value[0],
				       obj->value[1]);
		}
		extract_obj(obj, 0);
	} else {
		obj_to_char(obj, ch);
		oprog_call(OPROG_GET, obj, ch, NULL);
	}
}

void quaff_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	act("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You quaff $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	if (IS_PUMPED(ch) || ch->fighting != NULL)
		WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

	obj_to_char(create_obj(get_obj_index(OBJ_VNUM_POTION_VIAL), 0), ch);

	obj_cast_spell(obj->value[1], obj->value[0], ch, ch);

	if (!IS_EXTRACTED(ch))
		obj_cast_spell(obj->value[2], obj->value[0], ch, ch);

	if (!IS_EXTRACTED(ch))
		obj_cast_spell(obj->value[3], obj->value[0], ch, ch);

	if (!IS_EXTRACTED(ch))
		obj_cast_spell(obj->value[4], obj->value[0], ch, ch);

	extract_obj(obj, 0);
}

/*
 * Remove an object.
 */
bool remove_obj(CHAR_DATA * ch, int iWear, bool fReplace)
{
	OBJ_DATA       *obj;
	if ((obj = get_eq_char(ch, iWear)) == NULL)
		return TRUE;

	if (!fReplace)
		return FALSE;

	if (IS_SET(obj->extra_flags, ITEM_NOREMOVE)) {
		act_puts("You can't remove $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}
	if ((obj->pObjIndex->item_type == ITEM_TATTOO) && (!IS_IMMORTAL(ch))) {
		act_puts("You must scratch it to remove $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}
	if (iWear == WEAR_STUCK_IN) {
		int wsn = get_weapon_sn(obj);
		unequip_char(ch, obj);

		if (obj->pObjIndex->item_type == ITEM_WEAPON
		&&  get_stuck_eq(ch, obj->value[0]) == NULL)
			if (is_affected(ch, wsn))
				affect_strip(ch, wsn);

		act_puts("You remove $p, in pain.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n removes $p, in pain.", ch, obj, NULL, TO_ROOM);
		/*Osyas patch. Adding damage remove arrow/spear*/
                damage(ch,ch, dice(obj->level,12),0,DAM_OTHER,DAMF_NONE);
                WAIT_STATE(ch, 4);
		return TRUE;
	}
	unequip_char(ch, obj);
	act("$n stops using $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You stop using $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	if (iWear == WEAR_WIELD
	    && (obj = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL) {
		unequip_char(ch, obj);
		equip_char(ch, obj, WEAR_WIELD);
	}
	return TRUE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj(CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace)
{
	int wear_level = get_wear_level(ch, obj);

	if (wear_level < obj->level) {
		char_printf(ch, "You must be level %d to use this object.\n",
			    obj->level - wear_level + ch->level);
		act("$n tries to use $p, but is too inexperienced.",
		    ch, obj, NULL, TO_ROOM);
		return;
	}

	if (obj->pObjIndex->item_type == ITEM_LIGHT) {
		if (!remove_obj(ch, WEAR_LIGHT, fReplace))
			return;
		act("$n lights $p and holds it.", ch, obj, NULL, TO_ROOM);
		act_puts("You light $p and hold it.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_LIGHT);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_FINGER)) {
		if (get_eq_char(ch, WEAR_FINGER_L) != NULL
		&&  get_eq_char(ch, WEAR_FINGER_R) != NULL
		&&  !remove_obj(ch, WEAR_FINGER_L, fReplace)
		&&  !remove_obj(ch, WEAR_FINGER_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_FINGER_L) == NULL) {
			act("$n wears $p on $s left finger.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p on your left finger.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_FINGER_L);
			return;
		}
		if (get_eq_char(ch, WEAR_FINGER_R) == NULL) {
			act("$n wears $p on $s right finger.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p on your right finger.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_FINGER_R);
			return;
		}
		bug("Wear_obj: no free finger.", 0);
		char_puts("You already wear two rings.\n", ch);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_NECK)) {
		if (get_eq_char(ch, WEAR_NECK_1) != NULL
		    && get_eq_char(ch, WEAR_NECK_2) != NULL
		    && !remove_obj(ch, WEAR_NECK_1, fReplace)
		    && !remove_obj(ch, WEAR_NECK_2, fReplace))
			return;

		if (get_eq_char(ch, WEAR_NECK_1) == NULL) {
			act("$n wears $p around $s neck.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your neck.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_NECK_1);
			return;
		}
		if (get_eq_char(ch, WEAR_NECK_2) == NULL) {
			act("$n wears $p around $s neck.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your neck.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_NECK_2);
			return;
		}
		bug("Wear_obj: no free neck.", 0);
		char_puts("You already wear two neck items.\n", ch);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_BODY)) {
		if (!remove_obj(ch, WEAR_BODY, fReplace))
			return;
		act("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your torso.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_BODY);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
		if (!remove_obj(ch, WEAR_HEAD, fReplace))
			return;
		act("$n wears $p on $s head.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your head.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HEAD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_LEGS)) {
		if (!remove_obj(ch, WEAR_LEGS, fReplace))
			return;
		act("$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your legs.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_LEGS);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_FEET)) {
		if (!remove_obj(ch, WEAR_FEET, fReplace))
			return;
		act("$n wears $p on $s feet.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your feet.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FEET);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_HANDS)) {
		if (!remove_obj(ch, WEAR_HANDS, fReplace))
			return;
		act("$n wears $p on $s hands.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your hands.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HANDS);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_ARMS)) {
		if (!remove_obj(ch, WEAR_ARMS, fReplace))
			return;
		act("$n wears $p on $s arms.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your arms.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_ARMS);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_ABOUT)) {
		if (!remove_obj(ch, WEAR_ABOUT, fReplace))
			return;
		act("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your torso.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_ABOUT);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_WAIST)) {
		if (!remove_obj(ch, WEAR_WAIST, fReplace))
			return;
		act("$n wears $p about $s waist.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p about your waist.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_WAIST);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_WRIST)) {
		if (get_eq_char(ch, WEAR_WRIST_L) != NULL
		    && get_eq_char(ch, WEAR_WRIST_R) != NULL
		    && !remove_obj(ch, WEAR_WRIST_L, fReplace)
		    && !remove_obj(ch, WEAR_WRIST_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_WRIST_L) == NULL) {
			act("$n wears $p around $s left wrist.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your left wrist.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_WRIST_L);
			return;
		}
		if (get_eq_char(ch, WEAR_WRIST_R) == NULL) {
			act("$n wears $p around $s right wrist.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your right wrist.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_WRIST_R);
			return;
		}
		bug("Wear_obj: no free wrist.", 0);
		char_puts("You already wear two wrist items.\n", ch);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_SHIELD)) {
		OBJ_DATA       *weapon;
		if (get_eq_char(ch, WEAR_SECOND_WIELD) != NULL) {
			char_puts("You can't use a shield while using a second weapon.\n", ch);
			return;
		}
		if (!remove_obj(ch, WEAR_SHIELD, fReplace))
			return;

		weapon = get_eq_char(ch, WEAR_WIELD);
		if (weapon != NULL && ch->size < SIZE_LARGE
		&&  IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS)) {
			char_puts("Your hands are tied up with your weapon!\n", ch);
			return;
		}
		if (weapon && (weapon->value[0] == WEAPON_STAFF)) {
			char_puts("You need both hands for this type of "
				"weapon.\n", ch);
			return;
		}
		act("$n wears $p as a shield.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p as a shield.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_SHIELD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WIELD)) {
		int             skill;
		OBJ_DATA       *dual;
		if ((dual = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL)
			unequip_char(ch, dual);

		if (!remove_obj(ch, WEAR_WIELD, fReplace))
			return;

		if (!IS_NPC(ch)
		&& get_obj_weight(obj) >
			  str_app[get_curr_stat(ch, STAT_STR)].wield * 10) {
			char_puts("It is too heavy for you to wield.\n", ch);
			if (dual)
				equip_char(ch, dual, WEAR_SECOND_WIELD);
			return;
		}
		if ((get_eq_char(ch, WEAR_SHIELD)
			|| get_eq_char(ch, WEAR_SECOND_WIELD))
		  && (obj->value[0] == WEAPON_STAFF
		    || (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)
		       && !IS_NPC(ch) && ch->size < SIZE_LARGE))) {
				char_puts("You need two hands free for that"
					" weapon.\n", ch);
			if (dual)
				equip_char(ch, dual, WEAR_SECOND_WIELD);
			return;
		}
		act("$n wields $p.", ch, obj, NULL, TO_ROOM);
		act_puts("You wield $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		obj = equip_char(ch, obj, WEAR_WIELD);
		if (dual)
			equip_char(ch, dual, WEAR_SECOND_WIELD);

		if (obj == NULL)
			return;

		skill = get_weapon_skill(ch, get_weapon_sn(obj));

		if (skill >= 100)
			act_puts("$p feels like a part of you!",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 85)
			act_puts("You feel quite confident with $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 70)
			act_puts("You are skilled with $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 50)
			act_puts("Your skill with $p is adequate.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 25)
			act_puts("$p feels a little clumsy in your hands.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 1)
			act_puts("You fumble and almost drop $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else
			act_puts("You don't even know which end is up on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_HOLD)) {
		OBJ_DATA *wield;

		if (get_eq_char(ch, WEAR_SECOND_WIELD) != NULL) {
			act_puts("You can't hold an item while using 2 weapons.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}
		if ((wield = get_eq_char(ch, WEAR_WIELD))
		  && wield->value[0] == WEAPON_STAFF) {
			char_puts("You cannot hold something with this weapon"
				" wielded.\n", ch);
			return;
		}
		if (!remove_obj(ch, WEAR_HOLD, fReplace))
			return;
		act("$n holds $p in $s hand.", ch, obj, NULL, TO_ROOM);
		act_puts("You hold $p in your hand.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HOLD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_FLOAT)) {
		if (!remove_obj(ch, WEAR_FLOAT, fReplace))
			return;
		act("$n releases $p to float next to $m.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You release $p and it floats next to you.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FLOAT);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_TATTOO) && IS_IMMORTAL(ch)) {
		if (!remove_obj(ch, WEAR_TATTOO, fReplace))
			return;
		act("$n now uses $p as tattoo of $s religion.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You now use $p as the tattoo of your religion.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_TATTOO);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_CLANMARK)) {
		if (!remove_obj(ch, WEAR_CLANMARK, fReplace))
			return;
		act("$n now uses $p as $s clan mark.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You now use $p as  your clan mark.",
		    ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_CLANMARK);
		return;
	}
	if (fReplace)
		char_puts("You can't wear, wield, or hold that.\n", ch);
}

void wiznet(const char *msg, CHAR_DATA *ch, const void *arg,
	    flag32_t flag, flag32_t flag_skip, int min_level)
{
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *vch = d->original ? d->original : d->character;

		if (d->connected != CON_PLAYING
		||  !vch
		||  vch->level < LEVEL_IMMORTAL
		||  !IS_SET(PC(vch)->wiznet, WIZ_ON)
		||  (flag && !IS_SET(PC(vch)->wiznet, flag))
		||  (flag_skip && IS_SET(PC(vch)->wiznet, flag_skip))
		||  vch->level < min_level
		||  vch == ch)
			continue;

		if (IS_SET(PC(vch)->wiznet, WIZ_PREFIX))
			act_puts("--> ", d->character,
				 NULL, NULL, TO_CHAR | ACT_NOLF,
				 POS_DEAD);
		act_puts(msg, d->character, arg, ch,
			 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	}
}

ROOM_INDEX_DATA *find_location(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if (is_number(argument))
		return get_room_index(atoi(argument));

	if ((victim = get_char_world(ch, argument)) != NULL)
		return victim->in_room;

	if ((obj = get_obj_world(ch, argument)) != NULL)
		return obj->in_room;

	return NULL;
}

void reboot_mud(void)
{
	extern bool merc_down;
	DESCRIPTOR_DATA *d,*d_next;

	log("Rebooting SoG");
	for (d = descriptor_list; d != NULL; d = d_next) {
		d_next = d->next;
		write_to_buffer(d,"SoG is going down for rebooting NOW!\n\r",0);
		close_descriptor(d, SAVE_F_REBOOT);
	}

	if (!rebooter) {
		FILE *fp = dfopen(TMP_PATH, EQCHECK_FILE, "w");
		if (!fp)
			log("reboot_mud: unable to activate eqcheck");
		else {
			log("reboot_mud: eqcheck activated");
			fclose(fp);
		}
	}

	merc_down = TRUE;    
}

/* object condition aliases */
const char *get_cond_alias(OBJ_DATA *obj)
{
	char *stat;
	int istat = obj->condition;

	     if	(istat >= COND_EXCELLENT)	stat = "excellent";
	else if (istat >= COND_FINE)		stat = "fine";
	else if (istat >= COND_GOOD)		stat = "good";
	else if (istat >= COND_AVERAGE)		stat = "average";
	else if (istat >= COND_POOR)		stat = "poor";
	else					stat = "fragile";

	return stat;
}

void damage_to_obj(CHAR_DATA *ch, OBJ_DATA *wield, OBJ_DATA *worn, int damage) 
{

 	if (damage == 0) return;

 	worn->condition -= damage;

	act_puts("{gThe $p inflicts damage on {r$P{g.{x",
		 ch, wield, worn, TO_ROOM, POS_RESTING);

	if (worn->condition < 1) {
		act_puts("{gThe {r$P{g breaks into pieces.{x",
			 ch, wield, worn, TO_ROOM, POS_RESTING);
		extract_obj(worn, 0);
		return;
	}
 
	if (IS_SET(wield->extra_flags, ITEM_ANTI_EVIL) 
	&&  IS_SET(wield->extra_flags, ITEM_ANTI_NEUTRAL)
	&&  IS_SET(worn->extra_flags, ITEM_ANTI_EVIL) 
	&&  IS_SET(worn->extra_flags, ITEM_ANTI_NEUTRAL)) {
		act_puts("$p doesn't want to fight against $P.",
			 ch, wield, worn, TO_ROOM, POS_RESTING);
		act_puts("$p removes itself from you!",
			 ch, wield, worn, TO_CHAR, POS_RESTING);
		act_puts("$p removes itself from $n.",
			 ch, wield, worn, TO_ROOM, POS_RESTING);
		unequip_char(ch, wield);
		return;
 	}

	if (IS_SET(wield->extra_flags, ITEM_ANTI_EVIL) 
	&&  IS_SET(worn->extra_flags, ITEM_ANTI_EVIL)) {
		act_puts("The $p worries for the damage to $P.",
			 ch, wield, worn, TO_ROOM, POS_RESTING);
		return;
	}
}

/*----------------------------------------------------------------------------
 * eq damage functions
 *	- the third parameter is the location of wielded weapon
 *	  (must be WEAR_WIELD or WEAR_SECOND_WIELD), not the
 *	  location of damaged eq
 */

void check_eq_damage(CHAR_DATA *ch, CHAR_DATA *victim, int loc)
{
	OBJ_DATA *wield, *destroy;
	int skill, chance=0, sn, i;

	if (IS_NPC(victim) || number_percent() < 94)
		return;

	if ((wield = get_eq_char(ch, loc)) == NULL)
		return;
 	sn = get_weapon_sn(wield);
 	skill = get_skill(ch, sn);

	for (i = 0; i < MAX_WEAR; i++) {
		if ((destroy = get_eq_char(victim,i)) == NULL 
		||  number_percent() > 95
		||  number_percent() > 94
		||  ch->level < (victim->level - 10) 
		||  check_material(destroy,"platinum") 
		||  destroy->pObjIndex->limit != -1
		||  (i == WEAR_WIELD || i== WEAR_SECOND_WIELD ||
		     i == WEAR_TATTOO || i == WEAR_STUCK_IN ||
		     i == WEAR_CLANMARK ))
			continue;
	
		if (is_metal(wield)) {
	 		if (number_percent() > skill)
				continue;

			chance += 20;
			if (check_material(wield, "platinium")
			||  check_material(wield, "titanium"))
	 			chance += 5;

			if (is_metal(destroy))
				chance -= 20;
			else
				chance += 20; 

			chance += ((ch->level - victim->level) / 5);
			chance += ((wield->level - destroy->level) / 2);
		}
		else {
	 		if (number_percent() < skill)
				continue;

			chance += 10;

			if (is_metal(destroy))
				chance -= 20;
			chance += (ch->level - victim->level);
			chance += (wield->level - destroy->level);
		}

		/* sharpness */
		if (IS_WEAPON_STAT(wield, WEAPON_SHARP))
			chance += 10;

		if (sn == gsn_axe)
			chance += 10;

		/* spell affects */
		if (IS_OBJ_STAT(destroy, ITEM_BLESS))
			chance -= 10;
		if (IS_OBJ_STAT(destroy, ITEM_MAGIC))
			chance -= 20;
	 
		chance += skill - 85;
		chance += get_curr_stat(ch, STAT_STR);

		if (number_percent() < chance && chance > 50) {
			damage_to_obj(ch, wield, destroy, chance / 5);
			break;
		}
	}
}

void check_shield_damage(CHAR_DATA *ch, CHAR_DATA *victim, int loc)
{
	OBJ_DATA *wield, *destroy;
	int skill, chance=0, sn;

	if (IS_NPC(victim) || number_percent() < 94)
		return;

	if ((wield = get_eq_char(ch, loc)) == NULL)
		return;
 	sn = get_weapon_sn(wield);
 	skill = get_skill(ch, sn);

	if ((destroy = get_eq_char(victim, WEAR_SHIELD)) == NULL
	||  number_percent() > 94
	||  ch->level < (victim->level - 10) 
	||  check_material(destroy, "platinum") 
	||  destroy->pObjIndex->limit != -1)
		return;
	
	if (is_metal(wield)) {
		if (number_percent() > skill)
			return;

		chance += 20;
		if (check_material(wield, "platinium")
		||  check_material(wield, "titanium"))
			chance += 5;

		if (is_metal(destroy))
			chance -= 20;
		else
			chance += 20; 

		chance += ((ch->level - victim->level) / 5);
		chance += ((wield->level - destroy->level) / 2);
	}
	else {
		if (number_percent() < skill)
			return;

		chance += 10;
		if (is_metal(destroy))
			chance -= 20;

		chance += (ch->level - victim->level);
		chance += (wield->level - destroy->level);
	}

	/* sharpness */
	if (IS_WEAPON_STAT(wield, WEAPON_SHARP))
		chance += 10;

	if (sn == gsn_axe)
		chance += 10;

	/* spell affects */
	if (IS_OBJ_STAT(destroy, ITEM_BLESS))
		chance -= 10;
	if (IS_OBJ_STAT(destroy, ITEM_MAGIC))
		chance -= 20;
	 
 	chance += skill - 85;
 	chance += get_curr_stat(ch, STAT_STR);

	if (number_percent() < chance && chance > 20)
		damage_to_obj(ch, wield, destroy, chance / 4);
}

void check_weapon_damage(CHAR_DATA *ch, CHAR_DATA *victim, int loc)
{
	OBJ_DATA *wield, *destroy;
	int skill, chance=0, sn;

	if (IS_NPC(victim) || number_percent() < 94)
		return;

	if ((wield = get_eq_char(ch, loc)) == NULL)
		return;
 	sn = get_weapon_sn(wield);
 	skill = get_skill(ch, sn);

	if ((destroy = get_eq_char(victim, WEAR_WIELD)) == NULL
	||  number_percent() > 94
	||  ch->level < (victim->level - 10) 
	||  check_material(destroy, "platinum") 
	||  destroy->pObjIndex->limit != -1)
		return;
	
	if (is_metal(wield)) {
		if (number_percent() > skill)
			return;

		chance += 20;
		if (check_material(wield, "platinium")
		||  check_material(wield, "titanium"))
			chance += 5;

		if (is_metal(destroy))
			chance -= 20;
		else
			chance += 20; 

		chance += ((ch->level - victim->level) / 5);
		chance += ((wield->level - destroy->level) / 2);
	}
	else {
		if (number_percent() < skill)
			return;

		chance += 10;

		if (is_metal(destroy))
			chance -= 20;

		chance += (ch->level - victim->level);
		chance += (wield->level - destroy->level);
	}

	/* sharpness */
	if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
		chance += 10;

	if (sn == gsn_axe)
		chance += 10;

	/* spell affects */
	if (IS_OBJ_STAT(destroy, ITEM_BLESS))
		chance -= 10;
	if (IS_OBJ_STAT(destroy, ITEM_MAGIC))
		chance -= 20;
	 
	chance += skill - 85 ;
	chance += get_curr_stat(ch, STAT_STR);

	if (number_percent() < (chance / 2) && chance > 20)
		damage_to_obj(ch, wield, destroy, chance / 4);
}

/*
 * finds guard for ch (if any) when mob attacks
 * ch is assumed to be !IS_NPC
 */
CHAR_DATA *check_guard(CHAR_DATA *ch, CHAR_DATA *mob)
{
	int chance;
	CHAR_DATA *guarded_by = PC(ch)->guarded_by;

	if (guarded_by == NULL
	||  get_char_room(ch, guarded_by->name) == NULL)
		return ch;
	else {
		chance = get_skill(guarded_by, gsn_guard) - 
				3 * (ch->level - mob->level) / 2;
		if (number_percent() < chance) {
			act("$n jumps in front of $N!",
			    guarded_by, NULL, ch, TO_NOTVICT);
			act("$n jumps in front of you!",
			    guarded_by, NULL, ch, TO_VICT);
			act("You jump in front of $N!",
			    guarded_by, NULL, ch, TO_CHAR);
			check_improve(guarded_by, gsn_guard, TRUE, 3);
			return guarded_by;
		} else {
			check_improve(guarded_by, gsn_guard, FALSE, 3);
			return ch;
		}
	}
}

static int
get_played(CHAR_DATA *ch, bool add_age)
{
	int pl;

	if (IS_NPC(ch))
		return 0;

	pl = current_time - PC(ch)->logon + PC(ch)->played;
	if (add_age)
		pl += PC(ch)->add_age;
	return pl;
}

int get_age(CHAR_DATA *ch)
{
	return (17 + get_played(ch, TRUE) / 72000);
}

int get_hours(CHAR_DATA *ch)
{
	return get_played(ch, FALSE) / 3600;
}

int trust_level(CHAR_DATA *ch)
{
	ch = GET_ORIGINAL(ch);
	return IS_NPC(ch) ? UMIN((ch)->level, LEVEL_HERO - 1) : ch->level;
}

