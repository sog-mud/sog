/*
 * $Id: prayers.c,v 1.12 2001-07-31 18:14:54 fjoe Exp $
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

#include <stdio.h>

#include <merc.h>

#include <handler.h>

#include <magic.h>
#include "magic_impl.h"

void
prayer_detect_good(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_GOOD)) {
		if (victim == ch)
			act_char("You can already sense good.", ch);
		else {
			act("$N can already detect good.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= (5 + level / 3);
	paf->bitvector	= ID_GOOD;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void
prayer_detect_evil(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_EVIL)) {
		if (victim == ch)
			act_char("You can already sense evil.", ch);
		else {
			act("$N can already detect evil.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= (5 + level / 3);
	paf->bitvector	= ID_EVIL;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void
prayer_detect_poison(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD) {
		if (INT(obj->value[3]) != 0)
			act_char("You smell poisonous fumes.", ch);
		else {
			act_puts("$p looks delicious.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		}
	} else {
		act_puts("$p doesn't look poisoned.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
	}
}

void
prayer_refresh(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->move = UMIN(victim->move + level, victim->max_move);
	if (victim->max_move == victim->move)
		act_char("You feel fully refreshed!", victim);
	else
		act_char("You feel less tired.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void
prayer_restoring_light(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA *paf;

	if (is_affected_room(ch->in_room, sn)) {
		act_char("This room is already lit with magic light.", ch);
		return;
	}

	if (is_affected(ch, sn)) {
		act_char("Rest a while, you're tired from previous one.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 10;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->duration	= level / 25;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act("The room becomes lit with warm light.", ch, NULL, NULL, TO_ROOM);
	act_char("Ok.", ch);
}

void
prayer_word_of_recall(const char *sn, int level, CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *location;
	CHAR_DATA *pet;

	if (IS_NPC(victim))
		return;

	if (victim->fighting
	&&  !can_flee(victim)) {
		if (victim == ch)
			act_char("Your honour doesn't let you recall!", ch);
		else {
			act_puts("You can't cast this spell to a honourable fighting $t!",
				 ch, victim->class, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		}
		return;
	}

	if (victim->desc && IS_PUMPED(victim)) {
		act_puts("You are too pumped to pray now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	act("$n prays for transportation!", ch, NULL, NULL, TO_ROOM);

	if (IS_SET(victim->in_room->room_flags, ROOM_NORECALL)
	||  IS_AFFECTED(victim, AFF_CURSE)
	||  IS_AFFECTED(victim->in_room, RAFF_CURSE)) {
		act_char("Your god doesn't hear you.", ch);
		return;
	}

	if (victim->fighting) {
		if (victim == ch && !IS_NPC(ch))
			gain_exp(victim, 0 - (victim->level + 25));
		stop_fighting(victim, TRUE);
	}

	ch->move /= 2;
	pet = GET_PET(victim);
	location = get_recall(victim);
	recall(victim, location);

	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			dofun("stand", pet, str_empty);
		recall(pet, location);
	}
}

static void *
inspire_cb(void *vo, va_list ap)
{
	CHAR_DATA *gch = (CHAR_DATA *) vo;

	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	AFFECT_DATA *paf;

	if (!is_same_group(gch, ch))
		return NULL;

	if (spellbane(gch, ch, 100, dice(3, LEVEL(gch)))) {
		if (IS_EXTRACTED(ch))
			return gch;
		return NULL;
	}

	if (is_affected(gch, "bless")) {
		if (gch == ch)
			act_char("You are already inspired.", ch);
		else
			act("$N is already inspired.", ch, NULL, gch, TO_CHAR);
		return NULL;
	}

	paf = aff_new(TO_AFFECTS, "bless");
	paf->level	= level;
	paf->duration	= level + 6;

	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= level/12;
	affect_to_char(gch, paf);

	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= -level/12;
	affect_to_char(gch, paf);
	aff_free(paf);

	act_char("You feel inspired!", gch);
	if (ch != gch) {
		act("You inspire $N with the Creator's power!",
		    ch, NULL, gch, TO_CHAR);
	}

	return NULL;
}

void
prayer_inspire(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	vo_foreach(ch->in_room, &iter_char_room, inspire_cb, level, ch);
}

void
prayer_heal(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn, 100 + level / 10);
}

void
prayer_master_healing(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn,
	   300 + level + dice(1,40));
}

void
prayer_group_heal(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if (is_same_group(ch, gch)
		&&  !has_spec(gch, "clan_battleragers")) {
			spellfun_call("master healing", NULL, level, ch, gch);
			spellfun_call("refresh", NULL, level, ch, gch);
		}
	}
}

void
prayer_superior_heal(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn,
	    170 + level + dice(1, 20));
}

void
prayer_mass_healing(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if ((IS_NPC(ch) && IS_NPC(gch))
		||  (!IS_NPC(ch) && !IS_NPC(gch))) {
			spellfun_call("heal", NULL, level, ch, gch);
			spellfun_call("refresh", NULL, level, ch, gch);
		}
	}
}

void
prayer_dispel_good(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && IS_GOOD(ch))
		victim = ch;

	if (IS_EVIL(victim)) {
		if (ch == victim)
			act_char("You are protected by your evil.", ch);
		else {
			act("$N is protected by $S evil.",
			    ch, NULL, victim, TO_ROOM);
		}
		return;
	}

	if (IS_NEUTRAL(victim)) {
		if (ch == victim)
			act_char("You do not seem to be affected.", ch);
		else {
			act("$N does not seem to be affected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (victim->hit > (level * 4))
		dam = dice(level, 4);
	else
		dam = UMAX(victim->hit, dice(level,4));
	if (saves_spell(level, victim,DAM_NEGATIVE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_NEGATIVE, DAMF_SHOW);
}

void
prayer_dispel_evil(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && IS_EVIL(ch))
		victim = ch;

	if (IS_GOOD(victim)) {
		if (ch == victim)
			act_char("Gods protect you.", ch);
		else
			act("Gods protect $N.", ch, NULL, victim, TO_ROOM);
		return;
	}

	if (IS_NEUTRAL(victim)) {
		if (ch == victim)
			act_char("You do not seem to be affected.", ch);
		else {
			act("$N does not seem to be affected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (victim->hit > (level * 4))
		dam = dice(level, 4);
	else
		dam = UMAX(victim->hit, dice(level,4));
	if (saves_spell(level, victim,DAM_HOLY))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_HOLY, DAMF_SHOW);
}

void
prayer_cure_light_wounds(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn,
	    dice(1, 8) + level / 4 + 5);
}

void
prayer_cure_serious_wounds(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn,
	    dice(2, 8) + level / 3 + 10);
}

void
prayer_cure_critical_wounds(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn, dice(3, 8) + level);
}

void
prayer_cure_blindness(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim, "blindness")) {
		if (victim == ch)
		  act_char("You aren't blind.", ch);
		else
		  act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (check_dispel(level, victim, "blindness")) {
		act_char("Your vision returns!", victim);
		act("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
	} else
		act_char("Your god doesn't hear you.", ch);
}

void
prayer_cure_poison(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim, "poison")) {
		if (victim == ch)
			act_char("You aren't poisoned.", ch);
		else {
			act("$N doesn't appear to be poisoned.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (check_dispel(level, victim, "poison")) {
		act_char("A warm feeling runs through your body.", victim);
		act("$n looks much better.", victim, NULL, NULL, TO_ROOM);
	} else
		act_char("Your god doesn't hear you.", ch);
}

/* RT added to cure plague */
void
prayer_cure_disease(const char *sn, int level, CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim, "plague")) {
		if (victim == ch)
			act_char("You aren't ill.", ch);
		else {
			act("$N doesn't appear to be diseased.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (check_dispel(level, victim, "plague")) {
		act("$n looks relieved as $s sores vanish.",
		    victim, NULL, NULL, TO_ROOM);
	} else
		act_char("Your god doesn't hear you.", ch);
}

static void *
group_defense_cb(void *vo, va_list ap)
{
	CHAR_DATA *gch = (CHAR_DATA *) vo;

	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if (!is_same_group(gch, ch))
		return NULL;

	if (spellbane(gch, ch, 100, dice(2, LEVEL(gch)))) {
		if (IS_EXTRACTED(ch))
			return gch;
		return NULL;
	}

	if (is_affected(gch, "armor")) {
		if (gch == ch)
			act_char("You are already armored.", ch);
		else
			act("$N is already armored.", ch, NULL, gch, TO_CHAR);
	}  else {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, "armor");
		paf->level     = level;
		paf->duration  = level;
		INT(paf->location) = APPLY_AC;
		paf->modifier  = -20;
		affect_to_char(gch, paf);
		aff_free(paf);

		act_char("You feel someone protecting you.", gch);
		if (ch != gch) {
			act("$N is protected by your magic.",
			    ch, NULL, gch, TO_CHAR);
		}
	}

	if (is_affected(gch, "shield")) {
		if (gch == ch)
			act_char("You are already shielded.", ch);
		else
			act("$N is already shielded.", ch, NULL, gch, TO_CHAR);
	} else {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, "shield");
		paf->level	= level;
		paf->duration	= level;
		INT(paf->location)= APPLY_AC;
		paf->modifier	= -20;
		affect_to_char(gch, paf);
		aff_free(paf);

		act_char("You are surrounded by a force shield.", gch);
		if(ch != gch) {
			act("$N is surrounded by a force shield.",
			    ch, NULL, gch, TO_CHAR);
		}
	}

	return NULL;
}

void
prayer_group_defense(const char *sn, int level, CHAR_DATA *ch, void *vo )
{
	vo_foreach(ch->in_room, &iter_char_room, group_defense_cb, level, ch);
}

static void *
turn_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int dam, align;

	if (is_safe_spell(ch, vch, TRUE))
		return NULL;
	if (IS_EVIL(ch)) {
		vch = ch;
		act_char("The energy explodes inside you!", ch);
	}

	if (vch != ch) {
		act("$n raises $s hand, and a blinding ray of light shoots forth!",
		    ch, NULL, NULL, TO_ROOM);
		act_char("You raise your hand and a blinding ray of light shoots forth!", ch);
	}

	if (IS_GOOD(vch) || IS_NEUTRAL(vch)) {
		act("$n seems unharmed by the light.",
		    vch, NULL, vch, TO_ROOM);
		act_char("The light seems powerless to affect you.", vch);
		return NULL;
	}

	dam = dice(level, 10);
	if (saves_spell(level, vch, DAM_HOLY))
		dam /= 2;

	align = vch->alignment - 350;
	if (align < -1000)
		align = -1000 + (align + 1000) / 3;

	dam = dam * align * align / 1000000;
	damage(ch, vch, dam, sn, DAM_HOLY, DAMF_SHOW);
	if (!IS_EXTRACTED(vch))
		dofun("flee", vch, str_empty);
	return NULL;
}

void
prayer_turn(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA *paf;

	if (is_affected(ch, sn)) {
		act_char("This power is used too recently.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 5;
	affect_to_char(ch, paf);
	aff_free(paf);

	vo_foreach(ch->in_room, &iter_char_room, turn_cb, sn, level, ch);
}

void
prayer_mana_restore(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	int restore;

	restore = 5 * level / 2;
	vch->mana = UMIN(vch->max_mana, vch->mana + restore);
	act("A warm glow passes through you.", vch, NULL, NULL, TO_CHAR);
	if (ch != vch) act_char("Ok.", ch);
}

void
prayer_severity_force(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	act_puts("You cracked the ground towards the $N.",
		 ch, NULL, victim, TO_CHAR, POS_DEAD);
	act("$n cracked the ground towards you!", ch, NULL, victim, TO_VICT);
	dam = dice(level, 12);
	damage(ch, victim, dam, sn, DAM_NONE, DAMF_SHOW);
}

#define OBJ_VNUM_SPRING			22

void
prayer_create_spring(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	OBJ_DATA *spring;

	spring = create_obj(get_obj_index(OBJ_VNUM_SPRING), 0);
	spring->timer = level;
	obj_to_room(spring, ch->in_room);
	act("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
	act("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
}

#define OBJ_VNUM_MUSHROOM		20

void
prayer_create_food(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *mushroom;

	mushroom = create_obj(get_obj_index(OBJ_VNUM_MUSHROOM), 0);
	INT(mushroom->value[0]) = level / 2;
	INT(mushroom->value[1]) = level;
	mushroom->level = level;
	obj_to_room(mushroom, ch->in_room);
	act("$p suddenly appears.", ch, mushroom, NULL, TO_ALL);
}

void
prayer_create_water(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int water;

	if (obj->item_type != ITEM_DRINK_CON) {
		act_char("It is unable to hold water.", ch);
		return;
	}

	if (str_cmp(STR(obj->value[2]), "water")
	&&  INT(obj->value[1]) != 0) {
		act_char("It contains some other liquid.", ch);
		return;
	}

	water = UMIN(level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		     INT(obj->value[0]) - INT(obj->value[1]));

	if (water > 0) {
		STR_ASSIGN(obj->value[2], str_dup("water"));
		INT(obj->value[1]) += water;

		if (!IS_OBJ_NAME(obj, "water"))
			label_add(obj, "water");
		act("$p is filled.", ch, obj, NULL, TO_CHAR);
	}
}

void
prayer_resilience(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	if (!is_affected(ch, sn)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_RESIST, sn);
		paf->duration	= level / 10;
		paf->level	= level;
		INT(paf->location)= DAM_NEGATIVE;
		paf->modifier	= 45;
		affect_to_char(ch, paf);
		aff_free(paf);

		act_char("You are now resistive to draining attacks.", ch);
	} else
		act_char("You are already resistive to draining attacks.", ch);
}

static void *
mass_sanctuary_cb(void *vo, va_list ap)
{
	CHAR_DATA *gch = (CHAR_DATA *) vo;

	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if (!is_same_group(gch, ch))
		return NULL;

	if (spellbane(gch, ch, 100, dice(3, LEVEL(gch)))) {
		if (IS_EXTRACTED(ch))
			return gch;
		return NULL;
	}

	spellfun_call("sanctuary", NULL, level, ch, gch);
	return NULL;
}

void
prayer_mass_sanctuary(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	vo_foreach(ch->in_room, &iter_char_room, mass_sanctuary_cb, level, ch);
}

void
prayer_lightning_bolt(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level,4) + 12;
	if (saves_spell(level, victim,DAM_LIGHTNING))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
}

void
prayer_sanctify_lands(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	if (number_bits(1) == 0) {
		act_char("You failed.", ch);
		return;
	}

	if (is_affected_room(ch->in_room, "cursed lands")) {
		affect_strip_room(ch->in_room, "cursed lands");
		act_char("The curse of the land wears off.", ch);
		act("The curse of the land wears off.",
		    ch, NULL, NULL, TO_ROOM);
	}

	if (is_affected_room(ch->in_room, "deadly venom")) {
		affect_strip_room(ch->in_room, "deadly venom");
		act_char("The land seems more healthy.", ch);
		act("The land seems more healthy.", ch, NULL, NULL, TO_ROOM);
	}

	if (is_affected_room(ch->in_room, "mysterious dream")) {
		affect_strip_room(ch->in_room, "mysterious dream");
		act_char("The land wake up from mysterious dream.", ch);
		act("The land wake up from mysterious dream.",
		    ch, NULL, NULL, TO_ROOM);
	}

	if (is_affected_room(ch->in_room, "black death")) {
		act_char("The disease of the land has been treated.", ch);
		act("The disease of the land has been treated.",
		    ch, NULL, NULL, TO_ROOM);
		affect_strip_room(ch->in_room, "black death");
	}

	if (is_affected_room(ch->in_room, "lethargic mist")) {
		act_char("The lethargic mist dissolves.", ch);
		act("The lethargic mist dissolves.", ch, NULL, NULL, TO_ROOM);
		affect_strip_room(ch->in_room, "lethargic mist");
	}
}

void
prayer_cursed_lands(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (IS_AFFECTED(ch->in_room, RAFF_CURSE)) {
		act_char("This room has already been cursed.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->bitvector	= RAFF_CURSE;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The gods has forsaken the room.", ch);
	act("The gods has forsaken the room.", ch, NULL, NULL, TO_ROOM);
}

void
prayer_desert_fist(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if ((ch->in_room->sector_type != SECT_HILLS)
	&&  (ch->in_room->sector_type != SECT_MOUNTAIN)
	&&  (ch->in_room->sector_type != SECT_DESERT)) {
		act_char("You don't find any sand here to create a fist.", ch);
		ch->wait = 0;
		return;
	}

	act("An existing parcel of sand rises up and forms a fist and pummels $n.",
	    victim, NULL, NULL, TO_ROOM);
	act("An existing parcel of sand rises up and forms a fist and pummels you.",
	    victim, NULL, NULL, TO_CHAR);
	dam = dice(level, 14);
	sand_effect(victim, level, dam);
	damage(ch, victim, dam, sn, DAM_OTHER, DAMF_SHOW);
}

/* RT calm spell stops all fighting in the room */
void
prayer_calm(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch;
	int mlevel = 0;
	int count = 0;
	int high_level = 0;
	int chance;

	/* get sum of all mobile levels in the room */
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->position == POS_FIGHTING) {
			count++;
			if (IS_NPC(vch))
				mlevel += LEVEL(vch);
			else
				mlevel += LEVEL(vch)/2;
			high_level = UMAX(high_level, LEVEL(vch));
		}
	}

	/* compute chance of stopping combat */
	chance = 4 * level - high_level + 2 * count;

	if (IS_IMMORTAL(ch)) /* always works */
		mlevel = 0;

	if (number_range(0, chance) >= mlevel) { /* hard to stop large fights */
		for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
			AFFECT_DATA *paf;

			if (IS_SET(vch->form, FORM_UNDEAD)
			||  IS_SET(vch->form, FORM_CONSTRUCT))
				continue;

			if (IS_AFFECTED(vch, AFF_CALM | AFF_BERSERK)
			||  is_affected(vch, "frenzy"))
				continue;

			act_char("A wave of calm passes over you.", vch);
			if (vch->fighting || vch->position == POS_FIGHTING)
				stop_fighting(vch, FALSE);

			paf = aff_new(TO_AFFECTS, sn);
			paf->level = level;
			paf->duration = level/4;
			INT(paf->location) = APPLY_HITROLL;
			if (!IS_NPC(vch))
				paf->modifier = -5;
			else
				paf->modifier = -2;
			paf->bitvector = AFF_CALM;
			affect_to_char(vch, paf);

			INT(paf->location) = APPLY_DAMROLL;
			affect_to_char(vch, paf);
			aff_free(paf);
		}
	}
}

void
prayer_harm(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = UMAX( 20, victim->hit - dice(1,4));
	if (saves_spell(level, victim,DAM_HARM))
		dam = UMIN(50, dam / 2);
	dam = UMIN(100, dam);
	damage(ch, victim, dam, sn, DAM_HARM, DAMF_SHOW);
}

void
prayer_wrath(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	AFFECT_DATA *paf;

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
	damage(ch, victim, dam, sn, DAM_HOLY, DAMF_SHOW);
	if (IS_EXTRACTED(victim))
		return;

	if (IS_AFFECTED(victim, AFF_CURSE)
	||  saves_spell(level, victim, DAM_HOLY))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 2*level;
	INT(paf->location) = APPLY_HITROLL;
	paf->modifier	= -1 * (level / 8);
	affect_to_char(victim, paf);

	INT(paf->location) = APPLY_SAVING_SPELL;
	paf->modifier	= level / 8;
	paf->bitvector	= AFF_CURSE;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel unclean.", victim);
	if (ch != victim)
		act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
}

/*
 * can be called with ch == NULL, vo == ch (e.g.: obj prog of tattoo of venus)
 */
void
prayer_bless(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA *paf;

	/* deal with the object case first */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		AFFECT_DATA *paf;

		if (IS_OBJ_STAT(obj,ITEM_BLESS)) {
			act("$p is already blessed.", ch, obj, NULL, TO_CHAR);
			return;
		}

		if (IS_OBJ_STAT(obj, ITEM_EVIL)) {
			paf = affect_find(obj->affected, "curse");
			if (!saves_dispel(level,paf != NULL ? paf->level : obj->level, 0)) {
				if (paf != NULL)
					affect_remove_obj(obj,paf);
				act("$p glows a pale blue.",
				    ch, obj, NULL, TO_ALL);
				REMOVE_OBJ_STAT(obj, ITEM_EVIL);
				return;
			} else {
				act("The evil of $p is too powerful for you to overcome.",
				    ch, obj, NULL, TO_CHAR);
				return;
			}
		}

		paf = aff_new(TO_OBJECT, sn);
		paf->level	= level;
		paf->duration	= (6 + level / 2);
		INT(paf->location)= APPLY_SAVES;
		paf->modifier	= -1;
		paf->bitvector	= ITEM_BLESS;
		affect_to_obj(obj, paf);
		aff_free(paf);

		act("$p glows with a holy aura.", ch, obj, NULL, TO_ALL);
		return;
	}

	victim = (CHAR_DATA *) vo;

	if (is_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already blessed.", ch);
		else if (ch) {
			act("$N already has divine favor.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (6 + level / 2);
	INT(paf->location)= APPLY_LUCK;
	paf->modifier	= UMAX(1, level / 8);
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel righteous.", victim);
	if (ch && ch != victim) {
		act("You grant $N the favor of your god.",
		    ch, NULL, victim, TO_CHAR);
	}
}

void
prayer_inflict_light_wounds(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	focus_negative_energy(ch, (CHAR_DATA *) vo, sn, dice(1, 8) + level / 3);
}

void
prayer_inflict_serious_wounds(const char *sn, int level,
			      CHAR_DATA *ch,void *vo)
{
	focus_negative_energy(ch, (CHAR_DATA *) vo, sn, dice(2, 8) + level / 2);
}

void
prayer_inflict_critical_wounds(const char *sn, int level,
			       CHAR_DATA *ch, void *vo)
{
	focus_negative_energy(ch, (CHAR_DATA *) vo, sn, dice(3, 8) + level - 6);
}

/* RT replacement demonfire spell */
void
prayer_demonfire(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && !IS_EVIL(ch)) {
		victim = ch;
		act_char("The demons turn upon you!", ch);
	}

	if (victim != ch) {
		act("$n calls forth the demons of Hell upon $N!",
		    ch,NULL,victim,TO_ROOM);
		act("$n has assailed you with the demons of Hell!",
		    ch,NULL,victim,TO_VICT);
		act_char("You conjure forth the demons of hell!", ch);
	}
	dam = dice(level, 10);
	if (saves_spell(level, victim,DAM_NEGATIVE))
		dam /= 2;
	spellfun_call("curse", NULL, 3 * level / 4, ch, victim);
	damage(ch, victim, dam, sn, DAM_NEGATIVE, DAMF_SHOW);
}

/* mental */
void
prayer_mind_wrack(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 7);
	if (saves_spell(level, victim, DAM_MENTAL))
		dam /= 2;
	act("$n stares intently at $N, causing $N to seem very lethargic.",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_MENTAL, DAMF_SHOW);
}

void
prayer_remove_fear(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (check_dispel(level, victim, "fear"))
	{
	act_char("You feel more brave.", victim);
	act("$n looks more conscious.", victim, NULL, NULL, TO_ROOM);
	}
	else act_char("You failed.", ch);
}

static void *
call_lightning_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int dam = va_arg(ap, int);

	if (vch->in_room == ch->in_room) {
		if (is_safe_spell(ch, vch, TRUE))
			return NULL;
		damage(ch, vch,
		       saves_spell(level, vch, DAM_LIGHTNING) ?
		       dam / 2 : dam,
		       sn, DAM_LIGHTNING, DAMF_SHOW);
		return NULL;
	}

	if (vch->in_room->area == ch->in_room->area
	&&  IS_OUTSIDE(vch)
	&&  IS_AWAKE(vch))
		act_char("Lightning flashes in the sky.", vch);
	return NULL;
}

void
prayer_call_lightning(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	int dam;

	if (!IS_OUTSIDE(ch)) {
		act_char("You must be out of doors.", ch);
		return;
	}

	if (weather_info.sky < SKY_RAINING) {
		act_char("You need bad weather.", ch);
		return;
	}

	dam = dice(level, 9);
	act_char("Gods' lightning strikes your foes!", ch);
	act("$n calls lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM);
	vo_foreach(NULL, &iter_char_world, call_lightning_cb,
		   sn, level, ch, dam);
}

void
prayer_protection_good(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD)
	||  IS_AFFECTED(victim, AFF_PROTECT_EVIL)) {
		if (victim == ch)
			act_char("You are already protected.", ch);
		else {
			act("$N is already protected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (10 + level / 5);
	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= -(1+level/10);
	paf->bitvector	= AFF_PROTECT_GOOD;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel aligned with darkness.", victim);
	if (ch != victim)
		act("$N is protected from good.", ch, NULL, victim, TO_CHAR);
}

void
prayer_protection_evil(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL)
	||  IS_AFFECTED(victim, AFF_PROTECT_GOOD)) {
		if (victim == ch)
			act_char("You are already protected.", ch);
		else {
			act("$N is already protected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (10 + level / 5);
	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= -(1 + level/10);
	paf->bitvector	= AFF_PROTECT_EVIL;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel holy and pure.", victim);
	if (ch != victim)
		act("$N is protected from evil.", ch, NULL, victim, TO_CHAR);
}

void
prayer_restoration(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_affected(ch, sn)) {
		act("You do not have enough power.", ch, NULL, NULL, TO_CHAR);
		return;
	}
	victim->hit = victim->max_hit;
	update_pos(victim);
	act("A warm feeling fills your body.", victim, NULL, NULL, TO_CHAR);
	act("A bleeding wounds on $N's body vanish.", NULL, victim, NULL, TO_NOTVICT);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 5;
	affect_to_char(ch, paf);
	aff_free(paf);
}

#define OBJ_VNUM_HOLY_HAMMER		18

void
prayer_holy_hammer(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *hammer;
	AFFECT_DATA *paf;

	hammer = create_obj(get_obj_index(OBJ_VNUM_HOLY_HAMMER), 0);
	hammer->level = ch->level;
	hammer->timer = level * 3;
	INT(hammer->value[2]) = (level / 10) + 1;

	paf = aff_new(TO_OBJECT, sn);
	paf->level	= level;
	paf->duration	= -1;
	paf->modifier	= level/7 +3;

	INT(paf->location)= APPLY_HITROLL;
	affect_to_obj(hammer, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_obj(hammer, paf);
	aff_free(paf);

	obj_to_char(hammer, ch);

	act ("You create a Holy Hammer.", ch, NULL, NULL, TO_CHAR);
	act ("$n creates a Holy Hammer.", ch, NULL, NULL, TO_ROOM);
}

void
prayer_hold_person(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA* victim = (CHAR_DATA*) vo;
	AFFECT_DATA *paf;

	if (saves_spell(level + 2, victim, DAM_OTHER)) {
		act_char("You failed.", ch);
		act("$n tries to hold you, but fails.", ch, victim, NULL, TO_VICT);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->duration	= 1;
	paf->level	= level;
	paf->modifier	= -level/12;
	INT(paf->location)= APPLY_DEX;
	affect_to_char(victim, paf);
	aff_free(paf);
}

void
prayer_lethargic_mist(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_affected_room(ch->in_room, sn)) {
		act_char("This room has already been full of lethargic mist.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The air in the room makes you slowing down.", ch);
	act("The air in the room makes you slowing down.",
	    ch, NULL, NULL, TO_ROOM);
}

void
prayer_mind_wrench(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 9);
	if (saves_spell(level, victim, DAM_MENTAL))
		dam /= 2;
	act("$n stares intently at $N, causing $N to seem very hyperactive.",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_MENTAL, DAMF_SHOW);
}

static void *
holy_word_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch))) {
		act_char("You feel full more powerful.", vch);
		spellfun_call("frenzy", NULL, level, ch, vch);
		spellfun_call("bless", NULL, level, ch, vch);
		return NULL;
	}

	if (is_safe_spell(ch, vch, TRUE))
		return NULL;

	if ((IS_GOOD(ch) && IS_EVIL(vch))
	||  (IS_EVIL(ch) && IS_GOOD(vch))) {
		spellfun_call("curse", NULL, level, ch, vch);
		act_char("You are struck down!", vch);
		damage(ch, vch, dice(level, 6), sn, DAM_ENERGY, DAMF_SHOW);
		return NULL;
	}

	if (IS_NEUTRAL(ch)) {
		spellfun_call("curse", NULL, level/2, ch, vch);
		act_char("You are struck down!", vch);
		damage(ch, vch, dice(level, 4), sn, DAM_ENERGY, DAMF_SHOW);
		return NULL;
	}

	return NULL;
}

/* RT really nasty high-level attack spell */
void
prayer_holy_word(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	act("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
	act_char("You utter a word of divine power.", ch);
	vo_foreach(ch->in_room, &iter_char_room, holy_word_cb,
		   sn, level, ch);
	act_puts("You feel drained.", ch, NULL, NULL, TO_CHAR, POS_DEAD);
	if (!IS_NPC(ch))
		gain_exp(ch, -1 * number_range(1, 10) * 5);
	ch->move = ch->move * 3 / 4;
	ch->hit = ch->hit * 3 / 4;
}

void
prayer_healing_light(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA *paf;

	if (is_affected_room(ch->in_room, sn)) {
		act_char("This room has already been healed by light.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 10;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->duration	= level / 10;
	INT(paf->location)= APPLY_ROOM_HEAL;
	paf->modifier	= level * 3 / 2;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled with healing light.", ch);
	act("The room starts to be filled with $n's healing light.",
	    ch, NULL, NULL, TO_ROOM);
}

void
prayer_sanctuary(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
		if (victim == ch) {
			act_puts("You are already in sanctuary.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		} else {
			act("$N is already in sanctuary.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) {
		if (victim == ch) {
			act_puts("But you are surrounded by black shroud.",
				  ch, NULL, NULL, TO_CHAR, POS_DEAD);
		} else {
			act("But $N is surrounded by black shroud.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (IS_EVIL(ch)) {
		act_puts("The gods are infuriated!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		damage(ch, ch, dice(level, IS_EVIL(ch) ? 2 : 1),
		       NULL, DAM_HOLY, DAMF_SHOW);
		return;
	}

	if (IS_EVIL(victim)) {
		act("Your god does not seems to like $N.",
			ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 6;
	paf->bitvector	= AFF_SANCTUARY;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
	act_puts("You are surrounded by a white aura.",
		 victim, NULL, NULL, TO_CHAR, POS_DEAD);
}

void
prayer_black_shroud(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA*) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) {
		if (victim == ch) {
			act_puts("You are already protected.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		} else {
			act("$N is already protected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
		if (victim == ch) {
			act_puts("But you are in sanctuary.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		} else {
			act("But $N is in sanctuary.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (!IS_EVIL(ch)) {
		act_puts("The gods are infuriated!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		damage(ch, ch, dice(level, IS_GOOD(ch) ? 2 : 1),
		       NULL, DAM_HOLY, DAMF_SHOW);
		return;
	}

	if (!IS_EVIL(victim)) {
		act("Your god does not seems to like $N.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = level/6;
	paf->bitvector = AFF_BLACK_SHROUD;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n is surrounded by black aura.", victim, NULL, NULL, TO_ROOM);
	act_puts("You are surrounded by black aura.",
		 victim, NULL, NULL, TO_CHAR, POS_DEAD);
}

/*  Cleric version of astra_walk  */
void
prayer_solar_flight(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;

	if  (time_info.hour > 18 || time_info.hour < 8) {
		 act_char("You need sunlight for solar flight.", ch);
		 return;
	}

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  LEVEL(victim) >= level + 1
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	transfer_char(ch, NULL, victim->in_room,
		      "$N disappears in a blinding flash of light!",
		      "You dissolve in a blinding flash of light!",
		      "$N appears in a blinding flash of light!");
}

void
prayer_black_death(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_affected_room(ch->in_room, sn)) {
		act_char("This room has already been diseased.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled by disease.", ch);
	act("The room starts to be filled by disease.",
	    ch, NULL, NULL, TO_ROOM);
}

void
prayer_etheral_fist(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 8);
	if (saves_spell(level, victim, DAM_ENERGY))
		dam /= 2;
	act("A fist of black, otherworldly ether rams into $N, leaving $M looking stunned!",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_ENERGY, DAMF_SHOW);
}

static void *
earthquake_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if (vch->in_room == ch->in_room) {
		if (is_safe_spell(ch, vch, TRUE))
			return NULL;

		if (IS_AFFECTED(vch, AFF_FLYING))
			damage(ch, vch, 0, sn, DAM_BASH, DAMF_SHOW);
		else
			damage(ch, vch, level + dice(2, 8), sn,
			       DAM_BASH, DAMF_SHOW);
		return NULL;
	}

	if (vch->in_room->area == ch->in_room->area)
		act_char("The earth trembles and shivers.", vch);

	return NULL;
}

void
prayer_earthquake(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	act_char("The earth trembles beneath your feet!", ch);
	act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);
	vo_foreach(NULL, &iter_char_world, earthquake_cb, sn, level, ch);
}

void
prayer_blade_barrier(const char *sn, int level,CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	act("Many sharp blades appear around $n and crash $N.",
	    ch, NULL, victim, TO_ROOM);
	act("Many sharp blades appear around you and crash $N.",
	    ch, NULL, victim, TO_CHAR);
	act("Many sharp blades appear around $n and crash you!",
	    ch, NULL, victim, TO_VICT);

	dam = dice(level,5);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_PIERCE, DAMF_SHOW);

	act("The blade barriers crash $n!", victim, NULL, NULL, TO_ROOM);
	dam = dice(level, 4);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_PIERCE, DAMF_SHOW);
	act("The blade barriers crash you!", victim, NULL, NULL, TO_CHAR);

	if (number_percent() < 75)
		return;

	act("The blade barriers crash $n!", victim, NULL, NULL, TO_ROOM);
	dam = dice(level, 2);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_PIERCE, DAMF_SHOW);
	act("The blade barriers crash you!", victim, NULL, NULL, TO_CHAR);

	if (number_percent() < 50)
		return;

	act("The blade barriers crash $n!", victim, NULL, NULL, TO_ROOM);
	dam = dice(level, 3);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_PIERCE, DAMF_SHOW);
	act("The blade barriers crash you!", victim, NULL, NULL, TO_CHAR);
}

void
prayer_anathema(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;
	int strength;

	strength = (ch->alignment - victim->alignment) / 200 * level / 30;
	if (strength < 0) strength = -strength;
	if (strength < 7) {
		act_puts("Oh, no. Your god seems to like $N.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	if (is_affected(victim, sn)) {
		act_puts("$N is already cursed.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	level += strength / 6;

	if (saves_spell(level, victim, DAM_HOLY)) {
		act_char("You failed.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (8 + level/10);

	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= -strength;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= strength;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_LEVEL;
	paf->modifier	= -strength / 7;
	paf->bitvector	= AFF_CURSE;
	affect_to_char(victim, paf);

	paf->where	= TO_SKILLS;
	paf->location.s	= str_empty;
	paf->modifier	= -strength;
	paf->bitvector	= SK_AFF_ALL;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n looks very uncomfortable.", victim, NULL, NULL, TO_ROOM);
	act_char("You feel unclean.", victim);
}

void
prayer_heat_metal(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_lose, *obj_next;
	int dam = 0;

	if (saves_spell(level + 2, victim, DAM_FIRE)
	||  victim->shapeform) {
		act_char("Your spell had no effect.", ch);
		act_char("You feel momentarily warmer.", victim);
		return;
	}

	for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next) {
		obj_next = obj_lose->next_content;

		if (number_range(1, 2 * level) < obj_lose->level
		||  saves_spell(level, victim, DAM_FIRE)
		||  !material_is(obj_lose, MATERIAL_METAL)
		||  IS_OBJ_STAT(obj_lose, ITEM_BURN_PROOF))
			continue;

		switch (obj_lose->item_type) {
		case ITEM_ARMOR:
			if (obj_lose->wear_loc == -1) {
				/*
				 * in inventory -- drop it if we can
				 */
				if (can_drop_obj(victim, obj_lose)) {
					act("$n yelps and throws $p to the ground!",
					     victim, obj_lose, NULL, TO_ROOM);
					act("You yelp and drop $p before it burns you.",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1,obj_lose->level) / 6;
					obj_from_char(obj_lose);
					obj_to_room(obj_lose, victim->in_room);
				} else { /* cannot drop */
					act("Your skin is seared by $p!",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1, obj_lose->level) / 2;
			    }
			    break;
			}

			/*
			 * worn
			 */
			if (can_drop_obj(victim,obj_lose)
			&&  obj_lose->weight / 10 < number_range(1, 2 * get_curr_stat(victim, STAT_DEX))
			&&  remove_obj(victim, obj_lose->wear_loc, TRUE)) {
				act("$n yelps and throws $p to the ground!",
				    victim, obj_lose, NULL, TO_ROOM);
				act("You remove and drop $p before it burns you.",
				    victim, obj_lose, NULL, TO_CHAR);
				dam += number_range(1,obj_lose->level) / 3;
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
			} else { /* stuck on the body! ouch! */
				act("Your skin is seared by $p!",
				    victim, obj_lose, NULL, TO_CHAR);
				dam += number_range(1, obj_lose->level);
			}
			break;

		case ITEM_WEAPON:
			if (IS_WEAPON_STAT(obj_lose, WEAPON_FLAMING)
			||  obj_lose->wear_loc == WEAR_STUCK_IN)
				continue;

			if (obj_lose->wear_loc == -1) {
				if (can_drop_obj(victim, obj_lose)) {
					act("$n throws a burning hot $p to the ground!",
					    victim, obj_lose, NULL, TO_ROOM);
					act("You yelp and drop $p before it burns you.",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1, obj_lose->level) / 6;
					obj_from_char(obj_lose);
					obj_to_room(obj_lose, victim->in_room);
				} else { /* cannot drop */
					act("Your skin is seared by $p!",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1,obj_lose->level) / 2;
				}
			}

			if (can_drop_obj(victim, obj_lose)
			&&  remove_obj(victim, obj_lose->wear_loc, TRUE)) {
				act("$n is burned by $p, and throws it to the ground.",
				    victim, obj_lose, NULL, TO_ROOM);
				act_char("You throw your red-hot weapon to the ground!", victim);
				dam += 1;
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
			} else { /* YOWCH! */
				act_char("Your weapon sears your flesh!", victim);
				dam += number_range(1,obj_lose->level);
			}
			break;
		}
	}

	if (!dam) {
		act_char("Your spell had no effect.", ch);
		act_char("You feel momentarily warmer.", victim);
	} else { /* damage! */
		if (saves_spell(level, victim, DAM_FIRE))
			dam = 2 * dam / 3;
		damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
	}
}

void
prayer_mind_light(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA *paf;

	if (is_affected_room(ch->in_room, sn)) {
		act_char("This room has already had booster of mana.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 10;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->duration	= level / 30;
	INT(paf->location)= APPLY_ROOM_MANA;
	paf->modifier	= level * 3 / 2;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled with mind light.", ch);
	act("The room starts to be filled with $n's mind light.",
	    ch, NULL, NULL, TO_ROOM);
}

void
prayer_free_action(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_affected(ch, sn)) {
		act("Your movements are already free.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	paf = aff_new(TO_SKILLS, "free action");
	paf->level	= level;
	paf->duration	= level / 12;
	paf->location.s	= str_dup("swimming");
	paf->modifier	= 100;
	paf->bitvector	= SK_AFF_TEACH;
	affect_to_char(vch, paf);
	aff_free(paf);

	act("You can move easier.", vch, NULL, NULL, TO_CHAR);
	if (ch != vch)
		act("You help $N to move easier.", ch, NULL, vch, TO_CHAR);
}

void
prayer_deadly_venom(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_affected_room(ch->in_room, sn)) {
		act_char("This room has already been affected by deadly venom.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled by poison.", ch);
	act("The room starts to be filled by poison.", ch, NULL, NULL, TO_ROOM);
}

void
prayer_ray_of_truth(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, align;

	if (IS_EVIL(ch)) {
		victim = ch;
		act_char("The energy explodes inside you!", ch);
	}

	if (victim != ch) {
		act("$n raises $s hand, and a blinding ray of light shoots forth!",
		    ch, NULL, NULL, TO_ROOM);
		act_char("You raise your hand and a blinding ray of light shoots forth!", ch);
	}

	if (IS_GOOD(victim)) {
		act("$n seems unharmed by the light.",
		    victim, NULL, victim, TO_ROOM);
		act_char("The light seems powerless to affect you.", victim);
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

	spellfun_call("blindness", NULL, 4 * level / 3, ch, victim);
	damage(ch, victim, dam, sn, DAM_HOLY, DAMF_SHOW);
}

void
prayer_aid(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_affected(ch, sn)) {
		act_char("This power is used too recently.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 50;
	affect_to_char(ch, paf);
	aff_free(paf);

	victim->hit += level * 5;
	update_pos(victim);
	act_char("A warm feeling fills your body.", victim);
	act("$n looks better.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		act_char("Ok.", ch);
}

/* added by chronos */
void
prayer_bluefire(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && !IS_NEUTRAL(ch)) {
		victim = ch;
		act_char("Your blue fire turn upon you!", ch);
	}

	if (victim != ch) {
		act("$n calls forth the blue fire of earth $N!",
		    ch, NULL, victim, TO_ROOM);
		act("$n has assailed you with the neutrals of earth!",
		    ch, NULL, victim, TO_VICT);
		act_char("You conjure forth the blue fire!", ch);
	}

	dam = dice(level, 10);
	if (saves_spell(level, victim, DAM_FIRE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
}

void
prayer_bless_weapon(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;

	if (obj->item_type != ITEM_WEAPON) {
		act_char("That isn't a weapon.", ch);
		return;
	}

	if (obj->wear_loc != -1) {
		act_char("The item must be carried to be blessed.", ch);
		return;
	}

	if (IS_WEAPON_STAT(obj, WEAPON_HOLY)) {
		act("$p is already blessed for holy attacks.",
		    ch, obj, NULL, TO_CHAR);
		return;
	}

	if (IS_WEAPON_STAT(obj, WEAPON_FLAMING)
	||  IS_WEAPON_STAT(obj, WEAPON_FROST)
	||  IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
	||  IS_WEAPON_STAT(obj, WEAPON_SHARP)
	||  IS_WEAPON_STAT(obj, WEAPON_VORPAL)
	||  IS_WEAPON_STAT(obj, WEAPON_SHOCKING)
	||  IS_OBJ_STAT(obj, ITEM_BLESS)
	||  IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
		act("You can't seem to bless $p.", ch, obj, NULL, TO_CHAR);
		return;
	}

	paf = aff_new(TO_WEAPON, sn);
	paf->level	= level / 2;
	paf->duration	= level / 8;
	paf->bitvector	= WEAPON_HOLY;
	affect_to_obj(obj, paf);
	aff_free(paf);

	act("$p is prepared for holy attacks.",ch,obj,NULL,TO_ALL);
}

void
prayer_control_weather(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	if (!str_cmp(target_name, "better"))
		weather_info.change += dice(level / 3, 4);
	else if (!str_cmp(target_name, "worse"))
		weather_info.change -= dice(level / 3, 4);
	else  {
		act_char("Do you want it to get better or worse?", ch);
		return;
	}

	act_char("Ok.", ch);
}

void
prayer_benediction(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;
	int strength = 0;

	if (is_affected(victim, sn)) {
		if (victim == ch) {
			act("You are already blessed.",
				ch, NULL, NULL, TO_CHAR);
		} else {
			act("$N is already blessed.",
				ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (IS_EVIL(victim))
		strength = IS_EVIL(ch) ? 2 : (IS_GOOD(ch) ? 0 : 1);
	if (IS_GOOD(victim))
		strength = IS_GOOD(ch) ? 2 : (IS_EVIL(ch) ? 0 : 1);
	if (IS_NEUTRAL(victim))
		strength = IS_NEUTRAL(ch) ? 2 : 1;
	if (!strength) {
		act("Your god does not seems to like $N.",
			ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 5 + level / 2;
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= level / 8 * strength;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= 0 - level / 8 * strength;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_LEVEL;
	paf->modifier	= strength;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("You feel righteous.", victim, NULL, NULL, TO_CHAR);
	if (victim != ch) {
		act("You grant $N favor of your god.",
			ch, NULL, victim, TO_CHAR);
	}
}

void
prayer_curse(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA *paf;

	/* deal with the object case first */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		if (IS_OBJ_STAT(obj, ITEM_EVIL)) {
			act("$p is already filled with evil.",
			    ch, obj, NULL, TO_CHAR);
			return;
		}

		if (IS_OBJ_STAT(obj, ITEM_BLESS)) {
			AFFECT_DATA *paf = affect_find(obj->affected, "bless");
			int spell_level = paf != NULL ? paf->level : obj->level;

			if (!saves_dispel(level, spell_level, 0)) {
				if (paf != NULL)
					affect_remove_obj(obj,paf);
				act("$p glows with a red aura.",
				    ch, obj, NULL, TO_ALL);
				REMOVE_OBJ_STAT(obj, ITEM_BLESS);
				return;
			}

			act("The holy aura of $p is too powerful for you to overcome.", ch, obj, NULL, TO_CHAR);
			return;
		}

		paf = aff_new(TO_OBJECT, sn);
		paf->level	= level;
		paf->duration	= (8 + level / 5);
		INT(paf->location)= APPLY_SAVES;
		paf->modifier	= 1;
		paf->bitvector	= ITEM_EVIL;
		affect_to_obj(obj, paf);
		aff_free(paf);

		act("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character curses */
	victim = (CHAR_DATA *) vo;

	if (IS_AFFECTED(victim, AFF_CURSE)
	||  saves_spell(level, victim, DAM_NEGATIVE))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (8 + level / 10);
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= -(level / 8 + 1);
	paf->bitvector	= AFF_CURSE;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= level / 8 + 1;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel unclean.", victim);
	if (ch != victim)
		act("$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR);
}

void
prayer_remove_curse(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	bool found = FALSE;
	OBJ_DATA *obj;

	/* do object cases first */
	if (mem_is(vo, MT_OBJ)) {
		obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj, ITEM_NODROP)
		||  IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
			if (!OBJ_IS(obj, OBJ_NOUNCURSE)
			&&  !saves_dispel(level + 2, obj->level, 0)) {
				REMOVE_OBJ_STAT(
				    obj, ITEM_NODROP | ITEM_NOREMOVE);
				act("$p glows blue.", ch, obj, NULL, TO_ALL);
				return;
			}

			act("The curse on $p is beyond your power.",
			    ch, obj, NULL, TO_CHAR);
			return;
		}

		act_char("Nothing happens...", ch);
		return;
	}

	/* characters */
	victim = (CHAR_DATA *) vo;

	if (check_dispel(level, victim, "curse")) {
		act_char("You feel better.", victim);
		act("$n looks more relaxed.", victim, NULL, NULL, TO_ROOM);
	}

	for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content) {
		if ((IS_OBJ_STAT(obj, ITEM_NODROP) ||
		     IS_OBJ_STAT(obj, ITEM_NOREMOVE))
		&&  !OBJ_IS(obj, OBJ_NOUNCURSE)
		&&  !saves_dispel(level, obj->level, 0)) {
			found = TRUE;
			REMOVE_OBJ_STAT(obj, ITEM_NODROP | ITEM_NOREMOVE);
			act("Your $p glows blue.", victim, obj, NULL, TO_CHAR);
			act("$n's $p glows blue.", victim, obj, NULL, TO_ROOM);
		}
	}
}

void
prayer_flamestrike(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 10);
	if (saves_spell(level, victim, DAM_FIRE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
}

void
prayer_know_alignment(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	char *msg;

	if (IS_GOOD(victim))
		msg = "$N has a pure and good aura.";
	else if (IS_NEUTRAL(victim))
		msg = "$N act as no align.";
	else
		msg = "$N is the embodiment of pure evil!";
	act(msg, ch, NULL, victim, TO_CHAR);

	if (!IS_NPC(victim)) {
		switch (victim->ethos) {
		case ETHOS_LAWFUL:
			msg = "$N upholds the laws.";
			break;
		case ETHOS_NEUTRAL:
			msg = "$N seems ambivalent to society.";
			break;
		case ETHOS_CHAOTIC:
			msg = "$N seems very chaotic.";
			break;
		default:
			msg = "$N doesn't know where they stand on the laws.";
			break;
		}
		act(msg, ch, NULL, victim, TO_CHAR);
	}
}

/* RT clerical berserking spell */
void
prayer_frenzy(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_affected(victim, sn) || IS_AFFECTED(victim, AFF_BERSERK)) {
		if (victim == ch)
			act_char("You are already in a frenzy.", ch);
		else
			act("$N is already in a frenzy.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_affected(victim, "calm")) {
		if (victim == ch)
			act_char("Why don't you just relax for a while?", ch);
		else
			act("$N doesn't look like $e wants to fight anymore.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if ((IS_GOOD(ch) && !IS_GOOD(victim))
	||  (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim))
	||  (IS_EVIL(ch) && !IS_EVIL(victim))) {
		act("Your god doesn't seem to like $N.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= UMAX(2, level / 3);
	paf->modifier	= UMAX(1, level / 6);

	INT(paf->location)= APPLY_HITROLL;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_AC;
	paf->modifier	= 5 * UMAX(1, level / 6);
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You are filled with holy wrath!", victim);
	act("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
}
