/*
 * $Id: repair.c,v 1.4 1998-07-23 01:25:08 efdi Exp $
 */

#include <stdio.h>
#include "merc.h"
#include "db.h"
#include "comm.h"
#include "util.h"
#include "interp.h"
#include "mlstring.h"

void damage_to_obj(CHAR_DATA *ch,OBJ_DATA *wield, OBJ_DATA *worn, int damage) 
{

 	if (damage == 0) return;

 	worn->condition -= damage;

	act_puts("{gThe $p inflicts damage on {r$P{g.{x",
		ch,wield,worn,TO_ROOM,POS_RESTING);

	if (worn->condition < 1) {
		act_puts("{gThe {r$P{g breaks into pieces.{x",
			ch,wield,worn,TO_ROOM,POS_RESTING);
		extract_obj(worn);
		return;
	}
 
	if (IS_SET(wield->extra_flags,ITEM_ANTI_EVIL) 
	&&  IS_SET(wield->extra_flags,ITEM_ANTI_NEUTRAL)
	&&  IS_SET(worn->extra_flags,ITEM_ANTI_EVIL) 
	&&  IS_SET(worn->extra_flags,ITEM_ANTI_NEUTRAL)) {
		act_puts("$p doesn't want to fight against $P.", ch, wield, worn, TO_ROOM, POS_RESTING);
		act_puts("$p removes itself from you!", ch, wield, worn, TO_CHAR, POS_RESTING);
		act_puts("$p removes itself from $n.", ch, wield, worn, TO_ROOM, POS_RESTING);
		unequip_char(ch, wield);
		return;
 	}

	if (IS_SET(wield->extra_flags,ITEM_ANTI_EVIL) 
	&&  IS_SET(worn->extra_flags,ITEM_ANTI_EVIL)) {
		act_puts("The $p worries for the damage to $P.", ch, wield, worn, TO_ROOM, POS_RESTING);
		return;
	}
}


void check_weapon_destroy(CHAR_DATA *ch, CHAR_DATA *victim,bool second)
{
 OBJ_DATA *wield,*destroy;
 int skill,chance=0,sn,i;

 if (IS_NPC(victim) || number_percent() < 94)  return;

 if (!second)
	{
	 if ((wield = get_eq_char(ch, WEAR_WIELD)) == NULL)
	 return;
 	 sn = get_weapon_sn(ch, WEAR_WIELD);
 	 skill = get_skill(ch, sn);
	}
  else  {
	 if ((wield = get_eq_char(ch,WEAR_SECOND_WIELD)) == NULL)
	 return;
 	 sn = get_weapon_sn(ch, WEAR_SECOND_WIELD);
 	 skill = get_skill(ch, sn);
	}
 if (is_metal(wield))
	  {
	for (i=0;i < MAX_WEAR; i++) 
	{
	 if ((destroy = get_eq_char(victim,i)) == NULL 
		|| number_percent() > 95
		|| number_percent() > 94
	 	|| number_percent() > skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1
		|| (i == WEAR_WIELD || i== WEAR_SECOND_WIELD 
			|| i == WEAR_TATTOO || i == WEAR_STUCK_IN))
	 continue;
	
	 chance += 20;
	 if (check_material(wield, "platinium") ||
	      check_material(wield, "titanium"))
	 chance += 5;

	 if (is_metal(destroy))  chance -= 20;
	 else 			chance += 20; 

	 chance += ((ch->level - victim->level) / 5);

	 chance += ((wield->level - destroy->level) / 2);

	/* sharpness	*/
	 if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
		chance += 10;

	 if (sn == gsn_axe) chance += 10;
	/* spell affects */
	 if (IS_OBJ_STAT(destroy, ITEM_BLESS)) chance -= 10;
	 if (IS_OBJ_STAT(destroy, ITEM_MAGIC)) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat(ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < chance && chance > 50)
		{
		 damage_to_obj(ch,wield,destroy, (chance / 5));
		 break;
		}
	}
	  }
 else {
	for (i=0;i < MAX_WEAR;i++) 
	{
	 if ((destroy = get_eq_char(victim,i)) == NULL 
		|| number_percent() > 95
		|| number_percent() > 94
	 	|| number_percent() < skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1
		|| (i == WEAR_WIELD || i== WEAR_SECOND_WIELD 
			|| i == WEAR_TATTOO || i == WEAR_STUCK_IN))
	 continue;
	
	 chance += 10;

	 if (is_metal(destroy))  chance -= 20;

	 chance += (ch->level - victim->level);

	 chance += (wield->level - destroy->level);

	/* sharpness	*/
	 if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
		chance += 10;

	 if (sn == gsn_axe) chance += 10;

	/* spell affects */
	 if (IS_OBJ_STAT(destroy, ITEM_BLESS)) chance -= 10;
	 if (IS_OBJ_STAT(destroy, ITEM_MAGIC)) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat(ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < chance && chance > 50)
		{
		 damage_to_obj(ch,wield,destroy, chance / 5);
		 break;
		}
	}
	  }

 return;
}


void do_repair(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int cost;

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
	{
	    if (!IS_NPC(mob)) continue;
	if (mob->spec_fun == spec_lookup("spec_repairman"))
	        break;
	}
 
	if (mob == NULL)
	{
	    send_to_char("You can't do that here.\n\r", ch);
	    return;
	}

	one_argument(argument,arg);

	if (arg[0] == '\0') {
	do_say(mob,"I will repair a weapon for you, for a price.");
	send_to_char("Type estimate <weapon> to be assessed for damage.\n\r",ch);
	return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL)
	{
	do_say(mob,"You don't have that item");
	return;
	}

	if (obj->pIndexData->vnum == OBJ_VNUM_HAMMER)
	{
	 do_say(mob,"That hammer is beyond my power.");
	 return;
	}

	if (obj->condition >= 100) {
	do_say(mob,"But that item is not broken.");
	    return;
	}

	if (obj->cost == 0) {
		/* XXX */
		doprintf(do_say, mob, "%s is beyond repair.\n\r",
			 mlstr_mval(obj->short_descr));
   		return;
	}

	cost = ((obj->level * 10) +
		((obj->cost * (100 - obj->condition)) /100)   );
	cost /= 100;

	if (cost > ch->gold) {
		do_say(mob,"You do not have enough gold for my services.");
		return;
	}

	WAIT_STATE(ch,PULSE_VIOLENCE);

	ch->gold -= cost;
	mob->gold += cost;
	act_puts("$n takes $p from $N, repairs it, and returns it to $N",
		 mob, obj, ch, TO_ROOM, POS_RESTING);
	obj->condition = 100;
}

void do_estimate(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *mob; 
	char arg[MAX_INPUT_LENGTH];
	int cost;
	
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
	{
	    if (!IS_NPC(mob)) continue;
	if (mob->spec_fun == spec_lookup("spec_repairman"))
	        break;
	}
 
	if (mob == NULL)
	{
	    send_to_char("You can't do that here.\n\r", ch);
	    return;
	}
	
	one_argument(argument, arg);
	
	if (arg[0] == '\0')
	{
	do_say(mob,"Try estimate <item>");
   	return; 
	} 
	if ((obj = (get_obj_carry(ch, arg))) == NULL)
	{
	do_say(mob,"You don't have that item");
	return;
	}
	if (obj->pIndexData->vnum == OBJ_VNUM_HAMMER)
	{
	    do_say(mob,"That hammer is beyond my power.");
	    return;
	}
	if (obj->condition >= 100)
	{
	do_say(mob,"But that item's not broken");
	return;
	}
	if (obj->cost == 0)
	{
	do_say(mob,"That item is beyond repair");
		return;
	} 
	
	cost = ((obj->level * 10) +
		((obj->cost * (100 - obj->condition)) /100)   );
	cost /= 100;

	doprintf(do_say, mob, "It will cost %d to fix that item", cost);
}

void do_restring(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *mob;
#if 0
	char arg  [MAX_INPUT_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int cost = 2000;
#endif

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
	{
	    if (IS_NPC(mob) && IS_SET(mob->act, ACT_HEALER))
	        break;
	}
 
#if 0
	if (mob == NULL) {
#endif
	    send_to_char("You can't do that here.\n\r", ch);
	    return;
#if 0
	/* XXX */
	}

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg1);
	strcpy(arg2, argument);
	smash_tilde(arg2);

	if (arg[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax:\n\r",ch);
		send_to_char("  restring <obj> <field> <string>\n\r",ch);
		send_to_char("    fields: name short long\n\r",ch);
		return;
	}

	if ((obj = (get_obj_carry(ch, arg))) == NULL) {
		do_say(mob, "You don't have that item.");
		return;
	}

	cost += (obj->level * 1500);

	if (cost > ch->gold) {
		act("$N says 'You do not have enough gold for my services.'",
		  ch,NULL,mob,TO_CHAR);
		return;
	}
	
	if (!str_prefix(arg1, "name")) {
		free_string(obj->name);
		obj->name = str_dup(arg2);
	}
	else
	if (!str_prefix(arg1, "short")) {
		free_string(obj->short_descr);
	    obj->short_descr = str_dup(arg2);
	}
	else
	if (!str_prefix(arg1, "long")) {
		free_string(obj->description);
		obj->description = str_dup(arg2);
	}
	else {
		send_to_char("That's not a valid Field.\n\r",ch);
		return;
	}
	
	WAIT_STATE(ch,PULSE_VIOLENCE);

	ch->gold -= cost;
	mob->gold += cost;
	act_printf(ch, NULL, mob, TO_ROOM, POS_RESTING,
		  "$N takes $n's item, tinkers with it, and returns it to $n.");
	char_printf(ch, "%s takes your item, tinkers with it, and returns %s to you.\n\r", mob->short_descr, obj->short_descr);
	send_to_char("Remember, if we find your new string offensive, we will not be happy.\n\r", ch);
	send_to_char(" This is your ONE AND ONLY Warning.\n\r", ch);
#endif
}

void check_shield_destroyed(CHAR_DATA *ch, CHAR_DATA *victim,bool second)
{
 OBJ_DATA *wield,*destroy;
 int skill,chance=0,sn;

 if (IS_NPC(victim) || number_percent() < 94)  return;

 if (!second)
	{
	 if ((wield = get_eq_char(ch,WEAR_WIELD)) == NULL)
	 return;
 	 sn = get_weapon_sn(ch, WEAR_WIELD);
 	 skill = get_skill(ch, sn);
	}
  else  {
	 if ((wield = get_eq_char(ch,WEAR_SECOND_WIELD)) == NULL)
	 return;
 	 sn = get_weapon_sn(ch, WEAR_SECOND_WIELD);
 	 skill = get_skill(ch, sn);
	}

 destroy = get_eq_char(victim,WEAR_SHIELD);
 if (destroy == NULL) return; 

 if (is_metal(wield))
	  {
	 if (  number_percent() > 94
	 	|| number_percent() > skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1)
	 return;
	
	 chance += 20;
	 if (check_material(wield, "platinium") ||
	      check_material(wield, "titanium"))
	 chance += 5;

	 if (is_metal(destroy))  chance -= 20;
	 else 			chance += 20; 

	 chance += ((ch->level - victim->level) / 5);

	 chance += ((wield->level - destroy->level) / 2);

	/* sharpness	*/
	 if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
		chance += 10;

	 if (sn == gsn_axe) chance += 10;
	/* spell affects */
	 if (IS_OBJ_STAT(destroy, ITEM_BLESS)) chance -= 10;
	 if (IS_OBJ_STAT(destroy, ITEM_MAGIC)) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat(ch, STAT_STR);

/* 	 chance /= 2;	*/
	 if (number_percent() < chance && chance > 20)
		{
		 damage_to_obj(ch,wield,destroy, (chance / 4));
		 return;
		}
	  }
 else {
	 if (  number_percent() > 94
	 	|| number_percent() < skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1)
	 return;
	
	 chance += 10;

	 if (is_metal(destroy))  chance -= 20;

	 chance += (ch->level - victim->level);

	 chance += (wield->level - destroy->level);

	/* sharpness	*/
	 if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
		chance += 10;

	 if (sn == gsn_axe) chance += 10;

	/* spell affects */
	 if (IS_OBJ_STAT(destroy, ITEM_BLESS)) chance -= 10;
	 if (IS_OBJ_STAT(destroy, ITEM_MAGIC)) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat(ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < chance && chance > 20)
		{
		 damage_to_obj(ch,wield,destroy, (chance / 4));
		 return;
		}
	  }
 return;
}

void check_weapon_destroyed(CHAR_DATA *ch, CHAR_DATA *victim,bool second)
{
 OBJ_DATA *wield,*destroy;
 int skill,chance=0,sn;

 if (IS_NPC(victim) || number_percent() < 94)  return;

 if (!second)
	{
	 if ((wield = get_eq_char(ch,WEAR_WIELD)) == NULL)
	 return;
 	 sn = get_weapon_sn(ch, WEAR_WIELD);
 	 skill = get_skill(ch, sn);
	}
  else  {
	 if ((wield = get_eq_char(ch,WEAR_SECOND_WIELD)) == NULL)
	 return;
 	 sn = get_weapon_sn(ch, WEAR_SECOND_WIELD);
 	 skill = get_skill(ch, sn);
	}

  destroy = get_eq_char(victim,WEAR_WIELD);
  if (destroy == NULL) return;

 if (is_metal(wield))
	  {
	 if (  number_percent() > 94
	 	|| number_percent() > skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1)
	 return;
	
	 chance += 20;
	 if (check_material(wield, "platinium") ||
	      check_material(wield, "titanium"))
	 chance += 5;

	 if (is_metal(destroy))  chance -= 20;
	 else 			chance += 20; 

	 chance += ((ch->level - victim->level) / 5);

	 chance += ((wield->level - destroy->level) / 2);

	/* sharpness	*/
	 if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
		chance += 10;

	 if (sn == gsn_axe) chance += 10;
	/* spell affects */
	 if (IS_OBJ_STAT(destroy, ITEM_BLESS)) chance -= 10;
	 if (IS_OBJ_STAT(destroy, ITEM_MAGIC)) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat(ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < (chance / 2) && chance > 20)
		{
		 damage_to_obj(ch,wield,destroy, (chance / 4));
		 return;
		}
	  }
 else {
	 if (  number_percent() > 94
	 	|| number_percent() < skill
		|| ch->level < (victim->level - 10) 
		|| check_material(destroy,"platinum") 
		|| destroy->pIndexData->limit != -1)
	 return;
	
	 chance += 10;

	 if (is_metal(destroy))  chance -= 20;

	 chance += (ch->level - victim->level);

	 chance += (wield->level - destroy->level);

	/* sharpness	*/
	 if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
		chance += 10;

	 if (sn == gsn_axe) chance += 10;

	/* spell affects */
	 if (IS_OBJ_STAT(destroy, ITEM_BLESS)) chance -= 10;
	 if (IS_OBJ_STAT(destroy, ITEM_MAGIC)) chance -= 20;
	 
	 chance += skill - 85 ;
	 chance += get_curr_stat(ch, STAT_STR);

/*	 chance /= 2;	*/
	 if (number_percent() < (chance / 2) && chance > 20)
		{
		 damage_to_obj(ch,wield,destroy, chance / 4);
		 return;
		}
	  }
 return;
}


void do_smithing(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *hammer;

	if (IS_NPC(ch)
	||   ch->level < skill_table[gsn_smithing].skill_level[ch->class])
	{
	send_to_char("Huh?\n\r", ch);
	return;
	}


	if (ch->fighting)
	{
	    send_to_char("Wait until the fight finishes.\n\r", ch);
	    return;
	}

	one_argument(argument,arg);

	if (arg[0] == '\0')
	{
	send_to_char("Which object do you want to repair.\n\r",ch);
	return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL)
	{
	send_to_char("You are not carrying that.\n\r",ch);
	return;
	}

   if (obj->condition >= 100)
	{
	send_to_char("But that item is not broken.\n\r",ch);
	return;
	}

	if ((hammer = get_eq_char(ch, WEAR_HOLD)) == NULL)
	{
	send_to_char("You are not holding a hammer.\n\r",ch);
	return;
	}

	if (hammer->pIndexData->vnum != OBJ_VNUM_HAMMER)
	{
	send_to_char("That is not the correct hammer.\n\r",ch);
	return;
	}

	WAIT_STATE(ch,2 * PULSE_VIOLENCE);
	if (number_percent() > get_skill(ch,gsn_smithing)) {
		check_improve(ch, gsn_smithing, FALSE, 8);
		act_puts("$n tries to repair $p with the hammer but fails.",
			 ch, obj, NULL, TO_ROOM, POS_RESTING);
		act_puts("You failed to repair $p.",
			 ch, obj, NULL, TO_CHAR, POS_RESTING);
		hammer->condition -= 25;
	}
	else {
		check_improve(ch, gsn_smithing, TRUE, 4);
		act_puts("$n repairs $p with the hammer.",
			 ch, obj, NULL, TO_ROOM, POS_RESTING);
		act_puts("You repair $p.",
			 ch, obj, NULL, TO_CHAR, POS_RESTING);
		obj->condition = UMAX(100, obj->condition +
					   (get_skill(ch,gsn_smithing) / 2));
		hammer->condition -= 25;
	}

	if (hammer->condition < 1)
		extract_obj(hammer);
}

