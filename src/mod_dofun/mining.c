/*-
 * Copyright (c) 1998-2002 SoG Development Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id:
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

#define OBJ_VNUM_CHUNK_COAL		6517
#define OBJ_VNUM_PICK			6518
#define OBJ_VNUM_CHUNK_COPPER		6523
#define OBJ_VNUM_CHUNK_GOLD		6524
#define OBJ_VNUM_GEMS			6525
#define OBJ_VNUM_IRON_ORE		6526
#define OBJ_VNUM_COPPER_ORE		6527
#define OBJ_VNUM_GOLD_ORE		6528
#define OBJ_VNUM_PLIERS			6529
#define OBJ_VNUM_FORGE			6547
#define OBJ_VNUM_ANVIL			6548
#define OBJ_VNUM_DWARVEN_RING		6530
#define OBJ_VNUM_DWARVEN_SCEPTRE	6531
#define OBJ_VNUM_DWARVEN_BRACER		6532
#define OBJ_VNUM_DWARVEN_BOOTS		6533
#define OBJ_VNUM_DWARVEN_GAUNTLETS	6534
#define OBJ_VNUM_DWARVEN_LEGPLATES	6535
#define OBJ_VNUM_DWARVEN_ARMPLATES	6536
#define OBJ_VNUM_DWARVEN_COLLAR		6537
#define OBJ_VNUM_DWARVEN_CHAINMAIL	6538
#define OBJ_VNUM_DWARVEN_HELMET		6539
#define OBJ_VNUM_DWARVEN_DAGGER		6549
#define OBJ_VNUM_DWARVEN_SWORD		6540
#define OBJ_VNUM_DWARVEN_CLAYMORE	6541
#define OBJ_VNUM_DWARVEN_AXE		6542
#define OBJ_VNUM_DWARVEN_FLAIL		6543
#define OBJ_VNUM_DWARVEN_MACE		6544
#define OBJ_VNUM_DWARVEN_HAULBERD	6545
#define OBJ_VNUM_DWARVEN_SPEAR		6546

void do_mine(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *ore;
	OBJ_DATA *pick;
	AFFECT_DATA af;
	OBJ_INDEX_DATA *index;
	int count, mana, wait, number;
	char arg[MAX_INPUT_LENGTH];
	int chance;
	int color_chance = 100;
	int vnum = -1;
	int sn;
	int color = -1;
	int ore_chance;
	int chance_mod = 1;
	int carry_w, carry_n;

	if (IS_NPC(ch))
		return;

	if ((sn = sn_lookup("mining")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You don't know mining.\n", ch);
		return;
	}

	if ((pick = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		char_puts("You are not holding a pick.\n",ch);
		return;
	}

	if (pick->pObjIndex->vnum != OBJ_VNUM_PICK) {
		char_puts("You should have mining pick to do this.\n",ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_MOUNTAIN
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("You couldn't find rocky yields here.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	wait = SKILL(sn)->beats;

	ore_chance = number_range(1, 10000);

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (ore_chance < 3000)
			vnum = OBJ_VNUM_COPPER_ORE;
		else if (ore_chance < 8000)
			vnum = OBJ_VNUM_CHUNK_COAL;
		else if (ore_chance < 9950)
			vnum = OBJ_VNUM_IRON_ORE;
		else if (ore_chance < 9999)
			vnum = OBJ_VNUM_GOLD_ORE;
		else
			vnum = OBJ_VNUM_GEMS;
	} else if (!str_prefix(arg, "gold")) {
		color = sn_lookup("gold mining");
		vnum = OBJ_VNUM_GOLD_ORE;
		chance_mod = 10;
	} else if (!str_prefix(arg, "iron")) {
		color = sn_lookup("iron mining");
		vnum = OBJ_VNUM_IRON_ORE;
		chance_mod = 3;
	} else if (!str_prefix(arg, "coal")) {
		color = sn_lookup("coal mining");
		vnum = OBJ_VNUM_CHUNK_COAL;
	} else if (!str_prefix(arg, "copper")) {
		color = sn_lookup("copper mining");
		vnum = OBJ_VNUM_COPPER_ORE;
		chance_mod = 2;
	} else if (!str_prefix(arg, "gems")) {
		color = sn_lookup("precious mining");
		vnum = OBJ_VNUM_GEMS;
		chance_mod = 100;
	}

	if (vnum < 0) {
		char_puts("You don't know exactly how to mine "
			  "that kind of ore.\n", ch);
		return;
	}

	if (color > 0) {
		color_chance = get_skill(ch, color) / number_range(1, chance_mod);
		mana += SKILL(color)->min_mana;
		wait += SKILL(color)->beats;
	}

	if (ch->mana < mana) {
		char_puts("You don't have enough energy "
			  "to mine this.\n", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, wait);

	char_puts("You bit the rock with your pick.\n",ch);
	act("$n bits the rock with the pick.", ch, NULL, NULL, TO_ROOM);

	number = 1 + number_range(LEVEL(ch) / 30, LEVEL(ch) / 15);
	for (count = 0; count < number; count++) {
		if (number_percent() > chance * color_chance / 100) {
			char_puts("You failed to find an ore lode.\n", ch);
			check_improve(ch, sn, FALSE, 3);
			if (color > 0)
				check_improve(ch, color, FALSE, 3);
			continue;
		}

		check_improve(ch, sn, TRUE, 3);

		if (color > 0)
			check_improve(ch, color, TRUE, 3);

		index = get_obj_index(vnum);

		if (index == NULL) {
			char_puts("Something wrong. Report it to immortals.\n",
				  ch);
			log("[*****] BUG: Null object. Vnum %d.", vnum);
			return;
		}

		ore = create_obj(index, 0);

		ore->level = ore->level * ch->level;
		ore->cost = ore->cost * ch->level;

		if (vnum == OBJ_VNUM_GEMS) {
			af.where	 = TO_OBJECT;
			af.type		 = sn;
			af.level	 = ch->level;
			af.duration	 = -1;
			af.location	 = APPLY_MANA;
			af.modifier	 = 2 * ch->level;
			af.bitvector	 = 0;
			affect_to_obj(ore, &af);
		}

		if (((carry_n = can_carry_n(ch)) >= 0 &&
	      	      ch->carry_number + get_obj_number(ore) > carry_n)
		||  ((carry_w = can_carry_w(ch)) >= 0 &&
	             get_carry_weight(ch) + get_obj_weight(ore) > carry_w))
			obj_to_room(ore, ch->in_room);
		else
			obj_to_char(ore, ch);

		act_puts("You have find $p.",
			 ch, ore, NULL, TO_CHAR, POS_DEAD);
	}
}

void do_smelt(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *ore, *obj, *bar;
	OBJ_INDEX_DATA *index;
	int mana, wait;
	char arg[MAX_INPUT_LENGTH];
	int chance;
	int vnum = -1;
	int sn;
	int carry_w, carry_n;

	if (IS_NPC(ch))
		return;

	if ((sn = sn_lookup("smelting")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You don't know smelting.\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if ((ore = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n", ch);
		return;
	}

	if ((ore->pObjIndex->vnum != OBJ_VNUM_IRON_ORE)
	&&  (ore->pObjIndex->vnum != OBJ_VNUM_COPPER_ORE)
	&&  (ore->pObjIndex->vnum != OBJ_VNUM_GOLD_ORE)) {
		char_puts("Even you are not so silly to smelt this.\n", ch);
		return;
	}

	for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
		if (obj->pObjIndex->vnum == OBJ_VNUM_FORGE)
			break;
	}

	if (obj == NULL) {
		char_puts("You lack the forge.\n", ch);
		return;
	}

	obj = NULL;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->pObjIndex->vnum == OBJ_VNUM_CHUNK_COAL)
			break;
	}

	if (obj == NULL) {
		char_puts("You need coal to work.\n", ch);
		return;
	}

	if (!str_prefix(arg, "gold"))
		vnum = OBJ_VNUM_CHUNK_GOLD;
	else if (!str_prefix(arg, "iron"))
		vnum = OBJ_VNUM_CHUNK_IRON;
	else if (!str_prefix(arg, "copper"))
		vnum = OBJ_VNUM_CHUNK_COPPER;

	if (vnum == -1) {
		char_puts("Just put your bars in a bag.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	wait = SKILL(sn)->beats;


	if (ch->mana < mana) {
		char_puts("You don't have enough energy "
			  "to work in the smithy.\n", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, wait);

	char_puts("You try recover pure metal from a piece of ore.\n",ch);
	act("$n tries to recover pure metal from a piece of ore.",
	    ch, NULL, NULL, TO_ROOM);

		if (number_percent() > chance) {
			char_puts("You failed to recover this. "
				  "Try again later.\n", ch);
			check_improve(ch, sn, FALSE, 3);
			return;
		}

		check_improve(ch, sn, TRUE, 3);

		index = get_obj_index(vnum);
		if (index == NULL) {
			char_puts("Something wrong. Report it to immortals.\n",
				  ch);
			log("[*****] BUG: Null object. Vnum %d.", vnum);
			return;
		}

		bar = create_obj(index, 0);
		bar->level = bar->level * ch->level;
		bar->cost = bar->cost * ch->level;

		obj_from_char(ore);
		obj_from_char(obj);

		if (((carry_n = can_carry_n(ch)) >= 0 &&
	      	      ch->carry_number + get_obj_number(bar) > carry_n)
		||  ((carry_w = can_carry_w(ch)) >= 0 &&
	             get_carry_weight(ch) + get_obj_weight(bar) > carry_w))
			obj_to_room(bar, ch->in_room);
		else
			obj_to_char(bar, ch);

		act_puts("Now you have $p.",
			 ch, bar, NULL, TO_CHAR, POS_DEAD);
}

void do_forge(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *item, *obj, *bar, *hammer, *pliers, *forge;
	OBJ_INDEX_DATA *index;
	int mana, wait;
	char arg[MAX_INPUT_LENGTH];
	int chance;
	int vnum = -1;
	int sn;
	char *material;
	int carry_w, carry_n;

	if (IS_NPC(ch))
		return;

	if ((sn = sn_lookup("smithery")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You are not a blacksmith!\n", ch);
		return;
	}


	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {

/* Establishing the forge */

		if (ch->in_room->sector_type != SECT_MOUNTAIN
	        &&  ch->in_room->sector_type != SECT_HILLS) {
			char_puts("You can not find enough rocks to set forge here.\n", ch);
			return;
		}

		for (obj = ch->in_room->contents; obj; obj= obj->next_content) {
			if (obj->pObjIndex->vnum == OBJ_VNUM_FORGE)
				break;
		}

		if (obj != NULL) {
			char_puts("There IS already a forge here!\n", ch);
			return;
		}

		if ((sn = sn_lookup("forge")) < 0
		||  (chance = get_skill(ch, sn)) == 0) {
			char_puts("You don't know how to make a forge.\n", ch);
			return;
		} else {
			vnum = OBJ_VNUM_FORGE;
			mana = SKILL(sn)->min_mana;
			wait = SKILL(sn)->beats;

			if (ch->mana < mana) {
				char_puts("You don't have enough energy "
					  "to do this.\n", ch);
				return;
			}

			ch->mana -= mana;
			WAIT_STATE(ch, wait);

			char_puts("You try to heat the forge.\n",ch);
			act("$n tries to heat the forge.",
			    ch, NULL, NULL, TO_ROOM);

			if (number_percent() > chance) {
				char_puts("You failed to heat the forge. "
					  "Try again later.\n", ch);
				check_improve(ch, sn, FALSE, 3);
				return;
			}

			check_improve(ch, sn, TRUE, 3);

			index = get_obj_index(vnum);

			if (index == NULL) {
				char_puts("Something wrong. Report it to "
					  "immortals.\n", ch);
				log("[*****] BUG: Null object. Vnum %d.", vnum);
				return;
			}

			forge = create_obj(index, 0);
			obj_to_room(forge, ch->in_room);
			act_puts("You have successfully established $p.",
				 ch, forge, NULL, TO_CHAR, POS_DEAD);
			return;
		}
	}

	if ((hammer = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		char_puts("You are not holding a hammer.\n",ch);
		return;
	}

	if (hammer->pObjIndex->vnum != OBJ_VNUM_HAMMER) {
		char_puts("That is not a right hammer.\n",ch);
		return;
	}

	if ((pliers = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		char_puts("You do not have pliers in your hand.\n",ch);
		return;
	}

	if (pliers->pObjIndex->vnum != OBJ_VNUM_PLIERS) {
		char_puts("You should have forge pliers to forge.\n",ch);
		return;
	}


	if (!str_prefix(arg, "boots"))
		vnum = OBJ_VNUM_DWARVEN_BOOTS;
	else if (!str_prefix(arg, "legplates"))
		vnum = OBJ_VNUM_DWARVEN_LEGPLATES;
	else if (!str_prefix(arg, "chainmail"))
		vnum = OBJ_VNUM_DWARVEN_CHAINMAIL;
	else if (!str_prefix(arg, "armplates"))
		vnum = OBJ_VNUM_DWARVEN_ARMPLATES;
	else if (!str_prefix(arg, "gauntlets"))
		vnum = OBJ_VNUM_DWARVEN_GAUNTLETS;
	else if (!str_prefix(arg, "bracer"))
		vnum = OBJ_VNUM_DWARVEN_BRACER;
	else if (!str_prefix(arg, "collar"))
		vnum = OBJ_VNUM_DWARVEN_COLLAR;
	else if (!str_prefix(arg, "helmet"))
		vnum = OBJ_VNUM_DWARVEN_HELMET;
	else if (!str_prefix(arg, "ring"))
		vnum = OBJ_VNUM_DWARVEN_RING;
	else if (!str_prefix(arg, "sceptre"))
		vnum = OBJ_VNUM_DWARVEN_SCEPTRE;
	else if (!str_prefix(arg, "dagger"))
		vnum = OBJ_VNUM_DWARVEN_DAGGER;
	else if (!str_prefix(arg, "sword"))
		vnum = OBJ_VNUM_DWARVEN_SWORD;
	else if (!str_prefix(arg, "claymore"))
		vnum = OBJ_VNUM_DWARVEN_CLAYMORE;
	else if (!str_prefix(arg, "axe"))
		vnum = OBJ_VNUM_DWARVEN_AXE;
	else if (!str_prefix(arg, "flail"))
		vnum = OBJ_VNUM_DWARVEN_FLAIL;
	else if (!str_prefix(arg, "mace"))
		vnum = OBJ_VNUM_DWARVEN_MACE;
	else if (!str_prefix(arg, "haulberd"))
		vnum = OBJ_VNUM_DWARVEN_HAULBERD;
	else if (!str_prefix(arg, "spear"))
		vnum = OBJ_VNUM_DWARVEN_SPEAR;
	else if (!str_prefix(arg, "exotic"))
		vnum = OBJ_VNUM_PLIERS;
	else {
		char_puts("You cannot forge this.\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if ((bar = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have a bar of that metal.\n", ch);
		return;
	}

	if ((bar->pObjIndex->vnum != OBJ_VNUM_CHUNK_IRON)
	&&  (bar->pObjIndex->vnum != OBJ_VNUM_CHUNK_COPPER)
	&&  (bar->pObjIndex->vnum != OBJ_VNUM_CHUNK_GOLD)) {
		char_puts("Even you are not so silly to do this.\n", ch);
		return;
	}

	if ((bar->pObjIndex->vnum == OBJ_VNUM_IRON_ORE)
	&&  (bar->pObjIndex->vnum == OBJ_VNUM_COPPER_ORE)
	&&  (bar->pObjIndex->vnum == OBJ_VNUM_GOLD_ORE)) {
		char_puts("You must first SMELT this piece of ore.\n", ch);
		return;
	}


/* Looking for the forge in the room */
	for (obj = ch->in_room->contents; obj; obj= obj->next_content) {
		if (obj->pObjIndex->vnum == OBJ_VNUM_FORGE)
			break;
	}

	if (obj == NULL) {
		char_puts("You lack the forge.\n", ch);
		return;
	}

/* Looking for the anvil in the room */

	obj = NULL;

	for (obj = ch->in_room->contents; obj; obj= obj->next_content) {
		if (obj->pObjIndex->vnum == OBJ_VNUM_ANVIL)
			break;
	}

	if (obj == NULL) {
		char_puts("You need an anvil to forge!\n", ch);
		return;
	}

/* Looking for the coal in the inventory */

	obj = NULL;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->pObjIndex->vnum == OBJ_VNUM_CHUNK_COAL)
			break;
	}

	if (obj == NULL) {
		char_puts("You need coal to work.\n", ch);
		return;
	}

	if (!str_prefix(arg, "gold"))
		material = "gold";

	else if (!str_prefix(arg, "iron")) {
		material = "iron";
		if (vnum == OBJ_VNUM_DWARVEN_RING
		||  vnum == OBJ_VNUM_DWARVEN_SCEPTRE
		||  vnum == OBJ_VNUM_DWARVEN_BRACER){
			char_puts("You can only make rings, bracers and "
				  "sceptres of gold.\n", ch);
			return;
		}
	} else if (!str_prefix(arg, "copper")) {
		material = "copper";
		if (vnum == OBJ_VNUM_DWARVEN_RING
		||  vnum == OBJ_VNUM_DWARVEN_SCEPTRE
		||  vnum == OBJ_VNUM_DWARVEN_BRACER){
			char_puts("You can only make rings, bracers and "
				  "sceptres of gold.\n", ch);
			return;
		}
	} else {
		char_puts("You cannot make something of it.\n", ch);
		return;
	}


	mana = SKILL(sn)->min_mana;
	wait = SKILL(sn)->beats;


	if (ch->mana < mana) {
		char_puts("You don't have enough energy "
			  "to work in the smithy.\n", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, wait);

	char_puts("You begin to work on the anvil with your hammer.\n",ch);
	act("$n begins to work on the anvil with the hammer.",
	    ch, NULL, NULL, TO_ROOM);

	if (number_percent() > chance) {
		char_puts("You failed to make it. Try again later.\n", ch);
		check_improve(ch, sn, FALSE, 3);
		return;
	}

	check_improve(ch, sn, TRUE, 3);

	index = get_obj_index(vnum);
	if (index == NULL) {
		char_puts("Something wrong. Report it to immortals\n", ch);
		log("[*****] BUG: Null object. Vnum %d.", vnum);
		return;
	}

	item = create_obj(index, 0);
	item->level = ch->level;
	item->cost = item->cost * ch->level;
	item->material = material;

	if (item->pObjIndex->item_type == ITEM_WEAPON){
		item->value[1] = 3 + ch->level / 10;
		item->value[2] = 4 + ch->level / 15;
		if (IS_WEAPON_STAT(item, WEAPON_TWO_HANDS))
			item->value[2] = item->value[2] * 4 / 3;
	}

	if (item->pObjIndex->item_type == ITEM_ARMOR){
		item->value[0] = item->value[0] * ch->level / 5;
		item->value[1] = item->value[1] * ch->level / 6;
		item->value[2] = item->value[2] * ch->level / 5;
		item->value[3] = item->value[3] * ch->level / 9;
	}

	obj_from_char(bar);
	obj_from_char(obj);

	if (((carry_n = can_carry_n(ch)) >= 0 &&
      	      ch->carry_number + get_obj_number(item) > carry_n)
	||  ((carry_w = can_carry_w(ch)) >= 0 &&
             get_carry_weight(ch) + get_obj_weight(item) > carry_w))
		obj_to_room(item, ch->in_room);
	else
		obj_to_char(item, ch);

	act_puts("You have successfully made $p.",
		 ch, item, NULL, TO_CHAR, POS_DEAD);
}

void do_anvil(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *anvil;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *index;
	int mana, wait;
	int chance;
	int sn;

	if (IS_NPC(ch))
		return;

	if ((sn = sn_lookup("anvil")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You do not know how to make an anvil.\n", ch);
		return;
	}

/* Make the anvil */

	for (obj = ch->in_room->contents; obj; obj= obj->next_content) {
		if (obj->pObjIndex->vnum == OBJ_VNUM_ANVIL)
			break;
	}

	if (obj != NULL) {
		char_puts("There IS already an anvil here!\n", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_MOUNTAIN
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("You can not find big enough stone to"
			  " make an anvil.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	wait = SKILL(sn)->beats;

	if (ch->mana < mana) {
		char_puts("You don't have enough energy to do this.\n", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, wait);

	char_puts("You cut the big stone to make an anvil.\n",ch);
	act("$n tries to make an anvil from the big stone.", ch, NULL, NULL, TO_ROOM);

	if (number_percent() > chance) {
		char_puts("You failed to make an anvil. Try again later.\n", ch);
		check_improve(ch, sn, FALSE, 3);
		return;
	}

	check_improve(ch, sn, TRUE, 3);

	index = get_obj_index(OBJ_VNUM_ANVIL);
	if (index == NULL) {
		char_puts("Something wrong. Report it to immortals.\n", ch);
		log("[*****] BUG: Null object. Vnum %d.", OBJ_VNUM_ANVIL);
		return;
	}

	anvil = create_obj(index, 0);
	obj_to_room(anvil, ch->in_room);
	act_puts("You have made $p.", ch, anvil, NULL, TO_CHAR, POS_DEAD);
}
