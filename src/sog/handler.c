/*
 * $Id: handler.c,v 1.69 1998-10-11 16:52:44 fjoe Exp $
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
#include "hometown.h"
#include "obj_prog.h"
#include "raffects.h"
#include "interp.h"
#include "fight.h"
#include "quest.h"

/*
 * Local functions.
 */
void	affect_modify	(CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd);
int	age_to_num	(int age);

/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim)
{
	if (is_same_group(ch,victim))
		return TRUE;

	
	if (!IS_NPC(ch))
		return FALSE;

	if (!IS_NPC(victim))
	{
		if (IS_SET(ch->off_flags,ASSIST_PLAYERS))
		    return TRUE;
		else
		    return FALSE;
	}

	if (IS_AFFECTED(ch,AFF_CHARM))
		return FALSE;

	if (IS_SET(ch->off_flags,ASSIST_ALL))
		return TRUE;

	if (ch->group && ch->group == victim->group)
		return TRUE;

	if (IS_SET(ch->off_flags,ASSIST_VNUM) 
	&&  ch->pIndexData == victim->pIndexData)
		return TRUE;

	if (IS_SET(ch->off_flags,ASSIST_RACE) && RACE(ch) == RACE(victim))
		return TRUE;
	 
	if (IS_SET(ch->off_flags,ASSIST_ALIGN)
	&&  !IS_SET(ch->act,ACT_NOALIGN) && !IS_SET(victim->act,ACT_NOALIGN)
	&&  ((IS_GOOD(ch) && IS_GOOD(victim))
	||	 (IS_EVIL(ch) && IS_EVIL(victim))
	||   (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
		return TRUE;

	return FALSE;
}

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

	if (obj->pIndexData->item_type == ITEM_BOAT) 
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
 switch(obj->pIndexData->item_type)  
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
		case(DAM_MENTAL):	bit = IMM_MENTAL;	break;
		case(DAM_DISEASE):	bit = IMM_DISEASE;	break;
		case(DAM_DROWNING):	bit = IMM_DROWNING;	break;
		case(DAM_LIGHT):	bit = IMM_LIGHT;	break;
		case(DAM_CHARM):	bit = IMM_CHARM;	break;
		case(DAM_SOUND):	bit = IMM_SOUND;	break;
		default:		return def;
	}

	if (IS_SET(ch->imm_flags,bit))
		immune = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
		immune = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,bit))
	{
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

/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
	 int loc,mod,stat;
	 OBJ_DATA *obj;
	 AFFECT_DATA *af;
	 int i;

	 if (IS_NPC(ch))
		return;

	if (ch->pcdata->perm_hit == 0 
	||  ch->pcdata->perm_mana == 0
	||  ch->pcdata->perm_move == 0
	||  ch->pcdata->last_level == 0) {
	/* do a FULL reset */
		for (loc = 0; loc < MAX_WEAR; loc++) {
			obj = get_eq_char(ch,loc);
			if (obj == NULL)
				continue;
			if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
				reset_obj_affects(ch, obj,
						  obj->pIndexData->affected);
			reset_obj_affects(ch, obj, obj->affected);
	        }

		/* now reset the permanent stats */
		ch->pcdata->perm_hit 	= ch->max_hit;
		ch->pcdata->perm_mana 	= ch->max_mana;
		ch->pcdata->perm_move	= ch->max_move;
		ch->pcdata->last_level	= ch->played/3600;
		if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
			if (ch->sex > 0 && ch->sex < 3)
		    	    ch->pcdata->true_sex	= ch->sex;
			else
			    ch->pcdata->true_sex 	= 0;
	}

	/* now restore the character to his/her true condition */
	for (stat = 0; stat < MAX_STATS; stat++)
		ch->mod_stat[stat] = 0;

	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
		ch->pcdata->true_sex = 0; 
	ch->sex		= ch->pcdata->true_sex;
	ch->max_hit 	= ch->pcdata->perm_hit;
	ch->max_mana	= ch->pcdata->perm_mana;
	ch->max_move	= ch->pcdata->perm_move;
   
	for (i = 0; i < 4; i++)
		ch->armor[i]	= 100;

	ch->hitroll		= 0;
	ch->damroll		= 0;
	ch->saving_throw	= 0;

	/* now start adding back the effects */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
	        continue;
		for (i = 0; i < 4; i++)
		    ch->armor[i] -= apply_ac(obj, loc, i);

	    if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		for (af = obj->pIndexData->affected; af != NULL; af = af->next)
	    {
	        mod = af->modifier;
	        switch(af->location)
	        {
			case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod;	break;
			case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod; break;
			case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod; break;
			case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod; break;
			case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod; break;
 		case APPLY_CHA:		ch->mod_stat[STAT_CHA]	+= mod; break;

			case APPLY_MANA:	ch->max_mana		+= mod; break;
			case APPLY_HIT:		ch->max_hit		+= mod; break;
			case APPLY_MOVE:	ch->max_move		+= mod; break;
			case APPLY_AGE:		ch->played += age_to_num(mod); break;
			case APPLY_AC:		
			    for (i = 0; i < 4; i ++)
				ch->armor[i] += mod; 
			    break;
			case APPLY_HITROLL:	ch->hitroll		+= mod; break;
			case APPLY_DAMROLL:	ch->damroll		+= mod; break;
			case APPLY_SIZE:	ch->size		+= mod; break;
			case APPLY_SAVES:		ch->saving_throw += mod; break;
			case APPLY_SAVING_ROD: 		ch->saving_throw += mod; break;
			case APPLY_SAVING_PETRI:	ch->saving_throw += mod; break;
			case APPLY_SAVING_BREATH: 	ch->saving_throw += mod; break;
			case APPLY_SAVING_SPELL:	ch->saving_throw += mod; break;
		    }
	    }
 
	    for (af = obj->affected; af != NULL; af = af->next)
	    {
	        mod = af->modifier;
	        switch(af->location)
	        {
	            case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
	            case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
	            case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
	            case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
	            case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
			case APPLY_CHA:		ch->mod_stat[STAT_CHA]	+= mod; break;
 
	            case APPLY_MANA:        ch->max_mana            += mod; break;
	            case APPLY_HIT:         ch->max_hit             += mod; break;
	            case APPLY_MOVE:        ch->max_move            += mod; break;
			case APPLY_AGE:		ch->played += age_to_num(mod); break;
 
	            case APPLY_AC:
	                for (i = 0; i < 4; i ++)
	                    ch->armor[i] += mod;
	                break;
			case APPLY_HITROLL:     ch->hitroll             += mod; break;
	            case APPLY_DAMROLL:     ch->damroll             += mod; break;
 		case APPLY_SIZE:	ch->size		+= mod; break;
	            case APPLY_SAVES:	ch->saving_throw	+= mod; break;
	            case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
	            case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
	            case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
	            case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
	        }
		}
	}
  
	/* now add back spell effects */
	for (af = ch->affected; af != NULL; af = af->next)
	{
	    mod = af->modifier;
	    switch(af->location)
	    {
	            case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
	            case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
	            case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
	            case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
	            case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
 		case APPLY_CHA:		ch->mod_stat[STAT_CHA]	+= mod; break;

	            case APPLY_MANA:        ch->max_mana            += mod; break;
	            case APPLY_HIT:         ch->max_hit             += mod; break;
	            case APPLY_MOVE:        ch->max_move            += mod; break;
 
	            case APPLY_AC:
	                for (i = 0; i < 4; i ++)
	                    ch->armor[i] += mod;
	                break;
	            case APPLY_HITROLL:     ch->hitroll             += mod; break;
	            case APPLY_DAMROLL:     ch->damroll             += mod; break;
 		case APPLY_SIZE:	ch->size		+= mod; break;
	            case APPLY_SAVES:	ch->saving_throw	+= mod; break;
	            case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
	            case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
	            case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
	            case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
	    } 
	}
	/* make sure sex is RIGHT! */
	if (ch->sex < 0 || ch->sex > 2)
		ch->sex = ch->pcdata->true_sex;
 
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust(CHAR_DATA *ch)
{
	if (ch->desc != NULL && ch->desc->original != NULL)
		ch = ch->desc->original;

	if (ch->trust != 0)
		return ch->trust;

	if (IS_NPC(ch) && ch->level >= LEVEL_HERO)
		return LEVEL_HERO - 1;
	else
		return ch->level;
}

/*
 * Retrieve a character's age.
 */
int get_age(CHAR_DATA *ch)
{
	return 17 + (ch->played + (int) (current_time - ch->logon)) / 72000;
}

int age_to_num(int age)
{
	return  age * 72000;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return 1000;

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET))
		return 0;

	return MAX_WEAR + get_curr_stat(ch,STAT_DEX) - 10 + ch->size;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return 10000000;

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET))
		return 0;

	return str_app[get_curr_stat(ch,STAT_STR)].carry * 10 + ch->level * 25;
}

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
		str = one_argument(str, part);

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;
		for (; ;) { /* start parsing namelist */
			list = one_argument(list, name);
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
		strnzcat(buf, " ", len);
	if (have_spaces)
		strnzcat(buf, "'", len);
	strnzcat(buf, name, len);
	if (have_spaces)
		strnzcat(buf, "'", len);
}

void name_toggle(CHAR_DATA *ch, const char *name,
		 const char *editor_name, const char **nl)
{
	bool found;
	const char *p;
	char buf[MAX_STRING_LENGTH];

	if (!str_cmp(name, "all")) {
		free_string(*nl);
		*nl = str_dup(name);
		char_printf(ch, "%s: name list set to ALL.\n\r", editor_name);
		return;
	}

	if (!str_cmp(name, "none")) {
		free_string(*nl);
		*nl = str_empty;
		char_printf(ch, "%s: name list reset.\n\r", editor_name);
		return;
	}

	if (!str_cmp(*nl, "all")) {
		free_string(*nl);
		*nl = str_empty;
	}

	found = FALSE;
	p = *nl;
	buf[0] = '\0';
	for (;;) {
		char arg[MAX_STRING_LENGTH];

		p = one_argument(p, arg);

		if (arg[0] == '\0')
			break;

		if (!str_cmp(name, arg)) {
			found = TRUE;
			continue;
		}

		cat_name(buf, arg, sizeof(buf));
	}

	if (!found) {
		if (strlen(buf) + strlen(name) + 4 > MAX_STRING_LENGTH) {
			char_printf(ch, "%s: name list too long\n", editor_name);
			return;
		}
		cat_name(buf, name, sizeof(buf));
		char_printf(ch, "%s: name added.\n\r", editor_name);
	}
	else 
		char_printf(ch, "%s: name removed.\n\r", editor_name);

	free_string(*nl);
	*nl = str_dup(buf);
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
	/* okay, move all the old flags into new vectors if we have to */
	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED)) {
	    AFFECT_DATA *paf, *af_new;
	    SET_BIT(obj->extra_flags, ITEM_ENCHANTED);

	    for (paf = obj->pIndexData->affected;
	         paf != NULL; paf = paf->next)
	    {
		    af_new = new_affect();

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
	default:
		bug("Affect_modify: unknown location %d.", paf->location);
		return;

	case APPLY_NONE:						break;
	case APPLY_CLASS:						break;
	case APPLY_LEVEL:						break;
	case APPLY_HEIGHT:						break;
	case APPLY_WEIGHT:						break;
	case APPLY_GOLD:						break;
	case APPLY_EXP:						break;
	case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
	case APPLY_DEX:           ch->mod_stat[STAT_DEX]	+= mod;	break;
	case APPLY_INT:           ch->mod_stat[STAT_INT]	+= mod;	break;
	case APPLY_WIS:           ch->mod_stat[STAT_WIS]	+= mod;	break;
	case APPLY_CON:           ch->mod_stat[STAT_CON]	+= mod;	break;
	case APPLY_CHA:	      ch->mod_stat[STAT_CHA]	+= mod; break;
	case APPLY_AGE:	ch->played += age_to_num(mod);	break;
	case APPLY_MANA:          ch->max_mana		+= mod;	break;
	case APPLY_HIT:           ch->max_hit		+= mod;	break;
	case APPLY_MOVE:          ch->max_move		+= mod;	break;
	case APPLY_AC:
		for (i = 0; i < 4; i ++)
			ch->armor[i] += mod;
		break;
	case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
	case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
	case APPLY_SIZE:		ch->size		+= mod; break;
	case APPLY_SAVES:   ch->saving_throw		+= mod;	break;
	case APPLY_SAVING_ROD:    ch->saving_throw		+= mod;	break;
	case APPLY_SAVING_PETRI:  ch->saving_throw		+= mod;	break;
	case APPLY_SAVING_BREATH: ch->saving_throw		+= mod;	break;
	case APPLY_SAVING_SPELL:  ch->saving_throw		+= mod;	break;
	case APPLY_SPELL_AFFECT:  					break;
	case APPLY_RACE:
		{
			int from;
			int to;

			if (fAdd) {
				from = ORG_RACE(ch);
				to = RACE(ch) = paf->modifier > 0 &&
						paf->modifier < MAX_PC_RACE ?
							paf->modifier : 1;	    
			}
			else {
				from = RACE(ch);
				to = RACE(ch) = ORG_RACE(ch);
			}
				
				
			REMOVE_BIT(ch->affected_by, race_table[from].aff);
			SET_BIT(ch->affected_by, race_table[to].aff);
			affect_check(ch, TO_AFFECTS, race_table[from].aff);

			REMOVE_BIT(ch->imm_flags, race_table[from].imm);
			SET_BIT(ch->imm_flags, race_table[to].imm);
			affect_check(ch, TO_IMMUNE, race_table[from].imm);

			REMOVE_BIT(ch->res_flags, race_table[from].res);
			SET_BIT(ch->res_flags, race_table[to].res);
			affect_check(ch, TO_RESIST, race_table[from].res);

			REMOVE_BIT(ch->vuln_flags, race_table[from].vuln);
			SET_BIT(ch->vuln_flags, race_table[to].vuln);
			affect_check(ch, TO_VULN, race_table[from].vuln);

			ch->form = race_table[to].form;
			ch->parts = race_table[to].parts;
		}
		break;
	}

	/*
	 * Check for weapon wielding.
	 * Guard against recursion (for weapons with affects).
	 */
	if (!IS_NPC(ch) && (wield = get_eq_char(ch, WEAR_WIELD)) != NULL
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
		       int where, flag_t vector)
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
void affect_check(CHAR_DATA *ch, int where, flag_t vector)
{
	OBJ_DATA *obj;

	if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
		return;

	affect_check_list(ch, ch->affected, where, vector);
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == -1 || obj->wear_loc == WEAR_STUCK_IN)
			continue;
		affect_check_list(ch, obj->affected, where, vector);

		if (IS_SET(obj->extra_flags, ITEM_ENCHANTED))
			continue;

		affect_check_list(ch, obj->pIndexData->affected, where, vector);
	}
}

/*
 * Give an affect to a char.
 */
void affect_to_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new;

	paf_new = new_affect();

	*paf_new		= *paf;
	paf_new->next	= ch->affected;
	ch->affected	= paf_new;

	affect_modify(ch, paf_new, TRUE);
	return;
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new;

	paf_new = new_affect();
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
			if (obj->pIndexData->item_type == ITEM_WEAPON)
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

	free_affect(paf);
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
			if (obj->pIndexData->item_type == ITEM_WEAPON)
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

	free_affect(paf);

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
void affect_bit_strip(CHAR_DATA *ch, int where, flag_t bits)
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

bool is_bit_affected(CHAR_DATA *ch, int where, flag_t bits)
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

		for (paf = obj->pIndexData->affected; paf; paf = paf->next)
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
	ROOM_INDEX_DATA *prev_room = ch->in_room;

	if (ch->in_room == NULL)
	{
		bug("Char_from_room: NULL.", 0);
		return;
	}

	if (!IS_NPC(ch))
		--ch->in_room->area->nplayer;

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
	&&   obj->pIndexData->item_type == ITEM_LIGHT
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

	if (prev_room && prev_room->affected_by)
		  raffect_back_char(prev_room, ch);

	return;
}

/*
 * Move a char into a room.
 */
void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
{
	OBJ_DATA *obj;

	if (pRoomIndex == NULL)
	{
		ROOM_INDEX_DATA *room;

		bug("Char_to_room: NULL.", 0);
		
		if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
		    char_to_room(ch,room);
		
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
	&&   obj->pIndexData->item_type == ITEM_LIGHT
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
	        	char_puts("You feel hot and feverish.\n\r",vch);
	        	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
	        	affect_join(vch,&plague);
	        }
	    }
		break;
	}

	if (ch->in_room->affected_by) {
		 if (IS_IMMORTAL(ch))
			do_raffects(ch,str_empty);
		 else
			raffect_to_char(ch->in_room, ch);
	}
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

/* XXX */
	if (!IS_NPC(ch)) {
		int vnum = obj->pIndexData->vnum;
		const char *p = mlstr_mval(obj->short_descr);

		if (p && strstr(p, ch->name)
		&&  get_wear_level(ch, obj) < obj->level
		&&  IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST)) {
			ch->pcdata->questpoints += 1000;
			qtrouble_set(ch, vnum, 4);
		}
	}
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac(OBJ_DATA *obj, int iWear, int type)
{
	if (obj->pIndexData->item_type != ITEM_ARMOR)
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

/*
 * Equip a char with an obj.
 */
void equip_char(CHAR_DATA *ch, OBJ_DATA *obj, int iWear)
{
	AFFECT_DATA *paf;
	int i;

	if (iWear == WEAR_STUCK_IN) {
		obj->wear_loc = iWear;
		return;
	}

	if (get_eq_char(ch, iWear) != NULL) {
		bug("Equip_char: already equipped (%d).", iWear);
		return;
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
		return;
	}

	for (i = 0; i < 4; i++)
		ch->armor[i]      	-= apply_ac(obj, iWear,i);
	obj->wear_loc	 = iWear;

	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		    if (paf->location != APPLY_SPELL_AFFECT)
		        affect_modify(ch, paf, TRUE);
	for (paf = obj->affected; paf != NULL; paf = paf->next)
		if (paf->location == APPLY_SPELL_AFFECT)
			affect_to_char (ch, paf);
		else
			affect_modify(ch, paf, TRUE);

	if (obj->pIndexData->item_type == ITEM_LIGHT
	&&  obj->value[2] != 0
	&&  ch->in_room != NULL)
		++ch->in_room->light;

	oprog_call(OPROG_WEAR, obj, ch, NULL);
}

void strip_obj_affects(CHAR_DATA *ch, OBJ_DATA *obj, AFFECT_DATA *paf)
{
	AFFECT_DATA *lpaf_next = NULL;
	AFFECT_DATA *lpaf = NULL;

	for (; paf != NULL; paf = paf->next) {
		if (paf->location == APPLY_SPELL_AFFECT) {
		        for (lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next) {
				lpaf_next = lpaf->next;
				if ((lpaf->type == paf->type)
				&&  (lpaf->level == paf->level)
				&&  (lpaf->location == APPLY_SPELL_AFFECT)) {
					affect_remove(ch, lpaf);
					lpaf_next = NULL;
				}
		        }
		}
		else {
			affect_modify(ch, paf, FALSE);
			affect_check(ch,paf->where,paf->bitvector);
		}
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
		strip_obj_affects(ch, obj, obj->pIndexData->affected);
	strip_obj_affects(ch, obj, obj->affected);

	if (obj->pIndexData->item_type == ITEM_LIGHT
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
	for (obj = list; obj != NULL; obj = obj->next_content)
	{
		if (obj->pIndexData == pObjIndex)
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
		bug("obj_from_room: NULL.", 0);
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

	if (IS_WATER(pRoomIndex))
		if (may_float(obj))
			obj->water_float = -1;
		else
			obj->water_float = floating_time(obj);
}

/*
 * Move an object into an object.
 */
void obj_to_obj(OBJ_DATA *obj, OBJ_DATA *obj_to)
{
	if (obj == obj_to) {
		log_printf("obj_to_obj: obj == obj_to (vnum %d)",
			   obj->pIndexData->vnum);
		return;
	}

	obj->next_content	= obj_to->contains;
	obj_to->contains	= obj;
	obj->in_obj		= obj_to;
	obj->in_room		= NULL;
	obj->carried_by		= NULL;
	if (obj_is_pit(obj_to))
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
 * Extract an object consider limit
 */
void extract_obj(OBJ_DATA *obj)
{
	extract_obj_1(obj,TRUE);
}

/* 
 * Extract an object consider limit
 */
void extract_obj_nocount(OBJ_DATA *obj)
{
	extract_obj_1(obj,FALSE);
}

/*
 * Extract an obj from the world.
 */
void extract_obj_1(OBJ_DATA *obj, bool count)
{
	OBJ_DATA *obj_content;
	OBJ_DATA *obj_next;

	if (obj->extracted)  /* if the object has already been extracted once */
	  {
	    log_printf("extract_obj_1: %s, vnum %d already extracted",
			obj->name, obj->pIndexData->vnum);
	    return; /* if it's already been extracted, something bad is going on */
	  }
	else
	  obj->extracted = TRUE;  /* if it hasn't been extracted yet, now
	                           * it's being extracted. */
 
	if (obj->in_room != NULL)
		obj_from_room(obj);
	else if (obj->carried_by != NULL)
		obj_from_char(obj);
	else if (obj->in_obj != NULL)
		obj_from_obj(obj);

	for (obj_content = obj->contains; obj_content; obj_content = obj_next) {
		obj_next = obj_content->next_content;
		extract_obj_1(obj_content, count);
	}

	if (obj->pIndexData->vnum == OBJ_VNUM_MAGIC_JAR) {
		 CHAR_DATA *wch;
		 
		 for (wch = char_list; wch != NULL ; wch = wch->next) {
		 	if (IS_NPC(wch)) continue;
		 	if (is_name(obj->name, wch->name)) {
				REMOVE_BIT(wch->act,PLR_NOEXP);
				char_puts("Now you catch your spirit.\n\r", wch);
				break;
			}
		}
	}

	if (object_list == obj)
		object_list = obj->next;
	else {
		OBJ_DATA *prev;

		for (prev = object_list; prev != NULL; prev = prev->next)
		{
		    if (prev->next == obj)
		    {
			prev->next = obj->next;
			break;
		    }
		}

		if (prev == NULL)
		{
		    bug("Extract_obj: obj %d not found.", obj->pIndexData->vnum);
		    return;
		}
	}
	if (count)
		--obj->pIndexData->count;
	free_obj(obj);
}


void extract_char(CHAR_DATA *ch, bool fPull)
{
	extract_char_org(ch, fPull, TRUE);
	return;
}


void extract_char_nocount(CHAR_DATA *ch, bool fPull)
{
	extract_char_org(ch, fPull, FALSE);
	return;
}


/*
 * Extract a char from the world.
 */
void extract_char_org(CHAR_DATA *ch, bool fPull, bool Count)
{
	CHAR_DATA *wch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	OBJ_DATA *wield;
	int i;    
	char buf[MAX_STRING_LENGTH];

	if (fPull) /* only for total extractions should it check */
	{
	if (ch->extracted)  /* if the char has already been extracted once */
	  {
	    sprintf(buf, "Warning! Extraction of %s.", ch->name);
	    bug(buf, 0);
	    return; /* if it's already been extracted, something bad is going on */
	  }
	else
	  ch->extracted = TRUE;  /* if it hasn't been extracted yet, now
	                           * it's being extracted. */
	}


	if (ch->in_room == NULL)
	{
		bug("Extract_char: NULL.", 0);
		return;
	}
	
	nuke_pets(ch);
	ch->pet = NULL; /* just in case */

	if (fPull)

		die_follower(ch);
	
	stop_fighting(ch, TRUE);

	if ((wield = get_eq_char(ch, WEAR_WIELD)) != NULL)
		      unequip_char(ch, wield); 
	
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (Count)
  	  extract_obj(obj);
		else
		  extract_obj_nocount(obj);
	}
	
	char_from_room(ch);

	if (!fPull)
	{
	    if (IS_GOOD(ch))
	      i = 0;
	    if (IS_EVIL(ch))
	      i = 2;
	    else
	      i = 1;
	    char_to_room(ch, get_room_index(hometown_table[ch->hometown].altar[i]));
		return;
	}

	if (IS_NPC(ch))
		--ch->pIndexData->count;

	if (ch->desc != NULL && ch->desc->original != NULL) {
		do_return(ch, str_empty);
		ch->desc = NULL;
	}

	for (wch = char_list; wch != NULL; wch = wch->next) {
		if (wch->reply == ch)
			wch->reply = NULL;
		if (ch->mprog_target == wch)
			wch->mprog_target = NULL;
	}


	if (ch == char_list)
		char_list = ch->next;
	else {
		CHAR_DATA *prev;

		for (prev = char_list; prev != NULL; prev = prev->next)
		{
		    if (prev->next == ch)
		    {
			prev->next = ch->next;
			break;
		    }
		}

		if (prev == NULL)
		{
		    bug("Extract_char: char not found.", 0);
		    return;
		}
	}

	if (ch->desc != NULL)
		ch->desc->character = NULL;
	free_char(ch);
	return;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *rch;
	int number;
	int count;
	bool ugly = FALSE;

	if (IS_NULLSTR(argument))
		return NULL;

	number = number_argument(argument, arg);
	count  = 0;

	if (!str_cmp(arg, "self"))
		return ch;

	if (!str_cmp(arg, "ugly"))
		ugly = TRUE;

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (!can_see(ch, rch))
			continue;

		if (ugly && (count + 1) == number
		&&  is_affected(rch, gsn_vampire))
			return rch;

		if (!is_name(arg, rch->name))
			continue;

		if (++count == number)
			return rch;
	}

	return NULL;
}

/*
 * Find a char in the room.
 * Chronos uses in act_move.c
 */
CHAR_DATA *get_char_room2(CHAR_DATA *ch, ROOM_INDEX_DATA *room, const char *argument, int *number)
{
	CHAR_DATA *rch;
	int count;
	bool ugly = FALSE;

	if (IS_NULLSTR(argument))
		return NULL;

	if (room == NULL) return NULL;
	count  = 0;

	if (!str_cmp(argument, "ugly"))
		ugly = TRUE;

	for (rch = room->people; rch != NULL; rch = rch->next_in_room) {
		if (!can_see(ch, rch))
		    continue;

		if (ugly && (count + 1) == *number
		&&  is_affected(rch, gsn_vampire))
		   return rch;

		if (!is_name(argument, rch->name))
			continue;

		if (++count == *number)
		    return rch;
	}

	number -= count;
	return NULL;
}


/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *wch;
	int number;
	int count;

	if (IS_NULLSTR(argument))
		return NULL;

	if ((wch = get_char_room(ch, argument)) != NULL)
		return wch;

	number = number_argument(argument, arg);
	count  = 0;
	for (wch = char_list; wch != NULL ; wch = wch->next)
	{
		if (wch->in_room == NULL || !can_see(ch, wch) 
		||   !is_name(arg, wch->name))
		    continue;

		if (++count == number)
		    return wch;
	}

	return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type(OBJ_INDEX_DATA *pObjIndex)
{
	OBJ_DATA *obj;

	for (obj = object_list; obj != NULL; obj = obj->next)
	{
		if (obj->pIndexData == pObjIndex)
		    return obj;
	}

	return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list(CHAR_DATA *ch, const char *argument, OBJ_DATA *list)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

	if (IS_NULLSTR(argument))
		return NULL;

	number = number_argument(argument, arg);
	count  = 0;
	for (obj = list; obj; obj = obj->next_content)
		if (can_see_obj(ch, obj) && is_name(arg, obj->name))
			if (++count == number)
				return obj;

	return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

	if (IS_NULLSTR(argument))
		return NULL;

	number = number_argument(argument, arg);
	count  = 0;
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
		if (obj->wear_loc == WEAR_NONE
		&&   (can_see_obj(ch, obj)) 
		&&   is_name(arg, obj->name))
		{
		    if (++count == number)
			return obj;
		}
	}

	return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

	if (IS_NULLSTR(argument))
		return NULL;

	number = number_argument(argument, arg);
	count  = 0;
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE
		&&   can_see_obj(ch, obj)
		&&   is_name(arg, obj->name))
		{
		    if (++count == number)
			return obj;
		}
	}

	return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;

	if (IS_NULLSTR(argument))
		return NULL;

	obj = get_obj_list(ch, argument, ch->in_room->contents);
	if (obj)
		return obj;

	if ((obj = get_obj_carry(ch, argument)))
		return obj;

	if ((obj = get_obj_wear(ch, argument)))
		return obj;

	return NULL;
}

OBJ_DATA *get_obj_room(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *vch;

	if (IS_NULLSTR(argument))
		return NULL;

	if ((obj = get_obj_here(ch, argument)))
		return obj;

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if ((obj = get_obj_carry(vch, argument)))
			return obj;

		if ((obj = get_obj_wear(vch, argument)))
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
	int number;
	int count;

	if (IS_NULLSTR(argument))
		return NULL;

	if ((obj = get_obj_here(ch, argument)) != NULL)
		return obj;

	number = number_argument(argument, arg);
	count  = 0;
	for (obj = object_list; obj != NULL; obj = obj->next)
	{
		if (can_see_obj(ch, obj) && is_name(arg, obj->name))
		{
		    if (++count == number)
			return obj;
		}

	}

	return NULL;
}

/* deduct cost from a character */

void deduct_cost(CHAR_DATA *ch, int cost)
{
	int silver = 0, gold = 0;

	silver = UMIN(ch->silver,cost); 

	if (silver < cost)
	{
		gold = ((cost - silver + 99) / 100);
		silver = cost - 100 * gold;
	}

	ch->gold -= gold;
	ch->silver -= silver;

	if (ch->gold < 0)
	{
		bug("deduct costs: gold %d < 0",ch->gold);
		ch->gold = 0;
	}
	if (ch->silver < 0)
	{
		bug("deduct costs: silver %d < 0",ch->silver);
		ch->silver = 0;
	}
}   
/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money(int gold, int silver)
{
	OBJ_DATA *obj;

	if (gold < 0 || silver < 0 || (gold == 0 && silver == 0))
	{
		bug("Create_money: zero or negative money.",UMIN(gold,silver));
		gold = UMAX(1,gold);
		silver = UMAX(1,silver);
	}

	if (gold == 0 && silver == 1)
	{
		obj = create_obj(get_obj_index(OBJ_VNUM_SILVER_ONE), 0);
	}
	else if (gold == 1 && silver == 0)
	{
		obj = create_obj(get_obj_index(OBJ_VNUM_GOLD_ONE), 0);
	}
	else if (silver == 0)
	{
	    obj = create_obj(get_obj_index(OBJ_VNUM_GOLD_SOME), 0);
		obj->short_descr = mlstr_printf(obj->pIndexData->short_descr, gold);
	    obj->value[1]           = gold;
	    obj->cost               = gold;
		obj->weight		= gold/5;
	}
	else if (gold == 0)
	{
	    obj = create_obj(get_obj_index(OBJ_VNUM_SILVER_SOME), 0);
		obj->short_descr = mlstr_printf(obj->pIndexData->short_descr, silver);
	    obj->value[0]           = silver;
	    obj->cost               = silver;
		obj->weight		= silver/20;
	}
 
	else
	{
		obj = create_obj(get_obj_index(OBJ_VNUM_COINS), 0);
		obj->short_descr = mlstr_printf(obj->pIndexData->short_descr, silver, gold);
		obj->value[0]		= silver;
		obj->value[1]		= gold;
		obj->cost		= 100 * gold + silver;
		obj->weight		= gold / 5 + silver / 20;
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
	if (obj->pIndexData->item_type == ITEM_CONTAINER || obj->pIndexData->item_type == ITEM_MONEY
	||  obj->pIndexData->item_type == ITEM_GEM || obj->pIndexData->item_type == ITEM_JEWELRY)
	    number = 0;
*/
	if (obj->pIndexData->item_type == ITEM_MONEY)
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

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
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


bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	if (room->owner == NULL || room->owner[0] == '\0')
		return FALSE;

	return is_name(ch->name,room->owner);
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
	&&  get_trust(ch) < IMPLEMENTOR)
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
	&&  get_trust(ch) < GOD)
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
	&&  get_trust(ch) < HERO)
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
	&&  ch->level > 5 && !IS_IMMORTAL(ch))
		return FALSE;

	return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see(CHAR_DATA *ch, CHAR_DATA *victim)
{
/* RT changed so that WIZ_INVIS has levels */
	if (ch == victim)
		return TRUE;

	if (ch == NULL || victim == NULL)
		dump_to_scr(">>>>>>>> CAN_ SEE ERROR <<<<<<<<<<<\n\r");
	
	if (get_trust(ch) < victim->invis_level)
		return FALSE;


	if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
		return FALSE;

	if ((!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT)) 
	||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
		return TRUE;

	if (IS_AFFECTED(ch, AFF_BLIND))
		return FALSE;

	if (ch->in_room == NULL)
	    return FALSE;

	if (room_is_dark(ch) && !IS_AFFECTED(ch, AFF_INFRARED))
		return FALSE;

	if (IS_AFFECTED(victim, AFF_INVISIBLE)
	&&   !IS_AFFECTED(ch, AFF_DETECT_INVIS))
		return FALSE;

	if (IS_AFFECTED(victim, AFF_IMP_INVIS)
	&&   !IS_AFFECTED(ch, AFF_DETECT_IMP_INVIS))
		return FALSE;

/*
	if (IS_AFFECTED(victim, AFF_SNEAK)
	&&  !(IS_NPC(ch) && ch->pIndexData->pShop)
	&&  !IS_AFFECTED(ch,AFF_DETECT_HIDDEN)
	&&  !IS_NPC(victim) && !victim->fighting) {
		int chance;
		chance = get_skill(victim, gsn_sneak);
		chance += get_curr_stat(victim, STAT_DEX) * 3/2;
 		chance -= get_curr_stat(ch, STAT_INT) * 2;
		chance += (victim->level - ch->level) * 3/2;

		if (number_percent() < chance)
		    return FALSE;
	}
*/

	if (IS_AFFECTED(victim,AFF_CAMOUFLAGE) &&
	    !IS_AFFECTED(ch,AFF_ACUTE_VISION))
	  return FALSE;

	if (IS_AFFECTED(victim, AFF_HIDE)
	&&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
	&&   victim->fighting == NULL)
		return FALSE;

	if (IS_AFFECTED(victim, AFF_FADE)
	&&   !IS_AFFECTED(ch, AFF_DETECT_FADE)
	&&   victim->fighting == NULL)
		return FALSE;

	return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
		return TRUE;

	if (IS_SET(obj->extra_flags, ITEM_VIS_DEATH))
		return FALSE;

	if (IS_AFFECTED(ch, AFF_BLIND) && obj->pIndexData->item_type != ITEM_POTION)
		return FALSE;

	if (obj->pIndexData->item_type == ITEM_LIGHT && obj->value[2] != 0)
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

bool isn_dark_safe(CHAR_DATA *ch)
{
	CHAR_DATA *rch;
	OBJ_DATA *light;
	int light_exist;

	if (!is_affected(ch, gsn_vampire))
		return FALSE;

	if (IS_SET(ch->in_room->room_flags, ROOM_DARK))
		return FALSE;

	if (weather_info.sunlight == SUN_LIGHT
	||  weather_info.sunlight == SUN_RISE)
		return TRUE;

	light_exist = FALSE;
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if ((light = get_eq_char(rch, WEAR_LIGHT))
		&&  IS_OBJ_STAT(light, ITEM_MAGIC)) {
			light_exist = TRUE;
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
	char_puts("You are already controlling as many charmed mobs as you can!\n\r",ch);
	return count;
   }
  return 0;
}

/*
 * add_mind - remember 'str' in mind buffer of 'ch'
 *	      remember the place to return in mind buffer
 */
void add_mind(CHAR_DATA *ch, const char *str)
{
	if (!IS_NPC(ch) || ch->in_room == NULL)
		return;

	if (ch->in_mind == NULL) {
		/* remember a place to return */
		char buf[MAX_STRING_LENGTH];
		snprintf(buf, sizeof(buf), "%d", ch->in_room->vnum);
		ch->in_mind = str_dup(buf);
	}

	if (!is_name(str, ch->in_mind)) {
		const char *p = ch->in_mind;
		ch->in_mind = str_add(ch->in_mind, " ", str, NULL);
		free_string(p);
	}
}

/*
 * remove_mind - remove 'str' from mind buffer of 'ch'
 *		 if it was the last revenge - return home
 */
void remove_mind(CHAR_DATA *ch, const char *str)
{
	char buf[MAX_STRING_LENGTH];
	char buff[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	const char *mind = ch->in_mind;

	if (!IS_NPC(ch) || ch->in_room == NULL 
	||  mind == NULL || !is_name(str, mind)) return;

	buf[0] = '\0';
	do { 
		mind = one_argument(mind, arg);
		if (!is_name(str,arg))  {
			if (buf[0] == '\0')
				strcpy(buff, arg);
			else
				snprintf(buff, sizeof(buff), "%s %s", buf, arg);
			strcpy(buf,buff);
		}
	} while (mind[0] != '\0');
 
	free_string(ch->in_mind);
	if (is_number(buf)) {
		do_say(ch, "At last, I took my revenge!"); 
		back_home(ch);
		ch->in_mind = NULL;
	}
	else
		ch->in_mind = str_dup(buf);
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

void back_home(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *location;
	char arg[MAX_INPUT_LENGTH];

	if (!IS_NPC(ch) || ch->in_mind == NULL)
		return;

	one_argument(ch->in_mind, arg);
	if ((location = find_location(ch, arg)) == NULL) {
		log("back_home: reset place not found");
		return;
	}

	if (ch->fighting == NULL && location != ch->in_room) {
		act("$n prays for transportation.",ch,NULL,NULL,TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, location);
		act("$n appears in the room.",ch,NULL,NULL,TO_ROOM);
	}
}


CHAR_DATA * find_char(CHAR_DATA *ch, const char *argument,int door, int range) 
{
 EXIT_DATA *pExit,*bExit;
 ROOM_INDEX_DATA *dest_room,*back_room;
 CHAR_DATA *target;
 int number = 0,opdoor;
 char arg[MAX_INPUT_LENGTH];

 number = number_argument(argument,arg);
 dest_room = ch->in_room;
 if ((target = get_char_room2(ch,dest_room,arg,&number)) != NULL)
		return target;

 if ((opdoor = opposite_door(door)) == -1)
  {
   bug("In find_char wrong door: %d",door);
   char_puts("You don't see that there.\n\r",ch);
   return NULL;
 }
 while (range > 0)
 {
  range--;
  /* find target room */
  back_room = dest_room;
  if ((pExit = dest_room->exit[door]) == NULL
	  || (dest_room = pExit->u1.to_room) == NULL
	  || IS_SET(pExit->exit_info,EX_CLOSED))
   break;
  if ((bExit = dest_room->exit[opdoor]) == NULL
	  || bExit->u1.to_room != back_room)
   {
	char_puts("The path you choose prevents your power to pass.\n\r",ch);
	return NULL;
   }
  if ((target = get_char_room2(ch,dest_room,arg,&number)) != NULL) 
		return target;
 }

 char_puts("You don't see that there.\n\r",ch);
 return NULL;
}
		
int check_exit(char *arg)
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
 * Find a char for spell usage.
 */
CHAR_DATA *get_char_spell(CHAR_DATA *ch, const char *argument, int *door, int range)
{
 char buf[MAX_INPUT_LENGTH];
 int i;

 for(i=0;argument[i] != '\0' && argument[i] != '.';i++)
   buf[i] = argument[i];
 buf[i] = '\0';

 if (i == 0 || (*door = check_exit(buf)) == -1)
		return get_char_room(ch,argument);

 return find_char(ch,(argument+i+1),*door,range);   
}

void path_to_track(CHAR_DATA *ch, CHAR_DATA *victim, int door)
{
  ROOM_INDEX_DATA *temp;
  EXIT_DATA *pExit;
  int opdoor;
  int range = 0;

	SET_FIGHT_TIME(ch);
  	if (!IS_NPC(victim))
		SET_FIGHT_TIME(victim);

  if (IS_NPC(victim) && victim->position != POS_DEAD)
   {
	victim->last_fought = ch;

	if ((opdoor = opposite_door(door)) == -1)
		{
		 bug("In path_to_track wrong door: %d",door);
		 return;
		}
	temp = ch->in_room;
	while (1)
	 {
	  range++;
	  if (victim->in_room == temp) break;
	  if ((pExit = temp->exit[ door ]) == NULL
		  || (temp = pExit->u1.to_room) == NULL)
	   {
		bug("In path_to_track: couldn't calculate range %d",range);
		return;
	   }
	  if (range > 100)
	   {
		bug("In path_to_track: range exceeded 100",0);
		return;
	   }
	 }

	temp = victim->in_room;
	while (--range > 0)
	   {
	    room_record(ch->name,temp, opdoor);
	    if ((pExit = temp->exit[opdoor]) == NULL
		    || (temp = pExit->u1.to_room) == NULL)
		{
		 log_printf("[*****] Path to track: Range: %d Room: %d opdoor:%d",
			range,temp->vnum,opdoor); 
		 return;
		}
	   }
	do_track(victim,str_empty);
  }
}

int pk_range(int level)
{
	return UMAX(4, level/10 + 2);
}

bool in_PK(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (IS_NPC(ch) || IS_NPC(victim))
		return TRUE;

	if (victim->level < PK_MIN_LEVEL || ch->level < PK_MIN_LEVEL)
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
	||  IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||  IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	||  room_is_private(victim->in_room)
	||  IS_SET(ch->in_room->room_flags, ROOM_NORECALL)
	||  IS_SET(victim->in_room->room_flags, ROOM_NORECALL))
		return FALSE;

	if (IS_NPC(victim)) {
		if (IS_SET(victim->imm_flags, IMM_SUMMON))
			return FALSE;
	}
	else {
		if ((!in_PK(ch, victim) && IS_SET(victim->act, PLR_NOSUMMON))
		||  victim->level >= LEVEL_HERO		/* not trust (!) */
		||  (ch->in_room->area != victim->in_room->area && IS_SET(victim->act, PLR_NOSUMMON))
		||  guild_check(ch, victim->in_room) < 0)
			return FALSE;
	}

	return TRUE;
}

const char *PERS(CHAR_DATA *ch, CHAR_DATA *looker)
{
	if (can_see(looker, ch)) {
		if (IS_NPC(ch))
			return mlstr_cval(ch->short_descr, looker);
		else if (is_affected(ch, gsn_vampire) && !IS_IMMORTAL(looker))
			return "an ugly creature";
		return ch->name;
	}

	if (IS_IMMORTAL(ch))
		return "an immortal";

	return "someone";
}

bool obj_is_pit(OBJ_DATA *obj)
{
	HOMETOWN_DATA *h;

	for (h = hometown_table; h->name != NULL; h++) {
		int i;

		for (i = 0; i < 3; i++)
			if (obj->pIndexData->vnum == h->pit[i])
				return TRUE;
	}

	return FALSE;
}

void format_obj(BUFFER *output, OBJ_DATA *obj)
{
	buf_printf(output,
		"Object '%s' is type %s, extra flags %s.\n\r"
		"Weight is %d, value is %d, level is %d.\n\r",
		obj->name,
		flag_string(item_types, obj->pIndexData->item_type),
		flag_string(extra_flags, obj->extra_flags),
		obj->weight / 10,
		obj->cost,
		obj->level);

	if (obj->pIndexData->limit != -1)
		buf_printf(output,
			   "This equipment has been LIMITED by number %d \n\r",
			   obj->pIndexData->limit);

	switch (obj->pIndexData->item_type) {
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

		buf_add(output, ".\n\r");
		break;

	case ITEM_WAND: 
	case ITEM_STAFF: 
		buf_printf(output, "Has %d charges of level %d",
			   obj->value[2], obj->value[0]);
	  
		if (obj->value[3] >= 0)
			buf_printf(output, " '%s'", skill_name(obj->value[3]));

		buf_add(output, ".\n\r");
		break;

	case ITEM_DRINK_CON:
		buf_printf(output, "It holds %s-colored %s.\n\r",
			   liq_table[obj->value[2]].liq_color,
	        	   liq_table[obj->value[2]].liq_name);
		break;

	case ITEM_CONTAINER:
		buf_printf(output,
			   "Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
			   obj->value[0], obj->value[3],
			   flag_string(cont_flags, obj->value[1]));
		if (obj->value[4] != 100)
			buf_printf(output, "Weight multiplier: %d%%\n\r",
				   obj->value[4]);
		break;
			
	case ITEM_WEAPON:
		buf_printf(output, "Weapon type is %s.\n\r",
			   flag_string(weapon_class, obj->value[0]));
		buf_printf(output, "Damage is %dd%d (average %d).\n\r",
			   obj->value[1],obj->value[2],
			   (1 + obj->value[2]) * obj->value[1] / 2);
		if (obj->value[4])
	        	buf_printf(output, "Weapons flags: %s\n\r",
				   flag_string(weapon_type2, obj->value[4]));
		break;

	case ITEM_ARMOR:
		buf_printf(output, "Armor class is %d pierce, "
				   "%d bash, %d slash, and %d vs. magic.\n\r", 
			   obj->value[0], obj->value[1],
			   obj->value[2], obj->value[3]);
		break;
	}
}

void format_obj_affects(BUFFER *output, AFFECT_DATA *paf, bool duration)
{
	for (; paf; paf = paf->next) {
		if (paf->location != APPLY_NONE && paf->modifier != 0) { 
			buf_printf(output, "Affects %s by %d",
				   flag_string(apply_flags, paf->location),
				   paf->modifier);
			if (duration && paf->duration > -1)
				buf_printf(output, ", %d hours", paf->duration);
			buf_add(output, ".\n\r");

			if (paf->bitvector) {
				WHERE_DATA *wd;
				buf_add(output, "Adds ");
				if ((wd = where_lookup(paf->where)))
					buf_printf(output, wd->format,
						   flag_string(wd->table,
							paf->bitvector));
				else
					buf_add(output, "???");
				buf_add(output, ".\n\r");
			}
		}
	}
}

int get_wear_level(CHAR_DATA *ch, OBJ_DATA *obj)
{
	int wear_level = ch->level;
	CLASS_DATA *cl;

	if ((cl = class_lookup(ch->class)) == NULL)
		return wear_level;

	switch (obj->pIndexData->item_type) {
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_WAND:
	case ITEM_STAFF:
	case ITEM_SCROLL:
		return wear_level;
	}

	if (!IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST)
	&&  (obj->pIndexData->limit < 0 || obj->pIndexData->limit > 1))
		wear_level += pk_range(wear_level);

	if (IS_SET(cl->flags, CLASS_MAGIC)) {
		if (obj->pIndexData->item_type == ITEM_ARMOR)
			wear_level += 3;
	}
	else if (obj->pIndexData->item_type == ITEM_WEAPON)
		wear_level += 3;
	return wear_level;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell(int level, CHAR_DATA *victim, int dam_type)
{
	CLASS_DATA *vcl;
	int save;

	save = 40 + (victim->level - level) * 4 - 
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
	&&  IS_SET(vcl->flags, CLASS_MAGIC))
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
			SKILL_DATA *sk;

	                affect_strip(victim,sn);
			if ((sk = skill_lookup(sn))
			&&  !IS_NULLSTR(sk->msg_off))
				char_printf(victim, "%s\n\r", sk->msg_off);
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
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
		return TRUE;

	if (IS_AFFECTED(ch, AFF_BLIND))
		return FALSE;

	return TRUE;
}

bool check_blind(CHAR_DATA *ch)
{
	bool can_see = check_blind_raw(ch);

	if (!can_see)
		char_puts("You can't see a thing!\n\r", ch);

	return can_see;
}

