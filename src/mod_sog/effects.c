/*
 * $Id: effects.c,v 1.36 2000-10-04 20:28:46 fjoe Exp $
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

#include "magic.h"
#include "update.h"

#include "module.h"
#include "effects.h"

void
acid_effect(void *vo, int level, int dam)
{
    if (mem_is(vo, MT_ROOM)) { /* nail objects on the floor */
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    acid_effect(obj,level,dam);
	}
	return;
    }

    if (mem_is(vo, MT_CHAR)) { /* do the effect on a victim */
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	
	/* let's toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    acid_effect(obj,level,dam);
	}
	return;
    }

    if (mem_is(vo, MT_OBJ)) { /* toast an object */
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
	||  OBJ_IS(obj, OBJ_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	 if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

	chance -= obj->level * 2;

	switch (obj->item_type) {
	    default:
		return;
	    case ITEM_CONTAINER:
	    case ITEM_CORPSE_PC:
	    case ITEM_CORPSE_NPC:
		msg = "$p fumes and dissolves.";
		break;
	    case ITEM_ARMOR:
		msg = "$p is pitted and etched.";
		break;
	    case ITEM_CLOTHING:
		msg = "$p is corroded into scrap.";
	 	break;
	    case ITEM_STAFF:
	    case ITEM_WAND:
		chance -= 10;
		msg = "$p corrodes and breaks.";
		break;
	    case ITEM_SCROLL:
		chance += 10;
		msg = "$p is burned into waste.";
		break; 
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,obj,NULL,TO_ALL);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL);

	if (obj->item_type == ITEM_ARMOR)  /* etch it */
	{
	    AFFECT_DATA *paf;
	    bool af_found = FALSE;
	    int i;

 	    affect_enchant(obj);

	    for ( paf = obj->affected; paf != NULL; paf = paf->next)
            {
		if (paf->where == TO_SKILLS
		||  paf->where == TO_RESIST
		||  paf->where == TO_FORMRESIST
		||  paf->where == TO_RACE)
			continue;

                if (INT(paf->location) == APPLY_AC)
                {
                    af_found = TRUE;
                    paf->type = NULL;
                    paf->modifier += 1;
                    paf->level = UMAX(paf->level,level);
		    break;
                }
            }
 
            if (!af_found)
            /* needs a new affect */
            {
		paf = aff_new();
 
                paf->type       = NULL;
                paf->level      = level;
                paf->duration   = -1;
                INT(paf->location) = APPLY_AC;
                paf->modifier   =  1;
                paf->bitvector  = 0;
                paf->next       = obj->affected;
                obj->affected   = paf;
            }
 
            if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
                for (i = 0; i < 4; i++)
                    obj->carried_by->armor[i] += 1;
            return;
	}

	/* get rid of the object */
	if (obj->contains)  /* dump contents */
	{
	    for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
	    {
		n_obj = t_obj->next_content;
		obj_from_obj(t_obj);
		if (obj->in_room != NULL)
		    obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)
		    obj_to_room(t_obj,obj->carried_by->in_room);
		else
		{
		    extract_obj(t_obj, 0);
		    continue;
		}

		acid_effect(t_obj,level/2,dam/2);
	    }
 	}

	extract_obj(obj, 0);
	return;
    }
}

void
cold_effect(void *vo, int level, int dam)
{
    if (mem_is(vo, MT_ROOM)) { /* nail objects on the floor */
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;
 
        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            cold_effect(obj,level,dam);
        }
        return;
    }

    if (mem_is(vo, MT_CHAR)) { /* whack a character */
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	
	/* chill touch effect */
	if (!saves_spell(level/4 + dam / 20, victim, DAM_COLD))
	{
	    AFFECT_DATA af;

            act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
	    act("A chill sinks deep into your bones.",victim,NULL,NULL,TO_CHAR);
            af.where     = TO_AFFECTS;
            af.type      = "chill touch";
            af.level     = level;
            af.duration  = 6;
            INT(af.location) = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = 0;
	    af.owner = NULL;
            affect_join( victim, &af );
	}

	/* hunger! (warmth sucked out */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_HUNGER,dam/20);

	/* let's toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    cold_effect(obj,level,dam);
	}
	return;
   }

   if (mem_is(vo, MT_OBJ)) { /* toast an object */
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
	||  OBJ_IS(obj, OBJ_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

 	chance -= obj->level * 2;

	switch(obj->item_type)
	{
	    default:
		return;
	    case ITEM_POTION:
		msg = "$p freezes and shatters!";
		chance += 25;
		break;
	    case ITEM_DRINK_CON:
		msg = "$p freezes and shatters!";
		chance += 5;
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,obj,NULL,TO_ALL);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL);

	extract_obj(obj, 0);
	return;
    }
}

void
fire_effect(void *vo, int level, int dam)
{
    if (mem_is(vo, MT_ROOM)) { /* nail objects on the floor */
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    fire_effect(obj,level,dam);
	}
	return;
    }
 
    if (mem_is(vo, MT_CHAR)) { /* do the effect on a victim */
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* chance of blindness */
	if (!IS_AFFECTED(victim,AFF_BLIND)
	&&  !saves_spell(level / 4 + dam / 20, victim,DAM_FIRE))
	{
            AFFECT_DATA af;
            act("$n is blinded by smoke!",victim,NULL,NULL,TO_ROOM);
            act("Your eyes tear up from smoke...you can't see a thing!",
		victim,NULL,NULL,TO_CHAR);
	 
            af.where        = TO_AFFECTS;
            af.type         = "fire breath";
            af.level        = level;
            af.duration     = number_range(0,level/10);
            INT(af.location) = APPLY_HITROLL;
            af.modifier     = -4;
            af.bitvector    = AFF_BLIND;
	    af.owner = NULL;
 
            affect_to_char(victim,&af);
	}

	/* getting thirsty */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_THIRST,dam/20);

	/* let's toast some gear! */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;

	    fire_effect(obj,level,dam);
        }
	return;
    }

    if (mem_is(vo, MT_OBJ)) { /* toast an object */
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance;
	char *msg;

    	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  number_range(0,4) == 0)
            return;
 
        chance = level / 4 + dam / 10;
 
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

	if  (material_is(obj, MATERIAL_SUSC_HEAT)) {
		chance += 30;
		msg = "$p melts and evaporates!";
	} else
	switch (obj->item_type) {
	default:             
		return;
	case ITEM_CONTAINER:
		msg = "$p ignites and burns!";
		break;
	case ITEM_POTION:
		chance += 25;
		msg = "$p bubbles and boils!";
		break;
	case ITEM_SCROLL:
		chance += 50;
		msg = "$p crackles and burns!";
		break;
	case ITEM_STAFF:
		chance += 10;
		msg = "$p smokes and chars!";
		break;
	case ITEM_WAND:
		msg = "$p sparks and sputters!";
		break;
	case ITEM_FOOD:
		msg = "$p blackens and crisps!";
		break;
	case ITEM_PILL:
		msg = "$p melts and drips!";
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
		return;

	if (obj->carried_by != NULL)
		act( msg, obj->carried_by, obj, NULL, TO_ALL );
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
		act(msg,obj->in_room->people,obj,NULL,TO_ALL);

	if (obj->contains)
		for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
			n_obj = t_obj->next_content;
			obj_from_obj(t_obj);
			if (obj->in_room != NULL)
				obj_to_room(t_obj,obj->in_room);
			else if (obj->carried_by != NULL)
				obj_to_room(t_obj,obj->carried_by->in_room);
			else {
				extract_obj(t_obj, 0);
				continue;
			}
			fire_effect(t_obj,level/2,dam/2);
		}

	extract_obj(obj, 0);
	return;
    }
}

void
poison_effect(void *vo, int level, int dam)
{
    if (mem_is(vo, MT_ROOM)) {  /* nail objects on the floor */
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;
 
        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            poison_effect(obj,level,dam);
        }
        return;
    }
 
    if (mem_is(vo, MT_CHAR)) { /* do the effect on a victim */
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

	/* chance of poisoning */
        if (!saves_spell(level / 4 + dam / 20,victim,DAM_POISON))
        {
	    AFFECT_DATA af;

            act_char("You feel poison coursing through your veins.", victim);
            act("$n looks very ill.", victim, NULL, NULL, TO_ROOM);

            af.where     = TO_AFFECTS;
            af.type      = "poison";
            af.level     = level;
            af.duration  = level / 2;
            INT(af.location) = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = AFF_POISON;
	    af.owner = NULL;
            affect_join( victim, &af );
        }

	/* equipment */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    poison_effect(obj,level,dam);
	}
	return;
    }

    if (mem_is(vo, MT_OBJ)) {/* do some poisoning */
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
  	||  IS_OBJ_STAT(obj,ITEM_BLESS)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;
	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	chance -= obj->level * 2;

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_FOOD:
		break;
	    case ITEM_DRINK_CON:
		if (INT(obj->value[0]) == INT(obj->value[1]))
		    return;
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	INT(obj->value[3]) = 1;
	return;
    }
}

void
shock_effect(void *vo,int level, int dam)
{
    if (mem_is(vo, MT_ROOM)) {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    shock_effect(obj,level,dam);
	}
	return;
    }

    if (mem_is(vo, MT_CHAR)) {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* daze and confused? */
	if (!saves_spell(level/4 + dam/20,victim,DAM_LIGHTNING))
	{
	    act_char("Your muscles stop responding.", victim);
	    DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
	}

	/* toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    shock_effect(obj,level,dam);
	}
	return;
    }

    if (mem_is(vo, MT_OBJ)) {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
	||  OBJ_IS(obj, OBJ_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) /2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

 	chance -= obj->level * 2;

	switch(obj->item_type)
	{
	    default:
		return;
	   case ITEM_WAND:
	   case ITEM_STAFF:
		chance += 10;
		msg = "$p overloads and explodes!";
		break;
	   case ITEM_JEWELRY:
		chance -= 10;
		msg = "$p is fused into a worthless lump.";
	}
	
	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,obj,NULL,TO_ALL);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL);

	extract_obj(obj, 0);
	return;
    }
}

void
sand_effect(void *vo, int level, int dam)
{
    if (mem_is(vo, MT_ROOM)) { /* nail objects on the floor */
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    sand_effect(obj,level,dam);
	}
	return;
    }

    if (mem_is(vo, MT_CHAR)) {  /* do the effect on a victim */
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	
	if (!IS_AFFECTED(victim,AFF_BLIND)
	&&  !saves_spell(level / 4 + dam / 20, victim,DAM_COLD))
	{
            AFFECT_DATA af;
            act("$n is blinded by flying sands!",victim,NULL,NULL,TO_ROOM);
            act("Your eyes tear up from sands...you can't see a thing!",
		victim,NULL,NULL,TO_CHAR);
	 
            af.where        = TO_AFFECTS;
            af.type         = "sand storm";
            af.level        = level;
            af.duration     = number_range(0,level/10);
            INT(af.location) = APPLY_HITROLL;
            af.modifier     = -4;
            af.bitvector    = AFF_BLIND;
	    af.owner = NULL;
 
            affect_to_char(victim,&af);
	}

	/* let's toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    sand_effect(obj,level,dam);
	}
	return;
    }

    if (mem_is(vo, MT_OBJ)) { /* toast an object */
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
	||  OBJ_IS(obj, OBJ_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	 if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

	chance -= obj->level * 2;

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_CONTAINER:
	    case ITEM_CORPSE_PC:
	    case ITEM_CORPSE_NPC:
		chance += 50;
		msg = "$p is filled with sand and evaporates.";
		break;
	    case ITEM_ARMOR:
		chance -=10;
		msg = "$p is etched by sand.";
		break;
	    case ITEM_CLOTHING:
		msg = "$p is corroded by sands.";
	 	break;
	    case ITEM_WAND:
		chance = 50;
		msg = "$p mixes with crashing sands.";
		break;
	    case ITEM_SCROLL:
		chance += 20;
		msg = "$p is surrouned by sand.";
		break; 
	    case ITEM_POTION:
		chance +=10;
		msg = "$p is broken into pieces by crashing sands.";
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,obj,NULL,TO_ALL);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL);

	if (obj->item_type == ITEM_ARMOR)  /* etch it */
	{
	    AFFECT_DATA *paf;
	    bool af_found = FALSE;
	    int i;

 	    affect_enchant(obj);

	    for ( paf = obj->affected; paf != NULL; paf = paf->next)
            {
		if (paf->where == TO_SKILLS
		||  paf->where == TO_RESIST
		||  paf->where == TO_FORMRESIST
		||  paf->where == TO_RACE)
			continue;

                if (INT(paf->location) == APPLY_AC)
                {
                    af_found = TRUE;
                    paf->type = NULL;
                    paf->modifier += 1;
                    paf->level = UMAX(paf->level,level);
		    break;
                }
            }
 
            if (!af_found)
            /* needs a new affect */
            {
		paf = aff_new();
 
                paf->type       = NULL;
                paf->level      = level;
                paf->duration   = level;
                INT(paf->location) = APPLY_AC;
                paf->modifier   =  1;
                paf->bitvector  = 0;
                paf->next       = obj->affected;
                obj->affected   = paf;
            }
 
            if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
                for (i = 0; i < 4; i++)
                    obj->carried_by->armor[i] += 1;
            return;
	}

	/* get rid of the object */
	if (obj->contains)  /* dump contents */
	{
	    for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
	    {
		n_obj = t_obj->next_content;
		obj_from_obj(t_obj);
		if (obj->in_room != NULL)
		    obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)
		    obj_to_room(t_obj,obj->carried_by->in_room);
		else
		{
		    extract_obj(t_obj, 0);
		    continue;
		}

		sand_effect(t_obj,level/2,dam/2);
	    }
 	}

	extract_obj(obj, 0);
	return;
    }
}

void
scream_effect(void *vo, int level, int dam)
{
    if (mem_is(vo, MT_ROOM)) { /* nail objects on the floor */
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    scream_effect(obj,level,dam);
	}
	return;
    }
 
    if (mem_is(vo, MT_CHAR)) { /* do the effect on a victim */
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	if  (!saves_spell(level / 4 + dam / 20, victim,DAM_SOUND))
	{
            AFFECT_DATA af;
            act("$n can't hear anything!",victim,NULL,NULL,TO_ROOM);
            act("You can't hear a thing!",victim,NULL,NULL,TO_CHAR);
	 
            af.where        = TO_AFFECTS;
            af.type         = "scream";
            af.level        = level;
            af.duration     = 0;
            INT(af.location) = APPLY_NONE;
            af.modifier     = 0;
            af.bitvector    = AFF_SCREAM;
	    af.owner = NULL;
 
            affect_to_char(victim,&af);
	}

	/* daze and confused? */
	if (!saves_spell(level/4 + dam/20,victim,DAM_SOUND))
	{
	    act_char("You can't hear anything!", victim);
	    DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
	}

	 
	/* getting thirsty */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_THIRST,dam/20);

	/* let's toast some gear! */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;

	    scream_effect(obj,level,dam);
        }
	return;
    }

    if (mem_is(vo, MT_OBJ)) {  /* toast an object */
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance;
	char *msg;

    	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  number_range(0,4) == 0)
            return;
 
        chance = level / 4 + dam / 10;
 
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        if  (material_is(obj, MATERIAL_FRAGILE))  
	{
	 chance += 30;
	 msg = "$p breaks into tiny small pieces.";
	}
	else
        switch ( obj->item_type )
        {
        default:             
	    return;
        case ITEM_POTION:
            chance += 25;
            msg = "Vial of $p breaks and liquid spoils!";
            break;
        case ITEM_SCROLL:
            chance += 50;
            msg = "$p breaks into tiny pieces!";
            break;
        case ITEM_DRINK_CON:
	    msg = "$p breaks and liquid spoils!";
	    chance += 5;
	    break;
        case ITEM_PILL:
            msg = "$p breaks into pieces!";
            break;
        }

        chance = URANGE(5,chance,95);

        if (number_percent() > chance)
            return;
 
	if (obj->carried_by != NULL)
            act( msg, obj->carried_by, obj, NULL, TO_ALL );
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL);

        if (obj->contains)
        {
            /* dump the contents */
 
            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
            {
                n_obj = t_obj->next_content;
                obj_from_obj(t_obj);
		if (obj->in_room != NULL)
                    obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)
		    obj_to_room(t_obj,obj->carried_by->in_room);
		else
		{
		    extract_obj(t_obj, 0);
		    continue;
		}
		scream_effect(t_obj,level/2,dam/2);
            }
        }
 
        extract_obj(obj, 0);
	return;
    }
}
