/*
 * $Id: act_obj.c,v 1.304 2004-06-28 19:21:00 tatyana Exp $
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

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <merc.h>
#include <lang.h>

#include <sog.h>

DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_put);
DECLARE_DO_FUN(do_drop);
DECLARE_DO_FUN(do_give);
DECLARE_DO_FUN(do_envenom);
DECLARE_DO_FUN(do_feed);
DECLARE_DO_FUN(do_fill);
DECLARE_DO_FUN(do_pour);
DECLARE_DO_FUN(do_drink);
DECLARE_DO_FUN(do_eat);
DECLARE_DO_FUN(do_wear);
DECLARE_DO_FUN(do_remove);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_quaff);
DECLARE_DO_FUN(do_dip);
DECLARE_DO_FUN(do_recite);
DECLARE_DO_FUN(do_brandish);
DECLARE_DO_FUN(do_zap);
DECLARE_DO_FUN(do_steal);
DECLARE_DO_FUN(do_buy_pet);
DECLARE_DO_FUN(do_buy);
DECLARE_DO_FUN(do_list);
DECLARE_DO_FUN(do_sell);
DECLARE_DO_FUN(do_value);
DECLARE_DO_FUN(do_herbs);
DECLARE_DO_FUN(do_lore);
DECLARE_DO_FUN(do_butcher);
DECLARE_DO_FUN(do_crucify);
DECLARE_DO_FUN(do_balance);
DECLARE_DO_FUN(do_withdraw);
DECLARE_DO_FUN(do_deposit);
DECLARE_DO_FUN(do_second_wield);
DECLARE_DO_FUN(do_enchant);
DECLARE_DO_FUN(do_label);
DECLARE_DO_FUN(do_repair);
DECLARE_DO_FUN(do_estimate);
DECLARE_DO_FUN(do_smithing);
DECLARE_DO_FUN(do_outfit);
DECLARE_DO_FUN(do_keep);
DECLARE_DO_FUN(do_unkeep);
DECLARE_DO_FUN(do_sharpen_weapon);

/* command procedures needed */
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_mount);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_emote);

/*
 * Local functions.
 */
static CHAR_DATA *	find_keeper	(CHAR_DATA * ch);
static uint		get_cost	(CHAR_DATA * keeper, OBJ_DATA * obj,
					 bool fBuy);
static void		obj_to_keeper	(OBJ_DATA * obj, CHAR_DATA * ch);
static OBJ_DATA *	get_obj_keeper	(CHAR_DATA * ch, CHAR_DATA * keeper,
					 const char *argument);

static void		sac_obj		(CHAR_DATA * ch, OBJ_DATA *obj);
static bool		put_obj		(CHAR_DATA *ch, OBJ_DATA *container,
					 OBJ_DATA *obj, int* count);

static void		give_coins(CHAR_DATA *ch, CHAR_DATA *victim,
				   int silver, int gold);
static const char *	money_argument(const char *argument,
				    char *arg, size_t len,
				    int *silver, int *gold);
static OBJ_DATA *	get_container_here(CHAR_DATA *ch, const char *name);
static bool		check_money(CHAR_DATA *ch, OBJ_DATA *container,
				    int silver, int gold);
static void		get_money(CHAR_DATA *ch, OBJ_DATA *container,
				  int silver, int gold);

DO_FUN(do_get, ch, argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj, *obj_next;
	OBJ_DATA       *container;
	bool            found;

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOGET)) {
		act("You aren't able to take anything in this form.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	if (is_number(arg1)) {
		int silver, gold;
		int act_flags;

		argument = money_argument(
		    argument, arg1, sizeof(arg1), &silver, &gold);
		if (argument == NULL) {
			do_get(ch, str_empty);
			return;
		}

		argument = one_argument(argument, arg2, sizeof(arg2));
		if (arg2[0] == '\0')
			container = NULL;
		else {
			if (!str_cmp(arg2, "from")) {
				argument = one_argument(
				    argument, arg2, sizeof(arg2));
			}

			if ((container = get_container_here(ch, arg2)) == NULL)
				return;
		}

		if (!check_money(ch, container, silver, gold)) {
			if (container == NULL) {
				act("You do not see that much money here.",
				    ch, NULL, NULL, TO_CHAR);
			} else {
				act("You do not see that much money in $p.",
				    ch, container, NULL, TO_CHAR);
			}
			return;
		}

		if (!can_carry_more_w(ch, COINS_WEIGHT(silver, gold))) {
			act("You can't carry that much weight.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		ch->silver += silver;
		ch->gold += gold;
		act_flags = HAS_INVIS(ch, ID_SNEAK) &&
			    number_percent() < get_skill(ch, "stealth") ?
				ACT_NOMORTAL : 0;
		if (container != NULL) {
			if (gold) {
				act("You get $j $qj{gold coins} from $P.",
				    ch, (const void *) gold, container, TO_CHAR);
				act("$n gets some coins from $P.",
				    ch, NULL, container, TO_ROOM | act_flags);
			} else {
				act("You get $j $qj{silver coins} from $P.",
				    ch, (const void *) silver, container, TO_CHAR);
				act("$n gets some coins from $P.",
				    ch, NULL, container, TO_ROOM | act_flags);
			}
		} else {
			if (gold) {
				act("You get $j $qj{gold coins}.",
				    ch, (const void *) gold, NULL, TO_CHAR);
				act("$n gets some coins.",
				    ch, NULL, NULL, TO_ROOM | act_flags);
			} else {
				act("You get $j $qj{silver coins}.",
				    ch, (const void *) silver, NULL, TO_CHAR);
				act("$n gets some coins.",
				    ch, NULL, NULL, TO_ROOM | act_flags);
			}
		}
		get_money(ch, container, silver, gold);
		return;
	}

	argument = one_argument(argument, arg2, sizeof(arg2));
	if (!str_cmp(arg2, "from"))
		argument = one_argument(argument, arg2, sizeof(arg2));

	/* Get type. */
	if (arg1[0] == '\0') {
		act_char("Get what?", ch);
		return;
	}
	if (arg2[0] == '\0') {
		if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
			/* 'get obj' */
			obj = get_obj_list(ch, arg1,
					   ch->in_room->contents, GETOBJ_F_ANY);
			if (obj == NULL) {
				act_puts("I see no $T here.",
					 ch, NULL, arg1, TO_CHAR, POS_DEAD);
				return;
			}

			get_obj(ch, obj, NULL, NULL);
		} else {
			/* 'get all' or 'get all.obj' */
			found = FALSE;
			for (obj = ch->in_room->contents; obj != NULL;
			     obj = obj_next) {
				obj_next = obj->next_content;
				if ((arg1[3] == '\0' || IS_OBJ_NAME(obj, arg1+4))
				&& can_see_obj(ch, obj)) {
					found = TRUE;
					get_obj(ch, obj, NULL, NULL);
				}
			}

			if (!found) {
				if (arg1[3] == '\0')
					act_char("I see nothing here.", ch);
				else
					act_puts("I see no $T here.",
						 ch, NULL, arg1+4, TO_CHAR,
						 POS_DEAD);
			}
		}
		return;
	}

	/* 'get ... container' */
	if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2)) {
		act_char("You can't do that.", ch);
		return;
	}

	if ((container = get_container_here(ch, arg2)) == NULL)
		return;

	if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
		/* 'get obj container' */
		obj = get_obj_list(ch, arg1, container->contains, GETOBJ_F_ANY);
		if (obj == NULL) {
			act_puts("I see nothing like that in the $T.",
				 ch, NULL, arg2, TO_CHAR, POS_DEAD);
			return;
		}
		get_obj(ch, obj, container, NULL);
	} else {
		/* 'get all container' or 'get all.obj container' */
		found = FALSE;
		for (obj = container->contains; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if ((arg1[3] == '\0' || IS_OBJ_NAME(obj, arg1 + 4))
			    && can_see_obj(ch, obj)) {
				found = TRUE;
				if (OBJ_IS(container, OBJ_PIT)
				&&  !IS_IMMORTAL(ch)) {
					act_puts("Don't be so greedy!",
						 ch, NULL, NULL, TO_CHAR,
						 POS_DEAD);
					return;
				}
				get_obj(ch, obj, container, NULL);
			}
		}

		if (!found) {
			if (arg1[3] == '\0')
				act_puts("I see nothing in the $T.",
					 ch, NULL, arg2, TO_CHAR, POS_DEAD);
			else
				act_puts("I see nothing like that in the $T.",
					 ch, NULL, arg2, TO_CHAR, POS_DEAD);
		}
	}
}

DO_FUN(do_put, ch, argument)
{
	char		arg1[MAX_INPUT_LENGTH];
	char		arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *	container;
	OBJ_DATA *	obj;
	OBJ_DATA *	obj_next;
	OBJ_DATA *	objc;
	int		count;
	clan_t *	clan;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (!str_cmp(arg2, "in")
	||  !str_cmp(arg2, "on")
	||  !str_cmp(arg2, "into"))
		argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		act_char("Put what in what?", ch);
		return;
	}

	if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2)) {
		act_char("You can't do that.", ch);
		return;
	}

	if ((container = get_obj_here(ch, arg2)) == NULL) {
		act_puts("I see no $T here.",
			 ch, NULL, arg2, TO_CHAR, POS_DEAD);
		return;
	}

	if (container->pObjIndex->item_type != ITEM_CONTAINER) {
		act_char("That is not a container.", ch);
		return;
	}

	/*
	 * clan members can put item into clan altar even if it is closed
	 * check that obj is clan item is done in put_obj
	 */
	if (IS_SET(INT(container->value[1]), CONT_CLOSED)
	&&  ((clan = clan_lookup(ch->clan)) == NULL ||
	     clan->altar_ptr != container)) {
		act_puts("$P is closed.",
			 ch, NULL, container, TO_CHAR, POS_DEAD);
		return;
	}

	if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
		/* 'put obj container' */
		if ((obj = get_obj_carry(ch, ch, arg1)) == NULL) {
			act_char("You do not have that item.", ch);
			return;
		}

		if (obj == container) {
			act_char("You can't fold it into itself.", ch);
			return;
		}

		if (!can_drop_obj(ch, obj)) {
			act_char("You can't let go of it.", ch);
			return;
		}

		if (WEIGHT_MULT(obj) != 100) {
			act_char("You have a feeling that would be a bad idea.", ch);
			return;
		}

		if (obj->pObjIndex->limit != -1) {
			act_puts("This unworthy container won't hold $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		if (get_obj_weight(obj) + get_obj_realweight(container) >
		    (INT(container->value[0]) * 10)
		||  get_obj_weight(obj) > (INT(container->value[3]) * 10)) {
			act_char("It won't fit.", ch);
			return;
		}

		count = 0;
		for (objc = container->contains; objc; objc = objc->next_content)
			count++;

		put_obj(ch, container, obj, &count);
	} else {
		if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
			do_say(ch, "Nah, I won't do that.");
			return;
		}

		count = 0;
		for (objc = container->contains; objc; objc = objc->next_content)
			count++;

		/* 'put all container' or 'put all.obj container' */
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg1[3] == '\0' || IS_OBJ_NAME(obj, arg1+4))
			&&  can_see_obj(ch, obj)
			&&  WEIGHT_MULT(obj) == 100
			&&  obj->wear_loc == WEAR_NONE
			&&  obj != container
			&&  can_drop_obj(ch, obj)
			&&  get_obj_weight(obj) + get_obj_realweight(container) <=
			    INT(container->value[0]) * 10
			&&  get_obj_weight(obj) < INT(container->value[3]) * 10
			&&  !put_obj(ch, container, obj, &count))
				break;
		}
	}
}

DO_FUN(do_drop, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_next;
	bool            found;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Drop what?", ch);
		return;
	}

	if (is_number(arg)) {
		/* 'drop NNNN coins' */
		int silver, gold;

		argument = money_argument(
		    argument, arg, sizeof(arg), &silver, &gold);
		if (argument == NULL) {
			do_drop(ch, str_empty);
			return;
		}

		if (ch->silver < silver) {
			act_char("You don't have that much silver.", ch);
			return;
		}
		if (ch->gold < gold) {
			act_char("You don't have that much gold.", ch);
			return;
		}

		ch->silver -= silver;
		ch->gold -= gold;

		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			switch (obj->pObjIndex->vnum) {
			case OBJ_VNUM_SILVER_ONE:
			case OBJ_VNUM_GOLD_ONE:
			case OBJ_VNUM_COINS:
			case OBJ_VNUM_GOLD_SOME:
			case OBJ_VNUM_SILVER_SOME:
				silver += INT(obj->value[0]);
				gold += INT(obj->value[1]);
				extract_obj(obj, 0);
				break;
			}
		}

		if ((obj = create_money(gold, silver)) == NULL)
			return;

		obj_to_room(obj, ch->in_room);
		act("$n drops some coins.", ch, NULL, NULL,
		    TO_ROOM | (HAS_INVIS(ch, ID_SNEAK) && number_percent() <
		    get_skill(ch, "stealth") ? ACT_NOMORTAL : 0));
		act_char("Ok.", ch);
		if (IS_WATER(ch->in_room)) {
			extract_obj(obj, 0);
			act("The coins sink down, and disapear in the water.",
			    ch, NULL, NULL,
			    TO_ROOM);
			act_puts("The coins sink down, and disapear in the water.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		}
		return;
	}
	if (str_cmp(arg, "all") && str_prefix("all.", arg)) {
		/* 'drop obj' */
		if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
			act_char("You do not have that item.", ch);
			return;
		}

		if (!can_drop_obj(ch, obj)) {
			act_char("You can't let go of it.", ch);
			return;
		}
		drop_obj(ch, obj);
	} else {
/* 'drop all' or 'drop all.obj' */
		if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
			do_say(ch, "Nah, I won't do that.");
			return;
		}

		found = FALSE;
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg[3] == '\0' || IS_OBJ_NAME(obj, arg+4))
			&&  can_see_obj(ch, obj)
			&&  obj->wear_loc == WEAR_NONE
			&&  !IS_OBJ_STAT(obj, ITEM_KEEP)
			&&  can_drop_obj(ch, obj)) {
				found = TRUE;
				obj->last_owner = NULL;

				drop_obj(ch, obj);
				if (IS_EXTRACTED(ch))
					return;
			}
		}

		if (!found) {
			if (arg[3] == '\0')
				act_puts("You are not carrying anything.",
					 ch, NULL, arg, TO_CHAR, POS_DEAD);
			else
				act_puts("You are not carrying any $T.",
					 ch, NULL, &arg[4], TO_CHAR, POS_DEAD);
		}
	}
}

DO_FUN(do_give, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Give what to whom?", ch);
		return;
	}

	if (is_number(arg)) {
		/* 'give NNNN coins victim' */
		int silver, gold;
		int change_silver, change_gold;

		argument = money_argument(
		    argument, arg, sizeof(arg), &silver, &gold);
		if (argument == NULL) {
			do_give(ch, str_empty);
			return;
		}

		argument = one_argument(argument, arg, sizeof(arg));
		if (!str_cmp(arg, "to"))
			argument = one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0') {
			do_give(ch, str_empty);
			return;
		}

		if ((victim = get_char_here(ch, arg)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}

		if (!IS_NPC(victim)
		&&  IS_SET(PC(victim)->plr_flags, PLR_NOGIVE)) {
			act("$N do not wish take any.",
			    ch, NULL, victim, TO_CHAR);
			return;
		}

		if (ch->gold < gold || ch->silver < silver) {
			act_char("You haven't got that much.", ch);
			return;
		}

		if (!can_carry_more_w(victim, COINS_WEIGHT(silver, gold))) {
			act("$N can't carry that much weight.",
			    ch, NULL, victim, TO_CHAR);
			return;
		}

		ch->silver -= silver;
		ch->gold -= gold;

		act_puts3("$n gives you $J $t.",
			  ch, silver ? "silver" : "gold", victim,
			  (const void *) (silver ? silver : gold),
			  TO_VICT, POS_RESTING);
		act("$n gives $N some coins.", ch, NULL, victim, TO_NOTVICT);
		act_puts3("You give $N $J $t.",
			  ch, silver ? "silver" : "gold", victim,
			  (const void *) (silver ? silver : gold),
			  TO_CHAR, POS_DEAD);

		if (pull_mob_trigger(
		     TRIG_MOB_BRIBE, victim, ch,
		     (void *) (silver + gold * 100)) > 0
		||  IS_EXTRACTED(victim))
			return;

		if (!IS_NPC(victim)
		||  !MOB_IS(victim, MOB_CHANGER)) {
			victim->silver += silver;
			victim->gold += gold;
			return;
		}

		if (IS_EXTRACTED(ch) || !can_see(victim, ch))
			return;

		/*
		 * MOB_CHANGER
		 */
		change_silver = 95 * gold;
		change_gold = 95 * silver / 100 / 100;
		if (change_silver + change_gold < 1) {
			tell_char(victim, ch, "I'm sorry, you did not give me enough to change.");
			give_coins(victim, ch, silver, gold);
			return;

		}
		if (silver) {
			change_silver +=
			    (95 * silver / 100 - change_gold * 100);
		}

		/*
		 * Ensure changer has enough money
		 */
		if (change_silver > victim->silver)
			victim->silver += change_silver;
		if (change_gold > victim->gold)
			victim->gold += change_gold;

		give_coins(victim, ch, change_silver, change_gold);
		tell_char(victim, ch, "Thank you, come again.");
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (!str_cmp(arg, "to"))
		argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_give(ch, str_empty);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (!IS_NPC(victim) && IS_SET(PC(victim)->plr_flags, PLR_NOGIVE)) {
		act("$N do not wish take any.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	give_obj(ch, victim, obj);
}

/* for poisoning weapons and food/drink */
DO_FUN(do_envenom, ch, argument)
{
	OBJ_DATA *obj;
	int percent;
	int chance;

	/* find out what */
	if (argument == '\0') {
		act_char("Envenom what item?", ch);
		return;
	}

	obj = get_obj_list(ch, argument, ch->carrying, GETOBJ_F_ANY);
	if (obj == NULL) {
		act_char("You don't have that item.", ch);
		return;
	}

	chance = get_skill(ch, "envenom");
	if (obj->pObjIndex->item_type == ITEM_FOOD
	||  obj->pObjIndex->item_type == ITEM_DRINK_CON) {
		if (IS_OBJ_STAT(obj, ITEM_BLESS)
		||  IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
			return;
		}

		WAIT_STATE(ch, skill_beats("envenom"));
		if (number_percent() < chance) {	/* success! */
			act("$n treats $p with deadly poison.",
			    ch, obj, NULL, TO_ROOM);
			act("You treat $p with deadly poison.",
			    ch, obj, NULL, TO_CHAR);
			if (!INT(obj->value[3])) {
				INT(obj->value[3]) = 1;
				check_improve(ch, "envenom", TRUE, 4);
			}
			return;
		}
		act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
		if (!INT(obj->value[3]))
			check_improve(ch, "envenom", FALSE, 4);
		return;
	} else if (obj->pObjIndex->item_type == ITEM_WEAPON) {
		if (IS_WEAPON_STAT(obj, WEAPON_FLAMING)
		||  IS_WEAPON_STAT(obj, WEAPON_FROST)
		||  IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
		||  IS_WEAPON_STAT(obj, WEAPON_SHARP)
		||  IS_WEAPON_STAT(obj, WEAPON_VORPAL)
		||  IS_WEAPON_STAT(obj, WEAPON_SHOCKING)) chance/=2;

		if (IS_WEAPON_STAT(obj, WEAPON_HOLY)
		||  IS_OBJ_STAT(obj, ITEM_BLESS)
		||  IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You can't seem to envenom $p.",
			    ch, obj, NULL, TO_CHAR);
			return;
		}

		if (skill_damclass(obj->value[3].s) == DAM_BASH) {
			act_char("You can only envenom edged weapons.", ch);
			return;
		}

		if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
			act("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
			return;
		}

		WAIT_STATE(ch, skill_beats("envenom"));
		percent = number_percent();
		if (percent < chance) {
			AFFECT_DATA *paf;

			paf = aff_new(TO_WEAPON, "poison");
			paf->level = LEVEL(ch) * percent / 100;
			paf->duration = LEVEL(ch) * percent / 100;
			paf->bitvector = WEAPON_POISON;
			affect_to_obj(obj, paf);
			aff_free(paf);

			act("$n coats $p with deadly venom.", ch, obj, NULL,
			    TO_ROOM);
			act("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, "envenom", TRUE, 3);
			return;
		} else {
			act("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, "envenom", FALSE, 3);
			return;
		}
	}
	act("You can't poison $p.", ch, obj, NULL, TO_CHAR);
}

DO_FUN(do_feed, ch, argument)
{
	CHAR_DATA *vch;
	OBJ_DATA *obj;
	AFFECT_DATA *paf;

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if (vch->master == ch && IS_NPC(vch)
		&&  (vch->pMobIndex->vnum == MOB_VNUM_COCOON ||
		     vch->pMobIndex->vnum == MOB_VNUM_BONE_DRAGON))
			break;
	}

	if (!vch) {
		act("Hmmm. Where did that dragon go?", ch, NULL, NULL, TO_CHAR);
		return;
	}

	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->pObjIndex->item_type == ITEM_CORPSE_NPC
		||  obj->pObjIndex->item_type == ITEM_CORPSE_PC)
			break;

	if (!obj) {
		act("Are you going to feed $M with blueberries?",
		    ch, NULL, vch, TO_CHAR);
		return;
	}

	if (vch->pMobIndex->vnum == MOB_VNUM_BONE_DRAGON) {
		int what;

		if (vch->position < POS_RESTING)
			return;

		if (vch->position == POS_FIGHTING) {
			do_say(vch, "Tasty. But let's finish that fight "
				    "before meal!");
			return;
		}
		if (is_sn_affected(vch, "bone dragon")) {
			act("Your pet might get too fat and clumsy.",
				ch, NULL, NULL, TO_CHAR);
			return;
		}

		what = number_percent();
		act("$n hungrily devours $P.", vch, NULL, obj, TO_ROOM);

		paf = aff_new(TO_AFFECTS, "bone dragon");
		paf->level	= obj->level;

		if (what < 10) {
			paf->where	= TO_RESISTS;
			INT(paf->location)= DAM_MENTAL;
			paf->duration	= obj->level/5;
			paf->modifier	= 100;
			do_emote(vch, "looks clever!");
		} else if (what < 40) {
			paf->where	= TO_AFFECTS;
			paf->bitvector	= AFF_HASTE;
			paf->duration	= obj->level/3;
			do_emote(vch, "looks filled with energy!");
		} else if (what < 70) {
			paf->where	= TO_AFFECTS;
			paf->bitvector	= AFF_SLEEP;
			paf->duration	= obj->level/20;
			vch->position	= POS_SLEEPING;
			do_emote(vch, "yawns and goes to sleep.");
		} else {
			paf->where	= TO_RESISTS;
			INT(paf->location)= DAM_BASH;
			paf->modifier	= 33;
			paf->duration	= 10;
			affect_to_char(vch, paf);

			INT(paf->location)= DAM_PIERCE;
			paf->modifier	= 33;
			paf->duration	= 10;
			affect_to_char(vch, paf);

			INT(paf->location)= DAM_SLASH;
			paf->modifier	= 33;
			paf->duration	= 10;
			affect_to_char(vch, paf);

			INT(paf->location)= DAM_ENERGY;
			paf->modifier	= 33;
			paf->duration	= 10;

			do_emote(vch, "flexes his muscles, looking tough.");
		}

		affect_to_char(vch, paf);
		aff_free(paf);

		extract_obj(obj, 0);
		return;
	}

	for (paf = vch->affected; paf; paf = paf->next)
		if (IS_SKILL(paf->type, "bone dragon"))
			break;

	if (!paf) {
		printlog(LOG_BUG, "do_feed: bone dragon cocoon w/o affect");
		return;
	}

	if (paf->modifier > 0) {
		act("Your pet might get too fat and clumsy.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	paf->level += obj->level;
	paf->modifier = 3;

	act("Cocoon opens, devours $P and then closes again.",
	    vch, NULL,obj,TO_ROOM);

	extract_obj(obj, 0);
}

DO_FUN(do_fill, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *fountain;
	bool            found;
	liquid_t	*lq;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Fill what?", ch);
		return;
	}
	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}
	found = FALSE;
	for (fountain = ch->in_room->contents; fountain != NULL;
	     fountain = fountain->next_content) {
		if (fountain->pObjIndex->item_type == ITEM_FOUNTAIN) {
			found = TRUE;
			break;
		}
	}

	if (!found) {
		act_char("There is no fountain here!", ch);
		return;
	}

	if (obj->pObjIndex->item_type != ITEM_DRINK_CON) {
		act_char("You can't fill that.", ch);
		return;
	}

	if (INT(obj->value[1]) != 0
	&&  str_cmp(STR(obj->value[2]), STR(fountain->value[2]))) {
		act_char("There is already another liquid in it.", ch);
		return;
	}

	if (INT(obj->value[1]) >= INT(obj->value[0])
	&&  INT(obj->value[0]) >= 0) {
		act_char("Your container is full.", ch);
		return;
	}
        if ((lq = liquid_lookup(STR(fountain->value[2]))) == NULL) {
		printlog(LOG_BUG, "Unknown liquid: %s", STR(fountain->value[2]));
		return;
	}
	act_puts3("You fill $p with $V from $P.",
		   ch, obj, fountain, &lq->lq_name, TO_CHAR, POS_DEAD);
	act_puts3("$n fills $p with $V from $P.",
		   ch, obj, fountain, &lq->lq_name, TO_ROOM, POS_RESTING);
	STR_ASSIGN(obj->value[2], str_qdup(fountain->value[2].s));
	INT(obj->value[1]) = INT(obj->value[0]);

}

DO_FUN(do_pour, ch, argument)
{
	char            arg[MAX_STRING_LENGTH];
	OBJ_DATA       *out, *in;
	CHAR_DATA      *vch = NULL;
	int             amount;
	liquid_t	*lq;
	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || argument[0] == '\0') {
		act_char("Pour what into what?", ch);
		return;
	}
	if ((out = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You don't have that item.", ch);
		return;
	}
	if (out->pObjIndex->item_type != ITEM_DRINK_CON) {
		act_char("That's not a drink container.", ch);
		return;
	}
        if ((lq = liquid_lookup(STR(out->value[2]))) == NULL) {
		printlog(LOG_BUG, "Unknown liquid: %s", STR(out->value[2]));
		return;
	}
	if (!str_cmp(argument, "out")) {
		if (INT(out->value[1]) == 0) {
			act_char("It is empty.", ch);
			return;
		}
		INT(out->value[1]) = 0;
		INT(out->value[3]) = 0;
		act_puts3("You invert $p, spilling $V $T.",
			  ch, out, 
			  IS_WATER(ch->in_room) ? "in to the water" :
						  "all over the ground",
			  &lq->lq_name,
			  TO_CHAR, POS_DEAD);
		act_puts3("$n inverts $p, spilling $V $T.",
			  ch, out,
			  IS_WATER(ch->in_room) ? "in to the water" :
						  "all over the ground",
			  &lq->lq_name,
			  TO_ROOM, POS_RESTING);
		return;
	}
	if ((in = get_obj_here(ch, argument)) == NULL) {
		vch = get_char_here(ch, argument);

		if (vch == NULL) {
			act_char("Pour into what?", ch);
			return;
		}
		in = get_eq_char(vch, WEAR_HOLD);

		if (in == NULL) {
			act_char("They aren't holding anything.", ch);
			return;
		}
	}
	if (in->pObjIndex->item_type != ITEM_DRINK_CON) {
		act_char("You can only pour into other drink containers.", ch);
		return;
	}
	if (in == out) {
		act_char("You cannot change the laws of physics!", ch);
		return;
	}
	if (INT(in->value[1]) != 0
	&& str_cmp(STR(in->value[2]), STR(out->value[2]))) {
		act_char("They don't hold the same liquid.", ch);
		return;
	}
	if (INT(out->value[1]) == 0) {
		act("There's nothing in $p to pour.", ch, out, NULL, TO_CHAR);
		return;
	}
	if (INT(in->value[0]) < 0) {
		act("You cannot fill $p.", ch, in, NULL, TO_CHAR);
		return;
	}
	if (INT(in->value[1]) >= INT(in->value[0])) {
		act("$p is already filled to the top.", ch, in, NULL, TO_CHAR);
		return;
	}
	amount = UMIN(INT(out->value[1]), INT(in->value[0]) - INT(in->value[1]));

	INT(in->value[1]) += amount;
	if (INT(out->value[0]) > 0)
		INT(out->value[1]) -= amount;
	INT(in->value[2]) = INT(out->value[2]);

	if (vch == NULL) {
		act_puts3("You pour $V from $p into $P.",
			  ch, out, in, &lq->lq_name, TO_CHAR, POS_DEAD);
		act_puts3("$n pours $V from $p into $P.",
			  ch, out, in, &lq->lq_name, TO_ROOM, POS_RESTING);
	} else {
		act_puts3("You pour some $V for $N.",
			  ch, NULL, vch, &lq->lq_name, TO_CHAR, POS_DEAD);
		act_puts3("$n pours you some $V.",
			  ch, NULL, vch, &lq->lq_name, TO_VICT, POS_RESTING);
		act_puts3("$n pours some $V for $N.",
			  ch, NULL, vch, &lq->lq_name, TO_NOTVICT, POS_RESTING);
	}

}

DO_FUN(do_drink, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	int             amount;
	int		i;
	liquid_t	*lq;

	if (ch->fighting != NULL) {
		act_puts("You can't drink while fighting.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (is_sn_affected(ch, "bellyache")) {
		act("You aren't able to drink anything.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		for (obj = ch->in_room->contents; obj; obj= obj->next_content) {
			if (obj->pObjIndex->item_type == ITEM_FOUNTAIN)
				break;
		}

		if (obj == NULL) {
			act_char("Drink what?", ch);
			return;
		}
	} else {
		if ((obj = get_obj_here(ch, arg)) == NULL) {
			act_char("You can't find it.", ch);
			return;
		}
	}

	if (!IS_NPC(ch) && PC(ch)->condition[COND_DRUNK] > 10) {
		act_char("You fail to reach your mouth.  *Hic*", ch);
		return;
	}

	switch (obj->pObjIndex->item_type) {
	default:
		act_char("You can't drink from that.", ch);
		return;

	case ITEM_FOUNTAIN:
		amount = 3;
		break;

	case ITEM_DRINK_CON:
		if (INT(obj->value[1]) == 0) {
			act_char("It is empty.", ch);
			return;
		}
		amount = 1;
		break;
	}

	if ((lq = liquid_lookup(STR(obj->value[2]))) == NULL) {
		printlog(LOG_BUG, "Do_drink: bad liquid %s.", STR(obj->value[2]));
		return;
	}

	if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
	    && PC(ch)->condition[COND_FULL] > 80) {
		act_char("You're too full to drink more.", ch);
		return;
	}

	act_puts3("$n drinks $V from $p.",
		  ch, obj, NULL, &lq->lq_name, TO_ROOM, POS_RESTING);
	act_puts3("You drink $V from $p.",
		  ch, obj, NULL, &lq->lq_name, TO_CHAR, POS_DEAD);

	if (ch->fighting)
		WAIT_STATE(ch, 3 * get_pulse("violence"));

	for (i = 0; i < MAX_COND; i++)
		gain_condition(ch, i, lq->affect[i]);

	if (!IS_NPC(ch) && PC(ch)->condition[COND_DRUNK] > 10)
		act_char("You feel drunk.", ch);
	if (!IS_NPC(ch) && PC(ch)->condition[COND_FULL] > 60)
		act_char("You are full.", ch);
	if (!IS_NPC(ch) && PC(ch)->condition[COND_THIRST] > 60)
		act_char("Your thirst is quenched.", ch);

	if (INT(obj->value[3]) != 0) {
		/* The drink was poisoned ! */
		AFFECT_DATA *paf;

		act("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
		act_char("You choke and gag.", ch);

		paf = aff_new(TO_AFFECTS, "poison");
		paf->level = number_fuzzy(lq->sip);
		paf->duration = 3 * amount;
		paf->bitvector = AFF_POISON;
		affect_join(ch, paf);
		aff_free(paf);
	}

	if (INT(obj->value[0]) > 0)
		INT(obj->value[1]) = UMAX(INT(obj->value[1]) - lq->sip, 0);
}

DO_FUN(do_eat, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	int chance = 0;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Eat what?", ch);
		return;
	}

	if (is_sn_affected(ch, "bellyache")) {
		act("You cannot force yourself to eat.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}
	if (!IS_IMMORTAL(ch)) {
		if ((obj->pObjIndex->item_type != ITEM_FOOD ||
		     IS_OBJ_STAT(obj, ITEM_NOT_EDIBLE))
		&& obj->pObjIndex->item_type != ITEM_PILL
		&& obj->pObjIndex->item_type != ITEM_HERB) {
			act_char("That's not edible.", ch);
			return;
		}
		if (!IS_NPC(ch) && PC(ch)->condition[COND_FULL] > 80) {
			act_char("You are too full to eat more.", ch);
			return;
		}
	}

	if ((obj->pObjIndex->item_type == ITEM_FOOD ||
	     obj->pObjIndex->item_type == ITEM_HERB)
	&&  ch->fighting != NULL) {
		act_puts("You can't eat while fighting.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if ((obj->pObjIndex->item_type == ITEM_PILL ||
	     obj->pObjIndex->item_type == ITEM_HERB)
	&&  ch->level < obj->level) {
		act_puts("$p is too powerful for you to eat.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	act("$n eats $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You eat $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	switch (obj->pObjIndex->item_type) {
	case ITEM_FOOD:
		if (!IS_NPC(ch)) {
			int             condition;
			condition = PC(ch)->condition[COND_HUNGER];
			gain_condition(ch, COND_FULL, INT(obj->value[0]) * 2);
			gain_condition(ch, COND_HUNGER, INT(obj->value[1]) * 2);
			if (!condition
			&& PC(ch)->condition[COND_HUNGER] > 0)
				act_char("You are no longer hungry.", ch);
			else if (PC(ch)->condition[COND_FULL] > 60)
				act_char("You are full.", ch);
			if (obj->pObjIndex->vnum == OBJ_VNUM_TORN_HEART) {
				ch->hit = UMIN(ch->max_hit, ch->hit+obj->level+number_range(1,5));
				act_char("You feel empowered from the blood of your foe.", ch);
			};
		}

		if (INT(obj->value[3]) != 0) {
			/* The food was poisoned! */
			AFFECT_DATA *paf;

			act("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
			act_char("You choke and gag.", ch);

			paf = aff_new(TO_AFFECTS, "poison");
			paf->level = number_fuzzy(INT(obj->value[0]));
			paf->duration = 2 * INT(obj->value[0]);
			paf->bitvector = AFF_POISON;
			affect_join(ch, paf);
			aff_free(paf);
		}

		if (IS_OBJ_STAT(obj, ITEM_MAGIC)) {
			act_char("Magical heat flows through your blood.", ch);
			ch->mana = UMIN(ch->max_mana, ch->mana + obj->level + number_range(1, 5));
			if (number_percent() < 2) {
				AFFECT_DATA *paf;

				do_yell(ch, "Oh no! My eyes!");

				paf = aff_new(TO_AFFECTS, "blindness");
				paf->level = number_fuzzy(obj->level);
				paf->duration = obj->level / 25 + 1;
				INT(paf->location) = APPLY_HITROLL;
				paf->modifier = -4;
				paf->bitvector = AFF_BLIND;
				affect_join(ch, paf);
				aff_free(paf);
			}
		}
		break;

	case ITEM_PILL:
		if (ch->fighting != NULL)
			WAIT_STATE(ch, 3 * get_pulse("violence"));

		obj_cast_spell(obj->value[1].s, INT(obj->value[0]), ch, ch);
		if (IS_EXTRACTED(ch))
			break;

		obj_cast_spell(obj->value[2].s, INT(obj->value[0]), ch, ch);
		if (IS_EXTRACTED(ch))
			break;

		obj_cast_spell(obj->value[3].s, INT(obj->value[0]), ch, ch);
		if (IS_EXTRACTED(ch))
			break;

		obj_cast_spell(obj->value[4].s, INT(obj->value[0]), ch, ch);
		if (IS_EXTRACTED(ch))
			break;

		chance = (obj->level - LEVEL(ch)) * 5;
		chance = (chance > 0) ?
			chance + 18 - get_curr_stat(ch, STAT_INT) : 0;
		if (number_percent() < chance && !IS_NPC(ch)) {
			act("$P was too powerful for you.",
			    ch, NULL, obj, TO_CHAR);
			spellfun("hallucination", NULL, obj->level, ch,ch);
		}
		break;

	case ITEM_HERB:
		obj_cast_spell(obj->value[0].s, MAX_LEVEL, ch, ch);
		if (IS_EXTRACTED(ch))
			break;

		ch->hit = UMIN(ch->hit + INT(obj->value[1]), ch->max_hit);
		update_pos(ch);
		if (IS_EXTRACTED(ch))
			break;

		ch->mana = UMIN(ch->mana + INT(obj->value[2]), ch->max_mana);
		ch->move = UMIN(ch->move + INT(obj->value[3]), ch->max_move);
		break;
	}

	extract_obj(obj, 0);
}

DO_FUN(do_wear, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_next;
	one_argument(argument, arg, sizeof(arg));

	if (ch->shapeform) {
		act("You can't reach your items.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (arg[0] == '\0') {
		act_char("Wear, wield, or hold what?", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
				wear_obj(ch, obj, FALSE);
		}
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}

	wear_obj(ch, obj, TRUE);
}

DO_FUN(do_remove, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	one_argument(argument, arg, sizeof(arg));

	if (ch->shapeform) {
		act("You can't reach your items.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (arg[0] == '\0') {
		act_char("Remove what?", ch);
		return;
	}
	if (!str_cmp(arg, "all")) {
		OBJ_DATA       *obj_next;

		if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
			do_say(ch, "Nah, I won't do that.");
			return;
		}

		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj))
				remove_obj(ch, obj->wear_loc, TRUE);
		}
		return;
	}
	if ((obj = get_obj_wear(ch, ch, arg)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}
	remove_obj(ch, obj->wear_loc, TRUE);
	return;
}

DO_FUN(do_sacrifice, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *r_next_cont;

	if (!str_cmp(argument, "all")) {
		for (obj = ch->in_room->contents; obj; obj = r_next_cont) {
			r_next_cont = obj->next_content;
			if (IS_OBJ_STAT(obj, ITEM_KEEP))
				continue;
			sac_obj(ch, obj);
		}
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || !str_cmp(arg, ch->name)) {
		act("$n offers $mself to gods, who graciously declines.",
		    ch, NULL, NULL, TO_ROOM);
		act_char("Gods appreciates your offer and may accept it later.", ch);
		return;
	}

	obj = get_obj_list(ch, arg, ch->in_room->contents, GETOBJ_F_ANY);
	if (obj == NULL) {
		act_char("You can't find it.", ch);
		return;
	}

	sac_obj(ch, obj);
}

DO_FUN(do_quaff, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	one_argument(argument, arg, sizeof(arg));

	if (has_spec(ch, "clan_battleragers")) {
		act_char("You are Battle Rager, not a filthy magician!", ch);
		return;
	}

	if (arg[0] == '\0') {
		act_char("Quaff what?", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have that potion.", ch);
		return;
	}

	if (obj->pObjIndex->item_type != ITEM_POTION) {
		act_char("You can quaff only potions.", ch);
		return;
	}

	if ((obj->level - LEVEL(ch)) * 7 > number_percent()) {
		act_char("This liquid was too powerful for you to drink.", ch);
		spellfun("hallucination", NULL, obj->level, ch, ch);
		return;
	}

	quaff_obj(ch, obj);
}

#define CAN_TARGET_OBJ(target) (((target) == TAR_OBJ_INV) || \
    ((target) == TAR_OBJ_CHAR_DEF) || ((target) == TAR_OBJ_CHAR_OFF))

static bool
dip(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *potion, int num)
{
	skill_t *spell;
	int target;

	if (((spell = skill_lookup(potion->value[num].s)) == NULL)
	|| (spell->skill_type != ST_PRAYER && spell->skill_type != ST_SPELL))
		return FALSE;

	target = spell->target;

	if (!CAN_TARGET_OBJ(target))
		return FALSE;

	obj_cast_spell(potion->value[num].s, INT(potion->value[0]), ch, obj);

	return TRUE;
}

DO_FUN(do_dip, ch, argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *potion;
	OBJ_DATA *vial;
	bool success = FALSE;
	int i;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (!str_cmp(arg2, "into"))
		one_argument(argument, arg2, sizeof(arg2));

	if (IS_NULLSTR(arg1)) {
		act_char("Dip what?", ch);
		return;
	}

	if (IS_NULLSTR(arg2)) {
		act_char("Dip into which potion?", ch);
		return;
	}

	if ((potion = get_obj_carry(ch, ch, arg2)) == NULL) {
		act_char("You do not have that potion.", ch);
		return;
	}

	if (potion->pObjIndex->item_type != ITEM_POTION) {
		act("$p is not a potion.", ch, potion, NULL, TO_CHAR);
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg1)) == NULL) {
		act_char("You do not have that object.", ch);
		return;
	}

	act("$n dips $p into $P.", ch, obj, potion, TO_ROOM);
	act_puts("You dip $p into $P.", ch, obj, potion, TO_CHAR, POS_DEAD);

	for (i = 1; i < 5; i++) {
		success = dip(ch, obj, potion, i) || success;
		if (IS_EXTRACTED(ch))
			return;
	}

	if (!success)
		act("$p is wasted.", ch, potion, NULL, TO_ALL);

	if ((vial = create_obj(OBJ_VNUM_POTION_VIAL, 0)) != NULL) {
		vial->label = str_qdup(obj->label);
		obj_to_char_check(vial, ch);
	}
	extract_obj(potion, 0);
}

DO_FUN(do_recite, ch, argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	void *vo;
	OBJ_DATA *scroll;

	if (has_spec(ch, "clan_battleragers")) {
		act_char("RECITE? You are Battle Rager!", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if ((scroll = get_obj_carry(ch, ch, arg1)) == NULL) {
		act_char("You do not have that scroll.", ch);
		return;
	}

	if (scroll->pObjIndex->item_type != ITEM_SCROLL) {
		act_char("You can recite only scrolls.", ch);
		return;
	}

	if (ch->level < scroll->level) {
		act_char("This scroll is too complex for you to comprehend.", ch);
		return;
	}

	if (arg2[0] == '\0')
		vo = ch;
	else if ((vo = get_char_here(ch, arg2)) == NULL
	&&       (vo = get_obj_here(ch, arg2)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("You can't find it.", ch);
		return;
	}

	if (pull_obj_trigger(TRIG_OBJ_USE, scroll, ch, NULL) > 0
	||  IS_EXTRACTED(ch) || !mem_is(scroll, MT_OBJ))
		return;

	act("$n recites $p.", ch, scroll, NULL, TO_ROOM);
	act_puts("You recite $p.", ch, scroll, NULL, TO_CHAR, POS_DEAD);

	if (number_percent() >= get_skill(ch, "scrolls") * 4 / 5) {
		act_puts("You mispronounce a syllable.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		check_improve(ch, "scrolls", FALSE, 2);
	} else {
		check_improve(ch, "scrolls", TRUE, 2);

		if (IS_PUMPED(ch) || ch->fighting != NULL)
			WAIT_STATE(ch, 2 * get_pulse("violence"));

		do {
			obj_cast_spell(scroll->value[1].s, INT(scroll->value[0]), ch, vo);
			if (IS_EXTRACTED(ch))
				break;

			obj_cast_spell(scroll->value[2].s, INT(scroll->value[0]), ch, vo);
			if (IS_EXTRACTED(ch))
				break;

			obj_cast_spell(scroll->value[3].s, INT(scroll->value[0]), ch, vo);
			if (IS_EXTRACTED(ch))
				break;

			obj_cast_spell(scroll->value[4].s, INT(scroll->value[0]), ch, vo);
		} while (0);
	}

	extract_obj(scroll, 0);
}

DO_FUN(do_brandish, ch, argument)
{
	OBJ_DATA       *staff;
	skill_t *	sk;

	if (has_spec(ch, "clan_battleragers")) {
		act_char("BRANDISH? You are not a filthy magician!", ch);
		return;
	}

	if ((staff = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		act_char("You hold nothing in your hand.", ch);
		return;
	}

	if (staff->pObjIndex->item_type != ITEM_STAFF) {
		act_char("You can brandish only with a staff.", ch);
		return;
	}

	if ((sk = skill_lookup(staff->value[3].s)) == NULL)
		return;

	if (pull_obj_trigger(TRIG_OBJ_USE, staff, ch, NULL) > 0
	||  IS_EXTRACTED(ch) || !mem_is(staff, MT_OBJ))
		return;

	WAIT_STATE(ch, 2 * get_pulse("violence"));

	if (INT(staff->value[2]) > 0) {
		act("$n brandishes $p.", ch, staff, NULL, TO_ROOM);
		act("You brandish $p.", ch, staff, NULL, TO_CHAR);
		if (ch->level + 3 < staff->level
		||  number_percent() >= 10 + get_skill(ch, "staves") * 4 / 5) {
			act("You fail to invoke $p.", ch, staff, NULL, TO_CHAR);
			act("...and nothing happens.", ch, NULL, NULL, TO_ROOM);
			check_improve(ch, "staves", FALSE, 2);
		} else {
			CHAR_DATA *vch;

			check_improve(ch, "staves", TRUE, 2);
			foreach (vch, char_in_room(ch->in_room)) {
				switch (sk->target) {
				default:
					break;

				case TAR_IGNORE:
					if (vch != ch)
						continue;
					break;

				case TAR_CHAR_OFFENSIVE:
					if (IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch))
						continue;
					break;

				case TAR_CHAR_DEFENSIVE:
					if (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))
						continue;
					break;

				case TAR_CHAR_SELF:
					if (vch != ch)
						continue;
					break;
				}

				obj_cast_spell(
				    staff->value[3].s, INT(staff->value[0]),
				    ch, vch);
				if (IS_EXTRACTED(ch)
				||  IS_SET(sk->skill_flags, SKILL_AREA_ATTACK))
					break;
			} end_foreach(vch);
		}
	}

	if (--INT(staff->value[2]) <= 0) {
		act("$n's $p blazes bright and is gone.",
		    ch, staff, NULL, TO_ROOM);
		act("Your $p blazes bright and is gone.",
		    ch, staff, NULL, TO_CHAR);
		extract_obj(staff, 0);
	}
}

DO_FUN(do_zap, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *wand;
	void *vo;

	if (has_spec(ch, "clan_battleragers")) {
		act_char("You'd destroy magic, not use it!", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if ((wand = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		act_char("You hold nothing in your hand.", ch);
		return;
	}
	if (wand->pObjIndex->item_type != ITEM_WAND) {
		act_char("You can zap only with a wand.", ch);
		return;
	}

	if (arg[0] == '\0') {
		if (ch->fighting && ch->fighting->in_room == ch->in_room)
			vo = ch->fighting;
		else {
			act_char("Zap whom or what?", ch);
			return;
		}
	} else if ((vo = get_char_here(ch, arg)) == NULL
	&&       (vo = get_obj_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("You can't find it.", ch);
		return;
	}

	WAIT_STATE(ch, 2 * get_pulse("violence"));

	if (INT(wand->value[2]) > 0) {
		if (pull_obj_trigger(TRIG_OBJ_USE, wand, ch, NULL) > 0
		||  IS_EXTRACTED(ch) || !mem_is(wand, MT_OBJ))
			return;

		if (mem_is(vo, MT_CHAR)) {
			if (vo == ch) {
				act("$n zaps $mself with $p.",
				    ch, wand, vo, TO_ROOM);
				act("You zap yourself with $p.",
				    ch, wand, vo, TO_CHAR);
			} else {
				act("$n zaps $N with $p.",
				    ch, wand, vo, TO_NOTVICT);
				act("$n zaps you with $p.",
				    ch, wand, vo, TO_VICT);
				act("You zap $N with $p.",
				    ch, wand, vo, TO_CHAR);
			}
		} else {
			act("$n zaps $P with $p.", ch, wand, vo, TO_ROOM);
			act("You zap $P with $p.", ch, wand, vo, TO_CHAR);
		}

		if (ch->level + 5 < wand->level
		|| number_percent() >= 20 + get_skill(ch, "wands") * 4 / 5) {
			act("Your efforts with $p produce only smoke and sparks.",
			    ch, wand, NULL, TO_CHAR);
			act("$n's efforts with $p produce only smoke and sparks.",
			    ch, wand, NULL, TO_ROOM);
			check_improve(ch, "wands", FALSE, 2);
		} else {
			check_improve(ch, "wands", TRUE, 2);
			obj_cast_spell(wand->value[3].s, INT(wand->value[0]), ch, vo);
		}
	}

	if (--INT(wand->value[2]) <= 0) {
		act("$n's $p explodes into fragments.",
		    ch, wand, NULL, TO_ROOM);
		act("Your $p explodes into fragments.",
		    ch, wand, NULL, TO_CHAR);
		extract_obj(wand, 0);
	}
}

DO_FUN(do_steal, ch, argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	CHAR_DATA      *victim;
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_inve;
	int		chance;
	int             percent;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		act_char("Steal what from whom?", ch);
		return;
	}

	if (IS_NPC(ch) && IS_SET(ch->affected_by, AFF_CHARM)
	    && (ch->master != NULL)) {
		act_char("You are too dazed to steal anything.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("steal"));

	if ((victim = get_char_here(ch, arg2)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}
	if (victim == ch) {
		act_char("That's pointless.", ch);
		return;
	}

	if (victim->position == POS_FIGHTING) {
		act_char("You'd better not -- you might get hit.", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	chance = get_skill(ch, "steal") + (LEVEL(ch) - LEVEL(victim)) * 3;
	percent = number_percent() +
		  (IS_AWAKE(victim) ? 10 : -50) +
		  (!can_see(victim, ch) ? -10 : 0);

	if ((!IS_NPC(ch) && percent > chance)
	||  (IS_NPC(victim) && IS_SET(victim->pMobIndex->act, ACT_IMMSTEAL))
	||  IS_IMMORTAL(victim)
	||  (victim->in_room &&
	     IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA))) {
		/*
		 * Failure.
		 */

		act_char("Oops.", ch);

		if (HAS_INVIS(ch, ID_HIDDEN | ID_FADE) && !IS_NPC(ch)) {
			REMOVE_INVIS(ch, ID_HIDDEN | ID_FADE);
			act_puts("You step out of shadows.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n steps out of shadows.",
			    ch, NULL, NULL, TO_ROOM);
		}

		if (!IS_AFFECTED(victim, AFF_SLEEP)) {
			victim->position = victim->position == POS_SLEEPING ? POS_STANDING :
				victim->position;
			act("$n tried to steal from you.",
			    ch, NULL, victim, TO_VICT);
		}
		act("$n tried to steal from $N.", ch, NULL, victim, TO_NOTVICT);

		if (IS_AWAKE(victim)) {
			const char *msg;

			switch (number_range(0, 3)) {
			case 0:
				msg = "$lu{$N} is a lousy thief!";
				break;
			case 1:
				msg = "$lu{$N} couldn't rob $gN{his} way "
				      "out of a paper bag!";
				break;
			case 2:
				msg = "$lu{$N} tried to rob me!";
				break;
			default:
				msg = "Keep your hands out of there, $N!";
				break;
			}

			act_yell(victim, msg, ch, NULL);
		}

		if (!IS_NPC(ch) && IS_NPC(victim)) {
			check_improve(ch, "steal", FALSE, 2);
			multi_hit(victim, ch, NULL);
		}
		return;
	}

	if (!str_cmp(arg1, "silver")
	||  !str_cmp(arg1, "coins")
	||  !str_cmp(arg1, "coin")
	||  !str_cmp(arg1, "gold")) {
		int silver, gold;

		if (!str_cmp(arg1, "gold")) {
			silver = 0;
			gold = victim->gold * number_range(1, 7) / 100;
		} else {
			silver = victim->silver * number_range(1, 20) / 100;
			gold = 0;
		}

		if (silver + gold < 1) {
			act_char("You couldn't get any coins.", ch);
			return;
		}

		if (!can_carry_more_w(ch, COINS_WEIGHT(silver, gold))) {
			act_char("You can't carry that weight.", ch);
			return;
		}

		ch->gold += gold;
		victim->gold -= gold;
		ch->silver += silver;
		victim->silver -= silver;

		if (gold) {
			act_puts("Bingo!  You got $j $qj{gold coins}.",
				 ch, (const void *) gold, NULL,
				 TO_CHAR, POS_DEAD);
		} else {
			act_puts("Bingo!  You got $j $qj{silver coins}.",
				 ch, (const void *) silver, NULL,
				 TO_CHAR, POS_DEAD);
		}

		check_improve(ch, "steal", TRUE, 2);
		return;
	}

	obj = get_obj_list(ch, arg1, victim->carrying, GETOBJ_F_INV);
	if (obj == NULL) {
		act_char("You can't find it.", ch);
		return;
	}

	if (!can_drop_obj(ch, obj)
	/* ||   IS_OBJ_STAT(obj, ITEM_INVENTORY) */
	     /* ||  obj->level > ch->level */ ) {
		act_char("You can't pry it away.", ch);
		return;
	}

	if (!can_carry_more_n(ch, get_obj_number(obj))) {
		act_char("You have your hands full.", ch);
		return;
	}

	if (!can_carry_more_w(ch, get_obj_weight(obj))) {
		act_char("You can't carry that much weight.", ch);
		return;
	}

	if (!IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
		obj_to_char(obj, ch);
		act_char("You got it!", ch);
		check_improve(ch, "steal", TRUE, 2);
	} else {
		obj_inve = clone_obj(obj);
		REMOVE_OBJ_STAT(obj_inve, ITEM_INVENTORY);
		obj_to_char(obj_inve, ch);
		act_char("You got one of them!", ch);
		check_improve(ch, "steal", TRUE, 1);
	}

	pull_obj_trigger(TRIG_OBJ_GET, obj, ch, NULL);
}

/*
 * Shopping commands.
 */
DO_FUN(do_buy_pet, ch, argument)
{
	int		cost, roll;
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA	*pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Buy what?", ch);
		return;
	}

	pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
	if (pRoomIndexNext == NULL) {
		printlog(LOG_BUG, "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
		act_char("Sorry, you can't buy that here.", ch);
		return;
	}
	in_room = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet = get_char_here(ch, arg);
	ch->in_room = in_room;

	if (!pet
	||  !IS_NPC(pet)
	||  !IS_SET(pet->pMobIndex->act, ACT_PET)) {
		act_char("Sorry, you can't buy that here.", ch);
		return;
	}

	cost = 10 * pet->level * pet->level;
	if ((ch->silver + 100 * ch->gold) < cost) {
		act_char("You can't afford it.", ch);
		return;
	}

	if (LEVEL(ch) < LEVEL(pet)) {
		act_char("You're not powerful enough to master this pet.", ch);
		return;
	}

	/*
	 * ridable pets
	 */
	if (IS_SET(pet->pMobIndex->act, ACT_RIDEABLE)) {
		if (MOUNTED(ch)) {
			act_char("You are already riding.", ch);
			return;
		}

		if (get_skill(ch, "riding") == 0) {
			act_char("You're not powerful enough to master this pet.", ch);
			return;
		}
	} else {
		if (GET_PET(ch) != NULL) {
			act_char("You already own a pet.", ch);
			return;
		}

		/* haggle */
		roll = number_percent();
		if (roll < get_skill(ch, "haggle")) {
			cost -= cost / 2 * roll / 100;
			act_puts("You haggle the price down to $j $qj{coins}.",
				 ch, (const void *) cost, NULL, TO_CHAR, POS_DEAD);
			check_improve(ch, "haggle", TRUE, 4);
		}
	}

	deduct_cost(ch, cost);

	/*
	 * create_mob can't return NULL here
	 */
	pet = create_mob(pet->pMobIndex->vnum, 0);
	pet->comm = COMM_NOTELL;
	pet->chan = CHAN_NOSHOUT | CHAN_NOCHANNELS;

	char_to_room(pet, ch->in_room);
	if (IS_EXTRACTED(pet))
		return;

	if (IS_SET(pet->pMobIndex->act, ACT_RIDEABLE)) {
		act_char("Enjoy your mount.", ch);
		act("$n bought $N as a mount.", ch, NULL, pet, TO_ROOM);

		do_mount(ch, pet->name);
	} else {
		SET_BIT(pet->affected_by, AFF_CHARM);

		argument = one_argument(argument, arg, sizeof(arg));
		if (arg[0] != '\0')
			pet->name = str_printf(pet->pMobIndex->name, arg);
		mlstr_printf(&pet->description,
			     &pet->pMobIndex->description, ch->name);

		add_follower(pet, ch);
		pet->leader = ch;
		PC(ch)->pet = pet;

		act_char("Enjoy your pet.", ch);
		act("$n bought $N as a pet.", ch, NULL, pet, TO_ROOM);
	}
}

DO_FUN(do_buy, ch, argument)
{
	int		roll;
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj, *t_obj;
	char            arg[MAX_INPUT_LENGTH];
	uint		number, cost, count = 1;

	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)) {
		do_buy_pet(ch, argument);
		return;
	}

	number = mult_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || number == 0) {
		act_char("Buy what?", ch);
		return;
	}

	obj = get_obj_keeper(ch, keeper, arg);
	cost = get_cost(keeper, obj, TRUE);

	if (cost <= 0 || !can_see_obj(ch, obj)) {
		tell_char(keeper, ch, "I don't sell that -- try 'list'");
		return;
	}

	if (!IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
		for (t_obj = obj->next_content; count < number && t_obj != NULL;
		     t_obj = t_obj->next_content) {
			if (t_obj->pObjIndex != obj->pObjIndex
			||  !!mlstr_cmp(&t_obj->short_descr, &obj->short_descr))
				break;
			count++;
		}

		if (count < number) {
			tell_char(keeper, ch,
				    "I don't have that many in stock.");
			return;
		}
	}

	if (ch->silver + ch->gold * 100 < (int) (cost * number)) {
		if (number > 1) {
			tell_char(keeper, ch,
				    "You can't afford to buy that many.");
		} else {
			act("$n tells you '{GYou can't afford to buy $p.{x'",
			    keeper, obj, ch, TO_VICT);
		}
		return;
	}

	if (obj->level > ch->level) {
		act("$n tells you '{GYou can't use $p yet.{x'",
		    keeper, obj, ch, TO_VICT);
		return;
	}

	if (!can_carry_more_n(ch, number * get_obj_number(obj))) {
		act_char("You can't carry that many items.", ch);
		return;
	}

	if (!can_carry_more_w(ch, number * get_obj_weight(obj))) {
		act_char("You can't carry that much weight.", ch);
		return;
	}

	/* haggle */
	roll = number_percent();
	if (!OBJ_IS(obj, OBJ_SELL_EXTRACT)
	    && roll < get_skill(ch, "haggle")) {
		cost -= cost / 2 * roll / 100;
		act("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
		check_improve(ch, "haggle", TRUE, 4);
	}
	if (number > 1) {
		act_puts("$n buys $P[$j].",
			 ch, (const void*) number, obj, TO_ROOM, POS_RESTING);
		act_puts3("You buy $P[$j] for $J silver.",
			  ch, (const void*) number,
			  obj, (const void*) (cost*number),
			  TO_CHAR, POS_DEAD);
	} else {
		act("$n buys $P.", ch, NULL, obj, TO_ROOM);
		act_puts("You buy $P for $j silver.",
			 ch, (const void*) cost, obj, TO_CHAR, POS_DEAD);
	}

	deduct_cost(ch, cost * number);
	keeper->gold += cost * number / 100;
	keeper->silver += cost * number - (cost * number / 100) * 100;

	for (count = 0; count < number; count++) {
		if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
			t_obj = clone_obj(obj);
		else {
			t_obj = obj;
			obj = obj->next_content;
		}

		if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj, ITEM_HAD_TIMER))
			t_obj->timer = 0;
		REMOVE_OBJ_STAT(t_obj, ITEM_HAD_TIMER);
		obj_to_char(t_obj, ch);
		if (cost < t_obj->cost)
			t_obj->cost = cost;
	}
}

DO_FUN(do_list, ch, argument)
{
	if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)) {
		ROOM_INDEX_DATA *pRoomIndexNext;
		CHAR_DATA	*pet;
		BUFFER		*buf;

		pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
		if (pRoomIndexNext == NULL) {
			printlog(LOG_BUG, "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
			act_char("You can't do that here.", ch);
			return;
		}

		buf = NULL;
		for (pet = pRoomIndexNext->people; pet; pet = pet->next_in_room) {
			if (!IS_NPC(pet))
				continue;	/* :) */

			if (IS_SET(pet->pMobIndex->act, ACT_PET)) {
				if (!buf) {
					buf = buf_new(GET_LANG(ch));
					buf_append(buf, "Pets for sale:\n");
				}
				buf_printf(buf, BUF_END,
				    "[%2d] %8d - %s\n",	// notrans
				    pet->level,
				    10 * pet->level * pet->level,
				    PERS(pet, ch, GET_LANG(ch), ACT_FORMSH));
			}
		}

		if (!buf)
			act_char("Sorry, we're out of pets right now.", ch);
		else {
			page_to_char(buf_string(buf), ch);
			buf_free(buf);
		}

		return;
	} else {
		CHAR_DATA      *keeper;
		OBJ_DATA       *obj;
		int             cost, count;
		BUFFER		*buf;
		char            arg[MAX_INPUT_LENGTH];

		if ((keeper = find_keeper(ch)) == NULL)
			return;
		one_argument(argument, arg, sizeof(arg));

		buf = NULL;
		for (obj = keeper->carrying; obj; obj = obj->next_content) {
			if (obj->wear_loc == WEAR_NONE
			    && can_see_obj(ch, obj)
			    && (cost = get_cost(keeper, obj, TRUE)) > 0
			    && (arg[0] == '\0' || IS_OBJ_NAME(obj, arg))) {
				if (!buf) {
					buf = buf_new(GET_LANG(ch));
					buf_append(buf, "[Lv Price Qty] Item\n");
				}

				if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
					buf_printf(buf, BUF_END,
						   "[%2d %5d -- ] %s\n", // notrans
						   obj->level, cost,
						   format_short(&obj->short_descr, obj->pObjIndex->name, ch, GET_LANG(ch), 0));
				else {
					count = 1;

					while (obj->next_content != NULL
					       && obj->pObjIndex == obj->next_content->pObjIndex
					       && !mlstr_cmp(&obj->short_descr,
					   &obj->next_content->short_descr)) {
						obj = obj->next_content;
						count++;
					}

					buf_printf(buf, BUF_END,
					    "[%2d %5d %2d ] %s\n", // notrans
					    obj->level, cost, count,
					    format_short(&obj->short_descr,
						obj->pObjIndex->name, ch,
						GET_LANG(ch), 0));
				}
			}
		}

		if (!buf)
			act_char("You can't buy anything here.", ch);
		else {
			page_to_char(buf_string(buf), ch);
			buf_free(buf);
		}
		return;
	}
}

DO_FUN(do_sell, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj;
	int		roll;
	uint		cost, gold, silver;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Sell what?", ch);
		return;
	}
	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		tell_char(keeper, ch, "You don't have that item.");
		return;
	}
	if (!can_drop_obj(ch, obj)) {
		act_char("You can't let go of it.", ch);
		return;
	}
	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT);
		return;
	}
	if ((cost = get_cost(keeper, obj, FALSE)) <= 0) {
		act("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
		return;
	}

	if (IS_OBJ_STAT(obj, ITEM_KEEP)) {
		act_char("Unkeep it first!", ch);
		return;
	}

	if ((int) cost > (keeper->silver + 100 * keeper->gold)) {
		act("$n tells you '{GI'm afraid I don't have enough wealth to buy $p.{x'",
		    keeper, obj, ch, TO_VICT);
		return;
	}
	act("$n sells $p.", ch, obj, NULL, TO_ROOM);
	/* haggle */
	roll = number_percent();
	if (!OBJ_IS(obj, OBJ_SELL_EXTRACT) && roll < get_skill(ch, "haggle")) {
		roll = get_skill(ch, "haggle") + number_range(1, 20) - 10;
		act_char("You haggle with the shopkeeper.", ch);
		cost += obj->cost / 2 * roll / 100;
		cost = UMIN(cost, 95 * get_cost(keeper, obj, TRUE) / 100);
		cost = UMIN((int) cost, keeper->silver + 100 * keeper->gold);
		check_improve(ch, "haggle", TRUE, 4);
	}
	silver = cost - (cost / 100) * 100;
	gold = cost / 100;

	if (!can_carry_more_w(ch, COINS_WEIGHT(silver, gold))) {
		tell_char(keeper, ch, "I'm afraid you can't carry that weight.");
		return;
	}

	if (gold && silver) {
		act_puts3("You sell $P for $j gold and $J silver $qJ{pieces}.",
			  ch, (const void*) gold, obj, (const void*) silver,
			  TO_CHAR, POS_DEAD);
	}
	else if (gold) {
		act_puts("You sell $P for $j gold $qj{pieces}.",
			 ch, (const void*) gold, obj, TO_CHAR, POS_DEAD);
	}
	else if (silver) {
		act_puts("You sell $P for $j silver $qj{pieces}.",
			 ch, (const void*) silver, obj, TO_CHAR, POS_DEAD);
	}
	ch->gold += gold;
	ch->silver += silver;
	deduct_cost(keeper, cost);

	if (obj->pObjIndex->item_type == ITEM_TRASH
	||  OBJ_IS(obj, OBJ_SELL_EXTRACT))
		extract_obj(obj, 0);
	else {
		if (obj->timer)
			SET_OBJ_STAT(obj, ITEM_HAD_TIMER);
		else
			obj->timer = number_range(50, 100);
		obj_to_keeper(obj, keeper);
	}
}

DO_FUN(do_value, ch, argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj;
	int             cost;
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Value what?", ch);
		return;
	}
	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		tell_char(keeper, ch, "You don't have that item.");
		return;
	}
	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT);
		return;
	}
	if (!can_drop_obj(ch, obj)) {
		act_char("You can't let go of it.", ch);
		return;
	}
	if ((cost = get_cost(keeper, obj, FALSE)) <= 0) {
		act("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
		return;
	}

	act_puts("$N tells you '{G", ch, NULL, keeper,
		 TO_CHAR | ACT_NOLF, POS_DEAD);
	act_puts3("I'll give you $j silver and $J gold $qJ{coins} for $P.",
		  ch, (const void*) (cost%100), obj, (const void*) (cost/100),
		  TO_CHAR | ACT_NOLF, POS_DEAD);
	act_puts("{x'", ch, NULL, NULL, TO_CHAR, POS_DEAD);	// notrans
}

DO_FUN(do_herbs, ch, argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	bool is_healer;
	int chance;

	one_argument(argument, arg, sizeof(arg));

	if (is_sn_affected(ch, "herbs")) {
		act_char("You can't find any more herbs.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("herbs"));

	if (arg[0] == '\0')
		victim = ch;
	else if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They're not here.", ch);
		return;
	}

	is_healer = IS_NPC(ch) && MOB_IS(ch, MOB_HEALER);
	chance = get_skill(ch, "herbs");
	if (is_healer ||
	    (ch->in_room->sector_type != SECT_INSIDE &&
	     ch->in_room->sector_type != SECT_CITY &&
	     number_percent() < chance)) {
		if (!is_healer) {
			AFFECT_DATA *paf;

			paf = aff_new(TO_AFFECTS, "herbs");
			paf->level	= ch->level;
			paf->duration	= 5;
			affect_to_char(ch, paf);
			aff_free(paf);
		}

		if (is_healer) {
			act("$N gives you some herbs to eat.",
			    victim, NULL, ch, TO_CHAR);
			act("$N gives the herbs to $n.",
			    victim, NULL, ch, TO_NOTVICT);
		} else {
			act_char("You gather some beneficial herbs.", ch);
			act("$n gathers some herbs.", ch, NULL, NULL, TO_ROOM);

			if (ch != victim) {
				act("$n gives you some herbs to eat.",
				    ch, NULL, victim, TO_VICT);
				act("You give the herbs to $N.",
				    ch, NULL, victim, TO_CHAR);
				act("$n gives the herbs to $N.",
				    ch, NULL, victim, TO_NOTVICT);
			}
		}

		if (victim->hit < victim->max_hit) {
			act_char("You feel better.", victim);
			act("$n looks better.", victim, NULL, NULL, TO_ROOM);
		}
		victim->hit = UMIN(victim->max_hit, victim->hit + 5 * LEVEL(ch));
		check_improve(ch, "herbs", TRUE, 1);
		if (is_sn_affected(victim, "plague")
		&&  check_dispel(LEVEL(ch), victim, "plague")) {
			act("$n looks relieved as $s sores vanish.",
			    victim, NULL, NULL, TO_ROOM);
		}

		if (is_sn_affected(victim, "poison")
		&&  check_dispel(LEVEL(ch), victim, "poison")) {
			act("$n does not look so green anymore.",
			    victim, NULL, NULL, TO_ROOM);
		}
	} else {
		act_char("You search for herbs but find none here.", ch);
		act("$n looks around for herbs.", ch, NULL, NULL, TO_ROOM);
		if (chance)
			check_improve(ch, "herbs", FALSE, 1);
	}
}

static const char *
random_spell(void)
{
	skill_t *sk;

	for (;;) {
		sk = (skill_t *) c_random_elem(&skills);
		if (sk == NULL)
			return "agle-bargle";
		if (sk->skill_type == ST_SPELL
		||  sk->skill_type == ST_PRAYER)
			break;
	}

	return gmlstr_mval(&sk->sk_name);
}

static void
do_lore_raw(CHAR_DATA *ch, OBJ_DATA *obj, BUFFER *output)
{
	int chance;
	int percent;
	vo_t v0, v1, v2, v3, v4;
	int mana;

	if (OBJ_IS(obj, OBJ_NOIDENT)) {
		buf_append(output,
		   "You aren't able to determine true nature of the object.\n");
		return;
	}

	if ((percent = get_skill(ch, "lore")) < 10) {
		buf_append(output, "The meaning of this object escapes you for the moment.\n");
		return;
	}

	mana = skill_mana(ch, "lore");
	if (ch->mana < mana) {
		buf_append(output, "You don't have enough mana.\n");
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("lore"));

	/* a random lore */
	chance = number_percent();

	if (percent < 20) {
		buf_printf(output, BUF_END,
			   "Object '%s%s'.\n",
			   obj->pObjIndex->name, obj->label);
		check_improve(ch, "lore", TRUE, 8);
		return;
	} else if (percent < 40) {
		buf_printf(output, BUF_END,
			   "Object '%s%s'.\n"
			   "Weight is %d, value is %d.\n"
			   "Material is %s.\n",
			   obj->pObjIndex->name, obj->label,
			   chance < 60 ?
				obj->weight :
				number_range(obj->weight / 2, 2 * obj->weight),
			   chance < 60 ?
				number_range(obj->cost / 2, 2 * obj->cost) :
				(signed)obj->cost,
			   obj->material);
		check_improve(ch, "lore", TRUE, 7);
		return;
	} else if (percent < 60) {
		buf_printf(output, BUF_END,
			   "Object '%s%s'.\n"
			   "Weight is %d.\n"
			   "Value is %d, level is %d.\n"
			   "Material is %s.\n",
			   obj->pObjIndex->name, obj->label,
			   obj->weight,
			   chance < 60 ?
				number_range(obj->cost / 2, 2 * obj->cost) :
				(signed)obj->cost,
			   chance < 60 ?
				obj->level :
				number_range(obj->level / 2, 2 * obj->level),
			   obj->material);
		check_improve(ch, "lore", TRUE, 6);
		return;
	} else if (percent < 80) {
		buf_printf(output, BUF_END,
			   "Object '%s%s' is type %s, obj flags %s.\n"
			   "Extra flags %s.\n"
			   "Weight is %d, value is %d, level is %d.\n"
			   "Material is %s.\n",
			   obj->pObjIndex->name, obj->label,
			   flag_string(item_types, obj->pObjIndex->item_type),
			   flag_string(obj_flags, obj->pObjIndex->obj_flags),
			   flag_string(stat_flags, obj->stat_flags),
			   obj->weight,
			   chance < 60 ?
				number_range(obj->cost / 2, 2 * obj->cost) :
				(signed)obj->cost,
			   chance < 60 ?
				obj->level :
				number_range(obj->level / 2, 2 * obj->level),
			   obj->material);
		check_improve(ch, "lore", TRUE, 5);
		return;
	} else {
		buf_printf(output, BUF_END,
			   "Object '%s%s' is type %s, obj flags %s.\n"
			   "Extra flags %s.\n"
			   "Weight is %d, value is %d, level is %d.\n"
			   "Material is %s.\n",
			   obj->pObjIndex->name, obj->label,
			   flag_string(item_types, obj->pObjIndex->item_type),
			   flag_string(obj_flags, obj->pObjIndex->obj_flags),
			   flag_string(stat_flags, obj->stat_flags),
			   obj->weight,
			   obj->cost,
			   obj->level,
			   obj->material);
	}

	v0 = obj->value[0];
	v1 = obj->value[1];
	v2 = obj->value[2];
	v3 = obj->value[3];
	v4 = obj->value[4];

	switch (obj->pObjIndex->item_type) {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		if (percent < 85) {
			INT(v0) = number_range(1, 60);
			if (chance > 40) {
				v1.s = random_spell();
				if (chance > 60) {
					v2.s = random_spell();
					if (chance > 80)
						v3.s = random_spell();
				}
			}
		} else {
			if (chance > 60) {
				v1.s = random_spell();
				if (chance > 80) {
					v2.s = random_spell();
					if (chance > 95)
						v3.s = random_spell();
				}
			}
		}

		buf_printf(output, BUF_END, "Level %d spells of:", INT(v0));
		if (!IS_NULLSTR(v1.s))
			buf_printf(output, BUF_END, " '%s'", v1.s);
		if (!IS_NULLSTR(v2.s))
			buf_printf(output, BUF_END, " '%s'", v2.s);
		if (!IS_NULLSTR(v3.s))
			buf_printf(output, BUF_END, " '%s'", v3.s);
		if (!IS_NULLSTR(v4.s))
			buf_printf(output, BUF_END, " '%s'", v4.s);
		buf_append(output, ".\n");
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		if (percent < 85) {
			INT(v0) = number_range(1, 60);
			if (chance > 40) {
				v3.s = random_spell();
				if (chance > 60) {
					INT(v2) = number_range(0, 2 * INT(v2));
					if (chance > 80)
						INT(v1) = number_range(0, INT(v2));
				}
			}
		} else {
			if (chance > 60) {
				v3.s = random_spell();
				if (chance > 80) {
					INT(v2) = number_range(0, 2 * INT(v2));
					if (chance > 95)
						INT(v1) = number_range(0, INT(v2));
				}
			}
		}

		buf_printf(output, BUF_END, "Has %d(%d) charges of level %d '%s'.\n",
			    INT(v1), INT(v2), INT(v0), v3.s);
		break;

	case ITEM_WEAPON:
		buf_append(output, "Weapon type is ");
		if (percent < 85) {
			INT(v0) = number_range(0, 8);
			if (chance > 33) {
				INT(v1) = number_range(1, 2 * INT(v1));
				if (chance > 66)
					INT(v2) = number_range(1, 2 * INT(v2));
			}
		} else {
			if (chance > 50) {
				INT(v1) = number_range(1, 2 * INT(v1));
				if (chance > 75)
					INT(v2) = number_range(1, 2 * INT(v2));
			}
		}

		buf_printf(output, BUF_END, "%s.\n",		// notrans
		    flag_string(weapon_class, INT(v0)));

		buf_printf(output, BUF_END, "Damage is %dd%d (average %d).\n",
			   INT(v1), INT(v2), GET_AVE(v1, v2));
		break;

	case ITEM_ARMOR:
		if (percent < 85) {
			if (chance > 25) {
				INT(v2) = number_range(0, 2 * INT(v2));
				if (chance > 45) {
					INT(v0) = number_range(0, 2 * INT(v0));
					if (chance > 65) {
						INT(v3) = number_range(0, 2 * INT(v3));
						if (chance > 85)
							INT(v1) = number_range(0, 2 * INT(v1));
					}
				}
			}
		} else {
			if (chance > 45) {
				INT(v2) = number_range(0, 2 * INT(v2));
				if (chance > 65) {
					INT(v0) = number_range(0, 2 * INT(v0));
					if (chance > 85) {
						INT(v3) = number_range(0, 2 * INT(v3));
						if (chance > 95)
							INT(v1) = number_range(0, 2 * INT(v1));
					}
				}
			}
		}

		buf_printf(output, BUF_END,
			    "Armor class is %d pierce, %d bash, %d slash, and %d vs. exotic.\n",
			    INT(v0), INT(v1), INT(v2), INT(v3));
		break;
	}

	if (percent < 87) {
		check_improve(ch, "lore", FALSE, 5);
		return;
	}

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED)) {
		format_obj_affects(
		    output, obj->pObjIndex->affected, FOA_F_NODURATION);
	}
	format_obj_affects(output, obj->affected, 0);
	check_improve(ch, "lore", TRUE, 5);
}

DO_FUN(do_lore, ch, argument)
{
	char		arg[MAX_INPUT_LENGTH];
	BUFFER *	output;
	OBJ_DATA *	obj;

	if (ch->fighting != NULL) {
		act_puts("Concentrate on your fighting!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have that object.", ch);
		return;
	}

	output = buf_new(GET_LANG(ch));
	do_lore_raw(ch, obj, output);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

#define OBJ_VNUM_STEAK			27

DO_FUN(do_butcher, ch, argument)
{
	OBJ_DATA       *obj;
	char            arg[MAX_STRING_LENGTH];
	OBJ_DATA       *tmp_obj;
	OBJ_DATA       *tmp_next;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Butcher what?", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_BATTLE_ARENA)) {
		act_char("This place doesn't seem to be good for your cooking experiments.", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) == NULL) {
		act_char("You do not see that here.", ch);
		return;
	}
	if (obj->pObjIndex->item_type != ITEM_CORPSE_PC
	&&  obj->pObjIndex->item_type != ITEM_CORPSE_NPC) {
		act_char("You can't butcher that.", ch);
		return;
	}
	if (obj->carried_by != NULL) {
		act_char("Put it down first.", ch);
		return;
	}

	for (tmp_obj = obj->contains; tmp_obj != NULL; tmp_obj = tmp_next) {
		tmp_next = tmp_obj->next_content;
		obj_to_room(tmp_obj, ch->in_room);
	}

	if (number_percent() < get_skill(ch, "butcher")) {
		int             numsteaks;
		int             i;
		OBJ_DATA       *steak;
		numsteaks = number_bits(2) + 1;

		act("$n butchers $P and creates $j $qj{steaks}.",
		    ch, (const void*) numsteaks, obj, TO_ROOM);
		act_puts("You butcher $P and create $j $qj{steaks}.",
			 ch, (const void*) numsteaks, obj,
			 TO_CHAR, POS_DEAD);
		check_improve(ch, "butcher", TRUE, 1);

		for (i = 0; i < numsteaks; i++) {
			steak = create_obj_of(
			    OBJ_VNUM_STEAK, &obj->short_descr);
			if (steak != NULL)
				obj_to_room(steak, ch->in_room);
		}
	} else {
		act("You fail and destroy $p.", ch, obj, NULL, TO_CHAR);
		act("$n fails to butcher $p and destroys it.",
		    ch, obj, NULL, TO_ROOM);

		check_improve(ch, "butcher", FALSE, 1);
	}

	extract_obj(obj, 0);
}

#define OBJ_VNUM_CROSS			8

DO_FUN(do_crucify, ch, argument)
{
	OBJ_DATA *obj;
	char arg[MAX_STRING_LENGTH];
	OBJ_DATA *obj2, *next;
	OBJ_DATA *cross;

	if (is_sn_affected(ch, "crucify")) {
		act_char("You are not yet ready to make a sacrifice.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_BATTLE_ARENA)) {
		act_char("Some mystical force prevents you from doing that.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_BERSERK)
	||  is_sn_affected(ch, "frenzy")) {
		act_char("Calm down first.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg == '\0') {
		act_char("Crucify what?", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) == NULL) {
		act_char("You do not see that here.", ch);
		return;
	}

	if (obj->pObjIndex->item_type != ITEM_CORPSE_PC
	&&  obj->pObjIndex->item_type != ITEM_CORPSE_NPC) {
		act_char("You cannot crucify that.", ch);
		return;
	 }

	if (obj->carried_by != NULL) {
		act_char("Put it down first.", ch);
		return;
	}

	for (obj2 = obj->contains; obj2; obj2 = next) {
		next = obj2->next_content;
		obj_to_room(obj2, ch->in_room);
	}

	if (number_percent() >= get_skill(ch, "crucify")) {
		act("You attempt a ritual crucification of $p, "
		   "but fail and ruin it.", ch, obj, NULL, TO_CHAR);
		act("$n attempts to crucify $p, but fails and ruins it.",
		    ch, obj, NULL, TO_ROOM);
		extract_obj(obj, 0);
		check_improve(ch, "crucify", FALSE, 1);
	} else {
		AFFECT_DATA *paf;

		cross = create_obj_of(OBJ_VNUM_CROSS, &obj->owner);
		if (cross == NULL)
			return;

		cross->timer =  10 * ch->level;
		obj_to_room(cross, ch->in_room);
		act("With a crunch of bone and splash of blood you nail "
		    "$p to a sacrificial cross.", ch, obj, NULL, TO_CHAR);
		act("With a crunch of bone and splash of blood $n nails "
		    "$p to a sacrificial cross.", ch, obj, NULL, TO_ROOM);
		act_char("You are filled with a dark energy.", ch);
		ch->hit += obj->level*3/2;

		paf = aff_new(TO_AFFECTS, "crucify");
		paf->level	= ch->level;
		paf->duration	= 15;

		INT(paf->location)= APPLY_HITROLL;
		paf->modifier	= UMAX(1, obj->level/8);
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_DAMROLL;
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_AC;
		paf->modifier	= UMAX(10, obj->level);
		paf->bitvector	= AFF_BERSERK;
		affect_to_char(ch, paf);
		aff_free(paf);

		extract_obj(obj, 0);
		check_improve(ch, "crucify", TRUE, 1);
	}
}

DO_FUN(do_balance, ch, argument)
{
	int bank_g;
	int bank_s;

	if (IS_NPC(ch)) {
		act_char("You don't have a bank account.", ch);
		return;
	}

	bank_g = PC(ch)->bank_g;
	bank_s = PC(ch)->bank_s;

	if (bank_s + bank_g == 0)
		act_char("You don't have any money in the bank.", ch);
	else if (bank_s == 0) {
		act_puts("You have $j $qj{gold coins} in the bank.",
			 ch, (const void *) bank_g, NULL, TO_CHAR, POS_DEAD);
	} else if (bank_g == 0) {
		act_puts("You have $j $qj{silver coins} in the bank.",
			 ch, (const void *) bank_s, NULL, TO_CHAR, POS_DEAD);
	} else {
		act_puts3("You have $j gold and $J $qJ{silver coins} in the bank.",
			  ch, (const void *) bank_g, NULL, (const void *) bank_s,
			  TO_CHAR, POS_DEAD);
	}
}

DO_FUN(do_withdraw, ch, argument)
{
	int	silver, gold;
	int	silver_fee, gold_fee;
	char	arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		act_char("You don't have a bank account.", ch);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		act_char("The mosquito by your feet will not give you any money.", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Withdraw how much?", ch);
		return;
	}

	argument = money_argument(argument, arg, sizeof(arg), &silver, &gold);
	if (argument == NULL) {
		do_withdraw(ch, str_empty);
		return;
	}

	if (silver > PC(ch)->bank_s || gold > PC(ch)->bank_g) {
		act_char("Sorry, we don't give loans.", ch);
		return;
	}

	silver_fee = silver ? UMAX(1, silver * 10 / 100) : 0;
	gold_fee = gold ? UMAX(1, gold * 2 / 100) : 0;
	if (!can_carry_more_w(ch, COINS_WEIGHT(silver - silver_fee, gold - gold_fee))) {
		act_char("You can't carry that weight.", ch);
		return;
	}

	if (silver) {
		ch->silver += silver - silver_fee;
		PC(ch)->bank_s -= silver;
		act_puts("Here are your $j $qj{silver coins}",
			 ch, (const void *) silver, NULL,
			 TO_CHAR | ACT_NOLF, POS_DEAD);
		act_puts(", minus a $j $qj{coins} withdrawal fee.",
			 ch, (const void *) silver_fee, NULL,
			 TO_CHAR, POS_DEAD);
	}
	if (gold) {
		ch->gold += gold - gold_fee;
		PC(ch)->bank_g -= gold;
		act_puts("Here are your $j $qj{gold coins}",
			 ch, (const void *) gold, NULL,
			 TO_CHAR | ACT_NOLF, POS_DEAD);
		act_puts(", minus a $j $qj{coins} withdrawal fee.",
			 ch, (const void *) gold_fee, NULL,
			 TO_CHAR, POS_DEAD);
	}

	act("$n steps up to the teller window.", ch, NULL, NULL, TO_ROOM);
}

DO_FUN(do_deposit, ch, argument)
{
	int	silver, gold;
	char	arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		act_char("You don't have a bank account.", ch);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		act_char("The ant by your feet can't carry your gold.", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Deposit how much?", ch);
		return;
	}

	argument = money_argument(argument, arg, sizeof(arg), &silver, &gold);
	if (argument == NULL) {
		do_deposit(ch, str_empty);
		return;
	}

	if (silver > ch->silver || gold > ch->gold) {
		act_char("That's more than you've got.", ch);
		return;
	}

	if (silver) {
		PC(ch)->bank_s += silver;
		ch->silver -= silver;
		if (silver == 1)
			act_char("Oh $gn{boy}! One silver coin!", ch);
		else {
			act_puts("$j $qj{silver coins} deposited. Come again soon!",
				 ch, (const void *) silver, NULL,
				 TO_CHAR, POS_DEAD);
		}
	}
	if (gold) {
		PC(ch)->bank_g += gold;
		ch->gold -= gold;
		if (gold == 1)
			act_char("Oh $gn{boy}! One gold coin!", ch);
		else {
			act_puts("$j $qj{gold coins} deposited. Come again soon!",
				 ch, (const void *) gold, NULL,
				 TO_CHAR, POS_DEAD);
		}
	}

	act("$n steps up to the teller window.", ch, NULL, NULL, TO_ROOM);
}

/* wear object as a secondary weapon */
DO_FUN(do_second_wield, ch, argument)
{
	OBJ_DATA *obj;

	if (argument[0] == '\0') {
		act_char("Wear which weapon in your off-hand?", ch);
		return;
	}

	obj = get_obj_carry(ch, ch, argument);
	if (obj == NULL) {
		act_char("You don't have that item.", ch);
		return;
	}

	if (!CAN_WEAR(obj, ITEM_WIELD)) {
		act_char("You can't wield that as your second weapon.", ch);
		return;
	}

	if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS|WEAPON_NO_OFFHAND)
	||  WEAPON_IS_LONG(obj)) {
		act_char("You can't dual wield this weapon!", ch);
		return;
	}

	if (get_eq_char(ch, WEAR_WIELD) == NULL) {
		act_char("You need to wield a primary weapon, before using a secondary one!", ch);
		return;
	}

	if (get_obj_weight(obj) > str_app[get_curr_stat(ch, STAT_STR)].wield / 2) {
		act_char("This weapon is too heavy to be used as a secondary weapon by you.", ch);
		return;
	}

	if (free_hands(ch) < 1 && !get_eq_char(ch, WEAR_SECOND_WIELD)) {
		act("But your hands are both busy.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!remove_obj(ch, WEAR_SECOND_WIELD, TRUE))
		return;

	if (!pull_wear_triggers(ch, obj))
		return;

	act("$n wields $p in $s off-hand.", ch, obj, NULL, TO_ROOM);
	act("You wield $p in your off-hand.", ch, obj, NULL, TO_CHAR);
	equip_char(ch, obj, WEAR_SECOND_WIELD);

	show_weapon_skill(ch, obj);
}

DO_FUN(do_enchant, ch, argument)
{
	OBJ_DATA *obj;

	if (argument[0] == '\0') {	/* empty */
		act_char("Wear which weapon to enchant?", ch);
		return;
	}
	obj = get_obj_carry(ch, ch, argument);

	if (obj == NULL) {
		act_char("You don't have that item.", ch);
		return;
	}

	if (ch->level < obj->level) {
		act_puts("You must be level $j to be able to enchant this object.",
			 ch, (const void *) obj->level, NULL,
			 TO_CHAR, POS_DEAD);
		act("$n tries to enchant $p, but is too inexperienced.",
		    ch, obj, NULL, TO_ROOM);
		return;
	}

	if (ch->mana < 100) {
		act_char("You don't have enough mana.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("enchant sword"));
	if (number_percent() > get_skill(ch, "enchant sword")) {
		act_char("You lost your concentration.", ch);
		act("$n tries to enchant $p, but he forgets how for a moment.",
		    ch, obj, NULL, TO_ROOM);
		check_improve(ch, "enchant sword", FALSE, 6);
		ch->mana -= 50;
		return;
	}
	ch->mana -= 100;
	spellfun("enchant weapon", NULL, LEVEL(ch), ch, obj);
	check_improve(ch, "enchant sword", TRUE, 2);
}

DO_FUN(do_label, ch, argument)
{
	OBJ_DATA *obj;
	char obj_name[MAX_INPUT_LENGTH];
	char label[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, obj_name, sizeof(obj_name));
	first_arg(argument, label, sizeof(label), FALSE);

	if (!obj_name[0]) {
		act_char("Label what?", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, ch, obj_name)) == NULL) {
		act_char("You don't have that object.", ch);
		return;
	}

	if (!label[0]) {
		act_char("How do you want to label it?", ch);
		return;
	}

	if (PC(ch)->questpoints < 10) {
		act_char("You do not have enough questpoints for labeling.", ch);
		return;
	}

	if ((strlen(obj->label) + strlen(label) + 2) >= MAX_STRING_LENGTH) {
		act_char("You can't label this object with this label.", ch);
		return;
	}

	label_add(obj, label);
	PC(ch)->questpoints -= 10;
	act_char("Ok.", ch);
}

#define OBJ_VNUM_HAMMER			6522

DO_FUN(do_repair, ch, argument)
{
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int cost;

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && MOB_IS(mob, MOB_REPAIRMAN))
			break;
	}

	if (mob == NULL) {
		act_char("You can't do that here.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_say(mob,"I will repair a weapon for you, for a price.");
		act_char("Type estimate <weapon> to be assessed for damage.", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		do_say(mob, "You don't have that item.");
		return;
	}

	if (obj->pObjIndex->vnum == OBJ_VNUM_HAMMER) {
		act_say(mob, "$P is beyond my power.", obj);
		return;
	}

	if (obj->condition >= 100) {
		do_say(mob,"But that item is not broken.");
		return;
	}

	if (obj->cost == 0) {
		act_say(mob, "$P is beyond repair.", obj);
		return;
	}

	cost = obj->level * 10 + obj->cost * (100 - obj->condition) /100;
	cost /= 100;

	if (cost > ch->gold) {
		do_say(mob,"You do not have enough gold for my services.");
		return;
	}

	WAIT_STATE(ch, get_pulse("violence"));

	ch->gold -= cost;
	mob->gold += cost;

	if (pull_obj_trigger(TRIG_OBJ_REPAIR, obj, ch, NULL) > 0
	||  !mem_is(obj, MT_OBJ)
	||  IS_EXTRACTED(ch))
		return;

	act_puts("$N takes $p from you, repairs it, and returns it back.",
		 ch, obj, mob, TO_CHAR, POS_RESTING);
	act_puts("$N takes $p from $n, repairs it, and returns it back.",
		 ch, obj, mob, TO_ROOM, POS_RESTING);
	obj->condition = 100;
}

DO_FUN(do_estimate, ch, argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	int cost;

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && MOB_IS(mob, MOB_REPAIRMAN))
			break;
	}

	if (mob == NULL) {
		act_char("You can't do that here.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_say(mob, "Try estimate <item>.");
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		do_say(mob, "You don't have that item.");
		return;
	}

	if (obj->pObjIndex->vnum == OBJ_VNUM_HAMMER) {
		act_say(mob, "$P is beyond my power.", obj);
		return;
	}

	if (obj->condition >= 100) {
		do_say(mob, "But that item's not broken.");
		return;
	}

	if (obj->cost == 0) {
		do_say(mob,"That item is beyond repair.");
		return;
	}

	cost = obj->level * 10 + obj->cost * (100 - obj->condition) / 100;
	cost /= 100;

	act_say(mob, "It will cost you $K gold to fix that item.",
		(const void *) cost);
}

DO_FUN(do_smithing, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *hammer;
	int chance;

	if (ch->fighting) {
		act_char("Wait until the fight finishes.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Which object do you want to repair.", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You are not carrying that.", ch);
		return;
	}

	if (obj->condition >= 100) {
		act_char("But that item is not broken.", ch);
		return;
	}

	if ((hammer = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		act_char("You are not holding a hammer.", ch);
		return;
	}

	if (hammer->pObjIndex->vnum != OBJ_VNUM_HAMMER) {
		act_char("That is not the correct hammer.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("smithing"));
	if (number_percent() > (chance = get_skill(ch, "smithing"))) {
		check_improve(ch, "smithing", FALSE, 8);

		if (pull_obj_trigger(TRIG_OBJ_REPAIR, obj, ch, NULL) > 0
		||  !mem_is(obj, MT_OBJ)
		||  IS_EXTRACTED(ch))
			return;

		act_puts("$n tries to repair $p with the hammer but fails.",
			 ch, obj, NULL, TO_ROOM, POS_RESTING);
		act_puts("You failed to repair $p.",
			 ch, obj, NULL, TO_CHAR, POS_RESTING);
		hammer->condition -= 25;
	} else {
		check_improve(ch, "smithing", TRUE, 4);
		act_puts("$n repairs $p with the hammer.",
			 ch, obj, NULL, TO_ROOM, POS_RESTING);
		act_puts("You repair $p.",
			 ch, obj, NULL, TO_CHAR, POS_RESTING);
		obj->condition = UMAX(100, obj->condition + chance / 2);
		hammer->condition -= 25;
	}

	if (hammer->condition < 1)
		extract_obj(hammer, 0);
}

/*
 * static functions
 */
static CHAR_DATA *find_keeper(CHAR_DATA *ch)
{
	CHAR_DATA *keeper;
	SHOP_DATA *pShop = NULL;
	bool closed;
	int hour;

	for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room) {
		if (IS_NPC(keeper)
		&&  (pShop = keeper->pMobIndex->pShop) != NULL)
			break;
	}

	if (pShop == NULL) {
		act_char("You can't do that here.", ch);
		return NULL;
	}

	if (IS_WANTED(ch)) {
		do_say(keeper, "Criminals are not welcome!");
		act_yell(keeper, "$lu{$N} the CRIMINAL is over here!", ch, NULL);
		return NULL;
	}

	/*
	 * shop hours
	 */
	hour = time_info.hour;
	closed = pShop->open_hour < pShop->close_hour ?
		!(pShop->open_hour <= hour && hour < pShop->close_hour) :
		pShop->close_hour <= hour && hour < pShop->open_hour;
	if (closed) {
		if (hour < pShop->open_hour)
			do_say(keeper, "Sorry, I am closed. Come back later.");
		else
			do_say(keeper, "Sorry, I am closed. Come back tomorrow.");
		return NULL;
	}

	/*
	 * Invisible or hidden people.
	 */
	if (!can_see(keeper, ch) && !IS_IMMORTAL(ch)) {
		do_say(keeper, "I don't trade with folks I can't see.");
		do_emote(keeper, "looks all around.");
		return NULL;
	}
	return keeper;
}

/* insert an object at the right spot for the keeper */
static void obj_to_keeper(OBJ_DATA *obj, CHAR_DATA *ch)
{
	OBJ_DATA       *t_obj, *t_obj_next;
	/* see if any duplicates are found */
	for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next) {
		t_obj_next = t_obj->next_content;

		if (obj->pObjIndex == t_obj->pObjIndex
		&&  !mlstr_cmp(&obj->short_descr, &t_obj->short_descr)) {
			if (IS_OBJ_STAT(t_obj, ITEM_INVENTORY)) {
				extract_obj(obj, 0);
				return;
			}
			obj->cost = t_obj->cost;	/* keep it standard */
			break;
		}
	}

	if (t_obj == NULL) {
		obj->next_content = ch->carrying;
		ch->carrying = obj;
	} else {
		obj->next_content = t_obj->next_content;
		t_obj->next_content = obj;
	}

	obj->carried_by = ch;
	obj->in_room = NULL;
	obj->in_obj = NULL;
	ch->carry_number += get_obj_number(obj);
	ch->carry_weight += get_obj_weight(obj);
}

/* get an object from a shopkeeper's list */
static OBJ_DATA *get_obj_keeper(CHAR_DATA *ch, CHAR_DATA *keeper,
				const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	int             number;
	int             count;
	number = number_argument(argument, arg, sizeof(arg));
	count = 0;
	for (obj = keeper->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE
		    && can_see_obj(keeper, obj)
		    && can_see_obj(ch, obj)
		    && IS_OBJ_NAME(obj, arg)) {
			if (++count == number)
				return obj;

			/* skip other objects of the same name */
			while (obj->next_content != NULL
			  && obj->pObjIndex == obj->next_content->pObjIndex
			  && !mlstr_cmp(&obj->short_descr, &obj->next_content->short_descr))
				obj = obj->next_content;
		}
	}

	return NULL;
}

static uint get_cost(CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy)
{
	SHOP_DATA *	pShop;
	uint		cost;

	if (obj == NULL || (pShop = keeper->pMobIndex->pShop) == NULL)
		return 0;

	if (OBJ_IS(obj, OBJ_NOSELL))
		return 0;

	if (fBuy)
		cost = obj->cost * pShop->profit_buy / 100;
	else {
		OBJ_DATA       *obj2;
		int             itype;
		cost = 0;
		for (itype = 0; itype < MAX_TRADE; itype++) {
			if (obj->pObjIndex->item_type == pShop->buy_type[itype]) {
				cost = obj->cost * pShop->profit_sell / 100;
				break;
			}
		}

		if (!OBJ_IS(obj, OBJ_SELL_EXTRACT))
			for (obj2 = keeper->carrying; obj2; obj2 = obj2->next_content) {
				if (obj->pObjIndex == obj2->pObjIndex
				&&  !mlstr_cmp(&obj->short_descr,
					       &obj2->short_descr))
					return 0;
/*
		    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
				cost /= 2;
		    else
				cost = cost * 3 / 4;
*/
			}
	}

	if (obj->pObjIndex->item_type == ITEM_STAFF
	||  obj->pObjIndex->item_type == ITEM_WAND) {
		if (INT(obj->value[1]) == 0)
			cost /= 4;
		else
			cost = cost * INT(obj->value[2]) / INT(obj->value[1]);
	}
	return cost;
}

static void
sac_obj(CHAR_DATA * ch, OBJ_DATA *obj)
{
	uint		silver;
	CHAR_DATA      *gch;
	int             members;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if (gch->on == obj) {
			act("$N appears to be using $p.",
			    ch, obj, gch, TO_CHAR);
			return;
		}
	}

	if (!CAN_WEAR(obj, ITEM_TAKE) || OBJ_IS(obj, OBJ_NOSAC)) {
		act_puts("$p is not an acceptable sacrifice.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (pull_obj_trigger(TRIG_OBJ_SAC, obj, ch, NULL) > 0
	||  !mem_is(obj, MT_OBJ)
	||  IS_EXTRACTED(ch))
		return;

	silver = UMAX(1, number_fuzzy(obj->level));
	if (obj->pObjIndex->item_type != ITEM_CORPSE_NPC
	&&  obj->pObjIndex->item_type != ITEM_CORPSE_PC)
		silver = UMIN(silver, obj->cost);

	act("You sacrifice $p to gods.", ch, obj, NULL, TO_CHAR);
	act("$n sacrifices $p to gods.", ch, obj, NULL, TO_ROOM);
	if (can_carry_more_w(ch, COINS_WEIGHT(silver, 0))) {
		act_puts("Gods give you $j silver $qj{coins} for your sacrifice.",
			 ch, (const void *) silver, NULL, TO_CHAR, POS_DEAD);
		ch->silver += silver;

		if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_AUTOSPLIT)) {
			/* AUTOSPLIT code */
			members = 0;
			for (gch = ch->in_room->people; gch != NULL;
			     gch = gch->next_in_room)
				if (is_same_group(gch, ch))
					members++;

			if (members > 1 && silver > 1) {
				char buf[MAX_INPUT_LENGTH];

				snprintf(buf, sizeof(buf), "%d", silver);
				dofun("split", ch, buf);
			}
		}
	}

	wiznet("$N sends up $p as a burnt offering.",
	       ch, obj, WIZ_SACCING, 0, 0);
	if (obj->pObjIndex->item_type == ITEM_CORPSE_NPC
	||  obj->pObjIndex->item_type == ITEM_CORPSE_PC) {
		OBJ_DATA       *obj_content;
		OBJ_DATA       *obj_next;
		OBJ_DATA       *two_objs[2];

		int	iScatter = 0;

		const char *qty;
		const char *where;

		for (obj_content = obj->contains; obj_content;
		     obj_content = obj_next) {
			obj_next = obj_content->next_content;
			two_objs[iScatter < 1 ? 0 : 1] = obj_content;
			obj_to_room(obj_content, ch->in_room);
			iScatter++;
		}

		switch (iScatter) {
		case 0:
			break;

		case 1:
			act_puts("Your sacrifice reveals $p.",
				 ch, two_objs[0], NULL, TO_CHAR, POS_DEAD);
			act("$p is revealed by $n's sacrifice.",
			    ch, two_objs[0], NULL, TO_ROOM);
			break;

		case 2:
			act_puts("Your sacrifice reveals $p and $P.",
				 ch, two_objs[0], two_objs[1],
				 TO_CHAR, POS_DEAD);
			act("$p and $P are revealed by $n's sacrifice.",
			    ch, two_objs[0], two_objs[1], TO_ROOM);
			break;

		default:
			if (iScatter < 5)
				qty = "few things";
			else if (iScatter < 9)
				qty = "a bunch of objects";
			else if (iScatter < 15)
				qty = "many things";
			else
				qty = "a lot of objects";

			switch (ch->in_room->sector_type) {
			case SECT_FIELD:
			case SECT_FOREST:
				where = "on the dirt";
				break;

			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
				where = "over the water";
				break;

			default:
				where = "around";
				break;
			}
			act_puts("As you sacrifice the corpse, $t on it "
				 "scatter $T.",
				 ch, qty, where, TO_CHAR, POS_DEAD);
			act("As $n sacrifices the corpse, $t on it scatter $T.",
			    ch, qty, where, TO_ROOM);
			break;
		}
	}
	extract_obj(obj, 0);
}

static bool put_obj(CHAR_DATA *ch, OBJ_DATA *container,
		    OBJ_DATA *obj, int* count)
{
	OBJ_DATA *	objc;
	clan_t *clan;

	/*
	 * clan members can put item into clan altar even if it is closed
	 * check that container is clan altar is done in do_put
	 */
	if (IS_SET(INT(container->value[1]), CONT_CLOSED)
	&&  ((clan = clan_lookup(ch->clan)) == NULL ||
	     clan->obj_ptr != obj)) {
		act_puts("$P is closed.",
			 ch, NULL, container, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (IS_SET(INT(container->value[1]), CONT_QUIVER)
	&&  (obj->pObjIndex->item_type != ITEM_WEAPON ||
	     !WEAPON_IS(obj, WEAPON_ARROW))) {
		act_puts("You can only put arrows in $p.",
			 ch, container, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (OBJ_IS(container, OBJ_PIT)
	&&  !CAN_WEAR(obj, ITEM_TAKE)) {
		if (obj->timer)
			SET_OBJ_STAT(obj, ITEM_HAD_TIMER);
		else
			obj->timer = number_range(100, 200);
	}

	if (obj->pObjIndex->limit != -1
	||  OBJ_IS(obj, OBJ_QUEST)) {
		act_puts("This unworthy container won't hold $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return TRUE;
	}

	if (obj->pObjIndex->item_type == ITEM_POTION
	&&  IS_SET(container->wear_flags, ITEM_TAKE)) {
		int pcount = 0;
		for (objc = container->contains; objc; objc = objc->next_content)
			if (objc->pObjIndex->item_type == ITEM_POTION)
				pcount++;
		if (pcount > 15) {
			act_puts("It's not safe to put more potions into $p.",
				 ch, container, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}
	}

	(*count)++;
	if (*count > INT(container->value[0])) {
		act_puts("It's not safe to put that much items into $p.",
			 ch, container, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	obj_to_obj(obj, container);

	if (IS_SET(INT(container->value[1]), CONT_PUT_ON)) {
		act("$n puts $p on $P.", ch, obj, container, TO_ROOM);
		act_puts("You put $p on $P.",
			 ch, obj, container, TO_CHAR, POS_DEAD);
	} else {
		act("$n puts $p in $P.", ch, obj, container, TO_ROOM);
		act_puts("You put $p in $P.",
			 ch, obj, container, TO_CHAR, POS_DEAD);
	}

	return TRUE;
}

#define OBJ_VNUM_SCHOOL_VEST		3703
#define OBJ_VNUM_SCHOOL_SHIELD		3704
#define OBJ_VNUM_SCHOOL_BANNER		3716

/*
 * equips a character
 */
DO_FUN(do_outfit, ch, argument)
{
	OBJ_DATA *obj;
	class_t *cl = class_lookup(ch->ch_class);

	if ((ch->level > 5 && !IS_IMMORTAL(ch))
	||  IS_NPC(ch) || cl == NULL) {
		act_puts("Find it yourself!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) == NULL
	&&  (obj = create_obj(OBJ_VNUM_SCHOOL_BANNER, 0)) != NULL) {
		obj->cost = 0;
		obj->condition = 100;
		obj_to_char(obj, ch);
		equip_char(ch, obj, WEAR_LIGHT);
	}

	if ((obj = get_eq_char(ch, WEAR_BODY)) == NULL
	&&  (obj = create_obj(OBJ_VNUM_SCHOOL_VEST, 0)) != NULL) {
		obj->cost = 0;
		obj->condition = 100;
		obj_to_char(obj, ch);
		equip_char(ch, obj, WEAR_BODY);
	}

	/* do the weapon thing */
	if (((obj = get_eq_char(ch, WEAR_WIELD)) == NULL)
	&&  free_hands(ch)
	&&  (obj = create_obj(cl->weapon, 0)) != NULL) {
		obj->cost = 0;
		obj->condition = 100;
		obj_to_char(obj, ch);
		equip_char(ch, obj, WEAR_WIELD);
	}

	if (((obj = get_eq_char(ch, WEAR_SHIELD)) == NULL)
	&&  free_hands(ch)
	&&  (obj = create_obj(OBJ_VNUM_SCHOOL_SHIELD, 0)) != NULL) {
		obj->cost = 0;
		obj->condition = 100;
		obj_to_char(obj, ch);
		equip_char(ch, obj, WEAR_SHIELD);
	}

	act_puts("You have been equipped by gods.",
		 ch, NULL, NULL, TO_CHAR, POS_DEAD);
}
DO_FUN(do_keep, ch, argument)
{
	char		arg[MAX_INPUT_LENGTH];
	OBJ_DATA	*obj;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Keep what?", ch);
		return;
	}

	if (is_number(arg)) {
		act_char("You can't keep coins.", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}

	if (IS_OBJ_STAT(obj, ITEM_KEEP)) {
		act("$p is already kept.", ch, obj, NULL, TO_CHAR);
		return;
	}

	SET_OBJ_STAT(obj, ITEM_KEEP);
	act("You keep $p.", ch, obj, NULL, TO_CHAR);
}

DO_FUN(do_unkeep, ch, argument)
{
	char		arg[MAX_INPUT_LENGTH];
	OBJ_DATA	*obj;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Unkeep what?", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}

	if (!IS_OBJ_STAT(obj, ITEM_KEEP)) {
		act("$p is not kept.", ch, obj, NULL, TO_CHAR);
		return;
	}

	REMOVE_OBJ_STAT(obj, ITEM_KEEP);
	act("You unkeep $p.", ch, obj, NULL, TO_CHAR);
}

#define OBJ_VNUM_SHARPSTONE		34484

DO_FUN(do_sharpen_weapon, ch, argument)
{
	AFFECT_DATA *paf;
	int chance;		// success chance
	int survive = 80;	// base chance to survive weapon if failed
	OBJ_DATA *weapon, *stone;
	int mana;
	int pk_range;

	if (argument[0] == '\0') {
		act("What do you like to sharpen?", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((weapon = get_obj_carry(ch, ch, argument)) == NULL) {
		act("No such object.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (weapon->pObjIndex->item_type != ITEM_WEAPON) {
		act("That isn't a weapon.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (weapon->wear_loc != -1) {
		act("The weapon must be carried.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (IS_WEAPON_STAT(weapon, WEAPON_SHARP)) {
		act("$p is already sharp.", ch, weapon, NULL, TO_CHAR);
		return;
	}

	if (!WEAPON_IS(weapon, WEAPON_DAGGER)
	&&  !WEAPON_IS(weapon, WEAPON_SWORD)
	&&  !WEAPON_IS(weapon, WEAPON_AXE)) {
		act("You can sharpen only daggers, swords, axes.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	stone = get_eq_char(ch, WEAR_HOLD);
	if (stone == NULL
	||  stone->pObjIndex->vnum != OBJ_VNUM_SHARPSTONE) {
		act("You need a sharpstone to do it.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	mana = skill_mana(ch, "sharpen weapon");
	if (ch->mana < mana) {
		act("You have not enought energy to sharpen $p.",
		    ch, weapon, NULL, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, skill_beats("sharpen weapon"));
	ch->mana -= mana;
	chance = get_skill(ch, "sharpen weapon") * 3 / 5;

	pk_range = UMAX(4, ch->level / 10 + 2);
	if (weapon->level > LEVEL(ch) + pk_range) {
		chance = (chance / 10) * LEVEL(ch) / weapon->level;
		survive /= 2;
	}

	if (weapon->level < LEVEL(ch))
		chance = chance * LEVEL(ch) / weapon->level;

	if (IS_WEAPON_STAT(weapon, WEAPON_VORPAL))
		chance = chance * 3 / 4;

	if (IS_WEAPON_STAT(weapon, WEAPON_VAMPIRIC))
		chance = chance * 3 / 4;

	if (IS_WEAPON_STAT(weapon, WEAPON_HOLY))
		chance = chance * 3 / 4;

	if (number_percent() < chance) {

		paf = aff_new(TO_WEAPON, "sharpen weapon");
		paf->level	= ch->level;
		if (number_bits(4) == 1)
			paf->duration = -1;
		else
			paf->duration	= ch->level / 5;
		paf->bitvector	= WEAPON_SHARP;
		affect_to_obj(weapon, paf);
		aff_free(paf);

		act("You successfully sharpen $p!", ch, weapon, NULL, TO_CHAR);
		if (number_percent() < 75)
			act("You survive your sharpstone.",
			    ch, NULL, NULL, TO_CHAR);
		else {
			act("Your sharpstone is gone!",
			    ch, NULL, NULL, TO_CHAR);
			extract_obj(stone, 0);
		}
	} else {
		if (number_percent() < survive) {
			if (number_bits(2) == 0) {
				AFFECT_DATA *paf_next;

				act("You harm $p's edge! Oops.",
				    ch, weapon, NULL, TO_CHAR);
				/* remove all affects */
				for (paf = weapon->affected; paf != NULL; paf = paf_next) {
					paf_next = paf->next;
					aff_free(paf);
				}
				weapon->affected = NULL;
				return;
			} else {
				act("You did not harm your blade. Wooh...",
				    ch, NULL, NULL, TO_CHAR);
				return;
			}
		} else {
			act("You broke your $p! Oops.",
			    ch, weapon, NULL, TO_CHAR);
			extract_obj(weapon, 0);
			return;
		}
	}
}

static void
give_coins(CHAR_DATA *ch, CHAR_DATA *victim, int silver, int gold)
{
	char buf[MAX_INPUT_LENGTH];

	if (silver > 0) {
		snprintf(buf, sizeof(buf), "%d silver '%s'",	// notrans
			 silver, victim->name);
		dofun("give", ch, buf);
	}
	if (gold > 0) {
		snprintf(buf, sizeof(buf), "%d gold '%s'",	// notrans
			 gold, victim->name);
		dofun("give", ch, buf);
	}
}

static const char *
money_argument(const char *argument, char *arg, size_t len,
	       int *silver, int *gold)
{
	int amount;

	amount = atoi(arg);
	if (amount <= 0)
		return NULL;

	argument = one_argument(argument, arg, len);
	if (arg[0] == '\0')
		return NULL;

	if (!str_cmp(arg, "silver")
	||  !str_cmp(arg, "coins")
	||  !str_cmp(arg, "coin")) {
		*silver = amount;
		*gold = 0;
	} else if (!str_cmp(arg, "gold")) {
		*silver = 0;
		*gold = amount;
	} else
		return NULL;

	return argument;
}

static OBJ_DATA *
get_container_here(CHAR_DATA *ch, const char *name)
{
	OBJ_DATA *container;

	if ((container = get_obj_here(ch, name)) == NULL) {
		act_puts("I see no $T here.",
			 ch, NULL, name, TO_CHAR, POS_DEAD);
		return NULL;
	}
	switch (container->pObjIndex->item_type) {
	default:
		act_char("That is not a container.", ch);
		return NULL;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
		break;

	case ITEM_CORPSE_PC:
		if (!can_loot(ch, container)) {
			act_char("You can't do that.", ch);
			return NULL;
		}
	}
	if (IS_SET(INT(container->value[1]), CONT_CLOSED)) {
		act_puts("$p is closed.",
			 ch, container, NULL, TO_CHAR, POS_DEAD);
		return NULL;
	}

	return container;
}

static bool
check_money(CHAR_DATA *ch, OBJ_DATA *container, int silver, int gold)
{
	OBJ_DATA *obj;

	obj = container != NULL ? container->contains : ch->in_room->contents;
	for (; obj != NULL; obj = obj->next_content) {
		if (!can_see_obj(ch, obj)
		||  obj->pObjIndex->item_type != ITEM_MONEY)
			continue;

		silver -= INT(obj->value[0]);
		gold -= INT(obj->value[1]);
		if (silver <= 0 && gold <= 0)
			return TRUE;
	}

	return silver <= 0 && gold <= 0;
}

static void
get_money(CHAR_DATA *ch, OBJ_DATA *container, int silver, int gold)
{
	OBJ_DATA *obj, *obj_next;

	obj = container != NULL ? container->contains : ch->in_room->contents;
	for (; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;

		if (!can_see_obj(ch, obj)
		||  obj->pObjIndex->item_type != ITEM_MONEY)
			continue;

		INT(obj->value[0]) -= silver;
		if (INT(obj->value[0]) < 0) {
			silver = -INT(obj->value[0]);
			INT(obj->value[0]) = 0;
		} else
			silver = 0;
		INT(obj->value[1]) -= gold;
		if (INT(obj->value[1]) < 0) {
			gold = -INT(obj->value[1]);
			INT(obj->value[1]) = 0;
		} else
			gold = 0;
		if (INT(obj->value[0]) != 0 || INT(obj->value[1]) != 0) {
			OBJ_DATA *obj2;

			/*
			 * obj_to_obj and obj_to_room add obj2 to the head
			 * of the list. obj_next is already set so obj2
			 * will be skipped.
			 */
			obj2 = create_money(
			    INT(obj->value[1]), INT(obj->value[0]));
			if (container)
				obj_to_obj(obj2, container);
			else
				obj_to_room(obj2, ch->in_room);
		}
		extract_obj(obj, 0);
		if (silver == 0 && gold == 0)
			break;
	}
}
