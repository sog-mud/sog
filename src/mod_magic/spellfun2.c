/*
 * $Id: spellfun2.c,v 1.139 1999-09-30 10:41:22 osya Exp $
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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "fight.h"
#include "quest.h"
#include "rating.h"
#include "fight.h"

ROOM_INDEX_DATA * check_place(CHAR_DATA *ch, const char *argument) 
{
 EXIT_DATA *pExit;
 ROOM_INDEX_DATA *dest_room;
 int number,door;
 int range = (ch->level / 10) + 1;
 char arg[MAX_INPUT_LENGTH];

 number = number_argument(argument, arg, sizeof(arg));
 if ((door = check_exit(arg)) == -1) return NULL;

 dest_room = ch->in_room;
 while (number > 0)
 {
	number--;
	if (--range < 1) return NULL;
	if ((pExit = dest_room->exit[door]) == NULL
	  || (dest_room = pExit->to_room.r) == NULL
	  || IS_SET(pExit->exit_info,EX_CLOSED))
	 break;
	if (number < 1)    return dest_room;
 }
 return NULL;
}
	
void spell_portal(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	OBJ_DATA *portal, *stone;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  victim->level >= level + 3
	||  saves_spell(level, victim, DAM_NONE)
	||  !can_gate(ch, victim)) {
		char_puts("You failed.\n", ch);
		return;
	}   

	stone = get_eq_char(ch,WEAR_HOLD);
	if (!IS_IMMORTAL(ch) 
	&&  (stone == NULL || stone->pObjIndex->item_type != ITEM_WARP_STONE))
	{
	char_puts("You lack the proper component for this spell.\n",ch);
	return;
	}

	if (stone != NULL && stone->pObjIndex->item_type == ITEM_WARP_STONE)
	{
	 	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
	 	act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
	 	extract_obj(stone, 0);
	}

	portal = create_obj(get_obj_index(OBJ_VNUM_PORTAL),0);
	portal->timer = 2 + level / 25; 
	portal->value[3] = victim->in_room->vnum;

	obj_to_room(portal,ch->in_room);

	act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
	act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	OBJ_DATA *portal, *stone;
	ROOM_INDEX_DATA *to_room, *from_room;

	from_room = ch->in_room;
 
	if ((victim = get_char_world(ch, target_name)) == NULL
	||  victim->level >= level + 3
	||  !can_see_room(ch, from_room)
	||  saves_spell(level, victim, DAM_NONE)
	||  !can_gate(ch, victim)) {
		char_puts("You failed.\n", ch);
		return;
	}   
 
	to_room = victim->in_room;

	stone = get_eq_char(ch,WEAR_HOLD);
	if (!IS_IMMORTAL(ch)
	&&  (stone == NULL || stone->pObjIndex->item_type != ITEM_WARP_STONE)) {
		char_puts("You lack the proper component for this spell.\n",ch);
		return;
	}
 
	if (stone != NULL && stone->pObjIndex->item_type == ITEM_WARP_STONE) {
		act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
		act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
		extract_obj(stone, 0);
	}

	/* portal one */ 
	portal = create_obj(get_obj_index(OBJ_VNUM_PORTAL),0);
	portal->timer = 1 + level / 10;
	portal->value[3] = to_room->vnum;
 
	obj_to_room(portal,from_room);
 
	act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
	act("$p rises up before you.",ch,portal,NULL,TO_CHAR);

	/* no second portal if rooms are the same */
	if (to_room == from_room)
		return;

	/* portal two */
	portal = create_obj(get_obj_index(OBJ_VNUM_PORTAL),0);
	portal->timer = 1 + level/10;
	portal->value[3] = from_room->vnum;

	obj_to_room(portal,to_room);

	if (to_room->people != NULL) {
		act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
		act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
	}
}

void spell_disintegrate(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *tmp_ch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	int i,dam=0;
	OBJ_DATA *tattoo, *clanmark; 
	PC_DATA *vpc;
	
	if (saves_spell(level-2, victim, DAM_ENERGY)
	||  number_bits(1) == 0
	||  IS_IMMORTAL(victim)
	||  IS_CLAN_GUARD(victim)) {
		dam = dice(level, 24) ;
		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE);
		return;
	}

	act_puts("$N's thin light ray ###DISINTEGRATES### you!", 
	      victim, NULL, ch, TO_CHAR, POS_RESTING);
	act_puts("$n's thin light ray ###DISINTEGRATES### $N!", 
	      ch, NULL, victim, TO_NOTVICT, POS_RESTING);
	act_puts("Your thin light ray ###DISINTEGRATES### $N!", 
	      ch, NULL, victim, TO_CHAR, POS_RESTING);
	char_puts("You have been KILLED!\n", victim);

	act("$N does not exist anymore!\n", ch, NULL, victim, TO_CHAR);
	act("$N does not exist anymore!\n", ch, NULL, victim, TO_ROOM);

	char_puts("You turn into an invincible ghost for a few minutes.\n",
	             victim);
	char_puts("As long as you don't attack anything.\n", victim);

	/*  disintegrate the objects... */
	tattoo = get_eq_char(victim, WEAR_TATTOO); /* keep tattoos for later */
	if (tattoo != NULL)
		obj_from_char(tattoo);
	if ((clanmark = get_eq_char(victim, WEAR_CLANMARK)) != NULL) 
		obj_from_char(clanmark);

	victim->gold = 0;
	victim->silver = 0;

	for (obj = victim->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj, 0);
	}

	if (IS_NPC(victim)) {
		quest_handle_death(ch, victim);
		victim->pMobIndex->killed++;
		extract_char(victim, 0);
		return;
	}
	
	rating_update(ch, victim);
	extract_char(victim, XC_F_INCOMPLETE);

	while (victim->affected)
		affect_remove(victim, victim->affected);
	victim->affected_by   = 0;
	for (i = 0; i < 4; i++)
		victim->armor[i]= 100;
	victim->position      = POS_RESTING;
	victim->hit           = 1;
	victim->mana  	  = 1;

	vpc = PC(victim);
	REMOVE_BIT(vpc->plr_flags, PLR_BOUGHT_PET);
	SET_WANTED(victim, NULL);

	vpc->condition[COND_THIRST] = 40;
	vpc->condition[COND_HUNGER] = 40;
	vpc->condition[COND_FULL] = 40;
	vpc->condition[COND_BLOODLUST] = 40;
	vpc->condition[COND_DESIRE] = 40;

	if (tattoo != NULL) {
		obj_to_char(tattoo, victim);
		equip_char(victim, tattoo, WEAR_TATTOO);
	}

	if (clanmark != NULL) {
		obj_to_char(clanmark, victim);
		equip_char(victim, clanmark, WEAR_CLANMARK);
	}

	for (tmp_ch = npc_list; tmp_ch; tmp_ch = tmp_ch->next)
		if (NPC(tmp_ch)->last_fought == victim)
			NPC(tmp_ch)->last_fought = NULL;
}

void spell_bark_skin(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(ch, sn))
	{
	if (victim == ch)
	  char_puts("Your skin is already covered in bark.\n",ch); 
	else
	  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
	return;
	}
	af.where	 = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = -(level * 1.5);
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n's skin becomes covered in bark.", victim, NULL, NULL, TO_ROOM);
	char_puts("Your skin becomes covered in bark.\n", victim);
}

void spell_ranger_staff(int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *staff;
	AFFECT_DATA tohit;
	AFFECT_DATA todam;

	staff = create_obj(get_obj_index(OBJ_VNUM_RANGER_STAFF), 0);
	staff->level = ch->level;
	staff->value[1] = 5 + level / 14;
	staff->value[2] = 4 + level / 15;

	char_puts("You create a ranger's staff!\n",ch);
	act("$n creates a ranger's staff!",ch,NULL,NULL,TO_ROOM);

	tohit.where		 = TO_OBJECT;
	tohit.type               = sn;
	tohit.level              = ch->level; 
	tohit.duration           = -1;
	tohit.location           = APPLY_HITROLL;
	tohit.modifier           = 2 + level/5;
	tohit.bitvector          = 0;
	affect_to_obj(staff,&tohit);

	todam.where		 = TO_OBJECT;
	todam.type               = sn;
	todam.level              = ch->level; 
	todam.duration           = -1;
	todam.location           = APPLY_DAMROLL;
	todam.modifier           = 2 + level/5;
	todam.bitvector          = 0;
	affect_to_obj(staff,&todam);

	staff->timer = level;
	staff->level = ch->level;
	
	obj_to_char(staff,ch);
}

void spell_transform(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;

	if (is_affected(ch,sn) || ch->hit > ch->max_hit)
	{
	  char_puts("You are already overflowing with health.\n",ch);
	  return;
	}

	ch->hit += UMIN(30000 - ch->max_hit, ch->max_hit);

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 24;
	af.location           = APPLY_HIT;
	af.modifier           = UMIN(30000 - ch->max_hit, ch->max_hit);
	af.bitvector          = 0;
	affect_to_char(ch,&af);

	char_puts("Your mind clouds as your health increases.\n",ch);
}

void spell_mana_transfer(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim == ch)
	{
	  char_puts("You would implode if you tried to transfer mana to yourself.\n",ch);
	  return;
	}

	if (ch->clan != victim->clan)
	{
	  char_puts("You may only cast this spell on fellow clan members.\n",ch);
	  return;
	}

	if (ch->hit < 50)
	damage(ch,ch,50,sn,DAM_NONE, TRUE);
	else {
	victim->mana = UMIN(victim->max_mana, victim->mana + number_range(20,120));
	damage(ch,ch,50,sn,DAM_NONE, TRUE);
	}
}
	
void spell_mental_knife(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (ch->level < 40)
	dam = dice(level,8);
	else if (ch->level < 65)
	dam = dice(level,11);
	else dam = dice(level,14);

	if (saves_spell(level,victim, DAM_MENTAL))
	      dam /= 2;
	damage(ch,victim,dam,sn,DAM_MENTAL, TRUE);
	if (IS_EXTRACTED(victim))
		return;

	if (!is_affected(victim,sn) && !saves_spell(level, victim, DAM_MENTAL))
	{
	  af.where		    = TO_AFFECTS;
	  af.type               = sn;
	  af.level              = level; 
	  af.duration           = level;
	  af.location           = APPLY_INT;
	  af.modifier           = -7;
	  af.bitvector          = 0;
	  affect_to_char(victim,&af);

	  af.location = APPLY_WIS;
	  affect_to_char(victim,&af);
	  act("Your mental knife sears $N's mind!",ch,NULL,victim,TO_CHAR);
	  act("$n's mental knife sears your mind!",ch,NULL,victim,TO_VICT);
	  act("$n's mental knife sears $N's mind!",ch,NULL,victim,TO_NOTVICT);
	}
}

void spell_demon_summon(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *gch;
	CHAR_DATA *demon;
	AFFECT_DATA af;
	int i;

	if (is_affected(ch, sn)) {
		char_puts("You lack the power to summon another demon right now.\n", ch);
		return;
	}

	char_puts("You attempt to summon a demon.\n",ch);
	act("$n attempts to summon a demon.",ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_DEMON) {
			char_puts("Two demons are more than you can control!\n",
				  ch);
			return;
		}
	}

	demon = create_mob(get_mob_index(MOB_VNUM_DEMON));

	for (i = 0; i < MAX_STATS; i++) {
		demon->perm_stat[i] = ch->perm_stat[i];
	}

	demon->max_hit = IS_NPC(ch) ?
			URANGE(ch->max_hit, 1 * ch->max_hit, 30000) :
			URANGE(PC(ch)->perm_hit, ch->hit, 30000);
	demon->hit = demon->max_hit;
	demon->max_mana = IS_NPC(ch)? ch->max_mana : PC(ch)->perm_mana;
	demon->mana = demon->max_mana;
	demon->level = level;
	for (i=0; i < 3; i++)
	demon->armor[i] = interpolate(demon->level,100,-100);
	demon->armor[3] = interpolate(demon->level,100,0);
	demon->gold = 0;
	NPC(demon)->dam.dice_number = number_range(level/15, level/10);   
	NPC(demon)->dam.dice_type = number_range(level/3, level/2);
	demon->damroll = number_range(level/8, level/6);

	char_puts("A demon arrives from the underworld!\n",ch);
	act("A demon arrives from the underworld!",ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(ch, &af);  

	char_to_room(demon, ch->in_room);
	if (IS_EXTRACTED(demon))
		return;

	if (number_percent() < 40) {
		if (can_see(demon, ch))
			dofun("say", demon, "You dare disturb me??!!!");
		else
			dofun("say", demon, "Who dares disturb me??!!!");
		dofun("murder", demon, ch->name);
	} else {
		SET_BIT(demon->affected_by, AFF_CHARM);
		demon->master = demon->leader = ch;
	}
}

void spell_scourge(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;

	if (ch->level < 40)
		dam = dice(level, 6);
	else if (ch->level < 65)
		dam = dice(level, 9);
	else
		dam = dice(level, 12);

	for (vch = ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE)
		||  is_affected(vch, sn))	/* XXX ??? */
			continue;

		if (number_percent() < level)
	    		spellfun_call("poison", level, ch, vch);

		if (number_percent() < level)
			spellfun_call("blindness", level, ch, vch);

		if (number_percent() < level)
			spellfun_call("weaken", level, ch, vch);

		if (saves_spell(level,vch, DAM_FIRE))
			dam /= 2;
		damage(ch, vch, dam, sn, DAM_FIRE, TRUE);
	}
}

void spell_benediction(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int strength = 0;
	if (is_affected(victim, sn)) {
		if (victim == ch) 
			act("You are already blessed.", 
				ch, NULL, NULL, TO_CHAR);
		else 
			act("$N is already blessed.", 
				ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_EVIL(victim)) 
		strength = IS_EVIL(ch) ? 2 : (IS_GOOD(ch) ? 0 : 1);
	if (IS_GOOD(victim))
		strength = IS_GOOD(ch) ? 2 : (IS_EVIL(ch) ? 0 : 1);
	if (IS_NEUTRAL(victim)) 
		strength = IS_NEUTRAL(ch) ? 2 : 1;
	if (!strength) {
		act("Your god does not seems to like $N", 
			ch, NULL, victim, TO_CHAR);
		return;
	}
	af.where 	= TO_AFFECTS;
	af.type  	= sn;
	af.level	= level;
	af.duration	= 5+level/2;
	af.location	= APPLY_HITROLL;
	af.modifier	= level / 8 * strength;
	af.bitvector	= 0;

	affect_to_char(victim, &af);

	af.location	= APPLY_SAVING_SPELL;
	af.modifier	= 0 - level / 8 * strength;

	affect_to_char(victim, &af);

	af.location	= APPLY_LEVEL;
	af.modifier	= strength;

	affect_to_char(victim, &af);
	act("You feel righteous.\n", victim, NULL, NULL, TO_CHAR);
	if (victim != ch)
		act("You grant $N favor of your god.", 
			ch, NULL, victim, TO_CHAR);
}

void spell_manacles(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!IS_WANTED(victim)) {
		act("But $N is not wanted.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!is_affected(victim, sn) && !saves_spell(ch->level, victim,DAM_CHARM))
	{
	  AFFECT_DATA af;

	  af.where		    = TO_AFFECTS;
	  af.type               = sn;
	  af.level              = level; 
	  af.duration           = 5 + level/5;
	  af.bitvector          = 0;

	  af.modifier           = 0 - (get_curr_stat(victim, STAT_DEX)-4);
	  af.location           = APPLY_DEX;
	  affect_to_char(victim, &af);

	  af.modifier           = -5;
	  af.location           = APPLY_HITROLL;
	  affect_to_char(victim, &af);

	
	  af.modifier           = -10;
	  af.location           = APPLY_DAMROLL;
	  affect_to_char(victim, &af);

	  spellfun_call2("charm person", sn, level, ch, vo);
	}
}

void spell_shield_of_ruler(int sn, int level,
			   CHAR_DATA *ch, void *vo) 
{
	int shield_vnum;
	OBJ_DATA *shield;
	AFFECT_DATA af;

	if (level >= 71)
		shield_vnum = OBJ_VNUM_RULER_SHIELD4;
	else if (level >= 51)
		shield_vnum = OBJ_VNUM_RULER_SHIELD3;
	else if (level >= 31)
		shield_vnum = OBJ_VNUM_RULER_SHIELD2;
	else
		shield_vnum = OBJ_VNUM_RULER_SHIELD1;

	shield = create_obj(get_obj_index(shield_vnum), 0);
	shield->level = ch->level;
	shield->timer = level;
	shield->cost  = 0;
	obj_to_char(shield, ch);
  
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= -1;
	af.modifier	= level / 8;
	af.bitvector	= 0;

	af.location	= APPLY_HITROLL;
	affect_to_obj(shield, &af);

	af.location	= APPLY_DAMROLL;
	affect_to_obj(shield, &af);

	af.modifier	= -level/2;
	af.location	= APPLY_AC;
	affect_to_obj(shield, &af);

	af.modifier	= UMAX(1, level / 30);
	af.location	= APPLY_CHA;
	affect_to_obj(shield, &af);

	act("You create $p!", ch, shield, NULL, TO_CHAR);
	act("$n creates $p!", ch, shield, NULL, TO_ROOM);
}
 
void spell_guard_call(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *guard;
	CHAR_DATA *guard2;
	AFFECT_DATA af;
	int i;

	if (is_affected(ch, sn)) {
		char_puts("You lack the power to call another two guards now.\n",ch);
		return;
	}

	dofun("yell", ch, "Guards! Guards!");

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_SPECIAL_GUARD) {
			dofun("say", gch, "What? I'm not good enough?");
			return;
		}
	}

	guard = create_mob(get_mob_index(MOB_VNUM_SPECIAL_GUARD));

	for (i=0;i < MAX_STATS; i++)
	guard->perm_stat[i] = ch->perm_stat[i];

	guard->max_hit = 2*ch->max_hit;
	guard->hit = guard->max_hit;
	guard->max_mana = ch->max_mana;
	guard->mana = guard->max_mana;
	guard->alignment = ch->alignment;
	guard->level = ch->level;
	for (i=0; i < 3; i++)
	guard->armor[i] = interpolate(guard->level,100,-200);
	guard->armor[3] = interpolate(guard->level,100,-100);
	guard->sex = ch->sex;
	guard->gold = 0;

	NPC(guard)->dam.dice_number = number_range(level/18, level/14);   
	NPC(guard)->dam.dice_type = number_range(level/4, level/3);
	guard->damroll = number_range(level/10, level/8);

	guard2 = create_mob(guard->pMobIndex);
	clone_mob(guard,guard2);
	
	SET_BIT(guard->affected_by, AFF_CHARM);
	SET_BIT(guard2->affected_by, AFF_CHARM);
	guard->master = guard2->master = ch;
	guard->leader = guard2->leader = ch;

	char_puts("Two guards come to your rescue!\n",ch);
	act("Two guards come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 12;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(ch, &af);  

	char_to_room(guard,ch->in_room);
	char_to_room(guard2,ch->in_room);
}

void spell_nightwalker(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *gch;
	CHAR_DATA *walker;
	AFFECT_DATA af;
	int i;

	if (is_affected(ch,sn)) {
		act_puts("You feel too weak to summon a Nightwalker now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	act_puts("You attempt to summon a Nightwalker.",
		 ch, NULL, NULL, TO_CHAR, POS_DEAD);
	act("$n attempts to summon a Nightwalker.", ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_NIGHTWALKER) {
			act_puts("Two Nightwalkers are more than "
				 "you can control!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}
	}

	walker = create_mob(get_mob_index(MOB_VNUM_NIGHTWALKER));

	for (i = 0; i < MAX_STATS; i++)
		walker->perm_stat[i] = ch->perm_stat[i];

	walker->max_hit = IS_NPC(ch) ? ch->max_hit : PC(ch)->perm_hit;
	walker->hit = walker->max_hit;
	walker->max_mana = ch->max_mana;
	walker->mana = walker->max_mana;
	walker->level = level;
	for (i = 0; i < 3; i++)
		walker->armor[i] = interpolate(walker->level, 100, -100);
	walker->armor[3] = interpolate(walker->level, 100, 0);
	walker->gold = 0;
	NPC(walker)->dam.dice_number = number_range(level/15, level/10);   
	NPC(walker)->dam.dice_type   = number_range(level/3, level/2);
	walker->damroll  = 0;
	 
	act_puts("$N rises from the shadows!",
		 ch, NULL, walker, TO_CHAR, POS_DEAD);
	act("$N rises from the shadows!", ch, NULL, walker, TO_ROOM);
	act_puts("$N kneels before you.",
		 ch, NULL, walker, TO_CHAR, POS_DEAD);
	act("$N kneels before $n!", ch, NULL, walker, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level; 
	af.duration	= 24;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);  

	walker->master = walker->leader = ch;

	char_to_room(walker, ch->in_room);
}
	
void spell_eyes_of_intrigue(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;

	if ((victim = get_char_world(ch, target_name)) == NULL)
	  {
	char_puts("Your spy network reveals no such player.\n",ch);
	return;
	  }

	if ((victim->level > ch->level + 7) && saves_spell(level, victim, DAM_NONE))
	  {
	char_puts("Your spy network cannot find that player.\n",ch);
	return;
	  }

	if (ch->in_room == victim->in_room)
		dofun("look", ch, str_empty);
	else
		look_at(ch, victim->in_room);
}

void spell_shadow_cloak(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (ch->clan != victim->clan)
	  {
	char_puts("You may only use this spell on fellow clan members.\n",ch);
	return;
	  }

	if (is_affected(victim, sn))
	{
	if (victim == ch)
	  char_puts("You are already protected by a shadow cloak.\n",ch);
	else
	  act("$N is already protected by a shadow cloak.",ch,NULL,victim,TO_CHAR);
	return;
	}

	af.where	 = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 24;
	af.modifier  = -level;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel the shadows protect you.\n", victim);
	if (ch != victim)
	act("A cloak of shadows protect $N.",ch,NULL,victim,TO_CHAR);
}
	
void spell_nightfall(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch;
	OBJ_DATA  *obj;
	AFFECT_DATA af;
	
	if (is_affected(ch, sn)) {
		char_puts("You can't find the power to control objs.\n",
			  ch);
		return;
	}

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if (is_same_group(ch, vch))
			continue;

		for (obj = vch->carrying; obj; obj = obj->next_content) {
			if (obj->pObjIndex->item_type != ITEM_LIGHT
			||  obj->value[2] == 0
			||  saves_spell(level, vch, DAM_ENERGY))
				continue;

			act("$p flickers and goes out!", ch, obj, NULL, TO_ALL);
			obj->value[2] = 0; 

			if (obj->wear_loc == WEAR_LIGHT
			&&  ch->in_room->light > 0)
				ch->in_room->light--;
		}
	}

	for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
		if (obj->pObjIndex->item_type != ITEM_LIGHT
		||  obj->value[2] == 0)
			continue;

		act("$p flickers and goes out!", ch, obj, NULL, TO_ALL);
		obj->value[2] = 0; 
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 2;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	af.bitvector	= 0;
	affect_to_char(ch, &af);
}
	      
void spell_garble(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (ch == victim) 
	{
	char_puts("Garble whose speech?\n",ch);
	return;
	}

	if (is_affected(victim,sn)) 
	{
	act("$N's speech is already garbled.",ch,NULL,victim,TO_CHAR);
	return;
	}

	if (is_safe_nomessage(ch,victim)) {
	  char_puts("You cannot garble that person.\n",ch);
	  return;
	} 

	if (saves_spell(level,victim, DAM_MENTAL))
	return;

	af.where	= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 10;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = 0;
	affect_to_char(victim,&af);

	act("You have garbled $N's speech!",ch,NULL,victim,TO_CHAR);
	char_puts("You feel your tongue contort.\n",victim);
}

void spell_confuse(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	CHAR_DATA *rch;
	int count;

	if (is_affected(victim, gsn_confuse)) {
		act("$N is already thoroughly confused.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (saves_spell(level, victim, DAM_MENTAL))
		return;

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 10;
	af.modifier	= 0;
	af.location	= 0;
	af.bitvector	= 0;
	affect_to_char(victim, &af);

	count = 0;
	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch != ch
		&&  can_see(ch, rch)
		&&  !is_safe(ch, rch))
			count++;
	}

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch != ch
		&&  can_see(ch, rch)
		&&  !is_safe(ch, rch)
		&&  number_range(1, count) == 1)
			break;
	}

	if (rch)
		dofun("murder", victim, rch->name); 
	else
		dofun("murder", victim, ch->name);
}

void spell_terangreal(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	if (IS_NPC(victim))
	return;

	af.where		= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SLEEP;
	affect_join(victim, &af);

	if (IS_AWAKE(victim))
	{
	char_puts("You are overcome by a sudden surge of fatigue.\n",
		     victim);
	act("$n falls into a deep sleep.", victim, NULL, NULL, TO_ROOM);
	victim->position = POS_SLEEPING;
	}

	return;
}
	
void spell_kassandra(int sn, int level, CHAR_DATA *ch, void *vo)
{

	AFFECT_DATA af;

	if (is_affected(ch, sn))
	  {
	char_puts
	  ("The kassandra has been used for this purpose too recently.\n",
	   ch);
	return;
	  }
	af.where		= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 5;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);
	ch->hit = UMIN(ch->hit + 150, ch->max_hit);
	update_pos(ch);
	char_puts("A warm feeling fills your body.\n", ch);
	act("$n looks better.", ch, NULL, NULL, TO_ROOM);
}  
	
void spell_sebat(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;

	if (is_affected(ch, sn))
	{
	  char_puts("The kassandra has been used for that too recently.\n"
		   ,ch); 
	  return;
	}
	af.where		= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = -30;
	af.bitvector = 0;
	affect_to_char(ch, &af);
	act("$n is surrounded by a mystical shield.",ch, NULL,NULL,TO_ROOM);
	char_puts("You are surrounded by a mystical shield.\n", ch);
}
	
void spell_matandra(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	AFFECT_DATA af;
	
	if (is_affected(ch, sn))
	{
	  char_puts
	("The kassandra has been used for this purpose too recently.\n",
	 ch);
	  return;
	}
	af.where		= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 5;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);
	dam = dice(level, 7);
	
	damage(ch,victim,dam,sn,DAM_HOLY, TRUE);
}  
	
void spell_amnesia(int sn, int level, CHAR_DATA *ch, void *vo)  
{
	int i;
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (IS_NPC(victim))
		return;

	for (i = 0; i < PC(victim)->learned.nused; i++) {
		pcskill_t *ps = VARR_GET(&PC(victim)->learned, i);
		ps->percent /= 2;
	}

	act("You feel your memories slip away.",victim,NULL,NULL,TO_CHAR);
	act("$n gets a blank look on $s face.",victim,NULL,NULL,TO_ROOM);
}

void spell_chaos_blade(int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *blade;
	AFFECT_DATA af;
	
	blade = create_obj(get_obj_index(OBJ_VNUM_CHAOS_BLADE), 0);
	blade->level = level;
	blade->timer = level * 2;
	blade->value[2] = (level / 10) + 3;  

	char_puts("You create a blade of chaos!\n",ch);
	act("$n creates a blade of chaos!",ch,NULL,NULL,TO_ROOM);

	af.where        = TO_OBJECT;
	af.type         = sn;
	af.level        = level;
	af.duration     = -1;
	af.modifier     = level / 6;
	af.bitvector    = 0;

	af.location     = APPLY_HITROLL;
	affect_to_obj(blade,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_obj(blade,&af);

	obj_to_char(blade,ch);
}    

void spell_tattoo(int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *tattoo;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int i;

	if (IS_NPC(victim))
	{
	  act("$N is too dumb to worship you!", ch, NULL, victim, TO_CHAR);
	  return;
	}

	for (i = 0; i < MAX_RELIGION; i++)
	{
	  if (!str_cmp(ch->name, religion_table[i].leader))
	{
		  tattoo = get_eq_char(victim, WEAR_TATTOO);
	 	  if (tattoo != NULL)
		    {
	 	      act("$N is already tattooed!  You'll have to remove it first.",
		ch, NULL, victim, TO_CHAR);
	  	      act("$n tried to give you another tattoo but failed.",
		ch, NULL, victim, TO_VICT);
	          act("$n tried to give $N another tattoo but failed.",
		ch, NULL, victim, TO_NOTVICT);
	  	      return;
		    }
		  else
		    { 
	  	      tattoo = create_obj(get_obj_index(religion_table[i].vnum),0);
	   	      act("You tattoo $N with $p!",ch, tattoo, victim, TO_CHAR);
	  	      act("$n tattoos $N with $p!",ch,tattoo,victim,TO_NOTVICT);
	  	      act("$n tattoos you with $p!",ch,tattoo,victim,TO_VICT);

	  	      obj_to_char(tattoo,victim);
	  	      equip_char(victim, tattoo, WEAR_TATTOO);
	      return;
	    }
	}    
	}
	char_puts("You don't have a religious tattoo.\n", ch);
}

void spell_remove_tattoo(int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *tattoo;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
 
	tattoo = get_eq_char(victim, WEAR_TATTOO);
	if (tattoo != NULL)
	{
	  extract_obj(tattoo, 0);
	  act("Through a painful process, your tattoo has been destroyed by $n.",
	ch, NULL, victim, TO_VICT);
	  act("You remove the tattoo from $N.", ch, NULL, victim, TO_CHAR);
	  act("$N's tattoo is destroyed by $n.", ch, NULL, victim, TO_NOTVICT);
	}
	else
	act("$N doesn't have any tattoos.", ch, NULL, victim, TO_CHAR);
}

void spell_wrath(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	AFFECT_DATA af;
	
	if (!IS_NPC(ch) && IS_EVIL(ch))
		victim = ch;
	
	if (IS_GOOD(victim)) {
		act("The gods protect $N.", ch, NULL, victim, TO_ROOM);
		return;
	}
	
	if (IS_NEUTRAL(victim)) {
		act("$N does not seem to be affected.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	dam = dice(level, 12);

	if (saves_spell(level, victim, DAM_HOLY))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_HOLY, TRUE);
	if (IS_EXTRACTED(victim))
		return;

	if (IS_AFFECTED(victim, AFF_CURSE)
	||  saves_spell(level, victim, DAM_HOLY))
		return;
	af.where		= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 2*level;
	af.location  = APPLY_HITROLL;
	af.modifier  = -1 * (level / 8);
	af.bitvector = AFF_CURSE;
	affect_to_char(victim, &af);
	
	af.location  = APPLY_SAVING_SPELL;
	af.modifier  = level / 8;
	affect_to_char(victim, &af);
	
	char_puts("You feel unclean.\n", victim);
	if (ch != victim)
		act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
}

void spell_stalker(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	CHAR_DATA *stalker;
	AFFECT_DATA af;
	int i;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  victim == ch
	||  !IS_WANTED(victim)) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (is_affected(ch, sn)) {
		char_puts("This power is used too recently.\n", ch);
		return;
	}

	char_puts("You attempt to summon a stalker.\n",ch);
	act("$n attempts to summon a stalker.",ch,NULL,NULL,TO_ROOM);

	stalker = create_mob(get_mob_index(MOB_VNUM_STALKER));

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level; 
	af.duration	= 6;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);  

	for (i = 0; i < MAX_STATS; i++)
		stalker->perm_stat[i] = victim->perm_stat[i];

	stalker->max_hit = UMIN(30000, 2 * victim->max_hit);
	stalker->hit = stalker->max_hit;
	stalker->max_mana = victim->max_mana;
	stalker->mana = stalker->max_mana;
	stalker->level = victim->level;

	NPC(stalker)->dam.dice_number = 
		number_range(victim->level/18, victim->level/14);   
	NPC(stalker)->dam.dice_type = 
		number_range(victim->level/4, victim->level/3);
	stalker->damroll = 
		number_range(victim->level/10, victim->level/8);

	for (i=0; i < 3; i++)
	stalker->armor[i] = interpolate(stalker->level, 100, -100);
	stalker->armor[3] = interpolate(stalker->level, 100, 0);
	stalker->gold = 0;
	stalker->affected_by |=
		(AFF_DETECT_IMP_INVIS | AFF_DETECT_FADE | AFF_DETECT_EVIL |
		 AFF_DETECT_INVIS | AFF_DETECT_MAGIC | AFF_DETECT_HIDDEN |
		 AFF_DETECT_GOOD | AFF_DARK_VISION | AFF_IMP_INVIS);
	
	NPC(stalker)->target = victim;
	stalker->clan   = ch->clan;
	char_puts("An invisible stalker arrives to stalk you!\n",victim);
	act("An invisible stalker arrives to stalk $n!",victim,NULL,NULL,TO_ROOM);
	char_puts("An invisible stalker has been sent.\n", ch);

	char_to_room(stalker,victim->in_room);
}

static inline void
tesseract_other(CHAR_DATA *ch, CHAR_DATA *victim, ROOM_INDEX_DATA *to_room)
{
	transfer_char(victim, ch, to_room,
		      NULL,
		      "$n utters some strange words and, "
		      "with a sickening lurch, you feel time\n"
		      "and space shift around you.",
		      "$N arrives suddenly.");
}
				
void spell_tesseract(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA*) vo;
	CHAR_DATA *wch;
	CHAR_DATA *wch_next;
	CHAR_DATA *pet = NULL;

	if ((victim = get_char_world(ch,target_name)) == NULL
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		char_puts("You failed.\n", ch);
		return;
	}
	
	pet = GET_PET(ch);
	if (pet && pet->in_room != ch->in_room)
		pet = NULL;

	for (wch = ch->in_room->people; wch; wch = wch_next) {
		wch_next = wch->next_in_room;
		if (wch != ch && wch != pet && is_same_group(wch, ch))
			tesseract_other(ch, wch, victim->in_room);
	}

	act("With a sudden flash of light, $n and $s friends disappear!",
		ch, NULL, NULL, TO_ROOM);
	transfer_char(ch, NULL, victim->in_room,
		      NULL,
		      "As you utter the words, time and space seem to blur.\n."
		      "You feel as though space and time are shifting.\n"
		      "all around you while you remain motionless.",
		      "$N arrives suddenly.");
	
	if (pet) 
		tesseract_other(ch, pet, victim->in_room);
}

void spell_brew(int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *potion;
	OBJ_DATA *vial;
	int spell;

	if (obj->pObjIndex->item_type != ITEM_TRASH && obj->pObjIndex->item_type != ITEM_TREASURE
	&& obj->pObjIndex->item_type != ITEM_KEY)
	  {
	char_puts("That can't be transformed into a potion.\n",ch);
	return;
	  }
	
	if (obj->wear_loc != -1)
	  {
	char_puts("The item must be carried to be brewed.\n",ch);
	return;
	  }

	for(vial=ch->carrying; vial != NULL; vial=vial->next_content)
	  if (vial->pObjIndex->vnum == OBJ_VNUM_POTION_VIAL)
	    break;
	if ( vial == NULL)  {
	char_puts("You don't have any vials to brew the potion into.\n"
			, ch);
	return;
	}
 
	if (number_percent() < 50)
	  { 
	char_puts("You failed and destroyed it.\n", ch);
	extract_obj(obj, 0);
	return;
	  }
	
	if (obj->pObjIndex->item_type == ITEM_TRASH)
		potion = create_obj(get_obj_index(OBJ_VNUM_POTION_SILVER), 0);
	else if (obj->pObjIndex->item_type == ITEM_TREASURE)
		potion = create_obj(get_obj_index(OBJ_VNUM_POTION_GOLDEN), 0);
	else
		potion = create_obj(get_obj_index(OBJ_VNUM_POTION_SWIRLING), 0);
	potion->level = ch->level;
	potion->value[0] = level;

	spell = 0;

	switch (obj->pObjIndex->item_type) {
	 case ITEM_TRASH:
		switch(number_bits(3)) {
		case 0:
	  		spell = sn_lookup("fireball");
			break;
		case 1:
	  		spell = sn_lookup("cure poison");
			break;
		case 2:
	  		spell = sn_lookup("cure blind");
			break;
		case 3:
	  		spell = sn_lookup("cure disease");
			break;
		case 4:
	  		spell = sn_lookup("word of recall");
			break;
		case 5: 
			spell = sn_lookup("protection good");
			break;
		case 6:
			spell = sn_lookup("protection evil");
			break;
		case 7: 
			spell = sn_lookup("sanctuary");
			break;
		};
		break;
	case ITEM_TREASURE:
		switch(number_bits(3)) {
		case 0:
	  		spell = sn_lookup("cure critical");
	  		break;
		case 1:
	  		spell = sn_lookup("haste");
	  		break;
		case 2:
	  		spell = gsn_frenzy;
	  		break;
		case 3:
	  		spell = sn_lookup("create spring");
	  		break;
		case 4:
	  		spell = sn_lookup("holy word");
	  		break;
		case 5:
	  		spell = sn_lookup("invis");
	  		break;
		case 6:
	  		spell = sn_lookup("cure light");
	  		break;
		case 7:
	  		spell = sn_lookup("cure serious");
	  		break;
		};
		break;
	case ITEM_KEY:
	switch (number_bits(3)) {
		case 0:
	  		spell = sn_lookup("detect magic");
	 		break;
		case 1:
	  		spell = sn_lookup("detect invis");
	  		break;
		case 2:
	  		spell = sn_lookup("pass door");
	  		break;
		case 3:
	  		spell = sn_lookup("detect hidden");
	  		break;
		case 4:
	  		spell = sn_lookup("improved detect");
	  		break;
		case 5:
	  		spell = sn_lookup("acute vision");
	  		break;
		case 6:
	  		spell = sn_lookup("detect good");
	  		break;
		case 7:
	  		spell = sn_lookup("detect evil");
			break;
	  };
	};
	potion->value[1] = spell;
	extract_obj(obj, 0);
	act("You brew $p from your resources!", ch, potion, NULL, TO_CHAR);
	act("$n brews $p from $s resources!", ch, potion, NULL, TO_ROOM);

	obj_to_char(potion, ch);
	extract_obj(vial, 0);
}

void spell_shadowlife(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *shadow;
	AFFECT_DATA af;
	int i;

	if (IS_NPC(victim)) {
		char_puts("Now, why would you want to do that?!?\n", ch);
		return;
	}

	if (is_affected(ch,sn)) {
		char_puts("You don't have the strength to raise a Shadow now.\n", ch);
		return;
	}

	act("You give life to $N's shadow!", ch, NULL, victim, TO_CHAR);
	act("$n gives life to $N's shadow!", ch, NULL, victim, TO_NOTVICT);
	act("$n gives life to your shadow!", ch, NULL, victim, TO_VICT);
	
	shadow = create_mob_of(get_mob_index(MOB_VNUM_SHADOW),
			       &victim->short_descr);

	for (i = 0; i < MAX_STATS; i++)
		shadow->perm_stat[i] = ch->perm_stat[i];
	
	shadow->max_hit = (3 * ch->max_hit) / 4;
	shadow->hit = shadow->max_hit;
	shadow->max_mana = (3 * ch->max_mana) / 4;
	shadow->mana = shadow->max_mana;
	shadow->alignment = ch->alignment;
	shadow->level = level;
	for (i = 0; i < 3; i++)
		shadow->armor[i] = interpolate(shadow->level,100,-100);
	shadow->armor[3] = interpolate(shadow->level,100,0);
	shadow->sex = victim->sex;
	shadow->gold = 0;

	NPC(shadow)->target = victim;
	
	af.where	= TO_AFFECTS;
	af.type         = sn;
	af.level        = level; 
	af.duration     = 24;
	af.bitvector    = 0;
	af.modifier     = 0;
	af.location     = APPLY_NONE;
	affect_to_char(ch, &af);  

	char_to_room(shadow, ch->in_room);
	dofun("murder", shadow, victim->name);
}  

void spell_ruler_badge(int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *badge;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_next;
	AFFECT_DATA af;

	if ((get_eq_char(ch, WEAR_NECK_1)  != NULL) &&
	(get_eq_char(ch, WEAR_NECK_2)  != NULL))
	{
	char_puts("But you are wearing something else.\n", ch);
	return;
	}

	for (badge = ch->carrying; badge != NULL;
	   badge = obj_next)
	{
	  obj_next = badge->next_content;
	  if (badge->pObjIndex->vnum == OBJ_VNUM_DEPUTY_BADGE 
	  || badge->pObjIndex->vnum == OBJ_VNUM_RULER_BADGE)
	{
	  act("Your $p vanishes.",ch, badge, NULL, TO_CHAR);
	  obj_from_char(badge);
	  extract_obj(badge, 0);
	  continue;
	}
	}

	badge = create_obj(get_obj_index(OBJ_VNUM_RULER_BADGE), 0);
	badge->level = ch->level;

	af.where        = TO_OBJECT;
	af.type         = sn;
	af.level        = level;
	af.duration     = -1;
	af.modifier     = level;
	af.bitvector    = 0;

	af.location     = APPLY_HIT;
	affect_to_obj(badge,&af);

	af.location     = APPLY_MANA;
	affect_to_obj(badge,&af);

	af.where        = TO_OBJECT;
	af.type         = sn;
	af.level        = level;
	af.duration     = -1;
	af.modifier     = level / 8;
	af.bitvector    = 0;

	af.location     = APPLY_HITROLL;
	affect_to_obj(badge,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_obj(badge,&af);

	badge->timer = 200;
	act("You wear the ruler badge!",ch, NULL, NULL, TO_CHAR);
	act("$n wears the $s ruler badge!", ch, NULL, NULL, TO_ROOM);

	obj_to_char(badge,victim);
	if (get_eq_char(ch, WEAR_NECK_1)  == NULL)
	equip_char(ch, badge, WEAR_NECK_1);
	else if (get_eq_char(ch, WEAR_NECK_2)  == NULL)
	equip_char(ch, badge, WEAR_NECK_2);
	else
	{
	char_puts("But you are wearing something else.\n", ch);
	return;
	}

}    

void spell_remove_badge(int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *badge;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_next;

	badge = 0;

	for (badge = victim->carrying; badge != NULL;
	   badge = obj_next)
	{
	  obj_next = badge->next_content;
	  if (badge->pObjIndex->vnum == OBJ_VNUM_DEPUTY_BADGE 
	  || badge->pObjIndex->vnum == OBJ_VNUM_RULER_BADGE)
	{
	  act("Your $p vanishes.",ch, badge, NULL, TO_CHAR);
	  act("$n's $p vanishes.", ch, badge, NULL, TO_ROOM);
	  
	  obj_from_char(badge);
	  extract_obj(badge, 0);
	  continue;
	}
	}
	return;
}

void spell_dragon_strength(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;

	if (is_affected(ch, sn))
	{
	  char_puts("You are already full of the strength of the dragon.\n",
		ch);
	  return;
	}

	af.where		= TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level / 3;
	af.bitvector = 0;

	af.modifier = 2;
	af.location = APPLY_HITROLL;
	affect_to_char(ch, &af);

	af.modifier = 2;
	af.location = APPLY_DAMROLL;
	affect_to_char(ch, &af);

	af.modifier = 10;
	af.location = APPLY_AC;
	affect_to_char(ch, &af);

	af.modifier = 2;
	af.location = APPLY_STR;
	affect_to_char(ch, &af);

	af.modifier = -2;
	af.location = APPLY_DEX;
	affect_to_char(ch, &af);

	char_puts("The strength of the dragon enters you.\n", ch);
	act("$n looks a bit meaner now.", ch, NULL, NULL, TO_ROOM);
}

void spell_dragon_breath(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level , 6);
	if (!is_safe_spell(ch, victim, TRUE))
	{
	  if (saves_spell(level, victim, DAM_FIRE))
	dam /= 2;
	  damage(ch, victim, dam, sn, DAM_FIRE, TRUE);
	}
}

void spell_golden_aura(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	CHAR_DATA *vch = vo;

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	  if (!is_same_group(vch, ch)) 
	continue;

	  if (is_affected(vch, sn) || is_affected(vch, gsn_bless) || 
	  IS_AFFECTED(vch, AFF_PROTECT_EVIL))
	{
	  if (vch == ch)
	    char_puts("You are already protected by a golden aura.\n",ch);
	  else
	    act("$N is already protected by a golden aura.",ch,NULL,vch,TO_CHAR);
	  continue;
	}
	
	  af.where		= TO_AFFECTS;
	  af.type      = sn;
	  af.level	 = level;
	  af.duration  = 6 + level;
	  af.modifier  = 0;
	  af.location  = APPLY_NONE;
	  af.bitvector = AFF_PROTECT_EVIL;
	  affect_to_char(vch, &af);
	  
	  af.modifier = level/8;
	  af.location = APPLY_HITROLL;
	  af.bitvector = 0;
	  affect_to_char(vch, &af);
	  
	  af.modifier = 0 - level/8;
	  af.location = APPLY_SAVING_SPELL;
	  affect_to_char(vch, &af);

	  char_puts("You feel a golden aura around you.\n", vch);
	  if (ch != vch)
	act("A golden aura surrounds $N.",ch,NULL,vch,TO_CHAR);

	}
}

void spell_blue_dragon(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	int race;

	if(IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.", 
			ch, NULL, NULL, TO_CHAR);
		return;
	}
	
	race = rn_lookup("blue dragon");

	PC(ch)->form_name = "blue dragon";

	act("$n turns $self into blue dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into blue dragon.", ch, NULL, NULL, TO_CHAR);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level 	= level;
	af.duration	= level/10+8;
	af.location	= APPLY_RACE;
	af.modifier	= race;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	af.where	= TO_AFFECTS;
	af.location	= APPLY_HIT;
	af.modifier	= ch->max_hit/3;
	af.bitvector	= AFF_FLYING|AFF_TURNED;
	affect_to_char(ch, &af);

	af.where	= TO_SKILLS;
	af.type		= gsn_enhanced_damage;
	af.modifier	= 10 + level/20;
	af.bitvector	= 0;

	affect_to_char(ch, &af);

	ch->hit += ch->max_hit/3;
}


void spell_green_dragon(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	int race;

	if(IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.", 
			ch, NULL, NULL, TO_CHAR);
		return;
	}
	
	race = rn_lookup("green dragon");

	PC(ch)->form_name = "green dragon";

	act("$n turns $self into green dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into green dragon.", ch, NULL, NULL, TO_CHAR);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level 	= level;
	af.duration	= level/10+8;
	af.location	= APPLY_RACE;
	af.modifier	= race;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	af.where	= TO_AFFECTS;
	af.location	= APPLY_HIT;
	af.modifier	= ch->max_hit/5;
	af.bitvector	= AFF_FLYING|AFF_TURNED;
	affect_to_char(ch, &af);

	af.where	= TO_SKILLS;
	af.type		= gsn_enhanced_damage;
	af.modifier	= 7 + level/20;
	af.bitvector	= 0;

	affect_to_char(ch, &af);

	ch->hit += ch->max_hit/5;
}

void spell_white_dragon(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	int race;

	if(IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.", 
			ch, NULL, NULL, TO_CHAR);
		return;
	}
	
	race = rn_lookup("white dragon");

	PC(ch)->form_name = "white dragon";

	act("$n turns $self into white dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into white dragon.", ch, NULL, NULL, TO_CHAR);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level 	= level;
	af.duration	= level/10+8;
	af.location	= APPLY_RACE;
	af.modifier	= race;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	af.where	= TO_AFFECTS;
	af.location	= APPLY_HIT;
	af.modifier	= ch->max_hit/4;
	af.bitvector	= AFF_FLYING|AFF_TURNED;
	affect_to_char(ch, &af);

	af.where	= TO_SKILLS;
	af.type		= gsn_enhanced_damage;
	af.modifier	= 7 + level/20;
	af.bitvector	= 0;

	affect_to_char(ch, &af);

	ch->hit += ch->max_hit/4;
}


void spell_black_dragon(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	int race;

	if(IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.", 
			ch, NULL, NULL, TO_CHAR);
		return;
	}
	
	race = rn_lookup("black dragon");

	PC(ch)->form_name = "black dragon";

	act("$n turns $self into black dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into black dragon.", ch, NULL, NULL, TO_CHAR);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level 	= level;
	af.duration	= level/10+8;
	af.location	= APPLY_RACE;
	af.modifier	= race;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	af.where	= TO_AFFECTS;
	af.location	= APPLY_HIT;
	af.modifier	= ch->max_hit/4;
	af.bitvector	= AFF_FLYING|AFF_TURNED;
	affect_to_char(ch, &af);

	af.where	= TO_SKILLS;
	af.type		= gsn_enhanced_damage;
	af.modifier	= 7 + level/20;
	af.bitvector	= 0;

	affect_to_char(ch, &af);

	ch->hit += ch->max_hit/4;
}

void spell_red_dragon(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	int race;

	if(IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.", 
			ch, NULL, NULL, TO_CHAR);
		return;
	}
	
	race = rn_lookup("red dragon");

	PC(ch)->form_name = "red dragon";

	act("$n turns $self into red dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into red dragon.", ch, NULL, NULL, TO_CHAR);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level 	= level;
	af.duration	= level/10+8;
	af.location	= APPLY_RACE;
	af.modifier	= race;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	af.where	= TO_AFFECTS;
	af.location	= APPLY_HIT;
	af.modifier	= ch->max_hit/2;
	af.bitvector	= AFF_FLYING|AFF_TURNED;
	affect_to_char(ch, &af);
	
	af.location	= APPLY_DAMROLL;
	af.modifier	= 5+level/10;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	af.where	= TO_SKILLS;
	af.type		= gsn_enhanced_damage;
	af.modifier	= 12 + level/20;
	af.bitvector	= 0;

	affect_to_char(ch, &af);

	ch->hit += ch->max_hit/2;
}

void spell_dragonplate(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	OBJ_DATA *plate;
	AFFECT_DATA af;
	
	plate = create_obj(get_obj_index(OBJ_VNUM_PLATE), 0);
	plate->level = ch->level;
	plate->timer = 2 * level;
	plate->cost  = 0;
	plate->level  = ch->level;

	af.where        = TO_OBJECT;
	af.type         = sn;
	af.level        = level;
	af.duration     = -1;
	af.modifier     = level / 8;
	af.bitvector    = 0;

	af.location     = APPLY_HITROLL;
	affect_to_obj(plate,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_obj(plate,&af);

	obj_to_char(plate, ch);
	
	act("You create $p!",ch,plate,NULL,TO_CHAR);
	act("$n creates $p!",ch,plate,NULL,TO_ROOM);
}

void spell_squire(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *gch;
	CHAR_DATA *squire;
	AFFECT_DATA af;
	int i;

	if (is_affected(ch, sn)) {
		char_puts("You cannot command another squire right now.\n", ch);
		return;
	}

	char_puts("You attempt to summon a squire.\n",ch);
	act("$n attempts to summon a squire.", ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_SQUIRE) {
			char_puts("Two squires are more than you need!\n",ch);
			return;
		}
	}

	squire = create_mob(get_mob_index(MOB_VNUM_SQUIRE));

	for (i=0;i < MAX_STATS; i++)
		squire->perm_stat[i] = ch->perm_stat[i];

	squire->max_hit = ch->max_hit;
	squire->hit = squire->max_hit;
	squire->max_mana = ch->max_mana;
	squire->mana = squire->max_mana;
	squire->level = ch->level;
	for (i=0; i < 3; i++)
	squire->armor[i] = interpolate(squire->level,100,-100);
	squire->armor[3] = interpolate(squire->level,100,0);
	squire->gold = 0;

	NPC(squire)->dam.dice_number = number_range(level/20, level/15);   
	NPC(squire)->dam.dice_type = number_range(level/4, level/3);
	squire->damroll = number_range(level/10, level/8);

	char_puts("A squire arrives from nowhere!\n",ch);
	act("A squire arrives from nowhere!",ch,NULL,NULL,TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level; 
	af.duration	= 24;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);  

	squire->master = squire->leader = ch;
	char_to_room(squire,ch->in_room);
}

void spell_dragonsword(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	int sword_vnum;
	OBJ_DATA *sword;
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA af;
	
	target_name = one_argument(target_name, arg, sizeof(arg));
	sword_vnum = 0;

	if (!str_cmp(arg, "sword"))
	sword_vnum = OBJ_VNUM_DRAGONSWORD;
	else if (!str_cmp(arg, "mace"))
	sword_vnum = OBJ_VNUM_DRAGONMACE;
	else if (!str_cmp(arg, "dagger"))
	sword_vnum = OBJ_VNUM_DRAGONDAGGER;
	else if (!str_cmp(arg, "lance"))
	sword_vnum = OBJ_VNUM_DRAGONLANCE;
	else
	{
	  char_puts("You can't make a DragonSword like that!", ch);
	  return;
	}

	sword = create_obj(get_obj_index(sword_vnum), 0);
	sword->level = ch->level;
	sword->timer = level * 2;
	sword->cost  = 0;
	if (ch->level  < 50)
		sword->value[2] = (ch->level / 10);
	else sword->value[2] = (ch->level / 6) - 3;
	sword->level = ch->level;

	af.where        = TO_OBJECT;
	af.type         = sn;
	af.level        = level;
	af.duration     = -1;
	af.modifier     = level / 5;
	af.bitvector    = 0;

	af.location     = APPLY_HITROLL;
	affect_to_obj(sword,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_obj(sword,&af);

	if (IS_GOOD(ch))
	 SET_BIT(sword->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL));
	else if (IS_NEUTRAL(ch))
	 SET_BIT(sword->extra_flags,(ITEM_ANTI_GOOD | ITEM_ANTI_EVIL));
	else if (IS_EVIL(ch))
	 SET_BIT(sword->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD));	
	obj_to_char(sword, ch);
	
	act("You create $p!",ch,sword,NULL,TO_CHAR);
	act("$n creates $p!",ch,sword,NULL,TO_ROOM);
}

void spell_entangle(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	
	if (ch->in_room->sector_type == SECT_INSIDE
	||  ch->in_room->sector_type == SECT_CITY
	||  ch->in_room->sector_type == SECT_DESERT
	||  ch->in_room->sector_type == SECT_AIR) {
		char_puts("No plants can grow here.\n", ch);
		return;
	}
	  
	dam = number_range(level, 4 * level);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 2;
	
	damage(ch, victim, dam, sn, DAM_PIERCE, TRUE);
	if (IS_EXTRACTED(victim))
		return;
	
	act("The thorny plants spring up around $n, entangling $s legs!",
	    victim, NULL, NULL, TO_ROOM);
	act("The thorny plants spring up around you, entangling your legs!",
	    victim, NULL, NULL, TO_CHAR);

	if (saves_spell(level+2, victim, DAM_PIERCE))
		victim->move = victim->move / 3;
	else 
		victim->move = 0;

	if (!is_affected(victim, sn)) {
		AFFECT_DATA todex;
	  
		todex.type	= sn;
		todex.level	= level;
		todex.duration	= level / 10;
		todex.location	= APPLY_DEX;
		todex.modifier	= -1;
		todex.bitvector	= 0;
		affect_to_char(victim, &todex);
	}
}

void spell_holy_armor(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	AFFECT_DATA af;
	 
	if (is_affected(ch, sn))
	{
	  char_puts("You are already protected from harm.",ch);
	  return;
	}

	af.where	= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = - UMAX(10, 10 * (level / 5));
	af.bitvector = 0;
	affect_to_char(ch, &af);
	act("$n is protected from harm.",ch,NULL,NULL,TO_ROOM);
	char_puts("Your are protected from harm.\n", ch);

}  

void spell_love_potion(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 50;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(ch, &af);  

	char_puts("You feel like looking at people.\n", ch);
}

void spell_protective_shield(int sn, int level, CHAR_DATA *ch, void *vo) {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	 
	if (is_affected(victim, sn))
	{
	  if (victim == ch)
	   	char_puts("You are already surrounded by a protective shield.\n",
		     ch); 
	  else
	   	act("$N is already surrounded by a protective shield.",ch,NULL,
	    victim,TO_CHAR);
	  return;
	}
	af.where	= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = number_fuzzy(level / 30) + 3;
	af.location  = APPLY_AC;
	af.modifier  = 20;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n is surrounded by a protective shield.",victim,NULL,NULL,TO_ROOM);
	char_puts("You are surrounded by a protective shield.\n", victim);
}
	
void spell_deafen(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (ch == victim) {
	char_puts("Deafen who?\n",ch);
	return;
	}

	if (is_affected(victim,sn)) {
	act("$N is already deaf.",ch,NULL,victim,TO_CHAR);
	return;
	}

	if (is_safe_nomessage(ch,victim)) {
	  char_puts("You cannot deafen that person.\n",ch);
	  return;
	}

	if (saves_spell(level,victim, DAM_NONE))
	return;

	af.where		= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 10;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = 0;
	affect_to_char(victim,&af);

	act("You have deafened $N!",ch,NULL,victim,TO_CHAR);
	char_puts("A loud ringing fills your ears...you can't hear anything!\n",
	       victim);
}

void spell_disperse(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	AFFECT_DATA af;

	if (is_affected(ch, sn)) {
		char_puts("You aren't up to dispersing this crowd.\n",ch);
		return;
	}

	for (vch = ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (vch == ch
		||  !vch->in_room
		||  IS_SET(vch->in_room->room_flags, ROOM_NORECALL)
		||  IS_SET(vch->imm_flags, IMM_SUMMON)
		||  IS_IMMORTAL(vch))
			continue;

		if (IS_NPC(vch)) {
			if (IS_SET(vch->pMobIndex->act, ACT_AGGRESSIVE))
				continue;
		}
		else {
			if (is_safe_nomessage(ch, vch))
				continue;
		}

		transfer_char(vch, NULL, get_random_room(vch, NULL),
			      "$N vanishes!",
			      "The world spins around you!",
			      "$N slowly fades into existence.");
	}

	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = 15;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	affect_to_char(ch, &af);
}

void spell_honor_shield(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn))
	{
	  if (victim == ch)
	    char_puts("But you're already protected by your honor.\n", ch);
	  else
	char_puts("They're already protected by their honor.\n", ch);
	  return;
	}

	af.where		= TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 24;
	af.modifier  = -30;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	spellfun_call("remove curse", level, ch, victim);
	spellfun_call("bless", level, ch, victim);

	char_puts("Your honor protects you.\n", victim);
	act("$n's Honor protects $m.", victim, NULL, NULL, TO_ROOM);
}
	
void spell_acute_vision(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_ACUTE_VISION)) {
		if (victim == ch)
			char_puts("Your vision is already acute. \n",ch);
		else
			act("$N already sees acutely.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 3 + level / 5;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_ACUTE_VISION;
	affect_to_char(victim, &af);
	char_puts("Your vision sharpens.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_dragons_breath(int sn, int level, CHAR_DATA *ch,
			  void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch, *vch_next;
	int dam,hp_dam,dice_dam;
	int hpch;

	act("You call the dragon lord to help you.", ch, NULL, NULL, TO_CHAR);
	act("$n start to breath like a dragon.", ch, NULL, victim, TO_NOTVICT);
	act("$n breath disturbs you!", ch, NULL, victim, TO_VICT);
	act("You breath the breath of lord of Dragons.", ch, NULL, NULL, TO_CHAR);

	hpch = UMAX(10, ch->hit);
	hp_dam = number_range(hpch/9+1, hpch/5);
	dice_dam = dice(level,20);

	dam = UMAX(hp_dam + dice_dam / 5, dice_dam + hp_dam / 5);
	
	switch(dice(1, 5)) {
	case 1:
		fire_effect(victim->in_room, level, dam/2, TARGET_ROOM);

		for (vch = victim->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE)
		||  (IS_NPC(vch) && IS_NPC(ch) &&
		     (ch->fighting != vch || vch->fighting != ch)))
			continue;

	if (vch == victim) { /* full damage */
	    if (saves_spell(level,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_FIRE,TRUE);
	    }
	}
	else /* partial damage */
	{
	    if (saves_spell(level - 2,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	}
	}
	break;

	case 2:
		if (saves_spell(level,victim,DAM_ACID)) {
			acid_effect(victim, level/2, dam/4, TARGET_CHAR);
			damage(ch, victim, dam/2, sn, DAM_ACID,TRUE);
		}
		else {
			acid_effect(victim, level, dam, TARGET_CHAR);
			damage(ch, victim, dam, sn, DAM_ACID,TRUE);
		}
		break;

	case 3:
		cold_effect(victim->in_room, level, dam/2, TARGET_ROOM); 

		for (vch = victim->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE)
		||  (IS_NPC(vch) && IS_NPC(ch) &&
		     (ch->fighting != vch || vch->fighting != ch)))
			continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_COLD))
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
	else
	{
	    if (saves_spell(level - 2,vch,DAM_COLD))
	    {
		cold_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	}
	}
	break;
	case 4:
		poison_effect(ch->in_room, level, dam, TARGET_ROOM);

	for (vch = ch->in_room->people; vch; vch = vch_next) {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(ch) && IS_NPC(vch) 
	&&  (ch->fighting == vch || vch->fighting == ch)))
	    continue;

	if (saves_spell(level,vch,DAM_POISON))
	{
	    poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	    damage(ch,vch,dam/2,sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch,level,dam,TARGET_CHAR);
	    damage(ch,vch,dam,sn,DAM_POISON,TRUE);
	}
	}
	break;
	case 5:
		if (saves_spell(level, victim, DAM_LIGHTNING)) {
			shock_effect(victim, level/2, dam/4, TARGET_CHAR);
			damage(ch, victim, dam/2, sn, DAM_LIGHTNING, TRUE);
		}
		else {
			shock_effect(victim, level, dam, TARGET_CHAR);
			damage(ch, victim, dam, sn, DAM_LIGHTNING, TRUE); 
		}
		break;
	}
}

void spell_sand_storm(int sn, int level, CHAR_DATA *ch, void *vo)
 {
	CHAR_DATA *vch, *vch_next;
	int dam,hp_dam,dice_dam;
	int hpch;

	if ((ch->in_room->sector_type == SECT_AIR)
	|| (ch->in_room->sector_type == SECT_WATER_SWIM)
	|| (ch->in_room->sector_type == SECT_WATER_NOSWIM))
	{
	 char_puts("You don't find any sand here to make storm.\n",ch);
	 ch->wait = 0;
	 return;
	}

	act("$n creates a storm with sands on the floor.",ch,NULL,NULL,TO_ROOM);
	act("You create the ..sand.. storm.",ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(10, ch->hit);
	hp_dam  = number_range(hpch/9+1, hpch/5);
	dice_dam = dice(level,20);

	dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
	sand_effect(ch->in_room,level,dam/2,TARGET_ROOM);

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch)
	&&   (ch->fighting != vch /*|| vch->fighting != ch*/)))
	    continue;

	    if (saves_spell(level,vch,DAM_COLD))
	    {
		sand_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		sand_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
}

void spell_scream(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch, *vch_next;
	int dam=0,hp_dam,dice_dam;
	int hpch;

	act("$n screams with a disturbing NOISE!.",ch,NULL,NULL,TO_ROOM);
	act("You scream with a powerful sound.",ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(10, ch->hit);
	hp_dam  = number_range(hpch/9+1, hpch/5);
	dice_dam = dice(level,20);
	dam = UMAX(hp_dam + dice_dam /10 , dice_dam + hp_dam /10);

	scream_effect(ch->in_room,level,dam/2,TARGET_ROOM);

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE))
	    continue;

	    if (saves_spell(level,vch,DAM_ENERGY))
	    {
		scream_effect(vch,level/2,dam/4,TARGET_CHAR);
/*		damage(ch,vch,dam/2,sn,DAM_ENERGY,TRUE); */
	     if (vch->fighting)  stop_fighting(vch , TRUE);
	    }
	    else
	    {
		scream_effect(vch,level,dam,TARGET_CHAR);
/*		damage(ch,vch,dam,sn,DAM_ENERGY,TRUE); */
	     if (vch->fighting)  stop_fighting(vch , TRUE);
	    }
	}
}

void spell_attract_other(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (ch->sex == victim->sex) {
		char_puts("You'd better try your chance on other sex!\n", ch);
		return;
	}
	spellfun_call2("charm person", sn, level+2, ch, vo);
}

static void
cb_strip(int lang, const char **p, void *arg)
{
	char buf[MAX_STRING_LENGTH];
	const char *r = mlstr_val((mlstring*) arg, lang);
	const char *q;

	if (IS_NULLSTR(*p)
	||  (q = strstr(r, "%s")) == NULL)
		return;

	strnzncpy(buf, sizeof(buf), r, q-r);
	if (!str_prefix(buf, *p)) {
		const char *s = strdup(*p + strlen(buf));
		free_string(*p);
		*p = s;
	}
}

void spell_animate_dead(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	CHAR_DATA *undead;
	AFFECT_DATA af;
	int i;
	int chance;
	int u_level;

	/* deal with the object case first */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj, *obj2, *next;
		MOB_INDEX_DATA *undead_idx;
		mlstring ml;

		obj = (OBJ_DATA *) vo;

		if (!(obj->pObjIndex->item_type == ITEM_CORPSE_NPC 
		|| obj->pObjIndex->item_type == ITEM_CORPSE_PC)) {
			char_puts("You can animate only corpses!\n", ch);
			return;
		}

		if (is_affected(ch, sn)) {
			char_puts("You cannot summon the strength "
				     "to handle more undead bodies.\n", ch);
			return;
		}

		if (count_charmed(ch)) 
			return;

		if (ch->in_room != NULL
		&&  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
			char_puts("You can't animate deads here.\n", ch);
			return;
		}

		/* can't animate PC corpses in ROOM_BATTLE_ARENA */
		if (obj->pObjIndex->item_type == ITEM_CORPSE_PC
		&&  obj->in_room
		&&  IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA)
		&&  !IS_OWNER(ch, obj)) {
			char_puts("You cannot do that.\n", ch);
			return;
		}

		if (IS_SET(ch->in_room->room_flags,
			   ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
			char_puts("You can't animate here.\n", ch);
			return;
		}

		chance = URANGE(5, get_skill(ch, sn)+(level-obj->level)*7, 95);
		if (number_percent() > chance) {
			act_puts("You failed and destroyed it.\n",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n tries to animate $p, but fails and destroys it.",
			    ch, obj, NULL, TO_ROOM);
			for (obj2 = obj->contains; obj2; obj2 = next) {
				next = obj2->next_content;
				obj_from_obj(obj2);
				obj_to_room(obj2, ch->in_room);
			}
			extract_obj(obj, 0);
			return;
		}

		undead_idx = get_mob_index(MOB_VNUM_UNDEAD);
		mlstr_init(&ml, NULL);
		mlstr_cpy(&ml, &obj->owner);

		/*
		 * strip "The undead body of "
		 */
		mlstr_foreach(&ml, &undead_idx->short_descr, cb_strip);

		undead = create_mob_of(undead_idx, &ml);
		mlstr_destroy(&ml);

		for (i = 0; i < MAX_STATS; i++)
			undead->perm_stat[i] = UMIN(25, 15+obj->level/10);
		u_level = UMIN (obj->level, level+((obj->level-level)/3)*2); 

		undead->max_hit = dice(20,u_level*2)+u_level*20; 
		undead->hit = undead->max_hit;
		undead->max_mana = dice(u_level,10)+100;
		undead->mana = undead->max_mana;
		undead->alignment = -1000;
		undead->level = u_level;

		for (i = 0; i < 3; i++)
			undead->armor[i] = interpolate(undead->level,100,-100);
		undead->armor[3] = interpolate(undead->level, 50, -200);
		undead->sex = ch->sex;
		undead->gold = 0;
		NPC(undead)->dam.dice_number = 11;
		NPC(undead)->dam.dice_type   = 5;
		undead->damroll  = u_level/2 +10;
	
		undead->master = ch;
		undead->leader = ch;

		undead->name = str_printf(undead->name, obj->name);

		for (obj2 = obj->contains; obj2; obj2 = next) {
			next = obj2->next_content;
			obj_from_obj(obj2);
			obj_to_char(obj2, undead);
		}

		af.where     = TO_AFFECTS;
		af.type      = sn;
		af.level     = level;
		af.duration  = level / 10;
		af.modifier  = 0;
		af.bitvector = 0;
		af.location  = APPLY_NONE;
		affect_to_char(ch, &af);

		act_puts("With mystic power, you animate it!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("With mystic power, $n animates $p!",
		    ch, obj, NULL, TO_ROOM);

		act_puts("$N looks at you and plans to make you "
			 "pay for distrurbing its rest!",
			 ch, NULL, undead, TO_CHAR, POS_DEAD);

		extract_obj(obj, 0);
		char_to_room(undead, ch->in_room);
		if (!IS_EXTRACTED(undead))
			dofun("wear", undead, "all");
		return;
	}

	victim = (CHAR_DATA *) vo;

	if (victim == ch) {
		char_puts("But you aren't dead!!\n", ch);
		return;
	}

	char_puts("But it ain't dead!!\n", ch);
}

void spell_bone_dragon(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *gch, *coc;
	AFFECT_DATA af;
	int i;

	if (is_affected(ch,sn)) {
		act("You are still tired from growing previous one.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (gch->master == ch
		&&  (gch->pMobIndex->vnum == MOB_VNUM_COCOON ||
		     gch->pMobIndex->vnum == MOB_VNUM_BONE_DRAGON)) {
			char_puts("You cannot control two or more dragons.\n",
				  ch);
			return;
		}
	}

	coc = create_mob(get_mob_index(MOB_VNUM_COCOON));

	for (i = 0; i < MAX_STATS; i++)
		coc->perm_stat[i] = 5;

	coc->max_hit = number_range (100*level, 200*level);
	coc->hit = coc->max_hit;
	coc->mana = coc->max_mana = 0;
	coc->level = ch->level;
	for (i = 0; i < 4; i++)
		coc->armor[i] = 100 - 2*ch->level - number_range(0, 50);
	coc->gold = 0;
	NPC(coc)->dam.dice_number = number_range(1, level/20);
	NPC(coc)->dam.dice_type   = number_range(1, level/10);
	coc->damroll  = number_range(1, level/3);
	coc->master = ch;

	af.where	= TO_AFFECTS;
	af.type		= gsn_bone_dragon;
	af.level	= 0;
	af.duration	= level/3;
	af.modifier	= 0;
	af.bitvector	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(coc, &af);

	af.type		= sn;
	af.duration	= 100;
	affect_to_char(ch, &af);
	char_to_room(coc,ch->in_room);

	act("Half burrowed cocoon appears from the earth.",
	    ch, NULL, NULL, TO_ALL);
}

void spell_enhanced_armor(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn))
	{
	if (victim == ch)
	  char_puts("You are already enhancedly armored.\n",ch);
	else
	  act("$N is already enhancedly armored.",ch,NULL,victim,TO_CHAR);
	return;
	}
	af.where	 = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 24;
	af.modifier  = -60;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel protected from all attacks.\n", victim);
	if (ch != victim)
	act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
	return;
}

void spell_meld_into_stone(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	 
	if (is_affected(victim, sn))
	{
	  if (victim == ch)
	   	char_puts("Your skin is already covered with stone.\n",
		     ch); 
	  else
	   	act("$N's skin is already covered with stone.",ch,NULL,
	    victim,TO_CHAR);
	  return;
	}
	af.where	= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 10;
	af.location  = APPLY_AC;
	af.modifier  = -100;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n's skin melds into stone.",victim,NULL,NULL,TO_ROOM);
	char_puts("Your skin melds into stone.\n", victim);
	return;
}
	
void spell_web(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	 if (saves_spell (level, victim,DAM_OTHER)) return; 

	if (is_affected(victim, sn))
	{
	if (victim == ch)
	  char_puts("You are already webbed.\n",ch);
	else
	  act("$N is already webbed.",ch,NULL,victim,TO_CHAR);
	return;
	}

	af.type      = sn;
	af.level	 = level;
	af.duration  = 1;
	af.location  = APPLY_HITROLL;
	af.modifier  = -1 * (level / 6); 
	af.where	 = TO_AFFECTS;
	af.bitvector = AFF_DETECT_WEB;
	affect_to_char(victim, &af);

	af.location  = APPLY_DEX;
	af.modifier  = -2;
	affect_to_char(victim, &af);

	af.location  = APPLY_DAMROLL;
	af.modifier  = -1 * (level / 6);  
	affect_to_char(victim, &af);
	char_puts("You are emeshed in thick webs!\n", victim);
	if (ch != victim)
	act("You emesh $N in a bundle of webs!",ch,NULL,victim,TO_CHAR);
}

void spell_group_defense(int sn, int level, CHAR_DATA *ch, void *vo ) 
{
	CHAR_DATA *gch;
	AFFECT_DATA af;
	int shield_sn, armor_sn;

	shield_sn = sn_lookup("shield");
	armor_sn = sn_lookup("armor");

	for(gch=ch->in_room->people; gch != NULL; gch=gch->next_in_room)
	{
		if(!is_same_group(gch, ch))
			continue;
		if (spellbane(gch, ch, 100, dice(2, LEVEL(gch)))) {
			if (IS_EXTRACTED(ch))
				return;
			else
				continue;
		}
		if(is_affected(gch, armor_sn)) {
	    	    if(gch == ch)
			char_puts("You are already armored.\n",ch);
		    else
			act("$N is already armored.", ch, NULL, gch, TO_CHAR);
		  continue;
		}
		else {
			af.type      = armor_sn;
			af.level     = level;
			af.duration  = level;
			af.location  = APPLY_AC;
			af.modifier  = -20;
			af.bitvector = 0;
			affect_to_char(gch, &af);

			char_puts("You feel someone protecting you.\n",gch);
			if(ch != gch)
				act("$N is protected by your magic.",
					ch, NULL, gch, TO_CHAR);
		}

		if(is_affected(gch, shield_sn)) {
		  if(gch == ch) 	
			char_puts("You are already shielded.\n",ch);
		  else 
		        act("$N is already shielded.", ch, NULL, gch, TO_CHAR);
		  continue;
		}
		else {
			af.type      = shield_sn;
			af.level     = level;
			af.duration  = level;
			af.location  = APPLY_AC;
			af.modifier   = -20;
			af.bitvector = 0;
			affect_to_char(gch, &af);

			char_puts("You are surrounded by a force shield.\n",gch);
			if(ch != gch)
				act("$N is surrounded by a force shield.",
					ch, NULL, gch, TO_CHAR);
		}
	}
}

void spell_inspire(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *gch;
	AFFECT_DATA af;
	int bless_sn;

	bless_sn = sn_lookup("bless");

	for(gch=ch->in_room->people; gch != NULL; gch=gch->next_in_room)
	{
		if(!is_same_group(gch, ch))
			continue;
		if (spellbane(gch, ch, 100, dice(3, LEVEL(gch)))) {
			if (IS_EXTRACTED(ch))
				return;
			else
				continue;
		}
			
		if (is_affected(gch, bless_sn))
		{
		  if(gch == ch)
		     char_puts("You are already inspired.\n",ch);
		  else
		     act("$N is already inspired.",
			ch, NULL, gch, TO_CHAR);
		  continue;
		}
		af.type      = bless_sn;
		af.level     = level;
		af.duration  = 6 + level;
		af.location  = APPLY_HITROLL;
		af.modifier  = level/12;
		af.bitvector = 0;
		affect_to_char(gch, &af);

		af.location  = APPLY_SAVING_SPELL;
		af.modifier  = 0 - level/12;
		affect_to_char(gch, &af);

		char_puts("You feel inspired!\n", gch);
		if(ch != gch)
			act("You inspire $N with the Creator's power!",
				ch, NULL, gch, TO_CHAR);

	}
}

void spell_mass_sanctuary(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
		if (is_same_group(ch, gch)) {
			if (!spellbane(gch, ch, 100, dice(3, LEVEL(gch)))) 
				spellfun_call("sanctuary", level, ch, gch);
			else
				if(IS_EXTRACTED(ch))
					return;
		}
	}
}

void spell_mend(int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int result,skill;

	if (obj->condition > 99)
	{
	char_puts("That item is not in need of mending.\n",ch);
	return;
	}

	if (obj->wear_loc != -1)
	{
	char_puts("The item must be carried to be mended.\n",ch);
	return;
	}

	skill = get_skill(ch,gsn_mend) / 2;
	result = number_percent () + skill; 

	if (IS_OBJ_STAT(obj,ITEM_GLOW))
	  result -= 5;
	if (IS_OBJ_STAT(obj,ITEM_MAGIC))
	  result += 5;

	if (result >= 50)
	{
	act("$p glows brightly, and is whole again.  Good Job!",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, and is whole again.",ch,obj,NULL,TO_ROOM);
	obj->condition += result;
	obj->condition = UMIN(obj->condition , 100);
	return;
	}

	else if (result >=10)
	{
	char_puts("Nothing seemed to happen.\n",ch);
	return;
	}

	else
	{
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj, 0);
	return;
	}
}

void spell_shielding(int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (saves_spell(level, victim, DAM_NONE)) {
		act("$N shivers slightly, but it passes quickly.",
		    ch, NULL, victim, TO_CHAR);
		char_puts("You shiver slightly, but it passes quickly.\n",
			  victim);
		return;
	}

	if (is_affected(victim, sn)) {
		af.type    = sn;
		af.level   = level;
		af.duration = level / 20;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = 0;
		affect_to_char(victim, &af);
		act("You wrap $N in more flows of Spirit.",
		    ch, NULL, victim, TO_CHAR);
		char_puts("You feel the shielding get stronger.\n",victim);
		return;
	}

	af.type	= sn;
	af.level    = level;
	af.duration = level / 15;
	af.location = APPLY_NONE;
	af.modifier	= 0;
	af.bitvector = 0;
	affect_join(victim, &af);

	char_puts("You feel as if you have lost touch with something.\n",
		  victim);
	act("You shield $N from the True Source.", ch, NULL, victim, TO_CHAR);
}

void spell_link(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int random, tmpmana;

	random = number_percent ();
	tmpmana = ch->mana;
	ch->mana = 0;
	ch->endur /= 2;
	tmpmana = (.5 * tmpmana);
	tmpmana = ((tmpmana + random)/2);
	victim->mana = victim->mana + tmpmana;    
}

void spell_power_word_kill(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	act_puts("A stream of darkness from your finger surrounds $N.", 
		ch, NULL, victim, TO_CHAR, POS_RESTING);
	act_puts("A stream of darkness from $n's finger surrounds $N.", 
		ch, NULL, victim, TO_NOTVICT, POS_RESTING);
	act_puts("A stream of darkness from $N's finger surrounds you.", 
		victim, NULL, ch, TO_CHAR, POS_RESTING);

	if (saves_spell(level-2, victim, DAM_MENTAL)
	||  IS_IMMORTAL(victim)
	||  IS_CLAN_GUARD(victim)) {
		dam = dice(level , 24) ;
		damage(ch, victim , dam , sn, DAM_MENTAL, TRUE);
		return;
	}

	char_puts("You have been KILLED!\n", victim);

	act("$N has been killed!\n", ch, NULL, victim, TO_CHAR);
	act("$N has been killed!\n", ch, NULL, victim, TO_ROOM);

	raw_kill(ch, victim);
	return;
}

void spell_eyed_sword(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	OBJ_DATA *eyed;
	int i;
/*
	if (IS_SET(ch->quest,QUEST_EYE)
	{
	 char_puts("You created your sword ,before.\n",ch);
	 return;
	}
	SET_BIT(ch->quest,QUEST_EYE);
*/
	if (IS_GOOD(ch))
		i=0;
	else if (IS_EVIL(ch))
		i=2;
	else i = 1;
	
	eyed	= create_obj_of(get_obj_index(OBJ_VNUM_EYED_SWORD),
				&ch->short_descr);
	eyed->level = ch->level;
	mlstr_cpy(&eyed->owner, &ch->short_descr);
	eyed->ed = ed_new2(eyed->pObjIndex->ed, ch->name);
	eyed->value[2] = (ch->level / 10) + 3;  
	eyed->cost = 0;
	obj_to_char(eyed, ch);
	char_puts("You create YOUR sword with your name.\n",ch);
	char_puts("Don't forget that you won't be able to create this weapon anymore.\n",ch);
}

void spell_lion_help(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *lion;
	CHAR_DATA *victim;
	AFFECT_DATA af;
	char arg[MAX_INPUT_LENGTH];
	int i;
	
	target_name = one_argument(target_name, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Whom do you want to have killed?\n",ch);
		return;
	}

	if ((victim = get_char_area(ch,arg)) == NULL) {
		char_puts("Noone around with that name.\n", ch);
		return;
	}

	if (is_safe(ch, victim)) {
		return;
	}	

	char_puts("You call a hunter lion.\n", ch);
	act("$n shouts a hunter lion.", ch, NULL, NULL, TO_ROOM);

	if (is_affected(ch, sn)) {
		char_puts("You cannot summon the strength to handle more "
			  "lions right now.\n", ch);
		return;
	}

	if (ch->in_room == NULL
	&&  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		char_puts("No lions can hear you.\n", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags,
		   ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL &&
	     ch->in_room->exit[1] == NULL &&
	     ch->in_room->exit[2] == NULL &&
	     ch->in_room->exit[3] == NULL &&
	     ch->in_room->exit[4] == NULL &&
	     ch->in_room->exit[5] == NULL)
	||  (ch->in_room->sector_type != SECT_FIELD &&
	     ch->in_room->sector_type != SECT_FOREST &&
	     ch->in_room->sector_type != SECT_MOUNTAIN &&
	     ch->in_room->sector_type != SECT_HILLS)) {
		char_puts("No hunter lion can come to you.\n", ch);
		return;
	}

	lion = create_mob(get_mob_index(MOB_VNUM_HUNTER));

	for (i=0;i < MAX_STATS; i++)
		lion->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	lion->max_hit =  UMIN(30000,ch->max_hit * 1.2);
	lion->hit = lion->max_hit;
	lion->max_mana = ch->max_mana;
	lion->mana = lion->max_mana;
	lion->alignment = ch->alignment;
	lion->level = UMIN(100,LEVEL(ch));
	for (i=0; i < 3; i++)
	lion->armor[i] = interpolate(lion->level,100,-100);
	lion->armor[3] = interpolate(lion->level,100,0);
	lion->sex = ch->sex;
	lion->gold = 0;
	NPC(lion)->dam.dice_number = number_range(LEVEL(ch)/15, LEVEL(ch)/10);   
	NPC(lion)->dam.dice_type = number_range(LEVEL(ch)/3, LEVEL(ch)/2);
	lion->damroll = number_range(level/8, level/6);
	
	char_puts("A hunter lion comes to kill your victim!\n", ch);
	act("A hunter lion comes to kill $n's victim!",
	    ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= ch->level; 
	af.duration	= 24;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);  
	lion->hunting = victim;
	char_to_room(lion, ch->in_room);
	if (!IS_EXTRACTED(lion))
		return;
	dofun("hunt", lion, str_empty);
}

void spell_magic_jar(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *vial;
	OBJ_DATA *fire;

	if (victim == ch) {
		char_puts("You like yourself even better.\n",ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Your victim is a npc. Not necessary!\n",ch);
		return;
	}

	if (IS_SET(PC(ch)->plr_flags, PLR_NOEXP)) {
		char_puts("Seek their soul somewhere else.\n", ch);
		return;
	}

	if (saves_spell(level, victim, DAM_MENTAL)) {
		char_puts("You failed.\n", ch);
		return;
	}

	for(vial = ch->carrying; vial != NULL; vial = vial->next_content)
		if (vial->pObjIndex->vnum == OBJ_VNUM_POTION_VIAL)
			break;

	if (vial == NULL)  {
		char_puts("You don't have any vials to put your victim's spirit.\n", ch);
		return;
	}

	extract_obj(vial, 0);

	fire	= create_obj_of(get_obj_index(OBJ_VNUM_MAGIC_JAR),
				&victim->short_descr);
	fire->level = ch->level;
	mlstr_cpy(&fire->owner, &victim->short_descr);
	fire->ed = ed_new2(fire->pObjIndex->ed, victim->name);
	fire->cost = 0;
	obj_to_char(fire, ch);    
	SET_BIT(PC(victim)->plr_flags, PLR_NOEXP);
	act_puts("You catch $N's spirit into your vial.",
		 ch, NULL, victim, TO_CHAR, POS_DEAD);
	act_puts("$n catches your spirit into vial.",
		 ch, NULL, victim, TO_VICT, POS_DEAD);
}

void turn_spell(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, align;

	if (IS_EVIL(ch)) {
		victim = ch;
		char_puts("The energy explodes inside you!\n", ch);
	}

	if (victim != ch) {
		act("$n raises $s hand, and a blinding ray of light "
		    "shoots forth!",
		    ch, NULL, NULL, TO_ROOM);
		char_puts("You raise your hand and a blinding ray of light "
			  "shoots forth!\n", ch);
	}

	if (IS_GOOD(victim) || IS_NEUTRAL(victim)) {
		act("$n seems unharmed by the light.",
		    victim, NULL, victim, TO_ROOM);
		char_puts("The light seems powerless to affect you.\n",
			  victim);
		return;
	}

	dam = dice(level, 10);
	if (saves_spell(level, victim, DAM_HOLY))
		dam /= 2;

	align = victim->alignment;
	align -= 350;

	if (align < -1000)
		align = -1000 + (align + 1000) / 3;

	dam = (dam * align * align) / 1000000;
	damage(ch, victim, dam, sn, DAM_HOLY, TRUE);
	if (!IS_EXTRACTED(victim))
		dofun("flee", victim, str_empty);
}

void spell_turn(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	AFFECT_DATA af;

	if (is_affected(ch, sn)) {
		char_puts("This power is used too recently.",ch);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 5;
	af.modifier	= 0;
	af.location	= 0;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	for (vch = ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE))
			continue;
		turn_spell(sn, ch->level, ch, vch);
	}
}

void spell_fear(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	class_t *vcl;

	if ((vcl = class_lookup(victim->class))
	&&  !CAN_FLEE(victim, vcl)) {
		if (victim == ch)
			char_puts("You are beyond this power.\n", ch);
		else
			char_puts("Your victim is beyond this power.\n", ch);
		return; 
	}

	if (is_affected(victim,gsn_fear)
	||  saves_spell(level, victim,DAM_OTHER))
		return;

	af.where	= TO_AFFECTS;
	af.type		= gsn_fear;
	af.level	= level;
	af.duration	= level / 10;
	af.location	= 0;
	af.modifier	= 0;
	af.bitvector	= AFF_DETECT_FEAR;
	affect_to_char(victim, &af);
	char_puts("You are afraid as much as a rabbit.\n", victim);
	act("$n looks with afraid eyes.", victim, NULL, NULL, TO_ROOM);
}

void spell_protection_heat (int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, gsn_protection_heat))
	{
	if (victim == ch)
	  char_puts("You are already protected from heat.\n",ch);
	else
	  act("$N is already protected from heat.",ch,NULL,victim,TO_CHAR);
	return;
	}

	if (is_affected(victim, gsn_protection_cold))
	{
	if (victim == ch)
	  char_puts("You are already protected from cold.\n",ch);
	else
	  act("$N is already protected from cold.",ch,NULL,victim,TO_CHAR);
	return;
	}

	if (is_affected(victim, gsn_fire_shield))
	{
	if (victim == ch)
	  char_puts("You are already using fire shield.\n",ch);
	else
	  act("$N is already using fire shield.",ch,NULL,victim,TO_CHAR);
	return;
	}

	af.where     = TO_AFFECTS;
	af.type      = gsn_protection_heat;
	af.level     = level;
	af.duration  = 24;
	af.location  = APPLY_SAVING_SPELL;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel strengthed against heat.\n", victim);
	if (ch != victim)
	act("$N is protected against heat.",ch,NULL,victim,TO_CHAR);
	return;
}

void spell_protection_cold (int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, gsn_protection_cold))
	{
	if (victim == ch)
	  char_puts("You are already protected from cold.\n",ch);
	else
	  act("$N is already protected from cold.",ch,NULL,victim,TO_CHAR);
	return;
	}

	if (is_affected(victim, gsn_protection_heat))
	{
	if (victim == ch)
	  char_puts("You are already protected from heat.\n",ch);
	else
	  act("$N is already protected from heat.",ch,NULL,victim,TO_CHAR);
	return;
	}

	if (is_affected(victim, gsn_fire_shield))
	{
	if (victim == ch)
	  char_puts("You are already using fire shield.\n",ch);
	else
	  act("$N is already using fire shield.",ch,NULL,victim,TO_CHAR);
	return;
	}
	af.where     = TO_AFFECTS;
	af.type      = gsn_protection_cold;
	af.level     = level;
	af.duration  = 24;
	af.location  = APPLY_SAVING_SPELL;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel strengthed against cold.\n", victim);
	if (ch != victim)
	act("$N is protected against cold.",ch,NULL,victim,TO_CHAR);
	return;
}

void spell_fire_shield (int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_INDEX_DATA *pObjIndex;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *fire;
	int i;

	target_name = one_argument(target_name, arg, sizeof(arg));
	if (str_cmp(arg, "cold") && str_cmp(arg, "fire")) {
		char_puts("You must specify the type.\n",ch);
		return;
	}

	if (IS_GOOD(ch))
		i = 0;
	else if (IS_EVIL(ch))
		i = 2;
	else
		i = 1;
	
	pObjIndex = get_obj_index(OBJ_VNUM_FIRE_SHIELD);
	fire	= create_obj(pObjIndex, 0);
	fire->level = ch->level;
	name_add(&fire->name, arg, NULL, NULL);

	mlstr_cpy(&fire->owner, &ch->short_descr);
	fire->ed = ed_new2(fire->pObjIndex->ed, arg);

	fire->cost = 0;
	fire->timer = 5 * ch->level ;
	if (IS_GOOD(ch))
		SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL));
	else if (IS_NEUTRAL(ch))
		SET_BIT(fire->extra_flags,(ITEM_ANTI_GOOD | ITEM_ANTI_EVIL));
	else if (IS_EVIL(ch))
		SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD));
	obj_to_char(fire, ch);
	char_puts("You create the fire shield.\n",ch);
}

void spell_witch_curse(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (is_affected(victim, gsn_witch_curse)) {
		char_puts("It has already underflowing with health.\n",ch);
		return;
	}

	if (IS_IMMORTAL(victim)
	||  IS_CLAN_GUARD(victim)) {
		damage(ch, victim, dice(level, 8), sn, DAM_NEGATIVE, TRUE);
		return;
	}

	ch->hit -= (2 * level);

	af.where	= TO_AFFECTS;
	af.type         = gsn_witch_curse;
	af.level        = level; 
	af.duration     = 24;
	af.location     = APPLY_HIT;
	af.modifier     = - level;
	af.bitvector    = 0;
	affect_to_char(victim, &af);

	act("Now $n got the path to death.", victim, NULL, NULL, TO_ROOM);
	act("Now you got the path to death.", victim, NULL, NULL, TO_CHAR);
}

void spell_knock (int sn, int level, CHAR_DATA *ch, void *vo)
{
	char arg[MAX_INPUT_LENGTH];
	int chance=0;
	int door;
	const	int	rev_dir		[]		=
	{
	    2, 3, 0, 1, 5, 4
	};

	target_name = one_argument(target_name, arg, sizeof(arg));
 
	if (arg[0] == '\0')
	{
	char_puts("Knock which door or direction.\n",ch);
	return;
	}

	if (ch->fighting)
	{	
	char_puts("Wait until the fight finishes.\n",ch);
	return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if (!IS_SET(pexit->exit_info, EX_CLOSED))
	    { char_puts("It's already open.\n",      ch); return; }
	if (!IS_SET(pexit->exit_info, EX_LOCKED))
	    { char_puts("Just try to open it.\n",     ch); return; }
	if (IS_SET(pexit->exit_info, EX_NOPASS))
	    { char_puts("A mystical shield protects the exit.\n",ch); 
	      return; }
	chance = level / 5 + get_curr_stat(ch,STAT_INT) + get_skill(ch,sn) / 5;

	act("You knock $d, and try to open $d.",
	    ch, NULL, pexit->keyword, TO_CHAR);
	act("$n knocks $d, and tries to open $d.",
	    ch, NULL, pexit->keyword, TO_ROOM);

	if (room_dark(ch->in_room))
		chance /= 2;

	/* now the attack */
	if (number_percent() < chance)
	 {
	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act("$n knocks the $d and opens the lock.",
	    ch, NULL, pexit->keyword, TO_ROOM);
	act_puts("You successed to open the $d.",
		 ch, NULL, pexit->keyword, TO_CHAR, POS_DEAD);

	/* open the other side */
	if ((to_room   = pexit->to_room.r           ) != NULL
	&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	&&   pexit_rev->to_room.r == ch->in_room)
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
	    REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
	    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
		act("The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR);
	}
	 }
	else
	 {
	act("You couldn't knock the $d!",
	    ch,NULL,pexit->keyword,TO_CHAR);
	act("$n failed to knock the $d.",
	    ch,NULL,pexit->keyword,TO_ROOM);
	 }
	return;
	}

	char_puts("You can't see that here.\n",ch);
}

void spell_magic_resistance (int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;

	if (!is_affected(ch, sn))
	{
	  char_puts("You are now resistive to magic.\n", ch);

	  af.where = TO_RESIST;
	  af.type = sn;
	  af.duration = level / 10;
	  af.level = level;
	  af.bitvector = RES_MAGIC;
	  af.location = 0;
	  af.modifier = 0;
	  affect_to_char(ch, &af);
	}
	else 
	  char_puts("You are already resistive to magic.\n", ch);
 return;
}

void spell_hallucination (int sn, int level, CHAR_DATA *ch, void *vo) 
{
 char_puts("That spell is under construction.\n",ch);
 return;
}

void spell_wolf(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *gch;
	CHAR_DATA *demon;
	AFFECT_DATA af;
	int i;

	if (is_affected(ch,sn))
	{
	char_puts("You lack the power to summon another wolf right now.\n",ch);
	  return;
	}

	char_puts("You attempt to summon a wolf.\n",ch);
	act("$n attempts to summon a wolf.",ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_WOLF) {
			char_puts("Two wolfs are more than you can control!\n",
				  ch);
			return;
		}
	}

	demon = create_mob(get_mob_index(MOB_VNUM_WOLF));

	for (i=0;i < MAX_STATS; i++)
	{
	  demon->perm_stat[i] = ch->perm_stat[i];
	}

	demon->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: URANGE(PC(ch)->perm_hit,ch->hit,30000);
	demon->hit = demon->max_hit;
	demon->max_mana = IS_NPC(ch)? ch->max_mana : PC(ch)->perm_mana;
	demon->mana = demon->max_mana;
	demon->level = ch->level;
	for (i=0; i < 3; i++)
	demon->armor[i] = interpolate(demon->level,100,-100);
	demon->armor[3] = interpolate(demon->level,100,0);
	demon->gold = 0;
	NPC(demon)->dam.dice_number = number_range(level/15, level/10);   
	NPC(demon)->dam.dice_type = number_range(level/3, level/2);
	demon->damroll = number_range(level/8, level/6);

	char_puts("The wolf arrives and bows before you!\n",ch);
	act("A wolf arrives from somewhere and bows!",ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(ch, &af);  

	demon->master = demon->leader = ch;
	char_to_room(demon,ch->in_room);
}

void spell_vampiric_blast(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 12);
	if (saves_spell(level, victim, DAM_ACID))
	dam /= 2;
	damage(ch, victim, dam, sn,DAM_ACID,TRUE);
	return;
}

void spell_dragon_skin(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	 
	if (is_affected(victim, sn))
	{
	  if (victim == ch)
	   	char_puts("Your skin is already hard as rock.\n",
		     ch); 
	  else
	   	act("$N's skin is already hard as rock.",ch,NULL,
	    victim,TO_CHAR);
	  return;
	}
	af.where	= TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = - (2 * level);
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n's skin is now hard as rock.",victim,NULL,NULL,TO_ROOM);
	char_puts("Your skin is now hard as rock.\n", victim);
	return;
}
	
void spell_mind_light(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af2;
	ROOM_AFFECT_DATA af;

	if (is_affected_room(ch->in_room, sn))
	{
	char_puts("This room has already had booster of mana.\n",ch);
	return;
	}

	af.where     = TO_ROOM_CONST;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 30;
	af.location  = APPLY_ROOM_MANA;
	af.modifier  = level * 3 / 2;
	af.bitvector = 0;
	af.owner     = ch;
	af.events    = 0;
	affect_to_room(ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sn;
	af2.level	 = level;
	af2.duration  = level / 10;
	af2.modifier  = 0;
	af2.location  = APPLY_NONE;
	af2.bitvector = 0;
	affect_to_char(ch, &af2);
	char_puts("The room starts to be filled with mind light.\n", ch);
	act("The room starts to be filled with $n's mind light.",ch,NULL,NULL,TO_ROOM);
	return;
}

void spell_insanity (int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_NPC(victim))
	{
	 char_puts("This spell can cast on PC's only.\n",ch);
	 return;
	}

	if (IS_AFFECTED(victim,AFF_BLOODTHIRST) || saves_spell(level, victim,DAM_OTHER))
	return;

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 10;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_BLOODTHIRST;
	affect_to_char(victim, &af);
	char_puts("You are aggressive as a battlerager.\n", victim);
	act("$n looks with red eyes.",victim,NULL,NULL,TO_ROOM);
}

void spell_power_stun (int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;


	if (is_affected(victim,sn) || saves_spell(level, victim,DAM_OTHER))
	return;

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 90;
	af.location  = APPLY_DEX;
	af.modifier  = - 3;
	af.bitvector = AFF_STUN;
	affect_to_char(victim, &af);
	char_puts("You are stunned.\n", victim);
	act_puts("$n is stunned.",victim,NULL,NULL,TO_ROOM,POS_SLEEPING);
	return;
}


void spell_improved_invis(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_IMP_INVIS))
	return;

	act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 10 ;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_IMP_INVIS;
	affect_to_char(victim, &af);
	char_puts("You fade out of existence.\n", victim);
}

void spell_improved_detect(int sn, int level, CHAR_DATA *ch, void *vo) 
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_IMP_INVIS))
	{
	if (victim == ch)
	  char_puts("You can already see improved invisible.\n",ch);
	else
	  act("$N can already see improved invisible mobiles.",ch,NULL,victim,TO_CHAR);
	return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 3;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_IMP_INVIS;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (ch != victim)
	char_puts("Ok.\n", ch);
}

void spell_severity_force(int sn, int level, CHAR_DATA *ch, void *vo
)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	char_printf(ch,"You cracked the ground towards the %s.\n",victim->name);
	act("$n cracked the ground towards you!.", ch, NULL, victim, TO_VICT);
	dam = dice(level , 12);
	damage(ch,victim,dam,sn,DAM_NONE,TRUE);
}

void spell_randomizer(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af2;
	ROOM_AFFECT_DATA af;

	if (is_affected(ch, sn))
	{
	char_puts("Your power of randomness has been exhausted for now.\n", ch);
	return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW))
	{
	char_puts("This room is far too orderly for your powers"
		"to work on it.\n", ch);
	return;
	}
	if (IS_AFFECTED(ch->in_room, RAFF_RANDOMIZER))
	{
	char_puts("This room has already been randomized.\n",ch);
	return;
	}

	if (number_bits(1) == 0)
	{
	  char_puts("Despite your efforts, the universe resisted chaos.\n",ch);
	  af2.where     = TO_AFFECTS;
	  af2.type      = sn;
	  af2.level	= level;
	  af2.duration  = level / 10;
	  af2.modifier  = 0;
	  af2.location  = APPLY_NONE;
	  af2.bitvector = 0;
	  affect_to_char(ch, &af2);
	  return;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 2;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_RANDOMIZER;
	af.owner     = ch;
	af.events    = 0;
	affect_to_room(ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sn;
	af2.level     = level;
	af2.duration  = level / 5;
	af2.modifier  = 0;
	af2.location  = APPLY_NONE;
	af2.bitvector = 0;
	affect_to_char(ch, &af2);
	char_puts("The room was successfully randomized!\n", ch);
	char_puts("You feel very drained from the effort.\n", ch);
	ch->hit -= UMIN(200, ch->hit/2);
	act("The room starts to randomize exits.",ch,NULL,NULL,TO_ROOM);
	return;
}

void spell_bless_weapon(int sn, int level,CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;

	if (obj->pObjIndex->item_type != ITEM_WEAPON)
	{
	char_puts("That isn't a weapon.\n",ch);
	return;
	}

	if (obj->wear_loc != -1)
	{
	char_puts("The item must be carried to be blessed.\n",ch);
	return;
	}

	if (obj->pObjIndex->item_type == ITEM_WEAPON)
	{
	 if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
	    ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
	    ||  IS_WEAPON_STAT(obj,WEAPON_HOLY)
	    ||  IS_OBJ_STAT(obj,ITEM_BLESS) 
	    || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	 {
	act("You can't seem to bless $p.",ch,obj,NULL,TO_CHAR);
	return;
	 }
	}
	if (IS_WEAPON_STAT(obj,WEAPON_HOLY))
	{
	act("$p is already blessed for holy attacks.",ch,obj,NULL,TO_CHAR);
	return;
	}

	af.where	 = TO_WEAPON;
	af.type	 = sn;
	af.level	 = level / 2;
	af.duration	 = level/8;
	af.location	 = 0;
	af.modifier	 = 0;
	af.bitvector = WEAPON_HOLY;
	affect_to_obj(obj,&af);

	act("$p is prepared for holy attacks.",ch,obj,NULL,TO_ALL);
}

void spell_resilience(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;

	if (!is_affected(ch, sn))
	{
	  char_puts("You are now resistive to draining attacks.\n", ch);

	  af.where = TO_RESIST;
	  af.type = sn;
	  af.duration = level / 10;
	  af.level = level;
	  af.bitvector = RES_ENERGY;
	  af.location = 0;
	  af.modifier = 0;
	  affect_to_char(ch, &af);
	}
	else 
	  char_puts("You are already resistive to draining attacks.\n",ch);
}

void spell_superior_heal(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int bonus = 170 + level + dice(1,20);

	victim->hit = UMIN(victim->hit + bonus, victim->max_hit);
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_master_healing(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int bonus = 300 + level + dice(1,40);

	victim->hit = UMIN(victim->hit + bonus, victim->max_hit);
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_group_heal(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if (is_same_group(ch, gch) && !HAS_SKILL(gch, gsn_spellbane)) {
			spellfun_call("master healing", level, ch, gch);
			spellfun_call("refresh", level, ch, gch);
		}
	}
}

void spell_restoring_light(int sn, int level, CHAR_DATA *ch, void *vo)
{
	ROOM_AFFECT_DATA raf;
	AFFECT_DATA af;

	if (is_affected_room(ch->in_room, sn)) {
		char_puts("This room is already lit with magic light.\n", ch);
		return;
	}
	if (is_affected(ch, sn)) {
		char_puts("Rest a while, you're tired from previous one.\n", ch);
		return;
	}
	raf.where     = TO_ROOM_AFFECTS;
	raf.type      = sn;
	raf.level     = level;
	raf.duration  = level / 25;
	raf.location  = APPLY_NONE;
	raf.modifier  = 0;
	raf.bitvector = 0;
	raf.owner     = ch;
	raf.events    = EVENT_UPDATE | EVENT_ENTER;
	affect_to_room(ch->in_room, &raf);

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= level / 10;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	act("The room becomes lit with warm light.", ch, NULL, NULL, TO_ROOM);
	char_puts("Ok.\n", ch);
}

void spell_lesser_golem(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA af;
	int i=0;

	if (is_affected(ch,sn))
	{
	  char_puts("You lack the power to create another golem right now.\n",
		   ch);
	  return;
	}

	char_puts("You attempt to create a lesser golem.\n",ch);
	act("$n attempts to create a lesser golem.",ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_LESSER_GOLEM) {
			i++;
			if (i > 2) {
				char_puts("More golems are more than you can "
					  "control!\n",ch);
				return;
			}
		}
	}

	golem = create_mob(get_mob_index(MOB_VNUM_LESSER_GOLEM));

	for (i = 0; i < MAX_STATS; i ++)
	   golem->perm_stat[i] = UMIN(25,15 + level/10);
	        
	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 1;
	golem->perm_stat[STAT_CON] += 2;

	golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN((2 * PC(ch)->perm_hit) + 400,30000);
	golem->hit = golem->max_hit;
	golem->max_mana = IS_NPC(ch)? ch->max_mana : PC(ch)->perm_mana;
	golem->mana = golem->max_mana;
	golem->level = level;
	for (i=0; i < 3; i++)
	golem->armor[i] = interpolate(golem->level,100,-100);
	golem->armor[3] = interpolate(golem->level,100,0);
	golem->gold = 0;
	NPC(golem)->dam.dice_number = 3;   
	NPC(golem)->dam.dice_type = 10;
	golem->damroll = level / 2;

	char_puts("You created a lesser golem!\n",ch);
	act("$n creates a lesser golem!",ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(ch, &af);  

	golem->master = golem->leader = ch;
	char_to_room(golem,ch->in_room);
}

void spell_stone_golem(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA af;
	int i=0;

	if (is_affected(ch,sn)) 
	{
	  char_puts("You lack the power to create another golem right now.\n",
		   ch);
	  return;
	}

	char_puts("You attempt to create a stone golem.\n",ch);
	act("$n attempts to create a stone golem.",ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_STONE_GOLEM) {
			i++;
			if (i > 2) {
				char_puts("More golems are more than you can "
					  "control!\n",ch);
				return;
			}
		}
	}

	golem = create_mob(get_mob_index(MOB_VNUM_STONE_GOLEM));


	for (i = 0; i < MAX_STATS; i ++)
	   golem->perm_stat[i] = UMIN(25,15 + level/10);
	        
	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 1;
	golem->perm_stat[STAT_CON] += 2;

	golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN((5 * PC(ch)->perm_hit) + 2000, 30000);
	golem->hit = golem->max_hit;
	golem->max_mana = IS_NPC(ch)? ch->max_mana : PC(ch)->perm_mana;
	golem->mana = golem->max_mana;
	golem->level = level;
	for (i=0; i < 3; i++)
	golem->armor[i] = interpolate(golem->level,100,-100);
	golem->armor[3] = interpolate(golem->level,100,0);
	golem->gold = 0;
	NPC(golem)->dam.dice_number = 8;   
	NPC(golem)->dam.dice_type = 4;
	golem->damroll = level / 2;

	char_puts("You created a stone golem!\n",ch);
	act("$n creates a stone golem!",ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(ch, &af);  

	golem->master = golem->leader = ch;
	char_to_room(golem,ch->in_room);
}

void spell_iron_golem(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA af;
	int i = 0;

	if (is_affected(ch,sn)) 
	{
	  char_puts("You lack the power to create another golem right now.\n",
		   ch);
	  return;
	}

	char_puts("You attempt to create an iron golem.\n",ch);
	act("$n attempts to create an iron golem.",ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_IRON_GOLEM) {
			char_puts("More golems are more than you "
				  "can control!\n", ch);
			return;
		}
	}

	golem = create_mob(get_mob_index(MOB_VNUM_IRON_GOLEM));

	for (i = 0; i < MAX_STATS; i ++)
	   golem->perm_stat[i] = UMIN(25,15 + level/10);
	        
	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 1;
	golem->perm_stat[STAT_CON] += 2;

	golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN((10 * PC(ch)->perm_hit) + 1000, 30000);
	golem->hit = golem->max_hit;
	golem->max_mana = IS_NPC(ch)? ch->max_mana : PC(ch)->perm_mana;
	golem->mana = golem->max_mana;
	golem->level = level;
	for (i=0; i < 3; i++)
	golem->armor[i] = interpolate(golem->level,100,-100);
	golem->armor[3] = interpolate(golem->level,100,0);
	golem->gold = 0;
	NPC(golem)->dam.dice_number = 11;   
	NPC(golem)->dam.dice_type = 5;
	golem->damroll = level / 2 + 10;

	char_puts("You created an iron golem!\n",ch);
	act("$n creates an iron golem!",ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(ch, &af);  

	golem->master = golem->leader = ch;
	char_to_room(golem, ch->in_room);
}

void spell_adamantite_golem(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA af;
	int i = 0;

	if (is_affected(ch,sn))
	{
	  char_puts("You lack the power to create another golem right now.\n",
		   ch);
	  return;
	}

	char_puts("You attempt to create an Adamantite golem.\n",ch);
	act("$n attempts to create an Adamantite golem.",ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_ADAMANTITE_GOLEM) {
			char_puts("More golems are more than you "
				  "can control!\n", ch);
			return;
		}
	}

	golem = create_mob(get_mob_index(MOB_VNUM_ADAMANTITE_GOLEM));

	for (i = 0; i < MAX_STATS; i ++)
	   golem->perm_stat[i] = UMIN(25,15 + level/10);
	        
	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 1;
	golem->perm_stat[STAT_CON] += 2;

	golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: UMIN((10 * PC(ch)->perm_hit) + 4000, 30000);
	golem->hit = golem->max_hit;
	golem->max_mana = IS_NPC(ch)? ch->max_mana : PC(ch)->perm_mana;
	golem->mana = golem->max_mana;
	golem->level = level;
	for (i=0; i < 3; i++)
	golem->armor[i] = interpolate(golem->level,100,-100);
	golem->armor[3] = interpolate(golem->level,100,0);
	golem->gold = 0;
	NPC(golem)->dam.dice_number = 13;   
	NPC(golem)->dam.dice_type = 9;
	golem->damroll = level / 2 + 10;

	char_puts("You created an Adamantite golem!\n",ch);
	act("$n creates an Adamantite golem!",ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(ch, &af);  

	golem->master = golem->leader = ch;
	char_to_room(golem,ch->in_room);
}

void spell_sanctify_lands(int sn, int level, CHAR_DATA *ch, void *vo)
{
	if (number_bits(1) == 0)
	{
	  char_puts("You failed.\n",ch);
	  return;
	}

	if (is_affected_room(ch->in_room, gsn_cursed_lands))
	{
	 affect_strip_room(ch->in_room, gsn_cursed_lands);
	 char_puts("The curse of the land wears off.\n", ch);
	 act("The curse of the land wears off.\n", ch, NULL, NULL, TO_ROOM);
	}
	if (is_affected_room(ch->in_room, gsn_deadly_venom))
	{
	 affect_strip_room(ch->in_room, gsn_deadly_venom);
	 char_puts("The land seems more healthy.\n", ch);
	 act("The land seems more healthy.\n", ch, NULL, NULL, TO_ROOM);
	}
	if (is_affected_room(ch->in_room, gsn_mysterious_dream))
	{
	 char_puts("The land wake up from mysterious dream.\n", ch);
	 act("The land wake up from mysterious dream.\n",
			ch, NULL, NULL, TO_ROOM);
	 affect_strip_room(ch->in_room, gsn_mysterious_dream);
	}
	if (is_affected_room(ch->in_room, gsn_black_death))
	{
	 char_puts("The disease of the land has been treated.\n", ch);
	 act("The disease of the land has been treated.\n",
			 ch, NULL, NULL, TO_ROOM);
	 affect_strip_room(ch->in_room, gsn_black_death);
	}
	if (is_affected_room(ch->in_room, gsn_lethargic_mist))
	{
	 char_puts("The lethargic mist dissolves.\n", ch);
	 act("The lethargic mist dissolves.\n", ch, NULL, NULL, TO_ROOM);
	 affect_strip_room(ch->in_room, gsn_lethargic_mist);
	}
}

void spell_deadly_venom(int sn, int level, CHAR_DATA *ch, void *vo)
{
	ROOM_AFFECT_DATA af;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n",  ch);
	  return;
	}
	if (is_affected_room(ch->in_room, sn))
	{
	 char_puts("This room has already been effected by deadly venom.\n",ch);
	 return;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	af.owner     = ch;
	af.events    = EVENT_UPDATE;
	affect_to_room(ch->in_room, &af);

	char_puts("The room starts to be filled by poison.\n",ch);   
	act("The room starts to be filled by poison.\n",ch,NULL,NULL,TO_ROOM);
}

void spell_cursed_lands(int sn, int level, CHAR_DATA *ch, void *vo)
{
	ROOM_AFFECT_DATA af;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n",ch);
	  return;
	}
	if (IS_AFFECTED(ch->in_room, RAFF_CURSE))
	{
	 char_puts("This room has already been cursed.\n",ch);
	 return;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_CURSE;
	af.owner     = ch;
	af.events    = 0;
	affect_to_room(ch->in_room, &af);

	char_puts("The gods has forsaken the room.\n",ch);   
	act("The gos has forsaken the room.\n",ch,NULL,NULL,TO_ROOM);
}

void spell_lethargic_mist(int sn, int level, CHAR_DATA *ch, void *vo)
{
	 ROOM_AFFECT_DATA af;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n",  ch);
	  return;
	}
	if (is_affected_room(ch->in_room, sn))
	{
	 char_puts("This room has already been full of lethargic mist.\n",ch);
	 return;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	af.owner     = ch;
	af.events    = EVENT_UPDATE | EVENT_ENTER;
	affect_to_room(ch->in_room, &af);

	char_puts("The air in the room makes you slowing down.\n",ch);   
	act("The air in the room makes you slowing down.\n",ch,NULL,NULL,TO_ROOM);
}

void spell_black_death(int sn, int level, CHAR_DATA *ch, void *vo)
{
	ROOM_AFFECT_DATA af;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n",ch);
	  return;
	}
	if (is_affected_room(ch->in_room, sn))
	{
	 char_puts("This room has already been diseased.\n",ch);
	 return;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	af.owner     = ch;
	af.events    = EVENT_UPDATE;
	affect_to_room(ch->in_room, &af);

	char_puts("The room starts to be filled by disease.\n",ch);   
	act("The room starts to be filled by disease.\n",ch,NULL,NULL,TO_ROOM);
}

void spell_mysterious_dream(int sn, int level, CHAR_DATA *ch, void *vo)
{
	ROOM_AFFECT_DATA af;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n",  ch);
	  return;
	}
	if (is_affected_room(ch->in_room, sn))
	{
	 char_puts("This room has already been affected by sleep gas.\n",ch);
	 return;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	af.owner     = ch;
	af.events    = EVENT_UPDATE | EVENT_ENTER;
	affect_to_room(ch->in_room, &af);

	char_puts("The room starts to be seen good place to sleep.\n",ch);   
	act("The room starts to be seen good place to you.\n",ch,NULL,NULL,TO_ROOM);
}

void spell_polymorph(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	int race;
	race_t *r;

	if (is_affected(ch, sn) || is_affected(ch,gsn_deathen)) {
		char_puts("You are already polymorphed.\n",ch); 
		return;
	}

	if (target_name == NULL || target_name[0]=='\0') {
		char_puts("Usage: cast 'polymorph' <pcracename>.\n",ch); 
		return;
	}

	race = rn_lookup(target_name);
	r = race_lookup(race);
	if (!r || !r->race_pcdata || !r->race_pcdata->classes.nused) {
		char_puts("That is not a valid race to polymorph.\n",ch); 
		return;
	}

	if (IS_SET(r->form, FORM_UNDEAD)) {
		char_puts("You posess no necromantic powers to do this.\n",ch);
		return;
	}
	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= level/10;
	af.location	= APPLY_RACE;
	af.modifier	= race;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	act("$n polymorphes $mself to $t.", ch, r->name, NULL, TO_ROOM);
	act("You polymorph yourself to $t.", ch, r->name, NULL, TO_CHAR);
}

void spell_deathen(int sn, int level, CHAR_DATA *ch, void *vo)
{
        AFFECT_DATA af;
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        int race;
        race_t *r;

        if (is_affected(victim,sn)) {
	        act("$N is already decays.",ch,NULL,victim,TO_CHAR);
	        return;
        }

        if (saves_spell(level,victim, DAM_NEGATIVE)) {
	        act("You failed.", ch, NULL, NULL, TO_CHAR);
	        return;
	}

        race = rn_lookup("ghoul");
        r = RACE(race);
 
       if (!r->race_pcdata || !IS_SET(r->form, FORM_UNDEAD)) {
                char_puts("This is not a valid undead type.\n", ch);
                return;
        }
 

        af.where        = TO_AFFECTS;
        af.type         = gsn_deathen;
        af.level        = level;
        af.duration     = level/15;
        af.location     = APPLY_RACE;
        af.modifier     = race;
        af.bitvector    = 0;
        affect_to_char(victim, &af);

        act("$n's flesh starts to decay.", victim, NULL, NULL, TO_ROOM);
        act("Your flesh starts to decay.", victim, NULL, NULL, TO_CHAR);

}

void spell_lich(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	int race;
	race_t *r;
	int lev=0;

	if (is_affected(ch,sn) || is_affected(ch,gsn_deathen)) {
		act("Your flesh is already dead.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (IS_NULLSTR(target_name)) {
		char_puts("Usage: cast lich <type>", ch);
		return;
	}

	race = rn_lookup(target_name);
	r = RACE(race);
	if (!r->race_pcdata || !IS_SET(r->form, FORM_UNDEAD)) {
		char_puts("This is not a valid undead type.\n", ch);
		return;
	}

	if (!strcmp(r->name, "undead")) lev=45;
	if (!strcmp(r->name, "zombie")) lev=63;
	if (!strcmp(r->name, "lich"))   lev=81;

	if (ch->level<lev) {
		char_puts("You lack the power to do it.\n", ch);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= gsn_lich;
	af.level	= level;
	af.duration	= level/10;
	af.location	= APPLY_RACE;
	af.modifier	= race;
	af.bitvector	= 0;
	affect_to_char(ch, &af);

	act("$n deathens $mself, turning into $t.",
	    ch, r->name, NULL, TO_ROOM);
	act("You deathen yourself, turning into $t.",
	    ch, r->name, NULL, TO_CHAR);
}

void spell_plant_form(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;

	if (!ch->in_room
	||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
	||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
	||   (ch->in_room->sector_type != SECT_FOREST
		  &&   ch->in_room->sector_type != SECT_FIELD)
	||   IS_SET(ch->in_room->room_flags, ROOM_NORECALL))
	{
	 char_puts("Not here.\n",ch);
	 return;
	}

	af.where     = TO_AFFECTS;
	af.type      = gsn_fear;
	af.level     = level;
	af.duration  = level / 10;
	af.location  = 0;
	af.modifier  = 0;

	if (ch->in_room->sector_type == SECT_FOREST)
	{
	  char_puts("You start to look like a nearby tree!\n",ch);
	  act("$n starts look like a nearby tree!", ch, NULL, NULL,TO_ROOM);
	  af.bitvector = AFF_DETECT_FORM_TREE;
	}
	else
	{
	  char_puts("You start to look like some grass!\n",ch);
	  act("$n starts to look like some grass!", ch, NULL, NULL, TO_ROOM);
	  af.bitvector = AFF_DETECT_FORM_GRASS;
	}
	affect_to_char(ch, &af);
	return;
}

void spell_blade_barrier(int sn, int level,CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	act("Many sharp blades appear around $n and crash $N.",
	ch,NULL,victim,TO_ROOM);
	act("Many sharp blades appear around you and crash $N.",
	ch,NULL,victim,TO_CHAR);
	act("Many sharp blades appear around $n and crash you!",
	ch,NULL,victim,TO_VICT);

	dam = dice(level,5);
	if (saves_spell(level,victim,DAM_PIERCE))
	dam /= 3;
	damage(ch,victim,dam,sn,DAM_PIERCE,TRUE);

	act("The blade barriers crash $n!",victim,NULL,NULL,TO_ROOM);
	dam = dice(level,4);
	if (saves_spell(level,victim,DAM_PIERCE))
	dam /= 3;
	damage(ch,victim,dam,sn,DAM_PIERCE,TRUE);
	act("The blade barriers crash you!",victim,NULL,NULL,TO_CHAR);

	if (number_percent() < 75) return;

	act("The blade barriers crash $n!",victim,NULL,NULL,TO_ROOM);
	dam = dice(level,2);
	if (saves_spell(level,victim,DAM_PIERCE))
	dam /= 3;
	damage(ch,victim,dam,sn,DAM_PIERCE,TRUE);
	act("The blade barriers crash you!",victim,NULL,NULL,TO_CHAR);

	if (number_percent() < 50) return;

	act("The blade barriers crash $n!",victim,NULL,NULL,TO_ROOM);
	dam = dice(level,3);
	if (saves_spell(level,victim,DAM_PIERCE))
	dam /= 3;
	damage(ch,victim,dam,sn,DAM_PIERCE,TRUE);
	act("The blade barriers crash you!",victim,NULL,NULL,TO_CHAR);
}

void spell_protection_negative (int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;

	if (!is_affected(ch, sn))
	{
	  char_puts("You are now immune to negative attacks.\n", ch);

	  af.where = TO_IMMUNE;
	  af.type = sn;
	  af.duration = level / 4;
	  af.level = level;
	  af.bitvector = IMM_NEGATIVE;
	  af.location = 0;
	  af.modifier = 0;
	  affect_to_char(ch, &af);
	}
	else 
	  char_puts("You are already immune to negative attacks.\n", ch);
 return;
}

void spell_ruler_aura(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;

	if (!is_affected(ch, sn))
	{
	  char_puts("You now feel more self confident in rulership.\n", ch);

	  af.where = TO_IMMUNE;
	  af.type = sn;
	  af.duration = level / 4;
	  af.level = level;
	  af.bitvector = IMM_CHARM;
	  af.location = 0;
	  af.modifier = 0;
	  affect_to_char(ch, &af);
	}
	else 
	  char_puts("You are as much self confident as you can.\n", ch);
 return;
}

void spell_evil_spirit(int sn, int level, CHAR_DATA *ch, void *vo)
{
 AREA_DATA *pArea = ch->in_room->area;
 ROOM_INDEX_DATA *room;
 AFFECT_DATA af2;
 ROOM_AFFECT_DATA af;
 int i;

 if (IS_RAFFECTED(ch->in_room, RAFF_ESPIRIT)
	|| is_affected_room(ch->in_room,sn))
	{
	 char_puts("The zone is already full of evil spirit.\n",ch);
	 return;
	}

 if (is_affected(ch, sn))
	{
	  char_puts("Your power of evil spirit is less for you, now.\n",ch);
	  return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		char_puts("Holy aura in this room prevents your powers to work on it.\n",ch);
		return;
	}

	af2.where	= TO_AFFECTS;
	af2.type	= sn;
	af2.level	= level;
	af2.duration	= level / 5;
	af2.modifier	= 0;
	af2.location	= APPLY_NONE;
	af2.bitvector	= 0;
	affect_to_char(ch, &af2);

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 25;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_ESPIRIT;
	af.owner     = ch;
	af.events    = EVENT_UPDATE;

	for (i=pArea->min_vnum; i<pArea->max_vnum; i++)  
	{
	 if ((room = get_room_index(i)) == NULL) continue;
	 affect_to_room(room, &af);
	 if (room->people) 
	act("The zone is starts to be filled with evil spirit.",room->people,NULL,NULL,TO_ALL);
	}

}

void spell_disgrace(int sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim,sn) && !saves_spell(level, victim, DAM_MENTAL))
	{
	  af.where		    = TO_AFFECTS;
	  af.type               = sn;
	  af.level              = level; 
	  af.duration           = level;
	  af.location           = APPLY_CHA;
	  af.modifier           = - (5 + level / 10);
	  af.bitvector          = 0;
	  affect_to_char(victim,&af);

	  act("$N feels $M less confident!",ch,NULL,victim,TO_ALL);
	}
	else char_puts("You failed.\n",ch);
}

void spell_control_undead(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
 	AFFECT_DATA af;
 	race_t *r;	
 
 	if (count_charmed(ch))
 		return;
 
 	if (victim == ch) {
 		char_puts("You like yourself even better!\n", ch);
 		return;
 	}
 
 	r = RACE(victim->race);
  
 	if  ((!IS_NPC(victim) || !IS_SET(victim->pMobIndex->act, ACT_UNDEAD)) 
             && (!IS_SET(r->form, FORM_UNDEAD))) {
  		act("$N doesn't seem to be an undead.",ch,NULL,victim,TO_CHAR);
  		return;
  	}
 
 	if (!IS_NPC(victim) && !IS_NPC(ch)) 
 		level += get_curr_stat(ch, STAT_CHA) -
 			 get_curr_stat(victim, STAT_CHA); 
 
 
 	if (IS_IMMORTAL(victim)
	||  IS_AFFECTED(victim, AFF_CHARM)
 	||  IS_AFFECTED(ch, AFF_CHARM)
 	||  saves_spell(level, victim, DAM_OTHER) 
 	||  (IS_NPC(victim) && victim->pMobIndex->pShop != NULL)
 	||  (victim->in_room &&
 		IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)))
 			return;
 
 	if (is_safe(ch, victim))
 		return;
 
 	add_follower(victim, ch);
	set_leader(victim, ch);
 
 	af.where	= TO_AFFECTS;
 	af.type		= sn;
 	af.level	= level;
 	af.duration	= number_fuzzy(level / 5);
 	af.location	= 0;
 	af.modifier	= 0;
 	af.bitvector	= AFF_CHARM;
 	affect_to_char(victim, &af);
 	act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
 	act("$N looks at you with adoring eyes.",
 		    ch, NULL, victim, TO_CHAR);
 
 	if (IS_NPC(victim) && !IS_NPC(ch)) {
 		NPC(victim)->last_fought = ch;
 		if (number_percent() < (4 + (LEVEL(victim) - LEVEL(ch))) * 10)
 		 	add_mind(victim, ch->name);
 		else if (NPC(victim)->in_mind == NULL) {
 			NPC(victim)->in_mind =
				str_printf("%d", victim->in_room->vnum);
 		}
 	}
}

void spell_assist(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(ch, sn))
	  {
	char_puts("This power is used too recently.\n",ch);
	return;
	  }

	af.where	 = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 50;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);

	victim->hit += 100 + level * 5;
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	act("$n looks better.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}  

void spell_aid(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(ch, sn))
	  {
	char_puts("This power is used too recently.\n",ch);
	return;
	  }

	af.where	 = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 50;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);

	victim->hit += level * 5;
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	act("$n looks better.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}  

void spell_summon_shadow(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *gch;
	CHAR_DATA *shadow;
	AFFECT_DATA af;
	int i;

	if (is_affected(ch,sn))
	{
	  char_puts("You lack the power to summon another shadow right now.\n",
		   ch);
	  return;
	}

	char_puts("You attempt to summon a shadow.\n",ch);
	act("$n attempts to summon a shadow.",ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_SUM_SHADOW) {
			char_puts("Two shadows are more than you "
				  "can control!\n",ch);
			return;
		}
	}

	shadow = create_mob(get_mob_index(MOB_VNUM_SUM_SHADOW));

	for (i=0;i < MAX_STATS; i++)
	{
	  shadow->perm_stat[i] = ch->perm_stat[i];
	}

	shadow->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000)
		: URANGE(PC(ch)->perm_hit,ch->hit,30000);
	shadow->hit = shadow->max_hit;
	shadow->max_mana = IS_NPC(ch)? ch->max_mana : PC(ch)->perm_mana;
	shadow->mana = shadow->max_mana;
	shadow->level = level;
	for (i=0; i < 3; i++)
	shadow->armor[i] = interpolate(shadow->level,100,-100);
	shadow->armor[3] = interpolate(shadow->level,100,0);
	shadow->gold = 0;
	NPC(shadow)->dam.dice_number = number_range(level/15, level/10);   
	NPC(shadow)->dam.dice_type = number_range(level/3, level/2);
	shadow->damroll = number_range(level/8, level/6);

	act("A shadow conjures!",ch,NULL,NULL,TO_ALL);

	af.where		= TO_AFFECTS;
	af.type               = sn;
	af.level              = level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(ch, &af);  

	shadow->master = shadow->leader = ch;
	char_to_room(shadow,ch->in_room);
}

void spell_farsight(int sn, int level, CHAR_DATA *ch, void *vo)
{
	ROOM_INDEX_DATA *room;

	if (IS_NULLSTR(target_name)) {
		act("Farsight which direction?", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((room = check_place(ch, target_name)) == NULL) {
		char_puts("You cannot see that much far.\n", ch);
		return;
	}

	if (ch->in_room == room)
		dofun("look", ch, "auto");
	else 
		look_at(ch, room);
}

void spell_remove_fear(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (check_dispel(level,victim,gsn_fear))
	{
	char_puts("You feel more brave.\n",victim);
	act("$n looks more conscious.",victim,NULL,NULL,TO_ROOM);
	}
	else char_puts("You failed.\n",ch);
}

void spell_desert_fist(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if ((ch->in_room->sector_type != SECT_HILLS)
	&&  (ch->in_room->sector_type != SECT_MOUNTAIN)
	&&  (ch->in_room->sector_type != SECT_DESERT)) {
		char_puts("You don't find any sand here to create a fist.\n",
			  ch);
		ch->wait = 0;
		return;
	}

	act("An existing parcel of sand rises up and forms a fist and pummels $n.",
	    victim, NULL, NULL, TO_ROOM);
	act("An existing parcel of sand rises up and forms a fist and pummels you.",
	    victim, NULL, NULL, TO_CHAR);
	dam = dice(level, 14);
	sand_effect(victim, level, dam, TARGET_CHAR);
	damage(ch, victim, dam, sn, DAM_OTHER, TRUE);
}

void spell_mirror(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int mirrors, new_mirrors;
	CHAR_DATA *gch;
	int order;

	if (IS_NPC(victim)) {
		send_to_char("Only players can be mirrored.\n",ch);
		return;
	}

	for (mirrors = 0, gch = npc_list; gch; gch = gch->next)
		if (is_affected(gch, gsn_mirror)
		&&  is_affected(gch, gsn_doppelganger)
		&&  gch->doppel == victim)
			mirrors++;

	if (mirrors >= level/5) {
		if (ch == victim) 
			char_puts("You cannot be further mirrored.\n", ch);
		else
			act("$N cannot be further mirrored.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where	= TO_AFFECTS;
	af.level	= level;
	af.modifier	= 0;
	af.location	= 0;
	af.bitvector	= 0;

	order = number_range(0, level/5 - mirrors);

	for (new_mirrors = 0; mirrors + new_mirrors < level/5; new_mirrors++) {
		gch = create_mob(get_mob_index(MOB_VNUM_MIRROR_IMAGE));
		free_string(gch->name);
		gch->name = str_qdup(victim->name);
		mlstr_cpy(&gch->short_descr, &victim->short_descr);
		mlstr_printf(&gch->long_descr, &gch->pMobIndex->long_descr,
			     victim->name, PC(victim)->title);
		mlstr_cpy(&gch->description, &victim->description);
		gch->sex = victim->sex;
    
		af.type = gsn_doppelganger;
		af.duration = level;
		affect_to_char(gch, &af);

		af.type = gsn_mirror;
		af.duration = -1;
		affect_to_char(gch,&af);

		gch->max_hit = gch->hit = 1;
		gch->level = 1;
		gch->doppel = victim;
		gch->master = victim;

		if (ch == victim) {
			char_puts("A mirror image of yourself appears beside you!\n",
				  ch);
			act("A mirror image of $n appears beside $M!",
			    ch, NULL, victim, TO_ROOM);
		}
		else {
			act("A mirror of $N appears beside $M!",
			    ch, NULL, victim, TO_CHAR);
			act("A mirror of $N appears beside $M!",
			    ch,NULL,victim,TO_NOTVICT);
			char_puts("A mirror image of yourself appears beside you!\n",
				  victim);
		}

		char_to_room(gch, victim->in_room);
		if (new_mirrors == order) {
			char_from_room(victim);
			char_to_room(victim, gch->in_room);
			if (IS_EXTRACTED(victim))
				break;
		}
	}
}    
 
void spell_doppelganger(int sn, int level, CHAR_DATA *ch, void *vo)	
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (ch == victim || (is_affected(ch, sn) && ch->doppel == victim)) {
		act("You already look like $M.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_NPC(victim)) {
		act("$N is too different from yourself to mimic.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		char_puts("Yeah, sure. And I'm the Pope.\n",ch);
		return;
	}

	if (saves_spell(level, victim, DAM_CHARM)) {
		char_puts("You failed.\n", ch);
		return;
	}

	act("You change form to look like $N.", ch, NULL, victim, TO_CHAR);
	act("$n changes form to look like YOU!", ch, NULL, victim, TO_VICT);
	act("$n changes form to look like $N!", ch, NULL, victim, TO_NOTVICT);

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level; 
	af.duration	= 2 * level / 3;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= 0;

	affect_to_char(ch, &af); 
	ch->doppel = victim;
}
 
void spell_hunger_weapon(int sn, int level, CHAR_DATA *ch, void *vo)
{
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        AFFECT_DATA af;
	int chance;

        if (obj->pObjIndex->item_type != ITEM_WEAPON) {
        	char_puts("That's not a weapon.\n", ch);
        	return;
        } 

        if (obj->wear_loc != WEAR_NONE) {
        	char_puts("The item must be carried to be cursed.\n", ch);
        	return;
        } 

	if (IS_WEAPON_STAT(obj, WEAPON_HOLY)
	||  IS_OBJ_STAT(obj, ITEM_BLESS)
	||  IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)) {
		act("The gods are infuriated!", ch, NULL, NULL, TO_ALL);
		damage(ch, ch, (ch->hit - 1) > 1000 ? 1000 : (ch->hit - 1),
		       TYPE_HIT, DAM_HOLY, TRUE);
		return;
	} 

        if (IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)) {
        	act("$p is already hungry for enemy life.",
		    ch, obj, NULL, TO_CHAR);
        	return;
        }

	chance = get_skill(ch, sn);	

	if (IS_WEAPON_STAT(obj, WEAPON_FLAMING))	chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_FROST))		chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_SHARP))		chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_VORPAL))		chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_SHOCKING))	chance /= 2;
	 
        if (number_percent() < chance) {    
		af.where	= TO_WEAPON;
		af.type 	= sn;
		af.level	= level / 2;
		af.duration	= level/8;
		af.location	= 0;
		af.modifier	= 0;
		af.bitvector	= WEAPON_VAMPIRIC;
		affect_to_obj(obj, &af);
		SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD | ITEM_ANTI_NEUTRAL);
		act("You transmit part of your hunger to $p.",
		    ch, obj, NULL, TO_CHAR);
	} 
	else 
		act("You failed.", ch, obj, NULL, TO_ALL);
}

void spell_mana_restore(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	int restore;

	restore = 5 * level / 2;
	vch->mana = UMIN(vch->max_mana, vch->mana + restore);
	act("A warm glow passes through you.", vch, NULL, NULL, TO_CHAR);
	if (ch != vch) char_puts("Ok.\n", ch);
}

/*
	An alteration spell which enlarges a given person
*/

void spell_enlarge(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn)) {
		if (ch==victim)
			act("You are already as large as you can get.",
				ch, NULL, NULL, TO_CHAR);
		else	
			act("$N is already as large as $E can get.", 
				ch, NULL, victim, TO_CHAR);
		return;
	}

	if (check_trust(ch, victim) || 
	   !saves_spell(level, victim, DAM_NEGATIVE)) {

		if(is_affected(victim, sn_lookup("shrink"))) {
			affect_strip(victim, sn_lookup("shrink"));
			act("You grow back to your normal size.",
				victim, NULL, NULL, TO_CHAR);
			act("$n grows back to $s normal size.",
				victim, NULL, NULL, TO_ROOM);
			return;
		}
		af.where 	= TO_AFFECTS;
		af.where	= TO_AFFECTS;
		af.type		= sn;
		af.level	= level;
		af.duration	= 5 + level / 10;
		af.location	= APPLY_SIZE;
		af.modifier	= 1;
		af.bitvector	= 0;
		affect_to_char(victim, &af);

		af.location	= APPLY_STR;
		af.modifier	= 1;
		affect_to_char(victim, &af);

		af.location	= APPLY_DEX;
		af.modifier	= -1;
		affect_to_char(victim, &af);

		af.location	= APPLY_DAMROLL;
		af.modifier	= level/18+1;
		affect_to_char(victim, &af);
		
		act("Your body suddenly becomes much larger.", 
			victim, NULL, NULL, TO_CHAR);
		act("$n suddenly seems to become much larger.",
			victim, NULL, NULL, TO_ROOM);
	}
	else char_puts("You failed.", ch);
}
/* 
   A reversed enlarge spell.
*/

void spell_shrink(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn)) {
		if (ch==victim)
			act("You are already as small as you can get.",
				ch, NULL, NULL, TO_CHAR);
		else	
			act("$N is already as small as $E can get.", 
				ch, NULL, victim, TO_CHAR);
		return;
	}

	if (check_trust(ch, victim) || 
	   !saves_spell(level, victim, DAM_NEGATIVE)) {

		if(is_affected(victim, sn_lookup("enlarge"))) {
			affect_strip(victim, sn_lookup("enlarge"));
			act("You shrink back to your normal size.",
				victim, NULL, NULL, TO_CHAR);
			act("$n shrink back to $s normal size.",
				victim, NULL, NULL, TO_ROOM);
			return;
		}
		af.where 	= TO_AFFECTS;
		af.where	= TO_AFFECTS;
		af.type		= sn;
		af.level	= level;
		af.duration	= 5 + level / 10;
		af.location	= APPLY_SIZE;
		af.modifier	= -1;
		af.bitvector	= 0;
		affect_to_char(victim, &af);

		af.location	= APPLY_STR;
		af.modifier	= -1;
		affect_to_char(victim, &af);

		af.location	= APPLY_DEX;
		af.modifier	= 1;
		affect_to_char(victim, &af);

		af.location	= APPLY_HITROLL;
		af.modifier	= level/18+1;
		affect_to_char(victim, &af);
		
		act("Your body suddenly becomes much smaller.", 
			victim, NULL, NULL, TO_CHAR);
		act("$n suddenly seems to become much smaller.",
			victim, NULL, NULL, TO_ROOM);
	}
	else char_puts("You failed.", ch);
}

void spell_water_breathing(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level; 
	af.duration	= level / 12;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_WATER_BREATHING;
	affect_to_char(vch, &af); 
	char_puts("Breathing seems a bit easier.\n", vch);
}

void spell_free_action(int sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	af.where	= TO_AFFECTS;
	af.type		= gsn_free_action;
	af.level	= level; 
	af.duration	= level / 12;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_SWIM;
	affect_to_char(vch, &af); 
	char_puts("You can move easier.\n", vch);
}

void spell_find_familiar(int sn, int level, CHAR_DATA *ch, void *vo) 
{	
	CHAR_DATA *familiar=NULL;
	CHAR_DATA *gch;
	int i;
	
	int chance;
	int vnum;
	bool new=TRUE;
	chance = number_percent();

	if (chance < 60)
		vnum = MOB_VNUM_BLACK_CAT;
	else	
		vnum = MOB_VNUM_BLACK_CROW;

	for(gch=npc_list; gch; gch=gch->next) {
		if (IS_SET(gch->pMobIndex->act, ACT_FAMILIAR)
		&& gch->master == ch) {
			familiar = gch;
			new = FALSE;
			break;
		}
	}

	if (!familiar) {
		familiar = create_mob(get_mob_index(vnum));

		switch(vnum) {
		case MOB_VNUM_BLACK_CAT:
			familiar->perm_stat[STAT_STR] = 12;
			familiar->perm_stat[STAT_INT] = 23;
			familiar->perm_stat[STAT_DEX] = 24;
			familiar->perm_stat[STAT_WIS] = 12;
			familiar->perm_stat[STAT_CON] = 14;
			familiar->perm_stat[STAT_CHA] = get_curr_stat(ch, STAT_CHA)-1;
			break;
		case MOB_VNUM_BLACK_CROW:
			familiar->perm_stat[STAT_STR] = 13;
			familiar->perm_stat[STAT_INT] = 18;
			familiar->perm_stat[STAT_DEX] = 14;
			familiar->perm_stat[STAT_WIS] = 24;
			familiar->perm_stat[STAT_CON] = 13;
			familiar->perm_stat[STAT_CHA] = get_curr_stat(ch, STAT_CHA)-4;
			break;
		}

		for (i=0; i<MAX_STATS; i++)
			familiar->perm_stat[i] += number_range(-1,1);
	}
	
	familiar->max_hit = ch->max_hit/2 + (ch->max_hit)*get_curr_stat(familiar, STAT_CON)/50;
	familiar->max_mana = ch->max_mana * (get_curr_stat(familiar, STAT_INT) + get_curr_stat(familiar, STAT_WIS))/48;
	familiar->level = ch->level;
	familiar->hit = familiar->max_hit;
	familiar->mana = familiar->max_mana;
	for (i=0; i<4; i++)  
		familiar->armor[i] = interpolate(familiar->level,100,-100);
	
	familiar->gold = 0;

	NPC(familiar)->dam.dice_number = 5;
	NPC(familiar)->dam.dice_type   = 5;

	familiar->damroll = level/3;

	familiar->master = familiar->leader = ch;
	
	if (!new)
		transfer_char(familiar, ch, ch->in_room,
			"$N disappears suddenly.",
			"$n has summoned you!",
			"$N arrives suddenly.");
	else {
		char_to_room(familiar, ch->in_room);

		act("You attempt to find a familiar.", ch, NULL, NULL, TO_CHAR);
		act("$N comes to serve you.", ch, NULL, familiar, TO_CHAR);
		act("$N comes to serve $n.", ch, NULL, familiar, TO_NOTVICT);
	}
}

/* function for some necromancers area-attack */
int damage_to_all_in_room(int sn, int level, CHAR_DATA *ch,
				 ROOM_INDEX_DATA *room, int door)
{
        CHAR_DATA *vch, *vch_next;
	int dam, v_counter;

	if (sn == gsn_death_ripple) {
		dam = dice(level,9);
	} else {
		dam = dice(level,14);
	}
	v_counter = 0;
	for (vch = room->people; vch != NULL; vch = vch_next) {
		vch_next = vch->next_in_room;


                if (is_safe_spell(ch,vch,TRUE)
                ||  (IS_NPC(vch) && IS_NPC(ch))
                ||  (IS_NPC(vch) 
                    && IS_SET(vch->pMobIndex->act, ACT_NOTRACK)))
	                continue;


		v_counter++;
                if (saves_spell(level,vch,DAM_NEGATIVE)) {
                	damage(ch, vch, dam/2, sn, DAM_NEGATIVE, TRUE);
                } else {
                	damage(ch, vch, dam, sn, DAM_NEGATIVE, TRUE);
                }

		if(door != -1 && IS_NPC(vch)) 
	                path_to_track(ch, vch, door);
        }
	return v_counter;
}

void spell_death_ripple(int sn, int level, CHAR_DATA *ch, void *vo)
{
	ROOM_INDEX_DATA *prev_room, *next_room;
	EXIT_DATA *exit;
	int v_counter, range, door, i;

	prev_room = ch->in_room;
	range = level/10;

	act("You feel $n's deadly wave passing through your body.", ch,
						 NULL, NULL, TO_ROOM);
        act("Deadly wave emanates from you.", ch,
                                                 NULL, NULL, TO_CHAR);
	v_counter = damage_to_all_in_room(sn, level, ch, prev_room, -1);

        if (IS_NULLSTR(target_name)||!check_blind(ch))	
		return;
	
        switch (target_name[0]) {
        case 'N':
        case 'n':
                door = 0;
                break;
        case 'E':
        case 'e':
                door = 1;
                break;
        case 'S':
        case 's':
                door = 2;
                break;
        case 'W':
        case 'w':
                door = 3;
                break;
        case 'U':
        case 'u':
                door = 4;
                break;
        case 'D':
        case 'd':
                door = 5;
                break;
        default:
                char_puts("Wrong direction.\n", ch);
                return;
        }

        for (i = 1; i <= range; i++) {
                exit = prev_room->exit[door];

                if (exit == NULL 
                || (next_room = exit->to_room.r) == NULL
                || IS_SET(exit->exit_info, EX_CLOSED)
		|| v_counter > level/10) 
                        return;
		 
		if (next_room->exit[rev_dir[door]] ==  NULL
                || !can_see_room(ch, next_room)		
		|| IS_SET(next_room->exit[rev_dir[door]]->exit_info, EX_CLOSED))
			return;

		act("You feel breathing from the $T", next_room->people, NULL, from_dir_name[rev_dir[door]], TO_ALL);
		v_counter += damage_to_all_in_room(sn, level, ch, next_room, door);	
		prev_room = next_room ;
	}
	
}

