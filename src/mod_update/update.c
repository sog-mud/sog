/*
 * $Id: update.c,v 1.160 1999-10-17 08:55:51 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT		           *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Mandrake}	canpunar@rorqual.cc.metu.edu.tr    *	
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
#include "merc.h"
#include "mob_prog.h"
#include "obj_prog.h"
#include "fight.h"
#include "chquest.h"
#include "auction.h"
#include "quest.h"

/*
 * Local functions.
 */
int	hit_gain	(CHAR_DATA *ch);
int	mana_gain	(CHAR_DATA *ch);
int	move_gain	(CHAR_DATA *ch);
void	mobile_update	(void);
void	weather_update	(void);
void	char_update	(void);
void	obj_update	(void);
void	aggr_update	(void);
void 	clan_item_update(void);
int	potion_cure_level	(OBJ_DATA *potion);
int	potion_arm_level	(OBJ_DATA *potion);
bool	is_potion		(OBJ_DATA *potion, const char *sn);

/* below done by chronos */
void    quest_update    args((void));
void	auction_update	args((void));
void	light_update	args((void));
void	room_update	args((void));
void	room_affect_update	args((void));
void	check_reboot	args((void));
void	track_update	args((void));

/* used for saving */
int	rebooter = 0;

/*
 * assumes !IS_NPC(ch)
 */
void advance_level(CHAR_DATA *ch)
{
	int add_hp;
	int add_mana;
	int add_move;
	int add_prac;
	class_t *cl;

	if (IS_NPC(ch)) {
		bug("Advance_level: a mob to advance!", 0);
		return;
	}

	if ((cl = class_lookup(ch->class)) == NULL) {
		log("advance_level: %s: unknown class %s",
		    ch->name, ch->class);
		return;
	}

	set_title(ch, title_lookup(ch));
	update_skills(ch);

	if (PC(ch)->plevels > 0) {
		PC(ch)->plevels--;
		return;
	}

	add_hp = (con_app[get_curr_stat(ch,STAT_CON)].hitp +
		  number_range(1,5)) - 3;
	add_hp = (add_hp * cl->hp_rate) / 100;
	add_mana = number_range(get_curr_stat(ch,STAT_INT)/2,
				(2*get_curr_stat(ch,STAT_INT) +
				 get_curr_stat(ch,STAT_WIS)/5));
	add_mana = (add_mana * cl->mana_rate) / 100;

	add_move = number_range(1, (get_curr_stat(ch,STAT_CON) +
				    get_curr_stat(ch,STAT_DEX)) / 6);
	add_prac = wis_app[get_curr_stat(ch,STAT_WIS)].practice;

	add_hp = UMAX(3, add_hp);
	add_mana = UMAX(3, add_mana);
	add_move = UMAX(6, add_move);

	if (ch->sex == SEX_FEMALE) {
		add_hp   -= 1;
		add_mana += 2;
	}

	ch->max_hit += add_hp;
	ch->max_mana += add_mana;
	ch->max_move += add_move;
	PC(ch)->practice += add_prac;
	PC(ch)->train += ch->level % 5 ? 0 : 1;

	PC(ch)->perm_hit += add_hp;
	PC(ch)->perm_mana += add_mana;
	PC(ch)->perm_move += add_move;

	char_printf(ch, "Your gain is {C%d{x hp, {C%d{x mana, {C%d{x mv {C%d{x prac.\n",
			add_hp, add_mana, add_move, add_prac);
}   

/*
 * assumes !IS_NPC(victim)
 */
void advance(CHAR_DATA *victim, int level)
{
	int iLevel;
	int tra;
	int pra;

	tra = PC(victim)->train;
	pra = PC(victim)->practice;
	PC(victim)->plevels = 0;

	/*
	 * Lower level:
	 *   Reset to level 1.
	 *   Then raise again.
	 *   Currently, an imp can lower another imp.
	 *   -- Swiftest
	 */
	if (level <= victim->level) {
		int temp_prac;

		char_puts("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n", victim);
		temp_prac = PC(victim)->practice;
		victim->level		= 1;
		PC(victim)->exp	= base_exp(victim);
		victim->max_hit		= 10;
		victim->max_mana	= 100;
		victim->max_move	= 100;
		PC(victim)->practice= 0;
		victim->hit		= victim->max_hit;
		victim->mana		= victim->max_mana;
		victim->move		= victim->max_move;
		PC(victim)->perm_hit	= victim->max_hit;
		PC(victim)->perm_mana	= victim->max_mana;
		PC(victim)->perm_move	= victim->max_move;
		advance_level(victim);
		PC(victim)->practice= temp_prac;
	}
	else 
		char_puts("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n", victim);

	for (iLevel = victim->level; iLevel < level; iLevel++) {
		char_puts("{CYou raise a level!!{x ", victim);
		PC(victim)->exp += exp_to_level(victim);
		victim->level++;
		advance_level(victim);
	}
	PC(victim)->exp_tl	= 0;
	PC(victim)->train	= tra;
	PC(victim)->practice= pra;
	char_save(victim, 0);
}

/*
 * assumes !IS_NPC(ch)
 */
void gain_exp(CHAR_DATA *ch, int gain)
{
	if (ch->level >= LEVEL_HERO)
		return;

	if (IS_SET(PC(ch)->plr_flags, PLR_NOEXP) && gain > 0) {
		char_puts("You can't gain exp without your spirit.\n", ch);
		return;
	}

	PC(ch)->exp += gain;
	PC(ch)->exp_tl += gain;

	while (ch->level < LEVEL_HERO && exp_to_level(ch) <= 0) {
		class_t *cl;

		char_puts("{CYou raise a level!!{x ", ch);
		ch->level++;
		PC(ch)->exp_tl = 0;

		if ((cl = class_lookup(ch->class)) != NULL
		&&  cl->death_limit != 0
		&&  ch->level == LEVEL_PK)
			ch->wimpy = 0;

		if (ch->level == 91)
	        	log("%s made level 91.", ch->name);

		wiznet("$N has attained level $j!",
			ch, (const void*) ch->level, WIZ_LEVELS, 0, 0);
		advance_level(ch);
		char_save(ch, 0);
	}
}

/*
 * Regeneration stuff.
 */
int hit_gain(CHAR_DATA *ch)
{
	int gain;
	int number;

	if (IS_NPC(ch)) {
		gain =  5 + ch->level;
 		if (IS_AFFECTED(ch, AFF_REGENERATION))
			gain *= 2;

		switch(ch->position) {
		default:		gain /= 2;		break;
		case POS_SLEEPING:	gain = 3 * gain/2;	break;
		case POS_RESTING:				break;
		case POS_FIGHTING:	gain /= 3;		break;
 		}
	} else {
		class_t *cl;

		if ((cl = class_lookup(ch->class)) == NULL)
			return 0;

		gain = UMAX(3, 2 * get_curr_stat(ch, STAT_CON) +
			       (7 * ch->level) / 4); 
		gain = (gain * cl->hp_rate) / 100;
 		number = number_percent();
		if (number < get_skill(ch, "fast healing")) {
			gain += number * gain / 100;
			if (ch->hit < ch->max_hit)
				check_improve(ch, "fast healing", TRUE, 8);
		}

		if (number < get_skill(ch, "trance")) {
			gain += number * gain / 150;
			if (ch->hit < ch->max_hit)
				check_improve(ch, "trance", TRUE, 8);
		}

		switch (ch->position) {
		default:		gain /= 4;		break;
		case POS_SLEEPING: 				break;
		case POS_RESTING:	gain /= 2;		break;
		case POS_FIGHTING:	gain /= 6;		break;
		}

		if (PC(ch)->condition[COND_HUNGER]   < 0)
			gain = 0;

		if (PC(ch)->condition[COND_THIRST] < 0)
			gain = 0;
	}

	gain = gain * ch->in_room->heal_rate / 100;
	
	if (ch->on != NULL && ch->on->pObjIndex->item_type == ITEM_FURNITURE)
		gain = gain * INT_VAL(ch->on->value[3]) / 100;

	if (IS_AFFECTED(ch, AFF_POISON))
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED(ch, AFF_HASTE))
		gain /= 2;

	if (IS_AFFECTED(ch, AFF_SLOW))
		gain *= 2;

	if (get_curr_stat(ch, STAT_CON) > 20)
		gain = (gain * 14) / 10;

	if (IS_HARA_KIRI(ch))
		gain *= 3;

	return UMIN(gain, ch->max_hit - ch->hit);
}

int mana_gain(CHAR_DATA *ch)
{
	int gain;
	int number;

	if (IS_NPC(ch)) {
		gain = 5 + ch->level;
		switch (ch->position) {
		default:		gain /= 2;		break;
		case POS_SLEEPING:	gain = 3 * gain/2;	break;
		case POS_RESTING:				break;
		case POS_FIGHTING:	gain /= 3;		break;
		}
	} else {
		class_t *cl;

		if ((cl = class_lookup(ch->class)) == NULL)
			return 0;

		gain = get_curr_stat(ch, STAT_WIS) +
		       (2 * get_curr_stat(ch, STAT_INT)) + ch->level;
		gain = (gain * cl->mana_rate) / 100;
		number = number_percent();
		if (number < get_skill(ch, "meditation")) {
			gain += number * gain / 100;
			if (ch->mana < ch->max_mana)
				check_improve(ch, "meditation", TRUE, 8);
		}

		if (number < get_skill(ch, "trance")) {
			gain += number * gain / 100;
			if (ch->mana < ch->max_mana)
				check_improve(ch, "trance", TRUE, 8);
		}

		if (!IS_SET(cl->class_flags, CLASS_MAGIC))
			gain /= 2;

		switch (ch->position) {
		default:		gain /= 4;	break;
		case POS_SLEEPING: 			break;
		case POS_RESTING:	gain /= 2;	break;
		case POS_FIGHTING:	gain /= 6;	break;
		}

		if (PC(ch)->condition[COND_HUNGER] < 0
		||  PC(ch)->condition[COND_THIRST] < 0)
			gain = 0;
	}

	gain = gain * ch->in_room->mana_rate / 100;

	if (ch->on != NULL && ch->on->pObjIndex->item_type == ITEM_FURNITURE)
		gain = gain * INT_VAL(ch->on->value[4]) / 100;

	if (IS_AFFECTED(ch, AFF_POISON))
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED(ch, AFF_HASTE))
		gain /= 2;

	if (IS_AFFECTED(ch, AFF_SLOW))
		gain *= 2;

	if (get_curr_stat(ch, STAT_INT) > 20)
		gain = (gain * 13) / 10;
	if (get_curr_stat(ch, STAT_WIS) > 20)
		gain = (gain * 11) / 10;
	if (IS_HARA_KIRI(ch))
		gain *= 3;
	if (is_affected(ch, "lich"))
		gain -= 2*ch->level;

	return UMIN(gain, ch->max_mana - ch->mana);
}

int move_gain(CHAR_DATA *ch)
{
	int gain;

	if (ch->in_room == NULL)
		return 0;

	if (IS_NPC(ch))
		gain = ch->level;
	else {
		gain = UMAX(15, 2 * ch->level);

		switch (ch->position) {
		case POS_SLEEPING:
			gain += 2 * get_curr_stat(ch, STAT_DEX);
			break;

		case POS_RESTING:
			gain += get_curr_stat(ch, STAT_DEX);
			break;
		}

		if (PC(ch)->condition[COND_HUNGER] < 0
		||  PC(ch)->condition[COND_THIRST] < 0)
			gain = 3;
	}

	gain = gain * ch->in_room->heal_rate/100;

	if (ch->on != NULL && ch->on->pObjIndex->item_type == ITEM_FURNITURE)
		gain = gain * INT_VAL(ch->on->value[3]) / 100;

	if (IS_AFFECTED(ch, AFF_POISON))
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED(ch, AFF_HASTE | AFF_SLOW))
		gain /= 2;

	if (get_curr_stat(ch, STAT_DEX) > 20)
		gain = gain * 14 / 10;

	if (IS_HARA_KIRI(ch))
		gain *= 3;

	return UMIN(gain, ch->max_move - ch->move);
}

void gain_condition(CHAR_DATA *ch, int iCond, int value)
{
	int condition;
	int damage_hunger;
	int fdone;
	CHAR_DATA *vch,*vch_next;

	if (value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return;

	if (IS_VAMPIRE(ch)
	&&  (iCond == COND_THIRST ||
	     iCond == COND_FULL ||
	     iCond == COND_HUNGER))
		return;

	condition = PC(ch)->condition[iCond];

	PC(ch)->condition[iCond] = URANGE(-6, condition + value, 96);

	if (iCond == COND_FULL && (PC(ch)->condition[COND_FULL] < 0))
		PC(ch)->condition[COND_FULL] = 0;

	if ((iCond == COND_DRUNK) && (PC(ch)->condition[COND_DRUNK] < 1)) 
		PC(ch)->condition[COND_DRUNK] = 0;

	if (PC(ch)->condition[iCond] < 1
	&&  PC(ch)->condition[iCond] > -6) {
		switch (iCond) {
		case COND_HUNGER:
			char_puts("You are hungry.\n",  ch);
			break;

		case COND_THIRST:
			char_puts("You are thirsty.\n", ch);
			break;
	 
		case COND_DRUNK:
			if (condition != 0)
				char_puts("You are sober.\n", ch);
			break;

		case COND_BLOODLUST:
			if (condition != 0)
				char_puts("You are hungry for blood.\n",
					     ch);
			break;

		case COND_DESIRE:
			if (condition != 0)
				char_puts("You have missed your home.\n",
					     ch);
			break;
		}
	}

	if (PC(ch)->condition[iCond] == -6 && ch->level >= LEVEL_PK) {
		switch (iCond) {
		case COND_HUNGER:
			char_puts("You are starving!\n",  ch);
			act("$n is starving!",  ch, NULL, NULL, TO_ROOM);
			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, TYPE_UNDEFINED,
			       DAM_HUNGER, DAMF_SHOW | DAMF_HUNGER);
			if (ch->position == POS_SLEEPING) 
				return;       
			break;

		case COND_THIRST:
			char_puts("You are dying of thrist!\n", ch);
			act("$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, TYPE_UNDEFINED,
				DAM_THIRST, DAMF_SHOW | DAMF_HUNGER);
			if (ch->position == POS_SLEEPING) 
				return;       
			break;

		case COND_BLOODLUST:
			fdone = 0;
			char_puts("You are suffering from thrist of blood!\n",
				  ch);
			act("$n is suffering from thirst of blood!",
			    ch, NULL, NULL, TO_ROOM);
			if (ch->in_room && ch->in_room->people
			&&  ch->fighting == NULL) {
				if (!IS_AWAKE(ch))
					dofun("stand", ch, str_empty);
			        for (vch = ch->in_room->people;
			       	     vch != NULL && ch->fighting == NULL;
				     vch = vch_next) {
			        	vch_next = vch->next_in_room;

					if (IS_IMMORTAL(vch))
						continue;

			        	if (ch != vch && can_see(ch, vch)
					&&  !is_safe_nomessage(ch, vch)) {
						dofun("yell", ch,
						      "BLOOD! I NEED BLOOD!");
						dofun("murder", ch, vch->name);
						fdone = 1;
					}
			         }
			}

			if (fdone)
				break;

			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, TYPE_UNDEFINED,
				DAM_THIRST, DAMF_SHOW | DAMF_HUNGER);
			if (ch->position == POS_SLEEPING) 
				return;       		
			break;

		case COND_DESIRE:
			char_puts("You want to go your home!\n", ch);
			act("$n desires for $s home!", ch, NULL, NULL, TO_ROOM);
			if (ch->position >= POS_STANDING) 
				move_char(ch, number_door(), FALSE);
			break;
		}
	}
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update(void)
{
	CHAR_DATA *ch, *ch_next;
	EXIT_DATA *pexit;
	int door;
	OBJ_DATA *obj;

	/* Examine all mobs. */
	for (ch = char_list; ch; ch = ch_next) {
		bool bust_prompt = FALSE;
		flag64_t act;

		ch_next = ch->next;

		if (IS_EXTRACTED(ch)) {
			log("mobile_update: extracted char");
			continue;
		}

		if (ch->position == POS_FIGHTING)
			SET_FIGHT_TIME(ch);

		if (!IS_NPC(ch)) {
/* update ghost state */
			if (ch->last_death_time != -1
			&&  current_time - ch->last_death_time >=
							GHOST_DELAY_TIME
			&&  IS_SET(PC(ch)->plr_flags, PLR_GHOST)) {
				char_puts("You return to your normal form.\n",
					  ch);
				REMOVE_BIT(PC(ch)->plr_flags, PLR_GHOST);
			}
		}

/* update pumped state */
		if (ch->last_fight_time != -1
		&&  current_time - ch->last_fight_time >= FIGHT_DELAY_TIME
		&&  IS_PUMPED(ch)) {
			if (!IS_NPC(ch) && ch->desc != NULL
			&&  ch->desc->pString == NULL 
			&&  (ch->last_death_time == -1 ||
			     ch->last_death_time < ch->last_fight_time)) {
				REMOVE_BIT(PC(ch)->plr_flags, PLR_PUMPED);
				act("You settle down.",
				    ch, NULL, NULL, TO_CHAR);
			}
		}

		if (IS_AFFECTED(ch, AFF_REGENERATION) && ch->in_room != NULL) {
			ch->hit = UMIN(ch->hit + ch->level / 10, ch->max_hit);
			if (IS_RACE(ch->race, "troll"))
				ch->hit = UMIN(ch->hit + ch->level / 10,
					       ch->max_hit);
			if (ch->hit != ch->max_hit)
				bust_prompt = TRUE;
		}

		if (IS_AFFECTED(ch, AFF_CORRUPTION) && ch->in_room != NULL) {
			ch->hit -=  ch->level / 10;
			if (ch->hit < 1) {
				if (IS_IMMORTAL(ch)) 
					ch->hit = 1;
				else {
					ch->position = POS_DEAD;
					handle_death(ch, ch);
					continue;
				}
			}
			bust_prompt = TRUE;
		}

		if (ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER
		&& !IS_AFFECTED(ch, AFF_WATER_BREATHING)
		&& !IS_IMMORTAL(ch)) {
			act("$n gasps for fresh air, but inhales water.",
				ch, NULL, NULL, TO_ROOM);
			act("You gasp for fresh air, but inhale water.",
				ch, NULL, NULL, TO_CHAR);
			ch->hit -= ch->max_hit/20;
			if (ch->hit < 1) {
				ch->position = POS_DEAD;
				handle_death(ch, ch);
				continue;
			}
			bust_prompt = TRUE;
		}

		check_events(ch, ch->affected, EVENT_CHAR_UPDFAST);
		if (IS_EXTRACTED(ch)) continue;

		if (ch->desc
		&&  bust_prompt
		&&  !ch->desc->pString
		&&  !ch->desc->showstr_point
		&&  !IS_SET(ch->comm, COMM_NOBUST))
			char_puts(str_empty, ch);

		/*
		 * that's all for PCs and charmed mobiles
		 */
		if (!IS_NPC(ch)
		||  IS_AFFECTED(ch, AFF_CHARM))
			continue;

		act = ch->pMobIndex->act;
		if (IS_SET(act, ACT_HUNTER) && ch->hunting) {
			dofun("hunt", ch, str_empty);
			if (IS_EXTRACTED(ch))
				continue;
		}

		if (ch->in_room->area->empty
		&&  !IS_SET(act, ACT_UPDATE_ALWAYS))
			continue;

		/* Examine call for special procedure */
		if (ch->pMobIndex->spec_fun != 0) {
			if (ch->pMobIndex->spec_fun(ch))
				continue;
		}

		if (ch->pMobIndex->pShop != NULL /* give him some gold */
		||  (ch->gold * 100 + ch->silver) < ch->pMobIndex->wealth) {
			ch->gold += ch->pMobIndex->wealth * number_range(1,20)/5000000;
			ch->silver += ch->pMobIndex->wealth * number_range(1,20)/50000;
		}
	 
/* check triggers (only if mobile still in default position) */

		if (ch->position == ch->pMobIndex->default_pos) {
			if (HAS_TRIGGER(ch, TRIG_DELAY)
			&&  NPC(ch)->mprog_delay > 0) {
				if (--NPC(ch)->mprog_delay <= 0) {
					mp_percent_trigger(ch, NULL, NULL,
							   NULL, TRIG_DELAY);
					continue;
				}
			} 
			if (HAS_TRIGGER(ch, TRIG_RANDOM)) {
				if(mp_percent_trigger(ch, NULL, NULL,
						      NULL, TRIG_RANDOM))
					continue;
			}
		}

/* potion using and stuff for intelligent mobs */

		if (ch->pMobIndex->pShop == NULL
		&&  (ch->position == POS_STANDING ||
		     ch->position == POS_RESTING ||
		     ch->position == POS_FIGHTING)
		&&  get_curr_stat(ch, STAT_INT) > 15
		&&  (ch->hit < ch->max_hit * 90 / 100 ||
		     IS_AFFECTED(ch, AFF_BLIND) ||
		     IS_AFFECTED(ch, AFF_POISON) ||
		     IS_AFFECTED(ch, AFF_PLAGUE) ||
		     ch->fighting != NULL)) {
			for (obj = ch->carrying; obj; obj = obj->next_content) {
				if (obj->pObjIndex->item_type != ITEM_POTION)
					continue;

				if (ch->hit < ch->max_hit * 90 / 100) {
					int cl = potion_cure_level(obj);
					if (cl > 0) {
						if (ch->hit < ch->max_hit*0.5
						&&  cl > 3) {
							quaff_obj(ch, obj);
							continue;
						}
						if (ch->hit < ch->max_hit*0.7) {
							quaff_obj(ch, obj);
							continue;
						}
					}
				}

				if (IS_AFFECTED(ch, AFF_POISON)
				&&  is_potion(obj, "cure poison")) {
					quaff_obj(ch, obj);
					continue;
				}

				if (IS_AFFECTED(ch, AFF_PLAGUE)
				&&  is_potion(obj, "cure disease")) {
					quaff_obj(ch, obj);
					continue;
				}

				if (IS_AFFECTED(ch, AFF_BLIND)
				&&  is_potion(obj, "cure blindness")) {
					quaff_obj(ch, obj);
					continue;
				}

				if (ch->fighting) {
					int al = potion_arm_level(obj);

					if (ch->level - ch->fighting->level < 7
					&&  al > 3) {
						quaff_obj(ch, obj);
						continue;
					}

					if (ch->level - ch->fighting->level < 8
					&&  al > 2) {
						quaff_obj(ch, obj);
						continue;
					}

					if (ch->level - ch->fighting->level < 9
					&&  al > 1) {
						quaff_obj(ch, obj);
						continue;
					}

					if (ch->level - ch->fighting->level < 10
					&&  al > 0) {
						quaff_obj(ch, obj);
						continue;
					}
				}
			}
		}

/* That's all for sleeping / busy monster, and empty zones */
		if (ch->position != POS_STANDING)
			continue;

/* Scavenge */
		if (IS_SET(act, ACT_SCAVENGER)
		&&  ch->in_room->contents != NULL
		&&  number_bits(6) == 0) {
			OBJ_DATA *obj;
			OBJ_DATA *obj_best = NULL;
			int max = 1;

			for (obj = ch->in_room->contents; obj;
			     obj = obj->next_content) {
				if (CAN_WEAR(obj, ITEM_TAKE)
				&&  can_loot(ch, obj)
				&&  obj->cost > max) {
					obj_best = obj;
					max	 = obj->cost;
				}
			}

			if (obj_best)
				get_obj(ch, obj_best, NULL, NULL);
		}

/* Wander */
		if (!IS_SET(act, ACT_SENTINEL) 
		&&  number_bits(3) == 0
		&&  (door = number_bits(5)) <= 5
		&&  !RIDDEN(ch)
		&&  (pexit = ch->in_room->exit[door]) != NULL
		&&  pexit->to_room.r != NULL
		&&  !IS_SET(pexit->exit_info, EX_CLOSED)
		&&  !IS_SET(pexit->to_room.r->room_flags, ROOM_NOMOB)
		&&  (!IS_SET(act, ACT_STAY_AREA) ||
		     pexit->to_room.r->area == ch->in_room->area) 
		&&  (!IS_SET(act, ACT_AGGRESSIVE) ||
		     !IS_SET(pexit->to_room.r->room_flags, ROOM_PEACE))
		&&  (!IS_SET(act, ACT_OUTDOORS) ||
		     !IS_SET(pexit->to_room.r->room_flags, ROOM_INDOORS)) 
		&&  (!IS_SET(act, ACT_INDOORS) ||
		     IS_SET(pexit->to_room.r->room_flags, ROOM_INDOORS)))
			move_char(ch, door, FALSE);
	}
}

int potion_cure_level(OBJ_DATA *potion)
{
int cl;
int i;
  cl = 0;
  for (i=1;i<5;i++)
  {
	if (SKILL_IS(potion->value[i].s, "cure critical"))
	  cl += 3;
	if (SKILL_IS(potion->value[i].s, "cure light"))
	  cl += 1;
	if (SKILL_IS(potion->value[i].s, "cure serious"))
	  cl += 2;
	if (SKILL_IS(potion->value[i].s, "heal"))
	  cl += 4;
  }
  return(cl);
}
int potion_arm_level(OBJ_DATA *potion)
{
int al;
int i;
  al = 0;
  for (i=1;i<5;i++)
  {
	if (SKILL_IS(potion->value[i].s, "armor"))
	  al += 1;
	if (SKILL_IS(potion->value[i].s, "shield"))
	  al += 1;
	if (SKILL_IS(potion->value[i].s, "stone skin"))
	  al += 2;
	if (SKILL_IS(potion->value[i].s, "sanctuary"))
	  al += 4;
	if (SKILL_IS(potion->value[i].s, "protection"))
	  al += 3;
  }
  return(al);
}

bool is_potion(OBJ_DATA *potion, const char *sn)
{
	int i;
	for (i = 0; i < 5; i++) {
		if (SKILL_IS(potion->value[i].s, sn))
			return TRUE;
	}
	return FALSE;
}

/*
 * Update the weather.
 */
void weather_update(void)
{
	CHAR_DATA *ch;
	int diff;
	const char *msg_daytime = NULL;
	const char *msg_weather = NULL;

	switch (++time_info.hour) {
	case  5:
		weather_info.sunlight = SUN_LIGHT;
		msg_daytime = "The day has begun.";
		break;

	case  6:
		weather_info.sunlight = SUN_RISE;
		msg_daytime = "The sun rises in the east.";
		break;

	case 19:
		weather_info.sunlight = SUN_SET;
		msg_daytime = "The sun slowly disappears in the west.";
		break;

	case 20:
		weather_info.sunlight = SUN_DARK;
		msg_daytime = "The night has begun.";
		break;

	case 24:
		time_info.hour = 0;
		time_info.day++;
		break;
	}

	if (time_info.day >= 35) {
		time_info.day = 0;
		time_info.month++;
	}

	if (time_info.month >= 17) {
		time_info.month = 0;
		time_info.year++;
	}

	/*
	 * Weather change.
	 */
	if (time_info.month >= 9 && time_info.month <= 16)
		diff = weather_info.mmhg >  985 ? -2 : 2;
	else
		diff = weather_info.mmhg > 1015 ? -2 : 2;

	weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
	weather_info.change    = UMAX(weather_info.change, -12);
	weather_info.change    = UMIN(weather_info.change,  12);

	weather_info.mmhg += weather_info.change;
	weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
	weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

	switch (weather_info.sky) {
	default: 
		bug("Weather_update: bad sky %d.", weather_info.sky);
		weather_info.sky = SKY_CLOUDLESS;
		break;

	case SKY_CLOUDLESS:
		if (weather_info.mmhg < 990
		||  (weather_info.mmhg < 1010 && number_bits(2) == 0)) {
			msg_weather = "The sky is getting cloudy.";
			weather_info.sky = SKY_CLOUDY;
		}
		break;

	case SKY_CLOUDY:
		if (weather_info.mmhg < 970
		||  (weather_info.mmhg < 990 && number_bits(2) == 0)) {
			msg_weather = "It starts to rain.";
			weather_info.sky = SKY_RAINING;
		}
		else if (weather_info.mmhg > 1030 && number_bits(2) == 0) {
			msg_weather = "The clouds disappear.";
			weather_info.sky = SKY_CLOUDLESS;
		}
		break;

	case SKY_RAINING:
		if (weather_info.mmhg > 1030
		||  (weather_info.mmhg > 1010 && number_bits(2) == 0)) {
			msg_weather = "The rain stopped.";
			weather_info.sky = SKY_CLOUDY;
		}
		else if (weather_info.mmhg < 970 && number_bits(2) == 0) {
			msg_weather = "Lightning flashes in the sky.";
			weather_info.sky = SKY_LIGHTNING;
		}
		break;

	case SKY_LIGHTNING:
		if (weather_info.mmhg > 1010
		||  (weather_info.mmhg > 990 && number_bits(2) == 0)) {
			msg_weather = "The lightning has stopped.";
			weather_info.sky = SKY_RAINING;
		}
		break;
	}

	if (!msg_daytime && !msg_weather)
		return;

	for (ch = char_list; ch; ch = ch->next) {
		if (IS_NPC(ch) && !HAS_TRIGGER(ch, TRIG_ACT))
			continue;
		if (!IS_OUTSIDE(ch) || !IS_AWAKE(ch))
			continue;

		act(msg_daytime, ch, NULL, NULL, TO_CHAR);
		act(msg_weather, ch, NULL, NULL, TO_CHAR);
	}
}

/*
 * Update all chars, including mobs.
*/
void char_update(void)
{   
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;

	static time_t last_save_time = -1;

	for (ch = char_list; ch; ch = ch_next) {
		AFFECT_DATA *paf;
		AFFECT_DATA *paf_next;
		int chance;
		race_t *r = race_lookup(ch->race);

		ch_next = ch->next;
		if (!r)
			continue;

		/* reset path find */
		if (!IS_NPC(ch) && (chance = get_skill(ch, "path find"))) {
			if (number_percent() < chance) {
				ch->endur += chance / 2;
				check_improve(ch, "path find", TRUE, 8);
			}
			else
				check_improve(ch, "path find", FALSE, 16);
		}
		
		if (!ch->fighting) {
			flag64_t skip = AFF_FLYING;

			affect_check(ch, TO_AFFECTS, -1);

			/* Remove caltrops effect after fight off */
			if (is_affected(ch, "caltrops"))
				affect_strip(ch, "caltrops");

			if (!MOUNTED(ch)) {
				if (!IS_AFFECTED(ch, AFF_HIDE) 
				&&  (r->aff & AFF_HIDE))
					char_puts("You step back into the shadows.\n", ch);

				if (!IS_AFFECTED(ch, AFF_SNEAK)
				&&  (r->aff & AFF_SNEAK))
					char_puts("You move silently again.\n", ch);
			}
			else
				skip |= AFF_HIDE | AFF_FADE | AFF_INVIS |
					AFF_IMP_INVIS | AFF_SNEAK |
					AFF_CAMOUFLAGE;

			SET_BIT(ch->affected_by, r->aff & ~skip);
		}

		/* Remove vampire effect when morning. */
		if (is_affected(ch, "vampire")
		&&  (weather_info.sunlight == SUN_LIGHT ||
		     weather_info.sunlight == SUN_RISE))
			dofun("human", ch, str_empty);

		if (!IS_NPC(ch) && is_affected(ch, "thumbling")) {
			if (dice(5, 6) > get_curr_stat(ch, STAT_DEX)) {
				act("You failed to reach the true source of tennis ball power.", ch, NULL, NULL, TO_CHAR);
				act("$n falls to the ground flat on $s face.", ch, NULL, NULL, TO_ROOM);
				affect_strip(ch, "thumbling");
			}
		}

		if (ch->position >= POS_STUNNED) {
			int old_hit = ch->hit;
			int old_mana = ch->mana;
			int old_move = ch->move;
			NPC_DATA *npc;

			/* check to see if we need to go home */
			if (IS_NPC(ch)
			&&  (npc = NPC(ch))->zone != NULL 
			&&  ch->in_room != NULL
			&&  npc->zone != ch->in_room->area
			&&  ch->desc == NULL 
			&&  ch->fighting == NULL
/* && ch->progtypes==0 */
			&&  !IS_AFFECTED(ch, AFF_CHARM)
			&&  npc->last_fought == NULL
			&&  !RIDDEN(ch)) {
				back_home(ch);
				continue;
			}

			if (ch->hit < ch->max_hit)
				ch->hit += hit_gain(ch);
			else
				ch->hit = ch->max_hit;

			if (ch->mana < ch->max_mana)
				ch->mana += mana_gain(ch);
			else
				ch->mana = ch->max_mana;

			if (ch->move < ch->max_move)
				ch->move += move_gain(ch);
			else
				ch->move = ch->max_move;

			if (ch->desc
			&&  (old_hit != ch->hit || old_mana != ch->mana ||
			     old_move != ch->move)
			&&  !ch->desc->pString
			&&  !ch->desc->showstr_point
			&&  !IS_SET(ch->comm, COMM_NOBUST))
				char_puts(str_empty, ch);
		}

		if (ch->position == POS_STUNNED)
			update_pos(ch);

		if (!IS_NPC(ch) && ch->level < LEVEL_IMMORTAL) {
			OBJ_DATA *obj;
			PC_DATA *pc = PC(ch);

			if ((obj = get_eq_char(ch, WEAR_LIGHT))
			&&  obj->pObjIndex->item_type == ITEM_LIGHT
			&&  INT_VAL(obj->value[2]) > 0) {
				if (--INT_VAL(obj->value[2]) == 0) {
					if (ch->in_room->light > 0)
						--ch->in_room->light;
					act("$p goes out.",
					    ch, obj, NULL, TO_ROOM);
					act("$p flickers and goes out.",
					    ch, obj, NULL, TO_CHAR);
					extract_obj(obj, 0);
				} else if (INT_VAL(obj->value[2]) <= 5)
					act("$p flickers.",
					    ch, obj, NULL, TO_CHAR);
			}

			if (++pc->idle_timer >= 12) {
				if (pc->was_in_room == NULL) {
					pc->was_in_room = ch->in_room;
					if (ch->fighting != NULL)
						stop_fighting(ch, TRUE);
					act("$n disappears into the void.",
					    ch, NULL, NULL, TO_ROOM);
					char_puts("You disappear "
						  "into the void.\n", ch);
					char_save(ch, 0);
  					char_from_room(ch);
					char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
					if (IS_EXTRACTED(ch))
						continue;
				}
			}

			if (!pc->was_in_room) {
				gain_condition(ch, COND_DRUNK, -1);
				if (IS_VAMPIRE(ch))
					gain_condition(ch, COND_BLOODLUST, -1);
				gain_condition(ch, COND_FULL, 
					     ch->size > SIZE_MEDIUM ? -4 : -2);
				if (ch->in_room->sector_type == SECT_DESERT)
					gain_condition(ch, COND_THIRST, -3);
				else
					gain_condition(ch, COND_THIRST, -1);
				gain_condition(ch, COND_HUNGER, 
					     ch->size > SIZE_MEDIUM ? -2 : -1);
				if (IS_EXTRACTED(ch))
					continue;
			}
		}

		for (paf = ch->affected; paf != NULL; paf = paf_next) {
			paf_next = paf->next;
			if (paf->duration > 0) {
				paf->duration--;
				if (number_range(0, 4) == 0 && paf->level > 0)
					paf->level--;
				/* spell strength fades with time */
			}
			else if (paf->duration == 0) {
				skill_t *sk;

				if ((paf_next == NULL ||
				     paf_next->type != paf->type ||
				     paf_next->duration > 0)
				&&  paf->type > 0
				&&  (sk = skill_lookup(paf->type))
				&&  !IS_NULLSTR(sk->msg_off)) 
					act_puts(sk->msg_off, ch, NULL, NULL,
						 TO_CHAR, POS_DEAD);

				check_one_event(ch, paf, EVENT_CHAR_TIMEOUT);
				if (IS_EXTRACTED(ch))
					break;

				affect_remove(ch, paf);
			}
		}
		check_events(ch, ch->affected, EVENT_CHAR_UPDATE);

		if (IS_EXTRACTED(ch)) continue;

		if (ch->position == POS_INCAP 
		&&  number_range(0, 1) == 0)
			damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
		else if (ch->position == POS_MORTAL)
			damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
	} /* global for */

	/*
	 * Autosave and autoquit.
	 * Check that these chars still exist.
	 */
	if (last_save_time == -1 || current_time - last_save_time > 300) {
		last_save_time = current_time;
		for (ch = char_list; ch && !IS_NPC(ch); ch = ch_next) {
			ch_next = ch->next;
			if (PC(ch)->idle_timer > 20)
				quit_char(ch, 0);
			else
				char_save(ch, 0);
		}
	}
}

void water_float_update(void)
{
	OBJ_DATA *obj_next;
	OBJ_DATA *obj;
	CHAR_DATA *ch;

	for (obj = object_list; obj != NULL; obj = obj_next) {
		obj_next = obj->next;

		if (!obj->in_room || !IS_WATER(obj->in_room))
			continue;

		obj->water_float = obj->water_float > 0 ?
						obj->water_float - 1 : -1;

		if (obj->pObjIndex->item_type == ITEM_DRINK_CON) {
			INT_VAL(obj->value[1]) =
				URANGE(1, INT_VAL(obj->value[1]) + 8,
				       INT_VAL(obj->value[0]));
			if ((ch = obj->in_room->people))
				act("$p makes bubbles on the water.", ch, obj,
				    NULL, TO_ALL);
			obj->water_float = INT_VAL(obj->value[0]) -
					   INT_VAL(obj->value[1]);
			obj->value[2] = 0;
		}
		if (obj->water_float == 0) {
			if((ch = obj->in_room->people))
				act("$p sinks down the water.", ch, obj, NULL,
				    TO_ALL); 
			extract_obj(obj, 0);
		}
	}
}

void update_obj_affects(OBJ_DATA *obj)
{
	AFFECT_DATA *paf, *paf_next;

	/* go through affects and decrement */
	for (paf = obj->affected; paf != NULL; paf = paf_next) {
		paf_next    = paf->next;
		if (paf->duration > 0) {
        		paf->duration--;
			/* spell strength fades with time */
        		if (number_range(0,4) == 0 && paf->level > 0)
				paf->level--;
        	}
		else if (paf->duration == 0) {
			skill_t *sk;

			if ((paf_next == NULL || paf_next->type != paf->type ||
			     paf_next->duration > 0)
			&&  paf->type > 0
			&&  (sk = skill_lookup(paf->type))
			&&  !IS_NULLSTR(sk->msg_obj)) {
				if (obj->carried_by != NULL) 
					act(sk->msg_obj, obj->carried_by,
					   obj, NULL, TO_CHAR);

				if (obj->in_room != NULL 
				&&  obj->in_room->people)
					act(sk->msg_obj, obj->in_room->people,
					   obj, NULL, TO_ALL);
                	}
			affect_remove_obj(obj, paf);
        	}
	}
}

bool update_ice_obj(OBJ_DATA *obj)
{
	if (obj->carried_by != NULL
	&&  obj->carried_by->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 40) {
		act("The extreme heat melts $p.", obj->carried_by, obj, NULL,
		    TO_CHAR);
		extract_obj(obj, 0);
		return TRUE;
	}
	if (obj->in_room != NULL
	&&  obj->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 50) {
		act("The extreme heat melts $p.", obj->in_room->people, obj,
		    NULL, TO_ALL);
		extract_obj(obj, 0);
		return TRUE;
	}
	return FALSE;
}

bool update_glass_obj(OBJ_DATA *obj)
{
	if (obj->carried_by
	&&  obj->carried_by->in_room->sector_type == SECT_DESERT
	&&  !IS_NPC(obj->carried_by)
	&&  number_percent() < 20)  {
		act("$p evaporates.", obj->carried_by, obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
		return TRUE;
	}
	if (obj->in_room
	&&  obj->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 30) {
		act("$p evaporates by the extream heat.", obj->in_room->people,
		    obj, NULL, TO_ROOM);
		act("$p evaporates by the extream heat.", obj->in_room->people,
		    obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
		return TRUE;
	}
	return FALSE;
}

void update_pit(OBJ_DATA *obj)
{
	OBJ_DATA *t_obj, *next_obj;
	static int pit_count = 1;
	/* more or less an hour */
	pit_count = ++pit_count % 120;

	if (!IS_SET(obj->pObjIndex->extra_flags, ITEM_PIT)
	||  pit_count)
		return;

	for (t_obj = obj->contains; t_obj; t_obj = next_obj) {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);
		extract_obj(t_obj, 0);
	}
}

void contents_to_obj(OBJ_DATA *obj, OBJ_DATA *to_obj)
{
	OBJ_DATA *obj_next;

	for (; obj; obj = obj_next) {
		obj_next = obj->next_content;
		obj_from_obj(obj);
		obj_to_obj(obj, to_obj);
	}
}

static inline void
save_corpse_contents(OBJ_DATA *corpse)
{
	OBJ_DATA *obj, *obj_next;
	OBJ_DATA *pit;
	altar_t *altar;

/* in another object */
	if (corpse->in_obj) {
		contents_to_obj(corpse->contains, corpse->in_obj);
		return;
	}

/* carried by */
	if (corpse->carried_by) {
		for (obj = corpse->contains; obj; obj = obj_next) {
			obj_next = obj->next_content;
			obj_from_obj(obj);
			obj_to_char(obj, corpse->carried_by);
		}
		return;
	}

/* pit lookup */
	pit = NULL;
	if ((altar = corpse->altar)) {
		for (pit = altar->room->contents; pit; pit = pit->next_content)
			if (pit->pObjIndex == altar->pit)
				break;
	} else {
		log("save_corpse_contents: null altar (owner: %s)",
			   mlstr_mval(&corpse->owner));
	}

/* put contents into altar */
	if (!pit) {
		for (obj = corpse->contains; obj; obj = obj_next) {
			obj_next = obj->next_content;
			obj_from_obj(obj);
			obj_to_room(obj, altar->room);
		}
		return;
	}

/* put contents into pit */
	contents_to_obj(corpse->contains, pit);
}

void update_one_obj(OBJ_DATA *obj)
{
	OBJ_DATA *t_obj;
	CHAR_DATA *rch;
	char *message;

	if (IS_AUCTIONED(obj))
		return;

	update_obj_affects(obj);

	/* find the uppest obj container */
	for(t_obj = obj; t_obj->in_obj; t_obj = t_obj->in_obj)
		;

	if ((t_obj->in_room != NULL &&
	     t_obj->in_room->area->nplayer > 0)
        ||  (t_obj->carried_by &&
	     t_obj->carried_by->in_room &&
	     t_obj->carried_by->in_room->area->nplayer > 0))
		oprog_call(OPROG_AREA, obj, NULL, NULL);

	if (check_material(obj, "ice") 
	&&  update_ice_obj(obj))
		return;

	if (check_material(obj, "glass")
	&&  obj->pObjIndex->item_type == ITEM_POTION
	&&  update_glass_obj(obj))
		return;

	if (obj->condition > -1 && (obj->timer <= 0 || --obj->timer > 0))
		return;

	switch (obj->pObjIndex->item_type) {
	default:
		message = "$p crumbles into dust.";
		break;
	case ITEM_FOUNTAIN:
		message = "$p dries up.";
		break;
	case ITEM_CORPSE_NPC:
		message = "$p decays into dust.";
		break;
	case ITEM_CORPSE_PC:
		message = "$p decays into dust.";
		break;
	case ITEM_FOOD:
		message = "$p decomposes.";
		break;
	case ITEM_POTION:
		message = "$p has evaporated from disuse.";	
		break;
	case ITEM_PORTAL:
		message = "$p fades out of existence.";
		break;
	case ITEM_CONTAINER: 
		if (CAN_WEAR(obj, ITEM_WEAR_FLOAT))
			if (obj->contains)
				message = "$p flickers and vanishes, spilling "
					  "its contents on the floor.";
			else
				message = "$p flickers and vanishes.";
		else
			message = "$p crumbles into dust.";
			break;
	}

	if (obj->carried_by)
		if (IS_NPC(obj->carried_by) 
		&&  obj->carried_by->pMobIndex->pShop != NULL)
			obj->carried_by->silver += obj->cost/5;
		else {
			act(message, obj->carried_by, obj, NULL, TO_CHAR);
			if (obj->wear_loc == WEAR_FLOAT)
				act(message, obj->carried_by, obj, NULL,
				    TO_ROOM);
		}

	if (obj->in_room && (rch = obj->in_room->people)
	&&  !IS_SET(obj->pObjIndex->extra_flags, ITEM_PIT))
		act(message, rch, obj, NULL, TO_ALL);

	if (obj->pObjIndex->item_type == ITEM_CORPSE_PC && obj->contains)
		save_corpse_contents(obj);
	extract_obj(obj, 0);
}

OBJ_DATA *last_updated_obj;

void obj_update_list(OBJ_DATA *obj)
{
	int i;
	OBJ_DATA *obj_next;

/* some diagnostics */
	obj_next = obj;
	for (i = 0; obj && !mem_is(obj, MT_OBJ); obj = obj->next, i++)
		;
	if (i) {
		log("obj_update_list: skipped %d extracted objs, "
			   "object_list == %p, obj == %p, "
			   "last_updated_obj == %p, "
			   "last_updated_obj->next == %p",
			   i, object_list, obj_next,
			   last_updated_obj,
			   last_updated_obj ? last_updated_obj->next : NULL);
	}

	for (; obj; obj = obj_next) {
		obj_next = obj->next;

		if (!mem_is(obj, MT_OBJ)) {
			obj_update_list(last_updated_obj ?
					last_updated_obj->next : object_list);
			return;
		}

		update_one_obj(obj);

		if (mem_is(obj, MT_OBJ))
			last_updated_obj = obj;
	}
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update(void)
{   
	last_updated_obj = NULL;
	obj_update_list(object_list);
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update(void)
{
	CHAR_DATA *wch, *wch_next;

	for (wch = char_list; wch && !IS_NPC(wch); wch = wch_next) {
		CHAR_DATA *ch, *ch_next;
		CHAR_DATA *vch, *vch_next;
		wch_next = wch->next;

		if (!wch->in_room)
			continue;

		if (IS_AWAKE(wch)
		&&  IS_AFFECTED(wch, AFF_BLOODTHIRST)
		&&  wch->fighting == NULL) {
			for (vch = wch->in_room->people;
			     vch != NULL && wch->fighting == NULL;
			     vch = vch_next) {
				vch_next = vch->next_in_room;

				if (IS_IMMORTAL(vch))
					continue;

				if (wch != vch && can_see(wch,vch)
				&&  !is_safe_nomessage(wch,vch)) {
					act_puts("{RMORE BLOOD! MORE BLOOD! MORE BLOOD!!!{x", wch,NULL,NULL,TO_CHAR,POS_RESTING);
					dofun("murder", wch, vch->name);
					if (IS_EXTRACTED(wch))
						continue;
				}
			}
		}

		if (wch->level >= LEVEL_IMMORTAL)
			continue;

		for (ch = wch->in_room->people; ch; ch = ch_next) {
			int count;
			flag64_t act;
			NPC_DATA *npc;
			CHAR_DATA *victim;

			ch_next = ch->next_in_room;

			if (!IS_NPC(ch))
				continue;

			npc = NPC(ch);
			act = ch->pMobIndex->act;

			if ((!IS_SET(act, ACT_AGGRESSIVE) &&
			     npc->last_fought == NULL &&
			     npc->target == NULL)
			||  IS_SET(ch->in_room->room_flags,
				   ROOM_PEACE | ROOM_SAFE)
			||  IS_AFFECTED(ch, AFF_CALM)
			||  ch->fighting != NULL
			||  RIDDEN(ch)
			||  IS_AFFECTED(ch, AFF_CHARM)
			||  !IS_AWAKE(ch)
			||  (IS_SET(act, ACT_WIMPY) && IS_AWAKE(wch))
			||  !can_see(ch, wch) 
			||  number_bits(1) == 0
			||  is_safe_nomessage(ch, wch))
				continue;

			if ((victim = npc->target) != NULL) {
				if (victim == wch)
					multi_hit(ch, wch, TYPE_UNDEFINED);
				continue;
			}

			/* Mad mob attacks! */
			if (npc->last_fought == wch
			&&  !IS_AFFECTED(ch, AFF_SCREAM | AFF_CALM)) {
				act_yell(ch, "$i! Now you die!", wch, NULL);
				wch = check_guard(wch, ch); 
				multi_hit(ch, wch, TYPE_UNDEFINED);
				continue;
			}

			if (npc->last_fought)
				continue;

			/*
			 * Ok we have a 'wch' player character and a 'ch' npc
			 * aggressor. Now make the aggressor fight a RANDOM
			 * pc victim in the room, giving each 'vch' an equal
			 * chance of selection.
			 */
			count = 0;
			victim = NULL;
			for (vch = wch->in_room->people;
			     vch != NULL; vch = vch_next) {
				vch_next = vch->next_in_room;
				if (!IS_NPC(vch)
				&&  vch->level < LEVEL_IMMORTAL
				&&  ch->level >= vch->level - 5 
				&&  (!IS_SET(act, ACT_WIMPY) || !IS_AWAKE(vch))
				&&  can_see(ch, vch)
				/* do not attack vampires */
				&&  !IS_VAMPIRE(vch)
				/* good vs good :( */
				&&  !(IS_GOOD(ch) && IS_GOOD(vch))) {
					if (number_range(0, count) == 0)
						victim = vch;
					count++;
				}
			}

			if (victim == NULL)
				continue;

			if (!is_safe_nomessage(ch, victim)) {
				victim = check_guard(victim, ch); 
				if (get_skill(ch, "backstab"))
					dofun("backstab", ch, victim->name);
				else
					multi_hit(ch, victim, TYPE_UNDEFINED);
			}
		}
	}
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler(void)
{
	static int pulse_area;
	static int pulse_mobile;
	static int pulse_violence;
	static int pulse_point;
	static int pulse_music;
	static int pulse_water_float;
	static int pulse_raffect;
	static int pulse_track;

	if (--pulse_area <= 0) {
		wiznet("AREA & ROOM TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
		pulse_area = PULSE_AREA; 
		area_update();
		room_update();
		chquest_update();

		/* save explicit rules */
		dofun("asave", NULL, "explicit");
	}

	if (--pulse_music <= 0) {
		pulse_music	= PULSE_MUSIC;
/*		song_update(); */
	}

	if (--pulse_mobile <= 0) {
		pulse_mobile = PULSE_MOBILE;
		mobile_update();
		light_update();
	}

	if (--pulse_violence <= 0) {
		pulse_violence = PULSE_VIOLENCE;
		violence_update();
	}

	if (--pulse_water_float <= 0) {
		pulse_water_float = PULSE_WATER_FLOAT;
		water_float_update();
	}

	if (--pulse_raffect <= 0) {
		wiznet("RAFFECT TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
		pulse_raffect = PULSE_RAFFECT;
		room_affect_update();
	}

	if (--pulse_track <= 0) {
		pulse_track = PULSE_TRACK;
		track_update();
	}

	if (--pulse_point <= 0) {
		CHAR_DATA *ch;

		wiznet("CHAR TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
		pulse_point = PULSE_TICK;
		weather_update();
		char_update();
		quest_update(); 
		obj_update();
		if (time_info.hour == 0) clan_item_update();
		check_reboot();

		/* room counting */
		for (ch = char_list; ch && !IS_NPC(ch); ch = ch->next)
			if (ch->in_room)
				ch->in_room->area->count++;
	}

	aggr_update();
	auction_update();
	tail_chain();

	return;
}

void light_update(void)
{   
	CHAR_DATA *ch;
	int dam_light;
	DESCRIPTOR_DATA *d;


	for (d = descriptor_list; d != NULL; d = d->next) {
		if (d->connected != CON_PLAYING)
			continue;

		ch = (d->original != NULL) ? d->original : d->character;

		if (IS_IMMORTAL(ch))
			continue;

		/* also checks vampireness */
		if ((dam_light = isn_dark_safe(ch)) == 0) 	
			continue;	

		if (dam_light != 2
		&&  number_percent() < get_skill(ch, "light resistance")) {
			check_improve(ch, "light resistance", TRUE, 32);
			continue;
		}

		if (dam_light == 1)
			char_puts("The light in the room disturbs you.\n", ch);
		else
			char_puts("Sun light disturbs you.\n",ch);

		dam_light = 1 + (ch->max_hit * 4)/ 100;
		damage(ch, ch, dam_light, TYPE_UNDEFINED, DAM_LIGHT_V,
			DAMF_SHOW | DAMF_HUNGER);

		if (ch->position == POS_STUNNED)
			update_pos(ch);

		if (number_percent() < 10)
			gain_condition(ch, COND_DRUNK,  -1);
	}
}

void room_update(void)
{   
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *room_next;

	for (room = top_affected_room; room; room = room_next) {
		AFFECT_DATA *paf;
		AFFECT_DATA *paf_next;

		room_next = room->aff_next;

		for (paf = room->affected; paf != NULL; paf = paf_next) {
			paf_next = paf->next;

			if (paf->duration > 0) {
				paf->duration--;
				if (number_range(0,4) == 0 && paf->level > 0)
					paf->level--;
			}
			else if (paf->duration == 0) {
				check_one_event(NULL, paf, EVENT_ROOM_TIMEOUT);
				affect_remove_room(room, paf);
			}
		}
	}
}

void room_affect_update(void)
{   
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *room_next;
	CHAR_DATA	*vch;

	for (room = top_affected_room; room ; room = room_next) {
		CHAR_DATA *vch_next;
		room_next = room->aff_next;

		for (vch = room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;
			check_events(vch, room->affected,
				EVENT_ROOM_UPDATE);
		}
	}
}


void check_reboot(void)
{
	DESCRIPTOR_DATA *d;

	switch(reboot_counter) {
	case -1:
		break;
	case 0:
		reboot_mud();
		return;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 10:
	case 15:
		for (d = descriptor_list; d != NULL; d = d->next) 
			if (d->character != NULL)
				if (rebooter || !IS_IMMORTAL(d->character))
					char_printf(d->character, 
						    "{*{W*****{R rEBOOT IN {W%d{R MIN. {W*****{x\n",
					    	    reboot_counter);
				else
					char_printf(d->character, 
						    "{*{W*****{R AUTOMAGIC rEBOOT IN {W%d{R MIN. {W*****{x\n",
					    	    reboot_counter);

		/* FALLTHRU */
	default: 
		reboot_counter--;
		break;
	}
}

void track_update(void)
{   
	CHAR_DATA *ch, *ch_next;

	for (ch = npc_list; ch; ch = ch_next) {
		CHAR_DATA *victim;
		CHAR_DATA *vch, *vch_next;
		ROOM_INDEX_DATA *was_in_room;
		NPC_DATA *npc;

		ch_next = ch->next;
		if (IS_AFFECTED(ch, AFF_CALM | AFF_CHARM | AFF_SCREAM)
	        ||  ch->fighting
		||  !ch->in_room
	        ||  !IS_AWAKE(ch) 
		||  RIDDEN(ch))
			continue;

		/*
		 * track the victim
		 */
		npc = NPC(ch);
		victim = npc->target ? npc->target : npc->last_fought;

		if (victim != NULL) {
			add_mind(ch, victim->name);

	        	if (!IS_SET(ch->pMobIndex->act, ACT_NOTRACK)
			&&  (was_in_room = ch->in_room) != victim->in_room) {
				dofun("track", ch, victim->name);
				if (IS_EXTRACTED(ch))
					continue;

				/*
				 * forget about it if we could not find
				 * the tracks and have no specific target set
				 * (summoned shadow, stalker)
				 */
				if (was_in_room == ch->in_room
				&&  npc->target == NULL
				&&  number_range(0, 19) == 0) {
					npc->last_fought = NULL;
					back_home(ch);
					continue;
				}
			}
		}

		/*
		 * attack the victims in mind
		 */
		if (npc->in_mind == NULL)
			continue;

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;

			if (IS_IMMORTAL(vch)
			||  !can_see(ch, vch)
			||  is_safe_nomessage(ch, vch)
			||  !is_name(vch->name, npc->in_mind))
				continue;

			act_yell(ch, "So we meet again, $i!", vch, NULL);
			dofun("murder", ch, vch->name);
			break;
		}
	}
}

void clan_item_update(void)
{	
	int i;

	for (i = 0; i < clans.nused; i++) {
		OBJ_DATA *obj;
		clan_t *clan = CLAN(i);

		if (clan->obj_ptr == NULL) 
			continue;

		for (obj = clan->obj_ptr; obj->in_obj; obj = obj->in_obj)
			;

		/*
		 * do not move clan items which are in altars
		 */
		if (obj->in_room) {
			int j;
			bool put_back = TRUE;

			for (j = 0; j < clans.nused; j++) {
				if (obj->in_room->vnum == CLAN(j)->altar_vnum) {
					put_back = FALSE;
					break;
				}
			}

			if (!put_back)
				continue;
		}

		if (clan->obj_ptr->in_obj)
			obj_from_obj(clan->obj_ptr);
		if (clan->obj_ptr->carried_by)
			obj_from_char(clan->obj_ptr);
		if (clan->obj_ptr->in_room)
			obj_from_room(clan->obj_ptr);

		if (clan->altar_ptr) 
			obj_to_obj(clan->obj_ptr, clan->altar_ptr);
		else 
			bug("clan_item_update: no altar_ptr for clan %d", i);
	}
}

