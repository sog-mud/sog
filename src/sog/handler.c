/*
 * $Id: handler.c,v 1.33 1998-07-08 09:57:13 fjoe Exp $
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
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "db.h"
#include "comm.h"
#include "hometown.h"
#include "act_comm.h"
#include "log.h"
#include "act_move.h"
#include "lookup.h"
#include "obj_prog.h"
#include "raffects.h"
#include "interp.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_return	);
DECLARE_DO_FUN(do_wake		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_track		);



/*
 * Local functions.
 */
void	affect_modify	args((CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd));
int	age_to_num	args((int age));
ROOM_INDEX_DATA *	find_location	args((CHAR_DATA *ch, char *arg));

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

void room_record(char *name,ROOM_INDEX_DATA *room,int door)
{
  ROOM_HISTORY_DATA *rec;
  int i;

  for (i=0,rec = room->history;i < 5 && rec != NULL;
	   i++,rec = rec->next);

  if (i < 5) {
	rec = alloc_perm(sizeof(ROOM_HISTORY_DATA)); 
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


bool clan_ok(CHAR_DATA *ch, int sn) 
{
  int i;

  if (IS_NPC(ch) || skill_table[sn].clan == CLAN_NONE ||
	  clan_table[ch->clan].obj_ptr == NULL ||
	  clan_table[ch->clan].obj_ptr->in_room == NULL ||
	  clan_table[ch->clan].obj_ptr->in_room->vnum ==
	    clan_table[ch->clan].room_vnum)
	return TRUE;

  for (i=1;i < MAX_CLAN; i++)
	if (clan_table[ch->clan].obj_ptr->in_room->vnum ==
	      clan_table[i].room_vnum) {
	  send_to_char("You cannot find the Cabal Power within you.\n\r",ch);
	  return FALSE;
	}

  return TRUE;
}

int weapon_type (const char *name)
{
	int type;
 
	for (type = 0; weapon_table[type].name != NULL; type++)
	{
	    if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	    &&  !str_prefix(name,weapon_table[type].name))
	        return weapon_table[type].type;
	}
 
	return WEAPON_EXOTIC;
}


char *item_name(int item_type)
{
	int type;

	for (type = 0; item_table[type].name != NULL; type++)
		if (item_type == item_table[type].type)
			return item_table[type].name;
	return "none";
}


char *weapon_name(int weapon_type)
{
	int type;
 
	for (type = 0; weapon_table[type].name != NULL; type++)
		if (weapon_type == weapon_table[type].type)
	        	return weapon_table[type].name;
	return "exotic";
}


/*
 * Check the material
 */
bool check_material(OBJ_DATA *obj, char *material)
{
	if (strstr(obj->material, material) != NULL)
		return TRUE;
	else
	    return FALSE;

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

	if (obj->item_type == ITEM_BOAT) 
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
 switch(obj->item_type)  
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

/* checks mob format */
bool is_old_mob(CHAR_DATA *ch)
{
	if (ch->pIndexData == NULL)
		return FALSE;
	else if (ch->pIndexData->new_format)
		return FALSE;
	return TRUE;
}
 
/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
	int skill;

	if (sn == -1) /* shorthand for level based skills */
	{
		skill = ch->level * 5 / 2;
	}

	else if (sn < -1 || sn > MAX_SKILL)
	{
		bug("Bad sn %d in get_skill.",sn);
		skill = 0;
	}

	else if (!IS_NPC(ch))
	{
		if (!SKILL_OK(ch, sn))
		    skill = 0;
		else
		    skill = ch->pcdata->learned[sn];
	}

	else /* mobiles */
	{

	    if (skill_table[sn].spell_fun != spell_null)
		    skill = 40 + 2 * ch->level;

		else if (sn == gsn_sneak || sn == gsn_hide)
		    skill = ch->level * 2 + 20;

	    else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
 	||       (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)))
		    skill = ch->level * 2;

 	else if (sn == gsn_shield_block)
		    skill = 10 + 2 * ch->level;

		else if (sn == gsn_second_attack 
		&& (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF)))
		    skill = 10 + 3 * ch->level;

		else if (sn == gsn_third_attack && IS_SET(ch->act,ACT_WARRIOR))
		    skill = 4 * ch->level - 40;

		else if (sn == gsn_fourth_attack && IS_SET(ch->act,ACT_WARRIOR))
		    skill = 4 * ch->level - 60;

		else if (sn == gsn_hand_to_hand)
		    skill = 40 + 2 * ch->level;

 	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
		    skill = 10 + 3 * ch->level;

 	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
		    skill = 10 + 3 * ch->level;

		else if (sn == gsn_disarm 
		     &&  (IS_SET(ch->off_flags,OFF_DISARM) 
		     ||   IS_SET(ch->act,ACT_WARRIOR)
		     ||	  IS_SET(ch->act,ACT_THIEF)))
		    skill = 20 + 3 * ch->level;

		else if (sn == gsn_grip
		     &&  (IS_SET(ch->act,ACT_WARRIOR)
		     ||	  IS_SET(ch->act,ACT_THIEF)))
		    skill = ch->level;

		else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
		    skill = 3 * ch->level;

		else if (sn == gsn_kick)
		    skill = 10 + 3 * ch->level;

		else if (sn == gsn_backstab && IS_SET(ch->act,ACT_THIEF))
		    skill = 20 + 2 * ch->level;

  	else if (sn == gsn_rescue)
		    skill = 40 + ch->level; 

		else if (sn == gsn_recall)
		    skill = 40 + ch->level;

		else if (sn == gsn_sword
		||  sn == gsn_dagger
		||  sn == gsn_spear
		||  sn == gsn_mace
		||  sn == gsn_axe
		||  sn == gsn_flail
		||  sn == gsn_whip
		||  sn == gsn_polearm
		||  sn == gsn_bow
		||  sn == gsn_arrow
		||  sn == gsn_lance)
		    skill = 40 + 5 * ch->level / 2;

		else 
		   skill = 0;
	}

	if (ch->daze > 0)
	{
		if (skill_table[sn].spell_fun != spell_null)
		    skill /= 2;
		else
		    skill = 2 * skill / 3;
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10)
		skill = 9 * skill / 10;

	return URANGE(0,skill,100);
}

/* for returning weapon information */
int get_weapon_sn(CHAR_DATA *ch, int type)
{
	OBJ_DATA *wield;
	int sn;

	if (type != WEAR_WIELD && type != WEAR_SECOND_WIELD)
		return 0;

	wield = get_eq_char(ch, type);
	if (wield == NULL || wield->item_type != ITEM_WEAPON)
		sn = (type == WEAR_WIELD) ? gsn_hand_to_hand : 0;
	else
	switch (wield->value[0]) {
	    default :               sn = -1;		break;
	    case(WEAPON_SWORD):     sn = gsn_sword;	break;
	    case(WEAPON_DAGGER):    sn = gsn_dagger;	break;
	    case(WEAPON_SPEAR):     sn = gsn_spear;	break;
	    case(WEAPON_MACE):      sn = gsn_mace;	break;
	    case(WEAPON_AXE):       sn = gsn_axe;	break;
	    case(WEAPON_FLAIL):     sn = gsn_flail;	break;
	    case(WEAPON_WHIP):      sn = gsn_whip;	break;
	    case(WEAPON_POLEARM):   sn = gsn_polearm;	break;
	    case(WEAPON_BOW):	    sn = gsn_bow;	break;
	    case(WEAPON_ARROW):	    sn = gsn_arrow;	break;
	    case(WEAPON_LANCE):	    sn = gsn_lance;	break;
   }
   return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
	 int skill;

	 /* -1 is exotic */
	if (IS_NPC(ch)) {
		if (sn == -1)
		    skill = 3 * ch->level;
		else if (sn == gsn_hand_to_hand)
		    skill = 40 + 2 * ch->level;
		else 
		    skill = 40 + 5 * ch->level / 2;
	}
	
	else
	{
		if (sn == -1)
		    skill = 3 * ch->level;
		else
		    skill = ch->pcdata->learned[sn];
	}

	return URANGE(0,skill,100);
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
	||	ch->pcdata->perm_mana == 0
	||  ch->pcdata->perm_move == 0
	||	ch->pcdata->last_level == 0)
	{
	/* do a FULL reset */
		for (loc = 0; loc < MAX_WEAR; loc++)
		{
		    obj = get_eq_char(ch,loc);
		    if (obj == NULL)
			continue;
		    if (!obj->enchanted)
		    for (af = obj->pIndexData->affected; af != NULL; af = af->next)
		    {
			mod = af->modifier;
			switch(af->location)
			{
			    case APPLY_MANA:	ch->max_mana	-= mod;		break;
			    case APPLY_HIT:	ch->max_hit	-= mod;		break;
			    case APPLY_MOVE:	ch->max_move	-= mod;		break;
			}
		    }

	        for (af = obj->affected; af != NULL; af = af->next)
	        {
	            mod = af->modifier;
	            switch(af->location)
	            {
	                case APPLY_MANA:    ch->max_mana    -= mod;         break;
	                case APPLY_HIT:     ch->max_hit     -= mod;         break;
	                case APPLY_MOVE:    ch->max_move    -= mod;         break;
	            }
	        }
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

	    if (!obj->enchanted)
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

/* command for retrieving stats */
int get_curr_stat(CHAR_DATA *ch, int stat)
{
	int max;

	if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
		max = 25;

	else
	{
	    max = get_max_train(ch,stat);
 	max = UMIN(max,25);
	}
  
	return URANGE(3,ch->perm_stat[stat] + ch->mod_stat[stat], max);
}


/* command for returning max training score */
int get_max_train(CHAR_DATA *ch, int stat)
{
	int max;

	if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
		return 25;

	max = (20 + pc_race_table[ORG_RACE(ch)].stats[stat] + /* ORG_RACE && RACE serdar*/
			class_table[ch->class].stats[stat]);
	
	return UMIN(max,25);
}

/*   
 * command for returning max training score
 * for do_train and stat2train in comm.c
 */
int get_max_train2(CHAR_DATA *ch, int stat)
{
	int max;

	if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
		return 25;

	max = (20 + pc_race_table[ORG_RACE(ch)].stats[stat] + 
			class_table[ch->class].stats[stat]);
	
	return UMIN(max,25);
}
   
		
/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n(CHAR_DATA *ch)
{
	if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
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
	if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
		return 10000000;

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET))
		return 0;

	return str_app[get_curr_stat(ch,STAT_STR)].carry * 10 + ch->level * 25;
}



/*
 * See if a string is one of the names of an object.
 */

bool is_name(char *str, char *namelist)
{
	char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
	char *list, *string;
	
	if (!namelist)
		return FALSE;

	string = str;
	/* we need ALL parts of string to match part of namelist */
	for (; ;)  /* start parsing string */
	{
		str = one_argument(str,part);

		if (part[0] == '\0')
		    return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;
		for (; ;)  /* start parsing namelist */
		{
		    list = one_argument(list,name);
		    if (name[0] == '\0')  /* this name was not found */
			return FALSE;

		    if (!str_prefix(string,name))
			return TRUE; /* full pattern match */

		    if (!str_prefix(part,name))
			break;
		}
	}
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
	/* okay, move all the old flags into new vectors if we have to */
	if (!obj->enchanted)
	{
	    AFFECT_DATA *paf, *af_new;
	    obj->enchanted = TRUE;

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
	OBJ_DATA *wield,*obj2;
	int mod,i;

	mod = paf->modifier;

	if (fAdd)
	{
		switch (paf->where)
		{
		case TO_AFFECTS:
		     SET_BIT(ch->affected_by, paf->bitvector);
		    break;
		case TO_IMMUNE:
		    SET_BIT(ch->imm_flags,paf->bitvector);
		    break;
		case TO_RESIST:
		    SET_BIT(ch->res_flags,paf->bitvector);
		    break;
		case TO_ACT_FLAG:
		    SET_BIT(ch->act,paf->bitvector);
		    break;
		case TO_VULN:
		    SET_BIT(ch->vuln_flags,paf->bitvector);
		    break;
		case TO_DETECTS:
		    SET_BIT(ch->detection,paf->bitvector);
		    break;
		case TO_RACE:
		    RACE(ch) = paf->modifier < MAX_PC_RACE ? paf->modifier : 1;	    
		    REMOVE_BIT(ch->affected_by,race_table[ORG_RACE(ch)].det);
		    SET_BIT(ch->affected_by,race_table[RACE(ch)].det);
		    REMOVE_BIT(ch->affected_by,race_table[ORG_RACE(ch)].aff);
		    SET_BIT(ch->affected_by,race_table[RACE(ch)].aff);
		    REMOVE_BIT(ch->imm_flags,race_table[ORG_RACE(ch)].imm);
		    SET_BIT(ch->imm_flags,race_table[RACE(ch)].imm);
		    REMOVE_BIT(ch->res_flags,race_table[ORG_RACE(ch)].res);
		    SET_BIT(ch->res_flags,race_table[RACE(ch)].res);
		    REMOVE_BIT(ch->vuln_flags,race_table[ORG_RACE(ch)].vuln);
		    SET_BIT(ch->vuln_flags,race_table[RACE(ch)].vuln);
		    ch->form	= race_table[RACE(ch)].form;
		    ch->parts	= race_table[RACE(ch)].parts;
		    break;
		}
	}
	else
	{
	    switch (paf->where)
	    {
	    case TO_AFFECTS:
	        REMOVE_BIT(ch->affected_by, paf->bitvector);
	        break;
	    case TO_IMMUNE:
	        REMOVE_BIT(ch->imm_flags,paf->bitvector);
	        break;
	    case TO_RESIST:
	        REMOVE_BIT(ch->res_flags,paf->bitvector);
	        break;
		case TO_ACT_FLAG:
		    REMOVE_BIT(ch->act,paf->bitvector);
		    break;
	    case TO_VULN:
	        REMOVE_BIT(ch->vuln_flags,paf->bitvector);
	        break;
	    case TO_DETECTS:
	        REMOVE_BIT(ch->detection,paf->bitvector);
	        break;
		case TO_RACE:
		    REMOVE_BIT(ch->affected_by,race_table[RACE(ch)].det);
		    SET_BIT(ch->affected_by,race_table[ORG_RACE(ch)].det);
		    REMOVE_BIT(ch->affected_by,race_table[RACE(ch)].aff);
		    SET_BIT(ch->affected_by,race_table[ORG_RACE(ch)].aff);
		    REMOVE_BIT(ch->imm_flags,race_table[RACE(ch)].imm);
		    SET_BIT(ch->imm_flags,race_table[ORG_RACE(ch)].imm);
		    REMOVE_BIT(ch->res_flags,race_table[RACE(ch)].res);
		    SET_BIT(ch->res_flags,race_table[ORG_RACE(ch)].res);
		    REMOVE_BIT(ch->vuln_flags,race_table[RACE(ch)].vuln);
		    SET_BIT(ch->vuln_flags,race_table[ORG_RACE(ch)].vuln);
		    ch->form	= race_table[ORG_RACE(ch)].form;
		    ch->parts	= race_table[ORG_RACE(ch)].parts;
		    RACE(ch) = ORG_RACE(ch);	    
		    break;
	    }
		mod = 0 - mod;
	}

	switch (paf->location)
	{
	default:
		bug("Affect_modify: unknown location %d.", paf->location);
		return;

	case APPLY_NONE:						break;
	case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
	case APPLY_DEX:           ch->mod_stat[STAT_DEX]	+= mod;	break;
	case APPLY_INT:           ch->mod_stat[STAT_INT]	+= mod;	break;
	case APPLY_WIS:           ch->mod_stat[STAT_WIS]	+= mod;	break;
	case APPLY_CON:           ch->mod_stat[STAT_CON]	+= mod;	break;
	case APPLY_CHA:	      ch->mod_stat[STAT_CHA]	+= mod; break;
	case APPLY_CLASS:						break;
	case APPLY_LEVEL:						break;
	case APPLY_AGE:	ch->played += age_to_num(mod);	break;
	case APPLY_HEIGHT:						break;
	case APPLY_WEIGHT:						break;
	case APPLY_MANA:          ch->max_mana		+= mod;	break;
	case APPLY_HIT:           ch->max_hit		+= mod;	break;
	case APPLY_MOVE:          ch->max_move		+= mod;	break;
	case APPLY_GOLD:						break;
	case APPLY_EXP:						break;
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
	}

	/*
	 * Check for weapon wielding.
	 * Guard against recursion (for weapons with affects).
	 */
	if (!IS_NPC(ch) && (wield = get_eq_char(ch, WEAR_WIELD)) != NULL
	&&   get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10))
	{
		static int depth;

		if (depth == 0)
		{
		    depth++;
		    act("You drop $p.", ch, wield, NULL, TO_CHAR);
		    act("$n drops $p.", ch, wield, NULL, TO_ROOM);
		    obj_from_char(wield);
		    obj_to_room(wield, ch->in_room);

	if ((obj2 = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL)
		{
 act("You wield his second weapon as your first!",  ch, NULL,NULL,TO_CHAR);
 act("$n wields his second weapon as first!",  ch, NULL,NULL,TO_ROOM);
		unequip_char(ch, obj2);
		equip_char(ch, obj2 , WEAR_WIELD);
		}
		    depth--;
		}
	}

	return;
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

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch,int where,int vector)
{
	AFFECT_DATA *paf;
	OBJ_DATA *obj;

	if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
		return;

	for (paf = ch->affected; paf != NULL; paf = paf->next)
		if (paf->where == where && paf->bitvector == vector)
		{
		    switch (where)
		    {
		        case TO_AFFECTS:
			    SET_BIT(ch->affected_by,vector);
			    break;
		        case TO_IMMUNE:
			    SET_BIT(ch->imm_flags,vector);   
			    break;
		        case TO_RESIST:
			    SET_BIT(ch->res_flags,vector);
			    break;
			case TO_ACT_FLAG:
			    SET_BIT(ch->act,paf->bitvector);
			    break;
		        case TO_VULN:
			    SET_BIT(ch->vuln_flags,vector);
			    break;
		        case TO_DETECTS:
			    SET_BIT(ch->detection,vector);
			    break;
			case TO_RACE:
			    if (RACE(ch) == ORG_RACE(ch)) 
			    {
			     RACE(ch) = paf->modifier<MAX_PC_RACE ? paf->modifier:1; 
			     REMOVE_BIT(ch->affected_by,race_table[ORG_RACE(ch)].det);
			     SET_BIT(ch->affected_by,race_table[RACE(ch)].det);
		     	     REMOVE_BIT(ch->affected_by,race_table[ORG_RACE(ch)].aff);
			     SET_BIT(ch->affected_by,race_table[RACE(ch)].aff);
			     REMOVE_BIT(ch->imm_flags,race_table[ORG_RACE(ch)].imm);
			     SET_BIT(ch->imm_flags,race_table[RACE(ch)].imm);
			     REMOVE_BIT(ch->res_flags,race_table[ORG_RACE(ch)].res);
			     SET_BIT(ch->res_flags,race_table[RACE(ch)].res);
			     REMOVE_BIT(ch->vuln_flags,race_table[ORG_RACE(ch)].vuln);
			     SET_BIT(ch->vuln_flags,race_table[RACE(ch)].vuln);
			     ch->form	= race_table[RACE(ch)].form;
			     ch->parts	= race_table[RACE(ch)].parts;
			    }
			    break;
		    }
		    return;
		}

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
		if (obj->wear_loc == -1 || obj->wear_loc == WEAR_STUCK_IN)
		    continue;

	        for (paf = obj->affected; paf != NULL; paf = paf->next)
	        if (paf->where == where && paf->bitvector == vector)
	        {
	            switch (where)
	            {
	                case TO_AFFECTS:
	                    SET_BIT(ch->affected_by,vector);
	                    break;
	                case TO_IMMUNE:
	                    SET_BIT(ch->imm_flags,vector);
	                    break;
			    case TO_ACT_FLAG:
			        SET_BIT(ch->act,paf->bitvector);
			        break;
	                case TO_RESIST:
	                    SET_BIT(ch->res_flags,vector);
	                    break;
	                case TO_VULN:
	                    SET_BIT(ch->vuln_flags,vector);
	              	break;
	                case TO_DETECTS:
	                    SET_BIT(ch->detection,vector);
	              	break;
			    case TO_RACE:
			    	if (RACE(ch) == ORG_RACE(ch)) 
			    {
			     RACE(ch) = paf->modifier<MAX_PC_RACE ? paf->modifier:1; 
			     REMOVE_BIT(ch->affected_by,race_table[ORG_RACE(ch)].det);
			     SET_BIT(ch->affected_by,race_table[RACE(ch)].det);
		     	     REMOVE_BIT(ch->affected_by,race_table[ORG_RACE(ch)].aff);
			     SET_BIT(ch->affected_by,race_table[RACE(ch)].aff);
			     REMOVE_BIT(ch->imm_flags,race_table[ORG_RACE(ch)].imm);
			     SET_BIT(ch->imm_flags,race_table[RACE(ch)].imm);
			     REMOVE_BIT(ch->res_flags,race_table[ORG_RACE(ch)].res);
			     SET_BIT(ch->res_flags,race_table[RACE(ch)].res);
			     REMOVE_BIT(ch->vuln_flags,race_table[ORG_RACE(ch)].vuln);
			     SET_BIT(ch->vuln_flags,race_table[RACE(ch)].vuln);
			     ch->form	= race_table[RACE(ch)].form;
			     ch->parts	= race_table[RACE(ch)].parts;
			    }
			        break;
	            }
	            return;
	        }

	    if (obj->enchanted)
		    continue;

	    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	        if (paf->where == where && paf->bitvector == vector)
	        {
	            switch (where)
	            {
	                case TO_AFFECTS:
	                    SET_BIT(ch->affected_by,vector);
		                break;
	                case TO_IMMUNE:
	                    SET_BIT(ch->imm_flags,vector);
	                    break;
			   case TO_ACT_FLAG:
		                SET_BIT(ch->act,paf->bitvector);
			        break;
	                case TO_RESIST:
	                    SET_BIT(ch->res_flags,vector);
	                    break;
	                case TO_VULN:
	                    SET_BIT(ch->vuln_flags,vector);
	                    break;
	                case TO_DETECTS:
	                    SET_BIT(ch->detection,vector);
	                    break;
			   case TO_RACE:
			    	if (RACE(ch) == ORG_RACE(ch)) 
			    {
			     RACE(ch) = paf->modifier<MAX_PC_RACE ? paf->modifier:1; 
			     REMOVE_BIT(ch->affected_by,race_table[ORG_RACE(ch)].det);
			     SET_BIT(ch->affected_by,race_table[RACE(ch)].det);
		     	     REMOVE_BIT(ch->affected_by,race_table[ORG_RACE(ch)].aff);
			     SET_BIT(ch->affected_by,race_table[RACE(ch)].aff);
			     REMOVE_BIT(ch->imm_flags,race_table[ORG_RACE(ch)].imm);
			     SET_BIT(ch->imm_flags,race_table[RACE(ch)].imm);
			     REMOVE_BIT(ch->res_flags,race_table[ORG_RACE(ch)].res);
			     SET_BIT(ch->res_flags,race_table[RACE(ch)].res);
			     REMOVE_BIT(ch->vuln_flags,race_table[ORG_RACE(ch)].vuln);
			     SET_BIT(ch->vuln_flags,race_table[RACE(ch)].vuln);
			     ch->form	= race_table[RACE(ch)].form;
			     ch->parts	= race_table[RACE(ch)].parts;
			    }
			        break;
	            }
	            return;
	        }
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

	*paf_new		= *paf;
	paf_new->next	= obj->affected;
	obj->affected	= paf_new;

	/* apply any affect vectors to the object's extra_flags */
	if (paf->bitvector)
	    switch (paf->where)
	    {
	    case TO_OBJECT:
		    SET_BIT(obj->extra_flags,paf->bitvector);
		    break;
	    case TO_WEAPON:
		    if (obj->item_type == ITEM_WEAPON)
		        SET_BIT(obj->value[4],paf->bitvector);
		    break;
	    }
	

	return;
}



/*
 * Remove an affect from a char.
 */
void affect_remove(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	int where;
	int vector;

	if (ch->affected == NULL)
	{
		bug("Affect_remove: no affect.", 0);
		return;
	}

	affect_modify(ch, paf, FALSE);
	where = paf->where;
	vector = paf->bitvector;

	if (paf == ch->affected)
	{
		ch->affected	= paf->next;
	}
	else
	{
		AFFECT_DATA *prev;

		for (prev = ch->affected; prev != NULL; prev = prev->next)
		{
		    if (prev->next == paf)
		    {
			prev->next = paf->next;
			break;
		    }
		}

		if (prev == NULL)
		{
		    bug("Affect_remove: cannot find paf.", 0);
		    return;
		}
	}

	free_affect(paf);

	affect_check(ch,where,vector);
	return;
}

void affect_remove_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	int where, vector;
	if (obj->affected == NULL)
	{
	    bug("Affect_remove_object: no affect.", 0);
	    return;
	}

	if (obj->carried_by != NULL && obj->wear_loc != -1)
		affect_modify(obj->carried_by, paf, FALSE);

	where = paf->where;
	vector = paf->bitvector;

	/* remove flags from the object if needed */
	if (paf->bitvector)
		switch(paf->where)
	    {
	    case TO_OBJECT:
	        REMOVE_BIT(obj->extra_flags,paf->bitvector);
	        break;
	    case TO_WEAPON:
	        if (obj->item_type == ITEM_WEAPON)
	            REMOVE_BIT(obj->value[4],paf->bitvector);
	        break;
	    }

	if (paf == obj->affected)
	{
	    obj->affected    = paf->next;
	}
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
	return;
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip(CHAR_DATA *ch, int sn)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = ch->affected; paf != NULL; paf = paf_next)
	{
		paf_next = paf->next;
		if (paf->type == sn)
		    affect_remove(ch, paf);
	}

	return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected(CHAR_DATA *ch, int sn)
{
	AFFECT_DATA *paf;

	for (paf = ch->affected; paf != NULL; paf = paf->next)
	{
		if (paf->type == sn)
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
	&&   obj->item_type == ITEM_LIGHT
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
	&&   obj->item_type == ITEM_LIGHT
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
	        	send_to_char("You feel hot and feverish.\n\r",vch);
	        	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
	        	affect_join(vch,&plague);
	        }
	    }
		break;
	}

	if (ch->in_room->affected_by) {
		 if (IS_IMMORTAL(ch))
			do_raffects(ch,"");
		 else
			raffect_to_char(ch->in_room, ch);
	}
}



/*
 * Give an obj to a char.
 */
void obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch)
{
	obj->next_content	 = ch->carrying;
	ch->carrying	 = obj;
	obj->carried_by	 = ch;
	obj->in_room	 = NULL;
	obj->in_obj		 = NULL;
	ch->carry_number	+= get_obj_number(obj);
	ch->carry_weight	+= get_obj_weight(obj);
}



/*
 * Take an obj from its character.
 */
void obj_from_char(OBJ_DATA *obj)
{
	CHAR_DATA *ch;

	if ((ch = obj->carried_by) == NULL)
	{
		bug("Obj_from_char: null ch.", 0);
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(ch, obj);

	if (ch->carrying == obj)
	{
		ch->carrying = obj->next_content;
	}
	else
	{
		OBJ_DATA *prev;

		for (prev = ch->carrying; prev != NULL; prev = prev->next_content)
		{
		    if (prev->next_content == obj)
		    {
			prev->next_content = obj->next_content;
			break;
		    }
		}

		if (prev == NULL)
		    bug("Obj_from_char: obj not in list.", 0);
	}

	obj->carried_by	 = NULL;
	obj->next_content	 = NULL;
	ch->carry_number	-= get_obj_number(obj);
	ch->carry_weight	-= get_obj_weight(obj);
	return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac(OBJ_DATA *obj, int iWear, int type)
{
	if (obj->item_type != ITEM_ARMOR)
		return 0;

	switch (iWear)
	{
	case WEAR_BODY:	return 3 * obj->value[type];
	case WEAR_HEAD:	return 2 * obj->value[type];
	case WEAR_LEGS:	return 2 * obj->value[type];
	case WEAR_FEET:	return     obj->value[type];
	case WEAR_HANDS:	return     obj->value[type];
	case WEAR_ARMS:	return     obj->value[type];
	case WEAR_SHIELD:	return     obj->value[type];
	case WEAR_FINGER_L:	return     0;
	case WEAR_FINGER_R: return     obj->value[type];
	case WEAR_NECK_1:	return     obj->value[type];
	case WEAR_NECK_2:	return     obj->value[type];
	case WEAR_ABOUT:	return 2 * obj->value[type];
	case WEAR_WAIST:	return     obj->value[type];
	case WEAR_WRIST_L:	return     obj->value[type];
	case WEAR_WRIST_R:	return     obj->value[type];
	case WEAR_HOLD:	return     obj->value[type];
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
	{
		if (obj->wear_loc == iWear)
		    return obj;
	}

	return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char(CHAR_DATA *ch, OBJ_DATA *obj, int iWear)
{
	AFFECT_DATA *paf;
	int i;

	if (iWear == WEAR_STUCK_IN)
	{
		obj->wear_loc = iWear;
		return;
	}

	if (get_eq_char(ch, iWear) != NULL)
	{
		bug("Equip_char: already equipped (%d).", iWear);
		return;
	}

	if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)   )
	||   (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)   )
	||   (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
	{
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

	if (!obj->enchanted)
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		    if (paf->location != APPLY_SPELL_AFFECT)
		        affect_modify(ch, paf, TRUE);
	for (paf = obj->affected; paf != NULL; paf = paf->next)
		if (paf->location == APPLY_SPELL_AFFECT)
		    affect_to_char (ch, paf);
		else
		    affect_modify(ch, paf, TRUE);

	if (obj->item_type == ITEM_LIGHT
	&&   obj->value[2] != 0
	&&   ch->in_room != NULL)
		++ch->in_room->light;

	oprog_call(OPROG_WEAR, obj, ch, NULL);
}



/*
 * Unequip a char with an obj.
 */
void unequip_char(CHAR_DATA *ch, OBJ_DATA *obj)
{
	AFFECT_DATA *paf = NULL;
	AFFECT_DATA *lpaf = NULL;
	AFFECT_DATA *lpaf_next = NULL;
	int i;

	if (obj->wear_loc == WEAR_NONE)
	{
		bug("Unequip_char: already unequipped.", 0);
		return;
	}

	if (obj->wear_loc == WEAR_STUCK_IN)
	{
		obj->wear_loc = WEAR_NONE;
		return;
	}

	for (i = 0; i < 4; i++)
		ch->armor[i]	+= apply_ac(obj, obj->wear_loc,i);
	obj->wear_loc	 = -1;

	if (!obj->enchanted)
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		    if (paf->location == APPLY_SPELL_AFFECT)
		    {
		        for (lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next)
		        {
			    lpaf_next = lpaf->next;
			    if ((lpaf->type == paf->type) &&
			        (lpaf->level == paf->level) &&
			        (lpaf->location == APPLY_SPELL_AFFECT))
			    {
			        affect_remove(ch, lpaf);
				lpaf_next = NULL;
			    }
		        }
		    }
		    else
		    {
		        affect_modify(ch, paf, FALSE);
			affect_check(ch,paf->where,paf->bitvector);
		    }

	for (paf = obj->affected; paf != NULL; paf = paf->next)
		if (paf->location == APPLY_SPELL_AFFECT)
		{
		    bug ("Norm-Apply: %d", 0);
		    for (lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next)
		    {
			lpaf_next = lpaf->next;
			if ((lpaf->type == paf->type) &&
			    (lpaf->level == paf->level) &&
			    (lpaf->location == APPLY_SPELL_AFFECT))
			{
			    bug ("location = %d", lpaf->location);
			    bug ("type = %d", lpaf->type);
			    affect_remove(ch, lpaf);
			    lpaf_next = NULL;
			}
		    }
		}
		else
		{
		    affect_modify(ch, paf, FALSE);
		    affect_check(ch,paf->where,paf->bitvector);	
		}

	if (obj->item_type == ITEM_LIGHT
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

	if ((in_room = obj->in_room) == NULL)
	{
		bug("obj_from_room: NULL.", 0);
		return;
	}

	for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
		if (ch->on == obj)
		    ch->on = NULL;

	if (obj == in_room->contents)
	{
		in_room->contents = obj->next_content;
	}
	else
	{
		OBJ_DATA *prev;

		for (prev = in_room->contents; prev; prev = prev->next_content)
		{
		    if (prev->next_content == obj)
		    {
			prev->next_content = obj->next_content;
			break;
		    }
		}

		if (prev == NULL)
		{
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
	obj->next_content		= pRoomIndex->contents;
	pRoomIndex->contents	= obj;
	obj->in_room		= pRoomIndex;
	obj->carried_by		= NULL;
	obj->in_obj			= NULL;

	if (IS_WATER(pRoomIndex))
		if (may_float(obj))
		  obj->water_float = -1;
		else
		  obj->water_float = floating_time(obj);

	return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj(OBJ_DATA *obj, OBJ_DATA *obj_to)
{

	obj->next_content		= obj_to->contains;
	obj_to->contains		= obj;
	obj->in_obj			= obj_to;
	obj->in_room		= NULL;
	obj->carried_by		= NULL;
	if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
	    obj->cost = 0; 

	for (; obj_to != NULL; obj_to = obj_to->in_obj)
	{
		if (obj_to->carried_by != NULL)
		{
/*	    obj_to->carried_by->carry_number += get_obj_number(obj); */
		    obj_to->carried_by->carry_weight += get_obj_weight(obj)
			* WEIGHT_MULT(obj_to) / 100;
		}
	}

	return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj(OBJ_DATA *obj)
{
	OBJ_DATA *obj_from;

	if ((obj_from = obj->in_obj) == NULL)
	{
		bug("Obj_from_obj: null obj_from.", 0);
		return;
	}

	if (obj == obj_from->contains)
	{
		obj_from->contains = obj->next_content;
	}
	else
	{
		OBJ_DATA *prev;

		for (prev = obj_from->contains; prev; prev = prev->next_content)
		{
		    if (prev->next_content == obj)
		    {
			prev->next_content = obj->next_content;
			break;
		    }
		}

		if (prev == NULL)
		{
		    bug("Obj_from_obj: obj not found.", 0);
		    return;
		}
	}

	obj->next_content = NULL;
	obj->in_obj       = NULL;

	for (; obj_from != NULL; obj_from = obj_from->in_obj)
	{
		if (obj_from->carried_by != NULL)
		{
/*	    obj_from->carried_by->carry_number -= get_obj_number(obj); */
		    obj_from->carried_by->carry_weight -= get_obj_weight(obj) 
			* WEIGHT_MULT(obj_from) / 100;
		}
	}

	return;
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
	int i;

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

	for (i=1;i < MAX_CLAN;i++)
	  if (obj->pIndexData->vnum == clan_table[i].obj_vnum) {
	    obj->pIndexData->count--;
	    clan_table[i].obj_ptr = NULL;
	  }

	for (obj_content = obj->contains; obj_content; obj_content = obj_next)
	{
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

	if (ch->desc != NULL && ch->desc->original != NULL)
	{
		do_return(ch, "");
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
CHAR_DATA *get_char_room(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *rch;
	int number;
	int count;
	int ugly;

	number = number_argument(argument, arg);
	count  = 0;
	ugly   = 0;
	if (!str_cmp(arg, "self"))
		return ch;
	if (!str_cmp(arg, "ugly"))
		ugly = 1;

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (!can_see(ch, rch))
		    continue;

		if (ugly && (count + 1) == number && IS_VAMPIRE(rch))
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
CHAR_DATA *get_char_room2(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument, int *number)
{
	CHAR_DATA *rch;
	int count;
	int ugly;

	if (room == NULL) return NULL;
	count  = 0;
	ugly   = 0;

	if (!str_cmp(argument, "ugly"))
		ugly = 1;

	for (rch = room->people; rch != NULL; rch = rch->next_in_room) {
		if (!can_see(ch, rch))
		    continue;

		if (ugly && (count + 1) == *number && IS_VAMPIRE(rch))
		   return rch;

		if (!is_name(argument, rch->name))
			continue;

		if (++count == *number)
		    return rch;
	}

	*number -= count;
	return NULL;
}


/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *wch;
	int number;
	int count;

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
OBJ_DATA *get_obj_list(CHAR_DATA *ch, char *argument, OBJ_DATA *list)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

	number = number_argument(argument, arg);
	count  = 0;
	for (obj = list; obj != NULL; obj = obj->next_content)
	{
		if (can_see_obj(ch, obj) && is_name(arg, obj->name))
		{
		    if (++count == number)
			return obj;
		}
	}

	return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

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
OBJ_DATA *get_obj_wear(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

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
OBJ_DATA *get_obj_here(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;

	obj = get_obj_list(ch, argument, ch->in_room->contents);
	if (obj != NULL)
		return obj;

	if ((obj = get_obj_carry(ch, argument)) != NULL)
		return obj;

	if ((obj = get_obj_wear(ch, argument)) != NULL)
		return obj;

	return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

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
		obj = create_object(get_obj_index(OBJ_VNUM_SILVER_ONE), 0);
	}
	else if (gold == 1 && silver == 0)
	{
		obj = create_object(get_obj_index(OBJ_VNUM_GOLD_ONE), 0);
	}
	else if (silver == 0)
	{
	    obj = create_object(get_obj_index(OBJ_VNUM_GOLD_SOME), 0);
		str_printf(&obj->short_descr, obj->short_descr, gold);
	    obj->value[1]           = gold;
	    obj->cost               = gold;
		obj->weight		= gold/5;
	}
	else if (gold == 0)
	{
	    obj = create_object(get_obj_index(OBJ_VNUM_SILVER_SOME), 0);
		str_printf(&obj->short_descr, obj->short_descr, silver);
	    obj->value[0]           = silver;
	    obj->cost               = silver;
		obj->weight		= silver/20;
	}
 
	else
	{
		obj = create_object(get_obj_index(OBJ_VNUM_COINS), 0);
		str_printf(&obj->short_descr, obj->short_descr, silver, gold);
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
	if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
	||  obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY)
	    number = 0;
*/
	if (obj->item_type == ITEM_MONEY)
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

	if (IS_VAMPIRE (ch)) 
		{
	     return FALSE;
		}
		
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
	&&   !CAN_DETECT(ch, DETECT_INVIS))
		return FALSE;

	if (IS_AFFECTED(victim, AFF_IMP_INVIS)
	&&   !CAN_DETECT(ch, DETECT_IMP_INVIS))
		return FALSE;

/*
	if (IS_AFFECTED(victim, AFF_SNEAK)
	&&  !(IS_NPC(ch) && ch->pIndexData->pShop)
	&&  !CAN_DETECT(ch,DETECT_HIDDEN)
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
	    !CAN_DETECT(ch,ACUTE_VISION))
	  return FALSE;

	if (IS_AFFECTED(victim, AFF_HIDE)
	&&   !CAN_DETECT(ch, DETECT_HIDDEN)
	&&   victim->fighting == NULL)
		return FALSE;

	if (IS_AFFECTED(victim, AFF_FADE)
	&&   !CAN_DETECT(ch, DETECT_FADE)
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

	if (IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
		return FALSE;

	if (IS_AFFECTED(ch, AFF_BLIND) && obj->item_type != ITEM_POTION)
		return FALSE;

	if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
		return TRUE;

	if (IS_SET(obj->extra_flags, ITEM_INVIS)
	&&   !CAN_DETECT(ch, DETECT_INVIS))
	    return FALSE;

	if (IS_OBJ_STAT(obj,ITEM_GLOW))
		return TRUE;

	if (room_is_dark(ch) && !IS_AFFECTED(ch, AFF_INFRARED))
		return FALSE;

	if (obj->item_type == ITEM_TATTOO)
		return TRUE;

	return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (!IS_SET(obj->extra_flags, ITEM_NODROP))
		return TRUE;

	if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
		return TRUE;

	return FALSE;
}


char *flag_names_raw(const struct flag_type *f, int vector, char* buf)
{
	while (f->name != NULL) {
		if (IS_SET(vector, f->bit)) {
			strcat(buf, " ");
			strcat(buf, f->name);
		}
		f++;
	}

	return (buf[0] != '\0') ? buf+1 : "none";
}


char *flag_names(const struct flag_type *f, int vector)
{
	static char buf[MAX_STRING_LENGTH];
	buf[0] = '\0';
	return flag_names_raw(f, vector, buf);
}

/*
 * Return ascii name of an item type.
 */
char *item_type_name(OBJ_DATA *obj)
{
	return flag_name_lookup(type_flags, obj->item_type);
}


/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name(int location)
{
	return flag_name_lookup(apply_flags, location);
}


/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name(int vector)
{
	return flag_names(affect_flags, vector);
}


/*
 * Return ascii name of an affect bit vector.
 */
char *detect_bit_name(int vector)
{
	return flag_names(detect_flags, vector);
}



/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name(int vector)
{
	return flag_names(extra_flags, vector);
}

/* return ascii name of an act vector */
char *act_bit_name(int vector)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_SET(vector, ACT_NPC)) { 
 		strcpy(buf,"npc");
		return flag_names_raw(act_flags, vector, buf);
	}
	else {
		strcpy(buf,"player");
		return flag_names_raw(plr_flags, vector, buf);
	}
}

char *comm_bit_name(int vector)
{
	return flag_names(comm_flags, vector);
}


char *imm_bit_name(int vector)
{
	return flag_names(imm_flags, vector);
}


char *wear_bit_name(int vector)
{
	return flag_names(wear_flags, vector);
}


char *form_bit_name(int vector)
{
	return flag_names(form_flags, vector);
}


char *part_bit_name(int vector)
{
	return flag_names(part_flags, vector);
}


char *weapon_bit_name(int vector)
{
	return flag_names(weapon_type2, vector);
}

char *cont_bit_name(int vector)
{
	return flag_names(cont_flags, vector);
}

char *off_bit_name(int vector)
{
	return flag_names(off_flags, vector);
}


bool isn_dark_safe(CHAR_DATA *ch)
{
	CHAR_DATA *rch;
	OBJ_DATA *light;
	int light_exist;

	if (!IS_VAMPIRE(ch))  return 0;

	if (IS_SET(ch->in_room->room_flags, ROOM_DARK))
		return 0;

	if (weather_info.sunlight == SUN_LIGHT
	||   weather_info.sunlight == SUN_RISE)
		return 2;

	light_exist = 0;
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if ((light = get_eq_char(rch,WEAR_LIGHT)) != NULL
		      && IS_OBJ_STAT(light,ITEM_MAGIC))
		 {
		  light_exist = 1;
		  break;
		 }
	}

	return light_exist;
}


int ch_skill_nok(CHAR_DATA *ch, int skill)
{
	if (IS_NPC(ch) || !SKILL_OK(ch, skill)) {
		send_to_char("Huh?\n\r",ch);
		return 1;
	}
	return 0;
}

int skill_is_native(CHAR_DATA* ch, int sn)
{
	int i;

	if (IS_NPC(ch))
		return 0;

	for (i = 0; i < 5; i++) {
		int csn;

		csn = skill_lookup(pc_race_table[ch->pcdata->race].skills[i]);
		if (csn < 0)
			break;
		if (csn == sn)
			return 1;
	}

	return 0;
}

/*
 * Return ascii name of an affect bit vector.
 */
char *flag_room_name(int vector)
{
	return flag_names(room_flags, vector);
}

int affect_check_obj(CHAR_DATA *ch,int vector)
{
	AFFECT_DATA *paf;
	OBJ_DATA *obj;

	if (vector == 0) 	return 0;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
		if (obj->wear_loc == -1 || obj->wear_loc == WEAR_STUCK_IN)
		    continue;

	    for (paf = obj->affected; paf != NULL; paf = paf->next)
		    {
	        if (paf->bitvector == vector)
	            return 1;
	        }

	    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		    {
	        if (paf->bitvector == vector)
			return 1;
	        }
	}
  return 0;
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
	send_to_char("You are already controlling as many charmed mobs as you can!\n\r",ch);
	return count;
   }
  return 0;
}

/*
 * add_mind - remember 'str' in mind buffer of 'ch'
 *	      remember the place to return in mind buffer
 */
void add_mind(CHAR_DATA *ch, char *str)
{
		if (!IS_NPC(ch) || ch->in_room == NULL)
			return;

		if (ch->in_mind == NULL)
			/* remember a place to return */
			str_printf(&ch->in_mind, "%d", ch->in_room->vnum);

		if (!is_name(str, ch->in_mind))
			str_printf(&ch->in_mind, "%s %s", ch->in_mind, str);
}

/*
 * remove_mind - remove 'str' from mind buffer of 'ch'
 *		 if it was the last revenge - return home
 */
void remove_mind(CHAR_DATA *ch, char *str)
{
	char buf[MAX_STRING_LENGTH];
	char buff[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char *mind = ch->in_mind;

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


CHAR_DATA * find_char(CHAR_DATA *ch, char *argument,int door, int range) 
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
   send_to_char("You don't see that there.\n\r",ch);
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
	send_to_char("The path you choose prevents your power to pass.\n\r",ch);
	return NULL;
   }
  if ((target = get_char_room2(ch,dest_room,arg,&number)) != NULL) 
		return target;
 }

 send_to_char("You don't see that there.\n\r",ch);
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
CHAR_DATA *get_char_spell(CHAR_DATA *ch, char *argument, int *door, int range)
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

  ch->last_fight_time = current_time;
  if (!IS_NPC(victim)) victim->last_fight_time = current_time;

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
	do_track(victim,"");
  }
}


bool in_PK(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (IS_NPC(ch) || IS_NPC(victim))
		return TRUE;

	if (victim->level < PK_MIN_LEVEL || ch->level < PK_MIN_LEVEL)
		return FALSE;

	/* level adjustment */
	if (ch != victim && !IS_IMMORTAL(ch)
	&&  (ch->level >= (victim->level + UMAX(4,ch->level/10 +2)) ||
	     ch->level <= (victim->level - UMAX(4,ch->level/10 +2)))
	&&  (victim->level >= (ch->level + UMAX(4,victim->level/10 +2)) ||
	     victim->level <= (ch->level - UMAX(4,victim->level/10 +2))))
		return FALSE;

	return TRUE;
}


bool can_gate(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (victim == ch
	||  victim->in_room == NULL
	||  !can_see_room(ch, victim->in_room)
	||  IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||  IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	||  room_is_private(victim->in_room)
	||  IS_SET(ch->in_room->room_flags, ROOM_NOSUMMON)
	||  IS_SET(victim->in_room->room_flags, ROOM_NOSUMMON))
		return FALSE;

	if (IS_NPC(victim)) {
		if (IS_SET(victim->imm_flags, IMM_SUMMON))
			return FALSE;
	}
	else {
		if ((in_PK(ch, victim) && IS_SET(victim->act, PLR_NOSUMMON))
		||  victim->level >= LEVEL_HERO		/* not trust (!) */
		||  ch->in_room->area != victim->in_room->area
		||  guild_check(ch, victim->in_room) < 0)
			return FALSE;
	}

	return TRUE;
}

char *PERS(CHAR_DATA *ch, CHAR_DATA *looker)
{
	if (can_see(looker, ch)) {
		if (IS_NPC(ch))
			return ch->short_descr;
		else if (IS_VAMPIRE(ch) && !IS_IMMORTAL(looker))
			return "an ugly creature";
		return ch->name;
	}

	if (ch->level > MAX_LEVEL-7)
		return "an immortal";

	return "someone";
}

/*
 * Return ascii name of an raffect location.
 */
char *raffect_loc_name(int location)
{
	switch (location)
	{
	case APPLY_ROOM_NONE:	return "none";
	case APPLY_ROOM_HEAL:	return "heal rate";
	case APPLY_ROOM_MANA:	return "mana rate";
	}

	bug("Affect_location_name: unknown location %d.", location);
	return "(unknown)";
}

	 
/*
 * Return ascii name of an affect bit vector.
 */
char *raffect_bit_name(int vector)
{
	return flag_names(raff_flags, vector);
}

