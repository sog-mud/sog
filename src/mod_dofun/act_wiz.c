/*
 * $Id: act_wiz.c,v 1.39 1998-07-13 11:46:40 efdi Exp $
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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "db.h"
#include "comm.h"
#include "act_info.h"
#include "act_wiz.h"
#include "hometown.h"
#include "magic.h"
#include "resource.h"
#include "update.h"
#include "util.h"
#include "quest.h"
#include "log.h"
#include "act_move.h"
#include "obj_prog.h"
#include "buffer.h"
#include "tables.h"
#include "mlstring.h"
#include "olc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_grant        );
DECLARE_DO_FUN(do_limited      );
DECLARE_DO_FUN(do_help	       );

extern int max_on;

/*
 * Local functions.
 */
bool write_to_descriptor  args((int desc, char *txt, int length));
void reboot_muddy(void);
extern int rebooter;


void do_clan_scan(CHAR_DATA *ch, const char *argument)
{
	int i;

	for(i = 1; i<MAX_CLAN; i++) {
		char_printf(ch, "  Cabal: %s, room %d, item %d, ptr: %s ", 
			    clan_table[i].short_name, clan_table[i].room_vnum, 
			    clan_table[i].obj_vnum,
			    clan_table[i].obj_ptr != NULL ?
					clan_table[i].obj_ptr->short_descr :
					"(NULL)");
		if (clan_table[i].obj_ptr != NULL) {
			char *p;

			p = clan_table[i].obj_ptr->in_room != NULL ?
			    mlstr_val(ch, clan_table[i].obj_ptr->in_room->name):
			    "(NULL)";
			char_printf(ch, "now_in_room: %s", p);
		}
		char_puts("\n\r", ch);
	}
}

void do_objlist(CHAR_DATA *ch, const char *argument)
{
FILE *fp;
OBJ_DATA *obj;
AFFECT_DATA *paf;

	 if ((fp=fopen("objlist.txt", "w+")) == NULL)
	 {
	    char_puts("File error.\n\r", ch);
	    return;
	 }

	 for(obj=object_list; obj!=NULL; obj = obj->next)
	 {
	 if (obj->pIndexData->affected != NULL)		
	 /*  if (obj->item_type == ITEM_WEAPON)	*/
	 {
	   fprintf(fp, "\n#Obj: %s (Vnum : %d) \n", obj->short_descr ,obj->pIndexData->vnum);
	fprintf(fp,
		"Object '%s' is type %s, extra flags %s.\nWeight is %d, value is %d, level is %d.\n",

		obj->name,
		item_type_name(obj),
		extra_bit_name(obj->extra_flags),
		obj->weight / 10,
		obj->cost,
		obj->level
		);

	switch (obj->item_type)
	{
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		fprintf(fp, "Level %d spells of:", obj->value[0]);

		if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL)
		{
		    fprintf(fp, " '%s'", skill_table[obj->value[1]].name);
		}

		if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL)
		{
		    fprintf(fp, " '%s'", skill_table[obj->value[2]].name);
		}

		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
		{
		    fprintf(fp, " '%s'", skill_table[obj->value[3]].name);
		}

		if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
		{
		    fprintf(fp, " '%s'", skill_table[obj->value[4]].name);
		}

		fprintf(fp,".\n");
		break;

	case ITEM_WAND: 
	case ITEM_STAFF: 
		fprintf(fp, "Has %d charges of level %d", obj->value[2], obj->value[0]);
	  
		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
		{
		    fprintf(fp, " '%s'", skill_table[obj->value[3]].name);
		}

		fprintf(fp,".\n");
		break;

	case ITEM_DRINK_CON:
	    fprintf(fp,"It holds %s-colored %s.\n",
		    liq_table[obj->value[2]].liq_color,
	        liq_table[obj->value[2]].liq_name);
	    break;

	case ITEM_CONTAINER:
		fprintf(fp,"Capacity: %d#  Maximum weight: %d#  flags: %s\n",
		    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
		if (obj->value[4] != 100)
		{
		    fprintf(fp,"Weight multiplier: %d%%\n",
			obj->value[4]);
		}
		break;
			
	case ITEM_WEAPON:
		fprintf(fp,"Weapon type is ");
		switch (obj->value[0])
		{
		    case(WEAPON_EXOTIC) : fprintf(fp,"exotic.\n");	break;
		    case(WEAPON_SWORD)  : fprintf(fp,"sword.\n");	break;	
		    case(WEAPON_DAGGER) : fprintf(fp,"dagger.\n");	break;
		    case(WEAPON_SPEAR)	: fprintf(fp,"spear/staff.\n");	break;
		    case(WEAPON_MACE) 	: fprintf(fp,"mace/club.\n");	break;
		    case(WEAPON_AXE)	: fprintf(fp,"axe.\n");		break;
		    case(WEAPON_FLAIL)	: fprintf(fp,"flail.\n");	break;
		    case(WEAPON_WHIP)	: fprintf(fp,"whip.\n");	break;
		    case(WEAPON_POLEARM): fprintf(fp,"polearm.\n");	break;
		    case(WEAPON_BOW)	: fprintf(fp,"bow.\n");		break;
		    case(WEAPON_ARROW)	: fprintf(fp,"arrow.\n");	break;
		    case(WEAPON_LANCE)	: fprintf(fp,"lance.\n");	break;
		    default		: fprintf(fp,"unknown.\n");	break;
		}
		if (obj->pIndexData->new_format)
		    fprintf(fp,"Damage is %dd%d (average %d).\n",
			obj->value[1],obj->value[2],
			(1 + obj->value[2]) * obj->value[1] / 2);
		else
		    fprintf(fp, "Damage is %d to %d (average %d).\n",
		    	obj->value[1], obj->value[2],
		    	(obj->value[1] + obj->value[2]) / 2);
	    if (obj->value[4])  /* weapon flags */
	    {
	        fprintf(fp,"Weapons flags: %s\n",weapon_bit_name(obj->value[4]));
		}
		break;

	case ITEM_ARMOR:
		fprintf(fp, 
		"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n", 
		    obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
		break;
	}
	   for(paf=obj->pIndexData->affected; paf != NULL; paf = paf->next)
	   {
	        if (paf == NULL) continue;
	        fprintf(fp, "  Affects %s by %d.\n",
	            affect_loc_name(paf->location), paf->modifier);
	        if (paf->bitvector)
	        {
	            switch(paf->where)
	            {
	                case TO_AFFECTS:
	                    fprintf(fp,"   Adds %s affect.\n",
	                        affect_bit_name(paf->bitvector));
	                    break;
	                case TO_OBJECT:
	                    fprintf(fp,"   Adds %s object flag.\n",
	                        extra_bit_name(paf->bitvector));
	                    break;
	                case TO_IMMUNE:
	                    fprintf(fp,"   Adds immunity to %s.\n",
	                        imm_bit_name(paf->bitvector));
	                    break;
	                case TO_RESIST:
	                    fprintf(fp,"   Adds resistance to %s.\n\r",
	                        imm_bit_name(paf->bitvector));
	                    break;
	                case TO_VULN:
	                    fprintf(fp,"   Adds vulnerability to %s.\n\r",
	                        imm_bit_name(paf->bitvector));
	                    break;
	                case TO_DETECTS:
	                    fprintf(fp,"   Adds %s detection.\n\r",
	                        detect_bit_name(paf->bitvector));
	                    break;
	                default:
	                    fprintf(fp,"   Unknown bit %d: %d\n\r",
	                        paf->where,paf->bitvector);
	                    break;
	            }
	        }
	   }
	 }
	 }
	 fclose(fp);
	 return;
}

void do_limited(CHAR_DATA *ch, const char *argument)
{

	extern int top_obj_index;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *obj_index;
	int	lCount = 0;
	int	ingameCount;
	int 	nMatch;
	int 	vnum;

	if (argument[0] != '\0')  {
	obj_index = get_obj_index(atoi(argument));
	if (obj_index == NULL)  {
	  char_puts("Not found.\n\r", ch);
	  return;
	}
	if (obj_index->limit == -1)  {
	  char_puts("Thats not a limited item.\n\r", ch);
	  return;
	}
	nMatch = 0;
	      char_printf(ch, "%-35s [%5d]  Limit: %3d  Current: %3d\n\r", 
			   obj_index->short_descr, 
			   obj_index->vnum,
		           obj_index->limit, 
			   obj_index->count);
		  ingameCount = 0;
		  for (obj=object_list; obj != NULL; obj=obj->next)
		    if (obj->pIndexData->vnum == obj_index->vnum)  {
		      ingameCount++;
		      if (obj->carried_by != NULL) 
			char_printf(ch, "Carried by %-30s\n\r", obj->carried_by->name);
		      else if (obj->in_room != NULL) 
			char_printf(ch, "At %-20s [%d]\n\r",
				mlstr_val(ch, obj->in_room->name),
				obj->in_room->vnum);
		      else if (obj->in_obj != NULL) 
			char_printf(ch, "In %-20s [%d] \n\r", obj->in_obj->short_descr, obj->in_obj->pIndexData->vnum);
		    }
		    char_printf(ch, "  %d found in game. %d should be in pFiles.\n\r", 
				ingameCount, obj_index->count-ingameCount);
		return;
	}

	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
	  if ((obj_index = get_obj_index(vnum)) != NULL)
	  {
	    nMatch++;
		if (obj_index->limit != -1)  {
		  lCount++;
	      char_printf(ch, "%-37s [%5d]  Limit: %3d  Current: %3d\n\r", 
			   obj_index->short_descr, 
			   obj_index->vnum,
		           obj_index->limit, 
			   obj_index->count);
		}
	  }
	char_printf(ch, "\n\r%d of %d objects are limited.\n\r", lCount, nMatch);
	return;

}

void do_wiznet(CHAR_DATA *ch, const char *argument)
{
	int flag;

	if (argument[0] == '\0') {
		/* show wiznet status */
		BUFFER *output;

		output = buf_new(0);
		buf_printf(output, "Wiznet status: %s\n\r",
			   IS_SET(ch->wiznet, WIZ_ON) ? "ON" : "OFF");

		buf_add(output, "\n\rchannel    | status");
		buf_add(output, "\n\r-----------|-------\n\r");
		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			buf_printf(output, "%-11s|  %s\n\r",
				   wiznet_table[flag].name,
				   wiznet_table[flag].level > get_trust(ch) ?
				   "N/A" :
				   IS_SET(ch->wiznet, wiznet_table[flag].flag) ?
				   "ON" : "OFF");
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if (!str_prefix(argument,"on")) {
		char_puts("Welcome to Wiznet!\n\r", ch);
		SET_BIT(ch->wiznet, WIZ_ON);
		return;
	}

	if (!str_prefix(argument,"off")) {
		char_puts("Signing off of Wiznet.\n\r", ch);
		REMOVE_BIT(ch->wiznet, WIZ_ON);
		return;
	}

	flag = wiznet_lookup(argument);

	if (flag == -1 || get_trust(ch) < wiznet_table[flag].level) {
		char_puts("No such option.\n\r", ch);
		return;
	}
	 
	if (IS_SET(ch->wiznet,wiznet_table[flag].flag)) {
		char_printf(ch,"You will no longer see %s on wiznet.\n\r",
		        wiznet_table[flag].name);
		REMOVE_BIT(ch->wiznet, wiznet_table[flag].flag);
		return;
	} else {
		char_printf(ch, "You will now see %s on wiznet.\n\r",
			wiznet_table[flag].name);
		SET_BIT(ch->wiznet, wiznet_table[flag].flag);
		return;
	}

}

void wiznet_printf(CHAR_DATA *ch, OBJ_DATA *obj,
			   long flag, long flag_skip, int min_level,
			   char* format, ...) 
{
	va_list ap;
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];

	va_start(ap, format);
	for (d = descriptor_list; d != NULL; d = d->next) {
		if (d->connected == CON_PLAYING &&
		    IS_IMMORTAL(d->character) && 
		    IS_SET(d->character->wiznet,WIZ_ON) &&
		    (!flag || IS_SET(d->character->wiznet,flag)) &&
		    (!flag_skip || !IS_SET(d->character->wiznet,flag_skip)) &&
		    get_trust(d->character) >= min_level &&
		    d->character != ch) {
			if (IS_SET(d->character->wiznet,WIZ_PREFIX))
				char_puts("--> ",d->character);
			vsnprintf(buf, sizeof(buf), format, ap);
			act_puts(buf, d->character, obj, ch, TO_CHAR, POS_DEAD);
		}
	}
	va_end(ap); 
}

void do_tick(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	
	one_argument(argument, arg);
	if (arg[0] == '\0')  {
		char_puts("tick area : area update\n\r",ch);
		char_puts("tick char : char update\n\r",ch);
		char_puts("tick room : room update\n\r",ch);
		char_puts("tick track: track update\n\r",ch);
		return;
	}
	if (is_name(arg, "area"))  {
		area_update();
		char_puts("Area updated.\n\r", ch);
		return;
	}
	if (is_name(arg, "char player"))  {
		char_update();
		char_puts("Players updated.\n\r", ch);
		return;
	}
	if (is_name(arg, "room"))  {
		room_update();
		char_puts("Room updated.\n\r", ch);
		return;
	}
	if (is_name(arg, "track"))  {
		track_update();
		char_puts("Track updated.\n\r", ch);
		return;
	}
	do_tick(ch,"");
	return;
}

/* equips a character */
void do_outfit (CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	int sn,vnum;

	if ((ch->level > 5 || IS_NPC(ch)) && !IS_IMMORTAL(ch))
	{
		char_puts("Find it yourself!\n\r",ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) == NULL)
	{
	    obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0);
		obj->cost = 0;
		obj->condition = 100;
	    obj_to_char(obj, ch);
	    equip_char(ch, obj, WEAR_LIGHT);
	}
	
	if ((obj = get_eq_char(ch, WEAR_BODY)) == NULL)
	{
		obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0);
		obj->cost = 0;
		obj->condition = 100;
	    obj_to_char(obj, ch);
	    equip_char(ch, obj, WEAR_BODY);
	}


	
	/* do the weapon thing */
	if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
	{
		sn = 0; 
		vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */
	    vnum = class_table[ch->class].weapon;
		obj = create_object(get_obj_index(vnum),0);
		obj->condition = 100;
	 	obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_WIELD);
	}

	if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL 
	||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) 
	&&  (obj = get_eq_char(ch, WEAR_SHIELD)) == NULL)
	{
	    obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0);
		obj->cost = 0;
		obj->condition = 100;
	    obj_to_char(obj, ch);
	    equip_char(ch, obj, WEAR_SHIELD);
	}

	char_puts("You have been equipped by gods.\n\r",ch);
}

	 
/* RT nochannels command, for those spammers */
void do_nochannels(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	
	one_argument(argument, arg);
	
	if (arg[0] == '\0')
	{
	    char_puts("Nochannel whom?", ch);
	    return;
	}
	
	if ((victim = get_char_world(ch, arg)) == NULL)
	{
	    char_puts("They aren't here.\n\r", ch);
	    return;
	}
	
	if (get_trust(victim) >= get_trust(ch))
	{
	    char_puts("You failed.\n\r", ch);
	    return;
	}
	
	if (IS_SET(victim->comm, COMM_NOCHANNELS))
	{
	    REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
	    char_puts("The gods have restored your channel priviliges.\n\r", 
			      victim);
	    char_puts("NOCHANNELS removed.\n\r", ch);
		wiznet_printf(ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0,
			      "$N restores channels to %s",victim->name);
	}
	else
	{
	    SET_BIT(victim->comm, COMM_NOCHANNELS);
	    char_puts("The gods have revoked your channel priviliges.\n\r", 
			       victim);
	    char_puts("NOCHANNELS set.\n\r", ch);
		wiznet_printf(ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0,
				"$N revokes %s's channels.",victim->name);
	}
	
	return;
}


void do_smote(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch;
	char *letter,*name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	int matches = 0;
	
	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE))
	{
	    char_puts("You can't show your emotions.\n\r", ch);
	    return;
	}
	
	if (argument[0] == '\0')
	{
	    char_puts("Emote what?\n\r", ch);
	    return;
	}
	
	if (strstr(argument,ch->name) == NULL)
	{
		char_puts("You must include your name in an smote.\n\r",ch);
		return;
	}
	 
	char_puts(argument,ch);
	char_puts("\n\r",ch);
	
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	    if (vch->desc == NULL || vch == ch)
	        continue;
	
	    if ((letter = strstr(argument,vch->name)) == NULL)
	    {
		    char_puts(argument,vch);
		    char_puts("\n\r",vch);
	        continue;
	    }
	
	    strcpy(temp,argument);
	    temp[strlen(argument) - strlen(letter)] = '\0';
	    last[0] = '\0';
	    name = vch->name;
	
	    for (; *letter != '\0'; letter++)
	    {
	        if (*letter == '\'' && matches == strlen(vch->name))
	        {
	            strcat(temp,"r");
	            continue;
	        }
	
	        if (*letter == 's' && matches == strlen(vch->name))
	        {
	            matches = 0;
	            continue;
	        }
	
	        if (matches == strlen(vch->name))
	        {
	            matches = 0;
	        }
	
	        if (*letter == *name)
	        {
	            matches++;
	            name++;
	            if (matches == strlen(vch->name))
	            {
	                strcat(temp,"you");
	                last[0] = '\0';
	                name = vch->name;
	                continue;
	            }
	            strncat(last,letter,1);
	            continue;
	        }
	
	        matches = 0;
	        strcat(temp,last);
	        strncat(temp,letter,1);
	        last[0] = '\0';
	        name = vch->name;
	    }
	
		char_puts(temp,vch);
		char_puts("\n\r",vch);
	}
	
	return;
}

void do_bamfin(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch)) {
		if (argument[0] == '\0') {
			char_printf(ch, "Your poofin is %s\n\r",
				    ch->pcdata->bamfin);
			return;
		}

		if (strstr(argument, ch->name) == NULL) {
			char_puts("You must include your name.\n\r",ch);
			return;
		}
		     
		free_string(ch->pcdata->bamfin);
		ch->pcdata->bamfin = str_dup(argument);
		smash_tilde(ch->pcdata->bamfin);

		char_printf(ch, "Your poofin is now %s\n\r",
			    ch->pcdata->bamfin);
	}
}



void do_bamfout(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch)) {
		if (argument[0] == '\0') {
			char_printf(ch, "Your poofout is %s\n\r",
				    ch->pcdata->bamfout);
			return;
		}
	
		if (strstr(argument,ch->name) == NULL) {
			char_puts("You must include your name.\n\r", ch);
			return;
		}
	
		free_string(ch->pcdata->bamfout);
		ch->pcdata->bamfout = str_dup(argument);
		smash_tilde(ch->pcdata->bamfout);
	
		char_printf(ch, "Your poofout is now %s\n\r",
			    ch->pcdata->bamfout);
	}
}



void do_deny(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);
	if (arg[0] == '\0')
	{
		char_puts("Deny whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		char_puts("Not on NPC's.\n\r", ch);
		return;
	}

	if (get_trust(victim) >= get_trust(ch))
	{
		char_puts("You failed.\n\r", ch);
		return;
	}

	SET_BIT(victim->act, PLR_DENY);
	char_puts("You are denied access!\n\r", victim);
	wiznet_printf(ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0,
			"$N denies access to %s",victim->name);
	char_puts("OK.\n\r", ch);
	save_char_obj(victim, FALSE);
	stop_fighting(victim,TRUE);
	do_quit(victim, "");
}



void do_disconnect(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	one_argument(argument, arg);
	if (arg[0] == '\0')
	{
		char_puts("Disconnect whom?\n\r", ch);
		return;
	}

	if (is_number(arg))
	{
		int desc;

		desc = atoi(arg);
		for (d = descriptor_list; d != NULL; d = d->next)
		{
	        if (d->descriptor == desc)
	        {
	        	close_socket(d);
			char_nputs(OK, ch);
	        	return;
	        }
		}
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim->desc == NULL)
	{
		act("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
		return;
	}

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d == victim->desc)
		{
		    close_socket(d);
		    char_nputs(OK, ch);
		    return;
		}
	}

	bug("Do_disconnect: desc not found.", 0);
	char_puts("Descriptor not found!\n\r", ch);
	return;
}


void do_echo(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;
	
	if (argument[0] == '\0')
	{
		char_puts("Global echo what?\n\r", ch);
		return;
	}
	
	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING)
		{
		    if (get_trust(d->character) >= get_trust(ch))
			char_puts("global> ",d->character);
		    char_puts(argument, d->character);
		    char_puts("\n\r",   d->character);
		}
	}

	return;
}



void do_recho(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;
	
	if (argument[0] == '\0')
	{
		char_puts("Local echo what?\n\r", ch);

		return;
	}

	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING
		&&   d->character->in_room == ch->in_room)
		{
	        if (get_trust(d->character) >= get_trust(ch))
	            char_puts("local> ",d->character);
		    char_puts(argument, d->character);
		    char_puts("\n\r",   d->character);
		}
	}

	return;
}

void do_zecho(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (argument[0] == '\0')
	{
		char_puts("Zone echo what?\n\r",ch);
		return;
	}

	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING
		&&  d->character->in_room != NULL && ch->in_room != NULL
		&&  d->character->in_room->area == ch->in_room->area)
		{
		    if (get_trust(d->character) >= get_trust(ch))
			char_puts("zone> ",d->character);
		    char_puts(argument,d->character);
		    char_puts("\n\r",d->character);
		}
	}
}

void do_pecho(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg);
	
	if (argument[0] == '\0' || arg[0] == '\0')
	{
		char_puts("Personal echo what?\n\r", ch); 
		return;
	}
	 
	if  ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("Target not found.\n\r",ch);
		return;
	}

	if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
	    char_puts("personal> ",victim);

	char_puts(argument,victim);
	char_puts("\n\r",victim);
	char_puts("personal> ",ch);
	char_puts(argument,ch);
	char_puts("\n\r",ch);
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



void do_transfer(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0')
	{
		char_puts("Transfer whom (and where)?\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, "all")) {
		for (d = descriptor_list; d != NULL; d = d->next)
		    if (d->connected == CON_PLAYING
		    &&   d->character != ch
		    &&   d->character->in_room != NULL
		    &&   can_see(ch, d->character))
			doprintf(do_transfer, ch,
				"%s %s", d->character->name, arg2);
		return;
	}

	/*
	 * Thanks to Grodyn for the optional location parameter.
	 */
	if (arg2[0] == '\0')
		location = ch->in_room;
	else {
		if ((location = find_location(ch, arg2)) == NULL)
		{
		    char_puts("No such location.\n\r", ch);
		    return;
		}

/*	if (!is_room_owner(ch,location) && room_is_private(location) */
		if (room_is_private(location) 
		&&  get_trust(ch) < MAX_LEVEL)
		{
		    char_puts("That room is private right now.\n\r", ch);
		    return;
		}
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim->in_room == NULL) {
		char_puts("They are in limbo.\n\r", ch);
		return;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim, TRUE);
	act("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, location);
	act("$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		act("$n has transferred you.", ch, NULL, victim, TO_VICT);
	do_look(victim, "auto");
	char_nputs(OK, ch);
}



void do_at(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;
	CHAR_DATA *wch;
	
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		char_puts("At where what?\n\r", ch);
		return;
	}

	if ((location = find_location(ch, arg)) == NULL)
	{
		char_puts("No such location.\n\r", ch);
		return;
	}

/*    if (!is_room_owner(ch,location) && room_is_private(location) */
	if (room_is_private(location) 	
	&&  get_trust(ch) < MAX_LEVEL)
	{
		char_puts("That room is private right now.\n\r", ch);
		return;
	}

	original = ch->in_room;
	on = ch->on;
	char_from_room(ch);
	char_to_room(ch, location);
	interpret(ch, argument);

	/*
	 * See if 'ch' still exists before continuing!
	 * Handles 'at XXXX quit' case.
	 */
	for (wch = char_list; wch != NULL; wch = wch->next)
	{
		if (wch == ch)
		{
		    char_from_room(ch);
		    char_to_room(ch, original);
		    ch->on = on;
		    break;
		}
	}

	return;
}



void do_goto(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	int count = 0;

	if (argument[0] == '\0')
	{
		char_puts("Goto where?\n\r", ch);
		return;
	}

	if ((location = find_location(ch, argument)) == NULL)
	{
		char_puts("No such location.\n\r", ch);
		return;
	}

	count = 0;
	for (rch = location->people; rch != NULL; rch = rch->next_in_room)
	    count++;
/*
	if (!is_room_owner(ch,location) && room_is_private(location) 
	&&  (count > 1 || get_trust(ch) < MAX_LEVEL))
	{
		char_puts("That room is private right now.\n\r", ch);
		return;
	} */

	if (ch->fighting != NULL)
		stop_fighting(ch, TRUE);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (get_trust(rch) >= ch->invis_level)
		{
		    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
			act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
		    else
			act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
		}
	}

	char_from_room(ch);
	char_to_room(ch, location);


	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
	    if (get_trust(rch) >= ch->invis_level)
	    {
	        if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
	            act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
	        else
	            act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
	    }
	}

	do_look(ch, "auto");
	return;
}

void do_violate(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	
	if (argument[0] == '\0')
	{
	    char_puts("Goto where?\n\r", ch);
	    return;
	}
	
	if ((location = find_location(ch, argument)) == NULL)
	{
	    char_puts("No such location.\n\r", ch);
	    return;
	}

	if (!room_is_private(location))
	{
	    char_puts("That room isn't private, use goto.\n\r", ch);
	    return;
	}
	
	if (ch->fighting != NULL)
	    stop_fighting(ch, TRUE);
	
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
	    if (get_trust(rch) >= ch->invis_level)
	    {
	        if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
	            act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	        else
	            act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	    }
	}
	
	char_from_room(ch);
	char_to_room(ch, location);
	
	
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
	    if (get_trust(rch) >= ch->invis_level)
	    {
	        if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
	            act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
	        else
	            act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
	    }
	}
	
	do_look(ch, "auto");
	return;
}

/* RT to replace the 3 stat commands */

void do_stat (CHAR_DATA *ch, const char *argument)
{
	 char arg[MAX_INPUT_LENGTH];
	 const char *string;
	 OBJ_DATA *obj;
	 ROOM_INDEX_DATA *location;
	 CHAR_DATA *victim;

	 string = one_argument(argument, arg);
	 if (arg[0] == '\0') {
		char_puts("Syntax:\n\r", ch);
		char_puts("  stat <name>\n\r", ch);
		char_puts("  stat obj <name>\n\r", ch);
		char_puts("  stat mob <name>\n\r", ch);
		char_puts("  stat room <number>\n\r", ch);
		return;
	 }

	 if (!str_cmp(arg, "room")) {
		do_rstat(ch, string);
		return;
	 }
	
	 if (!str_cmp(arg, "obj")) {
		do_ostat(ch, string);
		return;
	 }

	 if(!str_cmp(arg, "char")  || !str_cmp(arg, "mob")) {
		do_mstat(ch, string);
		return;
	 }
	 
	 /* do it the old way */

	 obj = get_obj_world(ch, argument);
	 if (obj != NULL) {
	 	do_ostat(ch,argument);
	 	return;
	 }

	victim = get_char_world(ch, argument);
	if (victim != NULL) {
		do_mstat(ch,argument);
		return;
	}

	location = find_location(ch, argument);
	if (location != NULL) {
		do_rstat(ch,argument);
		return;
	}

	char_puts("Nothing by that name found anywhere.\n\r",ch);
}


void do_rstat(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_HISTORY_DATA *rh;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	BUFFER *output;
	int door;

	one_argument(argument, arg);
	location = (arg[0] == '\0') ? ch->in_room : find_location(ch, arg);
	if (location == NULL) {
		char_puts("No such location.\n\r", ch);
		return;
	}

/*    if (!is_room_owner(ch,location) && ch->in_room != location  */
	if (ch->in_room != location 
	&&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR)) {
		char_puts("That room is private right now.\n\r", ch);
		return;
	}

	output = buf_new(0);

	if (ch->in_room->affected_by)
		buf_printf(output, "Affected by %s\n\r", 
			   raffect_bit_name(ch->in_room->affected_by));

	if (ch->in_room->room_flags)
		buf_printf(output, "Roomflags %s\n\r", 
			   flag_room_name(ch->in_room->room_flags));

	mlstr_buf(output, "Name: ", location->name);
	buf_printf(output, "Area: '%s'\n\rOwner: '%s'\n\r",
		   location->area->name,
		   location->owner);

	buf_printf(output,
		   "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
		   location->vnum,
		   location->sector_type,
		   location->light,
		   location->heal_rate,
		   location->mana_rate);

	buf_printf(output, "Room flags: [%s].\n\r",
		   flag_string(room_flags, location->room_flags));
	buf_add(output, "Description:\n\r");
	mlstr_buf(output, "", location->description);

	if (location->extra_descr != NULL) {
		EXTRA_DESCR_DATA *ed;

		buf_add(output, "Extra description keywords: '");
		for (ed = location->extra_descr; ed; ed = ed->next) {
			buf_add(output, ed->keyword);
			if (ed->next != NULL)
				buf_add(output, " ");
		}
		buf_add(output, "'.\n\r");
	}

	buf_add(output, "Characters:");
	for (rch = location->people; rch; rch = rch->next_in_room) {
		if (can_see(ch,rch)) {
		    buf_add(output, " ");
		    one_argument(rch->name, buf);
		    buf_add(output, buf);
		}
	}

	buf_add(output, ".\n\rObjects:   ");
	for (obj = location->contents; obj; obj = obj->next_content) {
		buf_add(output, " ");
		one_argument(obj->name, buf);
		buf_add(output, buf);
	}
	buf_add(output, ".\n\r");

	for (door = 0; door <= 5; door++) {
		EXIT_DATA *pexit;

		if ((pexit = location->exit[door]) != NULL) {
			buf_printf(output, "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.\n\r",
				door,
				pexit->u1.to_room == NULL ?
				-1 : pexit->u1.to_room->vnum,
		    		pexit->key,
		    		pexit->exit_info,
		    		pexit->keyword);
			mlstr_buf(output, "Description: ",
				     pexit->description);
		}
	}
	buf_add(output, "Tracks:\n\r");
	for (rh = location->history;rh != NULL;rh = rh->next)
		buf_printf(output,"%s took door %i.\n\r", rh->name, rh->went);

	char_puts(buf_string(output), ch);
	buf_free(output);
}



void do_ostat(CHAR_DATA *ch, const char *argument)
{
	int i;
	BUFFER *output;
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	OBJ_DATA *obj;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Stat what?\n\r", ch);
		return;
	}

	if ((obj = get_obj_world(ch, argument)) == NULL) {
		char_puts("Nothing like that in hell, earth, or heaven.\n\r", ch);
		return;
	}

	char_printf(ch, "Name(s): %s\n\r", obj->name);

	char_printf(ch, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
		obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
		item_type_name(obj), obj->pIndexData->reset_num);

	char_printf(ch, "Short description: %s\n\rLong description: %s\n\r",
		obj->short_descr, obj->description);

	char_printf(ch, "Wear bits: %s\n\r", wear_bit_name(obj->wear_flags));
	char_printf(ch, "Extra bits: %s\n\r", extra_bit_name(obj->extra_flags));
	char_printf(ch, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
		1,           get_obj_number(obj),
		obj->weight, get_obj_weight(obj),get_true_weight(obj));

	char_printf(ch, "Level: %d  Cost: %d  Condition: %d  Timer: %d Count: %d\n\r",
		obj->level, obj->cost, obj->condition, obj->timer, obj->pIndexData->count);

	char_printf(ch,
		"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
		obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
		obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
		obj->carried_by == NULL    ? "(none)" : 
		    can_see(ch,obj->carried_by) ? obj->carried_by->name
					 	: "someone",
		obj->wear_loc);
	
	char_printf(ch, "Values: %d %d %d %d %d\n\r",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3],
		obj->value[4]);
	
	/* now give out vital statistics as per identify */
	
	switch (obj->item_type)
	{
		case ITEM_SCROLL: 
		case ITEM_POTION:
		case ITEM_PILL:
		    char_printf(ch, "Level %d spells of:", obj->value[0]);

		    if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL)
		    {
		    	char_puts(" '", ch);
		    	char_puts(skill_table[obj->value[1]].name, ch);
		    	char_puts("'", ch);
		    }

		    if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL)
		    {
		    	char_puts(" '", ch);
		    	char_puts(skill_table[obj->value[2]].name, ch);
		    	char_puts("'", ch);
		    }

		    if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
		    {
		    	char_puts(" '", ch);
		    	char_puts(skill_table[obj->value[3]].name, ch);
		    	char_puts("'", ch);
		    }

		    if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
		    {
			char_puts(" '",ch);
			char_puts(skill_table[obj->value[4]].name,ch);
			char_puts("'",ch);
		    }

		    char_puts(".\n\r", ch);
		break;

		case ITEM_WAND: 
		case ITEM_STAFF: 
		    char_printf(ch, "Has %d(%d) charges of level %d",
		    	obj->value[1], obj->value[2], obj->value[0]);
	  
		    if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
		    {
		    	char_puts(" '", ch);
		    	char_puts(skill_table[obj->value[3]].name, ch);
		    	char_puts("'", ch);
		    }

		    char_puts(".\n\r", ch);
			break;

		case ITEM_DRINK_CON:
		    char_printf(ch,"It holds %s-colored %s.\n\r",
			liq_table[obj->value[2]].liq_color,
			liq_table[obj->value[2]].liq_name);
		    break;
			
	  
		case ITEM_WEAPON:
		    char_puts("Weapon type is ",ch);
		    switch (obj->value[0])
		    {
		    	case(WEAPON_EXOTIC): 
			    char_puts("exotic\n\r",ch);
			    break;
		    	case(WEAPON_SWORD): 
			    char_puts("sword\n\r",ch);
			    break;	
		    	case(WEAPON_DAGGER): 
			    char_puts("dagger\n\r",ch);
			    break;
		    	case(WEAPON_SPEAR):
			    char_puts("spear/staff\n\r",ch);
			    break;
		    	case(WEAPON_MACE): 
			    char_puts("mace/club\n\r",ch);	
			    break;
		   	case(WEAPON_AXE): 
			    char_puts("axe\n\r",ch);	
			    break;
		    	case(WEAPON_FLAIL): 
			    char_puts("flail\n\r",ch);
			    break;
		    	case(WEAPON_WHIP): 
			    char_puts("whip\n\r",ch);
			    break;
		    	case(WEAPON_POLEARM): 
			    char_puts("polearm\n\r",ch);
			    break;
		    	case(WEAPON_BOW): 
			    char_puts("bow\n\r",ch);
			    break;
		    	case(WEAPON_ARROW): 
			    char_puts("arrow\n\r",ch);
			    break;
		    	case(WEAPON_LANCE): 
			    char_puts("lance\n\r",ch);
			    break;
		    	default: 
			    char_puts("unknown\n\r",ch);
			    break;
		    }
		    if (obj->pIndexData->new_format)
		    	char_printf(ch,"Damage is %dd%d (average %d)\n\r",
			    obj->value[1],obj->value[2],
			    (1 + obj->value[2]) * obj->value[1] / 2);
		    else
		    	char_printf(ch, "Damage is %d to %d (average %d)\n\r",
		    	    obj->value[1], obj->value[2],
		    	    (obj->value[1] + obj->value[2]) / 2);

		    char_printf(ch,"Damage noun is %s.\n\r",
			attack_table[obj->value[3]].noun);
		    
		    if (obj->value[4])  /* weapon flags */
		        char_printf(ch,"Weapons flags: %s\n\r",
			    weapon_bit_name(obj->value[4]));
		break;

		case ITEM_ARMOR:
		    char_printf(ch, 
		    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
		        obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
		break;

	    case ITEM_CONTAINER:
	        char_printf(ch,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	            obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
	        if (obj->value[4] != 100)
	            char_printf(ch,"Weight multiplier: %d%%\n\r",
			    obj->value[4]);
	    break;
	}


	if (obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
	{
		EXTRA_DESCR_DATA *ed;

		char_puts("Extra description keywords: '", ch);

		for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
		{
		    char_puts(ed->keyword, ch);
		    if (ed->next != NULL)
		    	char_puts(" ", ch);
		}

		for (ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next)
		{
		    char_puts(ed->keyword, ch);
		    if (ed->next != NULL)
			char_puts(" ", ch);
		}

		char_puts("'\n\r", ch);
	}

	for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
		char_printf(ch, "Affects %s by %d, level %d",
		    affect_loc_name(paf->location), paf->modifier,paf->level);
		if (paf->duration > -1)
		    char_printf(ch,", %d hours.\n\r",paf->duration);
		else
		    char_puts(".\n\r", ch);
		if (paf->bitvector)
		{
		    switch(paf->where)
		    {
			case TO_AFFECTS:
			    char_printf(ch,"Adds %s affect.\n",
				affect_bit_name(paf->bitvector));
			    break;
	            case TO_WEAPON:
	                char_printf(ch,"Adds %s weapon flags.\n",
	                    weapon_bit_name(paf->bitvector));
			    break;
			case TO_OBJECT:
			    char_printf(ch,"Adds %s object flag.\n",
				extra_bit_name(paf->bitvector));
			    break;
			case TO_IMMUNE:
			    char_printf(ch,"Adds immunity to %s.\n",
				imm_bit_name(paf->bitvector));
			    break;
			case TO_RESIST:
			    char_printf(ch,"Adds resistance to %s.\n\r",
				imm_bit_name(paf->bitvector));
			    break;
			case TO_VULN:
			    char_printf(ch,"Adds vulnerability to %s.\n\r",
				imm_bit_name(paf->bitvector));
			    break;
			case TO_DETECTS:
			    char_printf(ch,"Adds %s detection.\n\r",
				detect_bit_name(paf->bitvector));
			    break;
			default:
			    char_printf(ch,"Unknown bit %d: %d\n\r",
				paf->where,paf->bitvector);
			    break;
		    }
		}
	}

	if (!obj->enchanted)
	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	{
		char_printf(ch, "Affects %s by %d, level %d.\n\r",
		    affect_loc_name(paf->location), paf->modifier,paf->level);
	    if (paf->bitvector)
	    {
	        switch(paf->where)
	        {
	            case TO_AFFECTS:
	                char_printf(ch,"Adds %s affect.\n",
	                    affect_bit_name(paf->bitvector));
	                break;
	            case TO_OBJECT:
	                char_printf(ch,"Adds %s object flag.\n",
	                    extra_bit_name(paf->bitvector));
	                break;
	            case TO_IMMUNE:
	                char_printf(ch,"Adds immunity to %s.\n",
	                    imm_bit_name(paf->bitvector));
	                break;
	            case TO_RESIST:
	                char_printf(ch,"Adds resistance to %s.\n\r",
	                    imm_bit_name(paf->bitvector));
	                break;
	            case TO_VULN:
	                char_printf(ch,"Adds vulnerability to %s.\n\r",
	                    imm_bit_name(paf->bitvector));
	                break;
	            case TO_DETECTS:
	                char_printf(ch,"Adds %s detection.\n\r",
	                    detect_bit_name(paf->bitvector));
	                break;
	            default:
	                char_printf(ch,"Unknown bit %d: %d\n\r",
	                    paf->where,paf->bitvector);
	                break;
	        }
	    }
	}

	output = buf_new(0);
	buf_add(output, "Object progs:\n");
	for (i = 0; i < OPROG_MAX; i++)
		if (obj->pIndexData->oprogs[i] != NULL)
			buf_printf(output, "%s: %s\n",
				 optype_table[i],
				 oprog_name_lookup(obj->pIndexData->oprogs[i]));
	char_puts(buf_string(output), ch);
	buf_free(output);

	char_printf(ch,"Damage condition : %d (%s)\n\r", obj->condition,
				get_cond_alias(obj, ch));	
	return;
}



void do_mstat(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	CHAR_DATA *victim;
	BUFFER *output;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Stat whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	output = buf_new(0);

	buf_printf(output, "Name: [%s] Reset Zone: %s\n\r", victim->name,
		(IS_NPC(victim) &&victim->zone) ? victim->zone->name : "?");

	buf_printf(output, 
		"Vnum: %d  Format: %s  Race: %s(%s)  Group: %d  Sex: %s  Room: %d\n\r",
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
		race_table[RACE(victim)].name,race_table[ORG_RACE(victim)].name,
		IS_NPC(victim) ? victim->group : 0, sex_flags[victim->sex].name,
		victim->in_room == NULL    ?        0 : victim->in_room->vnum);

	if (IS_NPC(victim))
		buf_printf(output,"Count: %d  Killed: %d  ---  Status: %d\n\r",
		    victim->pIndexData->count,victim->pIndexData->killed, victim->status);

	buf_printf(output, "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d) Cha: %d(%d)\n\r",
		victim->perm_stat[STAT_STR],
		get_curr_stat(victim,STAT_STR),
		victim->perm_stat[STAT_INT],
		get_curr_stat(victim,STAT_INT),
		victim->perm_stat[STAT_WIS],
		get_curr_stat(victim,STAT_WIS),
		victim->perm_stat[STAT_DEX],
		get_curr_stat(victim,STAT_DEX),
		victim->perm_stat[STAT_CON],
		get_curr_stat(victim,STAT_CON),
		victim->perm_stat[STAT_CHA],
		get_curr_stat(victim,STAT_CHA));


	buf_printf(output, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n\r",
		victim->hit,         victim->max_hit,
		victim->mana,        victim->max_mana,
		victim->move,        victim->max_move,
		IS_NPC(ch) ? 0 : victim->practice);
		
	buf_printf(output,"It belives the religion of %s.\n\r",
		IS_NPC(victim) ? "Chronos" : religion_table[victim->religion].leader);

	if (IS_NPC(victim))
		sprintf(buf, "%d", victim->alignment);
	else  {
		sprintf(buf, "%s%s", 
			victim->ethos==1 ? "Law-":
			victim->ethos==2 ? "Neut-":
			victim->ethos==3 ? "Cha-": "none-",
			IS_GOOD(victim)		? "Good" :
			IS_NEUTRAL(victim)	? "Neut" :
			IS_EVIL(victim)		? "Evil" : "Other");
	}

	buf_printf(output,
		"Lv: %d  Class: %s  Align: %s  Gold: %ld  Silver: %ld  Exp: %d\n\r",
		victim->level,       
		IS_NPC(victim) ? "mobile" : class_table[victim->class].name,            
		buf,
		victim->gold, victim->silver, victim->exp);

	buf_printf(output,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
		    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
		    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));

	buf_printf(output, 
		"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
		GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
		size_table[victim->size].name, position_table[victim->position].name,
		victim->wimpy);

	if (IS_NPC(victim) && victim->pIndexData->new_format)
		buf_printf(output, "Damage: %dd%d  Message:  %s\n\r",
		    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
		    attack_table[victim->dam_type].noun);

	buf_printf(output, "Fighting: %s Deaths: %d Carry number: %d  Carry weight: %ld\n\r",
		victim->fighting ? victim->fighting->name : "(none)" ,
		IS_NPC(victim) ? 0 : victim->pcdata->death,
		victim->carry_number, get_carry_weight(victim) / 10);

	if (!IS_NPC(victim)) {
		buf_printf(output,
"Thirst: %d  Hunger: %d  Full: %d  Drunk: %d Bloodlust: %d Desire: %d\n\r",
		    victim->pcdata->condition[COND_THIRST],
		    victim->pcdata->condition[COND_HUNGER],
		    victim->pcdata->condition[COND_FULL],
		    victim->pcdata->condition[COND_DRUNK],
		    victim->pcdata->condition[COND_BLOODLUST],
		    victim->pcdata->condition[COND_DESIRE]);
		buf_printf(output, 
		    "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
		    get_age(victim), 
		    (int) (victim->played + current_time - victim->logon) / 3600, 
		    victim->pcdata->last_level, 
		    victim->timer);
	}

	buf_printf(output, "Act: %s\n\r",act_bit_name(victim->act));
	
	if (victim->comm)
		buf_printf(output, "Comm: %s\n\r",comm_bit_name(victim->comm));

	if (IS_NPC(victim) && victim->off_flags)
		buf_printf(output, "Offense: %s\n\r",off_bit_name(victim->off_flags));

	if (victim->imm_flags)
		buf_printf(output, "Immune: %s\n\r",imm_bit_name(victim->imm_flags));
	
	if (victim->res_flags)
		buf_printf(output, "Resist: %s\n\r", imm_bit_name(victim->res_flags));

	if (victim->vuln_flags)
		buf_printf(output, "Vulnerable: %s\n\r", imm_bit_name(victim->vuln_flags));

	if (victim->detection)
		buf_printf(output, "Detection: %s\n\r", 
		(victim->detection) ? detect_bit_name(victim->detection) : "(none)");

	buf_printf(output, "Form: %s\n\r", form_bit_name(victim->form));
	buf_printf(output, "Parts: %s\n\r", part_bit_name(victim->parts));

	if (victim->affected_by)
		buf_printf(output, "Affected by %s\n\r", 
		    affect_bit_name(victim->affected_by));

	buf_printf(output, "Master: %s  Leader: %s  Pet: %s\n\r",
		victim->master      ? victim->master->name   : "(none)",
		victim->leader      ? victim->leader->name   : "(none)",
		victim->pet 	    ? victim->pet->name	     : "(none)");

	/* OLC */
	if (!IS_NPC(victim))
		buf_printf(output, "Security: %d.\n\r", victim->pcdata->security);

	buf_printf(output, "Short description: %s\n\r", victim->short_descr);
	if (IS_NPC(victim))
		mlstr_buf(output, "Long description: ", victim->long_descr);

	if (IS_NPC(victim) && victim->spec_fun != 0)
		buf_printf(output, "Mobile has special procedure %s.\n\r",
			spec_name(victim->spec_fun));

	for (paf = victim->affected; paf != NULL; paf = paf->next)
		buf_printf(output,
		    "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
		    skill_table[(int) paf->type].name,
		    affect_loc_name(paf->location),
		    paf->modifier,
		    paf->duration,
		    affect_bit_name(paf->bitvector),
		    paf->level
		   );

	if (!IS_NPC(victim)) {
		if (IS_ON_QUEST(victim)) {
		 buf_printf(output,"Questgiver: %d QuestPnts: %d	Questnext: %d\n\r",
			victim->pcdata->questgiver,victim->pcdata->questpoints,
			victim->pcdata->questtime < 0 ? -victim->pcdata->questtime : 0);
		 buf_printf(output,"QuestCntDown: %d	QuestObj: %d	Questmob: %d\n\r",
			victim->pcdata->questtime > 0 ? victim->pcdata->questtime : 0,
			victim->pcdata->questobj, victim->pcdata->questmob);
		}
		if  (!IS_ON_QUEST(victim))
		 buf_printf(output,"QuestPnts: %d	Questnext: %d    NOT QUESTING\n\r",
			victim->pcdata->questpoints,
			victim->pcdata->questtime < 0 ? -victim->pcdata->questtime : 0);
	   }	

	buf_printf(output, "Last fought: %10s  Last fight time: %s  Pumped: %d", 
		victim->last_fought!=NULL?victim->last_fought->name:"none", 
		ctime(&(victim->last_fight_time)),
		victim->pumped);
	buf_printf(output, "In_mind: [%s]\n\r", 
			victim->in_mind != NULL ? victim->in_mind : "none");

	char_puts(buf_string(output), ch);
	buf_free(output);
}

void do_vnum(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	const char *string;

	string = one_argument(argument, arg);
	
	if (arg[0] == '\0') {
		char_puts("Syntax:\n\r",ch);
		char_puts("  vnum obj <name>\n\r",ch);
		char_puts("  vnum mob <name>\n\r",ch);
		return;
	}

	if (!str_cmp(arg, "obj")) {
		do_ofind(ch, string);
		return;
	}

	if (!str_cmp(arg, "mob") || !str_cmp(arg, "char")) { 
		do_mfind(ch, string);
		return;
	}

	/* do both */
	do_mfind(ch, argument);
	do_ofind(ch, argument);
}


void do_mfind(CHAR_DATA *ch, const char *argument)
{
	extern int top_mob_index;
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	int vnum;
	int nMatch;
	bool fAll;
	bool found;

	one_argument(argument, arg);
	if (arg[0] == '\0')
	{
		char_puts("Find whom?\n\r", ch);
		return;
	}

	fAll	= FALSE; /* !str_cmp(arg, "all"); */
	found	= FALSE;
	nMatch	= 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_mob_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_mob_index; vnum++)
	{
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
		{
		    nMatch++;
		    if (fAll || is_name(argument, pMobIndex->player_name))
		    {
			found = TRUE;
			char_printf(ch, "[%5d] %s\n\r",
			    pMobIndex->vnum, pMobIndex->short_descr);
		    }
		}
	}

	if (!found)
		char_puts("No mobiles by that name.\n\r", ch);

	return;
}



void do_ofind(CHAR_DATA *ch, const char *argument)
{
	extern int top_obj_index;
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;
	int nMatch;
	bool fAll;
	bool found;

	one_argument(argument, arg);
	if (arg[0] == '\0')
	{
		char_puts("Find what?\n\r", ch);
		return;
	}

	fAll	= FALSE; /* !str_cmp(arg, "all"); */
	found	= FALSE;
	nMatch	= 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_obj_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_obj_index; vnum++)
	{
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
		    nMatch++;
		    if (fAll || is_name(argument, pObjIndex->name))
		    {
			found = TRUE;
			char_printf(ch, "[%5d] %s\n\r",
			    pObjIndex->vnum, pObjIndex->short_descr);
		    }
		}
	}

	if (!found)
		char_puts("No objects by that name.\n\r", ch);

	return;
}


void do_owhere(CHAR_DATA *ch, const char *argument)
{
	BUFFER *buffer = NULL;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int number = 0, max_found = 200;

	if (argument[0] == '\0') {
		char_puts("Find what?\n\r",ch);
		return;
	}
	
	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!can_see_obj(ch, obj) || !is_name(argument, obj->name)
		||  ch->level < obj->level)
	        	continue;
	
		if (buffer == NULL)
			buffer = buf_new(0);
		number++;
	
		for (in_obj = obj; in_obj->in_obj != NULL;
		     in_obj = in_obj->in_obj)
	        	;
	
		if (in_obj->carried_by != NULL
		&&  can_see(ch,in_obj->carried_by)
		&&  in_obj->carried_by->in_room != NULL)
			buf_printf(buffer,
				   "%3d) %s is carried by %s [Room %d]\n\r",
				number,
				obj->short_descr,PERS(in_obj->carried_by, ch),
				in_obj->carried_by->in_room->vnum);
		else if (in_obj->in_room != NULL
		     &&  can_see_room(ch, in_obj->in_room))
	        	buf_printf(buffer, "%3d) %s is in %s [Room %d]\n\r",
	        		number, obj->short_descr,
				mlstr_val(ch, in_obj->in_room->name), 
				in_obj->in_room->vnum);
		else
			buf_printf(buffer, "%3d) %s is somewhere\n\r",number, obj->short_descr);
	
	    if (number >= max_found)
	        break;
	}
	
	if (buffer == NULL)
		char_puts("Nothing like that in heaven or earth.\n\r", ch);
	else {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
}


void do_mwhere(CHAR_DATA *ch, const char *argument)
{
	BUFFER *buffer;
	CHAR_DATA *victim;
	int count = 0;

	if (argument[0] == '\0') {
		DESCRIPTOR_DATA *d;

		/* show characters logged */

		buffer = buf_new(0);
		for (d = descriptor_list; d != NULL; d = d->next)
		{
		    if (d->character != NULL && d->connected == CON_PLAYING
		    &&  d->character->in_room != NULL && can_see(ch,d->character)
		    &&  can_see_room(ch,d->character->in_room))
		    {
			victim = d->character;
			count++;
			if (d->original != NULL)
			    buf_printf(buffer,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
				count, d->original->name,victim->short_descr,
				mlstr_val(ch, victim->in_room->name),
				victim->in_room->vnum);
			else
			    buf_printf(buffer,"%3d) %s is in %s [%d]\n\r",
				count, victim->name,
				mlstr_val(ch, victim->in_room->name),
				victim->in_room->vnum);
		    }
		}

	    page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
		return;
	}

	buffer = NULL;
	for (victim = char_list; victim != NULL; victim = victim->next)
	{
		if (victim->in_room != NULL
		&&   is_name(argument, victim->name))
		{
			if (buffer == NULL)
				buffer = buf_new(0);

		    count++;
		    buf_printf(buffer, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
			IS_NPC(victim) ? victim->pIndexData->vnum : 0,
			IS_NPC(victim) ? victim->short_descr : victim->name,
			victim->in_room->vnum,
			mlstr_val(ch, victim->in_room->name));
		}
	}

	if (buffer == NULL)
		act("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
	else {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
}



void do_reboo(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to REBOOT, spell it out.\n\r", ch);
	return;
}



void do_shutdow(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to SHUTDOWN, spell it out.\n\r", ch);
	return;
}



void do_shutdown(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	extern bool merc_down;
	DESCRIPTOR_DATA *d,*d_next;

	if (ch->invis_level < LEVEL_HERO)
	snprintf(buf, sizeof(buf), "Shutdown by %s.", ch->name);
	append_file(ch, SHUTDOWN_FILE, buf);
	strcat(buf, "\n\r");
	if (ch->invis_level < LEVEL_HERO)
		do_echo(ch, buf);
	do_force (ch, "all save");
	do_save (ch, "");
	merc_down = TRUE;
	for (d = descriptor_list; d != NULL; d = d_next)
	{
		d_next = d->next;
		close_socket(d);
	}
	return;
}

void do_protect(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;

	if (argument[0] == '\0') {
		char_puts("Protect whom from snooping?\n\r",ch);
		return;
	}

	if ((victim = get_char_world(ch,argument)) == NULL)
	{
		char_puts("You can't find them.\n\r",ch);
		return;
	}

	if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
	{
		act_puts("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
		char_puts("Your snoop-proofing was just removed.\n\r",victim);
		REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
	}
	else
	{
		act_puts("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
		char_puts("You are now immune to snooping.\n\r",victim);
		SET_BIT(victim->comm,COMM_SNOOP_PROOF);
	}
}
	


void do_snoop(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		char_puts("Snoop whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim->desc == NULL)
	{
		char_puts("No descriptor to snoop.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		char_puts("Cancelling all snoops.\n\r", ch);
		wiznet("$N stops being such a snoop.",
			ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
		for (d = descriptor_list; d != NULL; d = d->next)
		{
		    if (d->snoop_by == ch->desc)
			d->snoop_by = NULL;
		}
		return;
	}

	if (victim->desc->snoop_by != NULL)
	{
		char_puts("Busy already.\n\r", ch);
		return;
	}

	if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
	&&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
	{
	    char_puts("That character is in a private room.\n\r",ch);
	    return;
	}

	if (get_trust(victim) >= get_trust(ch) 
	||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
	{
		char_puts("You failed.\n\r", ch);
		return;
	}

	if (ch->desc != NULL)
	{
		for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by)
		{
		    if (d->character == victim || d->original == victim)
		    {
			char_puts("No snoop loops.\n\r", ch);
			return;
		    }
		}
	}

	victim->desc->snoop_by = ch->desc;
	wiznet_printf(ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch),
		      "$N starts snooping on %s",
		      (IS_NPC(ch) ? victim->short_descr : victim->name));
	char_nputs(OK, ch);
	return;
}



void do_switch(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);
	
	if (arg[0] == '\0')
	{
		char_puts("Switch into whom?\n\r", ch);
		return;
	}

	if (ch->desc == NULL)
		return;
	
	if (ch->desc->original != NULL)
	{
		char_puts("You are already switched.\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		char_nputs(OK, ch);
		return;
	}

	if (!IS_NPC(victim))
	{
		char_puts("You can only switch into mobiles.\n\r",ch);
		return;
	}

	if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
	&&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
	{
		char_puts("That character is in a private room.\n\r",ch);
		return;
	}

	if (victim->desc != NULL)
	{
		char_puts("Character in use.\n\r", ch);
		return;
	}

	wiznet_printf(ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch),
		      "$N switches into %s",victim->short_descr);

	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc        = ch->desc;
	ch->desc            = NULL;
	/* change communications to match */
	if (ch->prompt != NULL)
	    victim->prompt = str_dup(ch->prompt);
	victim->comm = ch->comm;
	victim->lines = ch->lines;
	char_nputs(OK, victim);
	return;
}



void do_return(CHAR_DATA *ch, const char *argument)
{
	if (ch->desc == NULL)
		return;

	if (ch->desc->original == NULL)
	{
		char_puts("You aren't switched.\n\r", ch);
		return;
	}

	char_puts(
"You return to your original body. Type replay to see any missed tells.\n\r", 
		ch);
	if (ch->prompt != NULL)
	{
		free_string(ch->prompt);
		ch->prompt = NULL;
	}

	wiznet_printf(ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,
			get_trust(ch), "$N returns from %s.",ch->short_descr);
	ch->desc->character       = ch->desc->original;
	ch->desc->original        = NULL;
	ch->desc->character->desc = ch->desc; 
	ch->desc                  = NULL;
	return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (IS_TRUSTED(ch,GOD)
		|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
		|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 10 && obj->cost <= 500)
		|| (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
		|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
		return TRUE;
	else
		return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
	OBJ_DATA *c_obj, *t_obj;


	for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
	{
		if (obj_check(ch,c_obj))
		{
		    t_obj = create_object(c_obj->pIndexData,0);
		    clone_object(c_obj,t_obj);
		    obj_to_obj(t_obj,clone);
		    recursive_clone(ch,c_obj,t_obj);
		}
	}
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	const char *rest;
	CHAR_DATA *mob;
	OBJ_DATA  *obj;

	rest = one_argument(argument,arg);

	if (arg[0] == '\0')
	{
		char_puts("Clone what?\n\r",ch);
		return;
	}

	if (!str_prefix(arg,"object"))
	{
		mob = NULL;
		obj = get_obj_here(ch,rest);
		if (obj == NULL)
		{
		    char_puts("You don't see that here.\n\r",ch);
		    return;
		}
	}
	else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
	{
		obj = NULL;
		mob = get_char_room(ch,rest);
		if (mob == NULL)
		{
		    char_puts("You don't see that here.\n\r",ch);
		    return;
		}
	}
	else /* find both */
	{
		mob = get_char_room(ch,argument);
		obj = get_obj_here(ch,argument);
		if (mob == NULL && obj == NULL)
		{
		    char_puts("You don't see that here.\n\r",ch);
		    return;
		}
	}

	/* clone an object */
	if (obj != NULL)
	{
		OBJ_DATA *clone;

		if (!obj_check(ch,obj))
		{
		    char_puts(
			"Your powers are not great enough for such a task.\n\r",ch);
		    return;
		}

		clone = create_object(obj->pIndexData,0); 
		clone_object(obj,clone);
		if (obj->carried_by != NULL)
		    obj_to_char(clone,ch);
		else
		    obj_to_room(clone,ch->in_room);
		recursive_clone(ch,obj,clone);

		act("$n has created $p.",ch,clone,NULL,TO_ROOM);
		act("You clone $p.",ch,clone,NULL,TO_CHAR);
		wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
		return;
	}
	else if (mob != NULL)
	{
		CHAR_DATA *clone;
		OBJ_DATA *new_obj;

		if (!IS_NPC(mob))
		{
		    char_puts("You can only clone mobiles.\n\r",ch);
		    return;
		}

		if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
		||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
		||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
		||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
		||  !IS_TRUSTED(ch,AVATAR))
		{
		    char_puts(
			"Your powers are not great enough for such a task.\n\r",ch);
		    return;
		}

		clone = create_mobile(mob->pIndexData);
		clone_mobile(mob,clone); 
		
		for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
		{
		    if (obj_check(ch,obj))
		    {
			new_obj = create_object(obj->pIndexData,0);
			clone_object(obj,new_obj);
			recursive_clone(ch,obj,new_obj);
			obj_to_char(new_obj,clone);
			new_obj->wear_loc = obj->wear_loc;
		    }
		}
		char_to_room(clone,ch->in_room);
	    act("$n has created $N.",ch,NULL,clone,TO_ROOM);
	    act("You clone $N.",ch,NULL,clone,TO_CHAR);
		wiznet_printf(ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch),
			"$N clones %s.",clone->short_descr);
	    return;
	}
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, const char *argument)
{
	 char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument,arg);

	if (arg[0] == '\0')
	{
		char_puts("Syntax:\n\r",ch);
		char_puts("  load mob <vnum>\n\r",ch);
		char_puts("  load obj <vnum> <level>\n\r",ch);
		return;
	}

	if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
	{
		do_mload(ch,argument);
		return;
	}

	if (!str_cmp(arg,"obj"))
	{
		do_oload(ch,argument);
		return;
	}
	/* echo syntax */
	do_load(ch,"");
}


void do_mload(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;
	
	one_argument(argument, arg);

	if (arg[0] == '\0' || !is_number(arg))
	{
		char_puts("Syntax: load mob <vnum>.\n\r", ch);
		return;
	}

	if ((pMobIndex = get_mob_index(atoi(arg))) == NULL)
	{
		char_puts("No mob has that vnum.\n\r", ch);
		return;
	}

	victim = create_mobile(pMobIndex);
	char_to_room(victim, ch->in_room);
	act("$n has created $N!", ch, NULL, victim, TO_ROOM);
	wiznet_printf(ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch),
			"$N loads %s.",victim->short_descr);
	char_nputs(OK, ch);
	return;
}



void do_oload(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	int level;
	
	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);

	if (arg1[0] == '\0' || !is_number(arg1))
	{
		char_puts("Syntax: load obj <vnum> <level>.\n\r", ch);
		return;
	}
	
	level = get_trust(ch); /* default */
	
	if (arg2[0] != '\0')  /* load with a level */
	{
		if (!is_number(arg2))
	    {
		  char_puts("Syntax: oload <vnum> <level>.\n\r", ch);
		  return;
		}
	    level = atoi(arg2);
	    if (level < 0 || level > get_trust(ch))
		{
		  char_puts("Level must be be between 0 and your level.\n\r",ch);
		  return;
		}
	}

	if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL)
	{
		char_puts("No object has that vnum.\n\r", ch);
		return;
	}

	obj = create_object(pObjIndex, level);
	if (CAN_WEAR(obj, ITEM_TAKE))
		obj_to_char(obj, ch);
	else
		obj_to_room(obj, ch->in_room);
	act("$n has created $p!", ch, obj, NULL, TO_ROOM);
	wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	char_nputs(OK, ch);
	return;
}



void do_purge(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	DESCRIPTOR_DATA *d;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		/* 'purge' */
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for (victim = ch->in_room->people; victim != NULL; victim = vnext)
		{
		    vnext = victim->next_in_room;
		    if (IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
		    &&   victim != ch /* safety precaution */)
			extract_char(victim, TRUE);
		}

		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
		{
		    obj_next = obj->next_content;
		    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
		      extract_obj(obj);
		}

		act("$n purges the room!", ch, NULL, NULL, TO_ROOM);
		char_nputs(OK, ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (!IS_NPC(victim))
	{

		if (ch == victim)
		{
		  char_puts("Ho ho ho.\n\r",ch);
		  return;
		}

		if (get_trust(ch) <= get_trust(victim))
		{
		  char_puts("Maybe that wasn't a good idea...\n\r",ch);
		  char_printf(ch,"%s tried to purge you!\n\r",ch->name);
		  return;
		}

		act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

		if (victim->level > 1)
		    save_char_obj(victim, FALSE);
		d = victim->desc;
		extract_char(victim, TRUE);
		if (d != NULL)
	      close_socket(d);

		return;
	}

	act("$n purges $N.", ch, NULL, victim, TO_NOTVICT);
	extract_char(victim, TRUE);
	return;
}


void do_trust(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2))
	{
		char_puts("Syntax: trust <char> <level>.\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		char_puts("That player is not here.\n\r", ch);
		return;
	}

	if ((level = atoi(arg2)) < 0 || level > 100)
	{
		char_puts("Level must be 0 (reset) or 1 to 100.\n\r", ch);
		return;
	}

	if (level > get_trust(ch))
	{
		char_puts("Limited to your trust.\n\r", ch);
		return;
	}

	victim->trust = level;
	return;
}



void do_restore(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *vch;
	DESCRIPTOR_DATA *d;

	one_argument(argument, arg);
	if (arg[0] == '\0' || !str_cmp(arg,"room"))
	{
	/* cure room */
		
	    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	    {
	        affect_strip(vch,gsn_plague);
	        affect_strip(vch,gsn_poison);
	        affect_strip(vch,gsn_blindness);
	        affect_strip(vch,gsn_sleep);
	        affect_strip(vch,gsn_curse);
	        
	        vch->hit 	= vch->max_hit;
	        vch->mana	= vch->max_mana;
	        vch->move	= vch->max_move;
	        update_pos(vch);
	        act("$n has restored you.",ch,NULL,vch,TO_VICT);
	    }

	    wiznet_printf(ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch),
	    		"$N restored room %d.",ch->in_room->vnum);
	    
	    char_puts("Room restored.\n\r",ch);
	    return;

	}
	
	if (get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
	{
	/* cure all */
		
	    for (d = descriptor_list; d != NULL; d = d->next)
	    {
		    victim = d->character;

		    if (victim == NULL || IS_NPC(victim))
			continue;
	            
	        affect_strip(victim,gsn_plague);
	        affect_strip(victim,gsn_poison);
	        affect_strip(victim,gsn_blindness);
	        affect_strip(victim,gsn_sleep);
	        affect_strip(victim,gsn_curse);
	        
	        victim->hit 	= victim->max_hit;
	        victim->mana	= victim->max_mana;
	        victim->move	= victim->max_move;
	        update_pos(victim);
		    if (victim->in_room != NULL)
	            act("$n has restored you.",ch,NULL,victim,TO_VICT);
	    }
		char_puts("All active players restored.\n\r",ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	affect_strip(victim,gsn_plague);
	affect_strip(victim,gsn_poison);
	affect_strip(victim,gsn_blindness);
	affect_strip(victim,gsn_sleep);
	affect_strip(victim,gsn_curse);
	victim->hit  = victim->max_hit;
	victim->mana = victim->max_mana;
	victim->move = victim->max_move;
	update_pos(victim);
	act("$n has restored you.", ch, NULL, victim, TO_VICT);
	wiznet_printf(ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch),
			"$N restored %s",
		IS_NPC(victim) ? victim->short_descr : victim->name);
	char_nputs(OK, ch);
	return;
}

		
void do_freeze(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		char_puts("Freeze whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		char_puts("Not on NPC's.\n\r", ch);
		return;
	}

	if (get_trust(victim) >= get_trust(ch))
	{
		char_puts("You failed.\n\r", ch);
		return;
	}

	if (IS_SET(victim->act, PLR_FREEZE))
	{
		REMOVE_BIT(victim->act, PLR_FREEZE);
		char_puts("You can play again.\n\r", victim);
		char_puts("FREEZE removed.\n\r", ch);
		sprintf(buf,"$N thaws %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->act, PLR_FREEZE);
		char_puts("You can't do ANYthing!\n\r", victim);
		char_puts("FREEZE set.\n\r", ch);
		sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}

	save_char_obj(victim, FALSE);
}



void do_log(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		char_puts("Log whom?\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "all"))
	{
		if (fLogAll)
		{
		    fLogAll = FALSE;
		    char_puts("Log ALL off.\n\r", ch);
		}
		else
		{
		    fLogAll = TRUE;
		    char_puts("Log ALL on.\n\r", ch);
		}
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		char_puts("Not on NPC's.\n\r", ch);
		return;
	}

	/*
	 * No level check, gods can log anyone.
	 */
	if (IS_SET(victim->act, PLR_LOG))
	{
		REMOVE_BIT(victim->act, PLR_LOG);
		char_puts("LOG removed.\n\r", ch);
	}
	else
	{
		SET_BIT(victim->act, PLR_LOG);
		char_puts("LOG set.\n\r", ch);
	}

	return;
}



void do_noemote(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		char_puts("Noemote whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}


	if (get_trust(victim) >= get_trust(ch))
	{
		char_puts("You failed.\n\r", ch);
		return;
	}

	if (IS_SET(victim->comm, COMM_NOEMOTE))
	{
		REMOVE_BIT(victim->comm, COMM_NOEMOTE);
		char_puts("You can emote again.\n\r", victim);
		char_puts("NOEMOTE removed.\n\r", ch);
		sprintf(buf,"$N restores emotes to %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->comm, COMM_NOEMOTE);
		char_puts("You can't emote!\n\r", victim);
		char_puts("NOEMOTE set.\n\r", ch);
		sprintf(buf,"$N revokes %s's emotes.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}

	return;
}



void do_noshout(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		char_puts("Noshout whom?\n\r",ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		char_puts("Not on NPC's.\n\r", ch);
		return;
	}

	if (get_trust(victim) >= get_trust(ch))
	{
		char_puts("You failed.\n\r", ch);
		return;
	}

	if (IS_SET(victim->comm, COMM_NOSHOUT))
	{
		REMOVE_BIT(victim->comm, COMM_NOSHOUT);
		char_puts("You can shout again.\n\r", victim);
		char_puts("NOSHOUT removed.\n\r", ch);
		sprintf(buf,"$N restores shouts to %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->comm, COMM_NOSHOUT);
		char_puts("You can't shout!\n\r", victim);
		char_puts("NOSHOUT set.\n\r", ch);
		sprintf(buf,"$N revokes %s's shouts.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}

	return;
}



void do_notell(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		char_puts("Notell whom?", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (get_trust(victim) >= get_trust(ch))
	{
		char_puts("You failed.\n\r", ch);
		return;
	}

	if (IS_SET(victim->comm, COMM_NOTELL))
	{
		REMOVE_BIT(victim->comm, COMM_NOTELL);
		char_puts("You can tell again.\n\r", victim);
		char_puts("NOTELL removed.\n\r", ch);
		sprintf(buf,"$N restores tells to %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->comm, COMM_NOTELL);
		char_puts("You can't tell!\n\r", victim);
		char_puts("NOTELL set.\n\r", ch);
		sprintf(buf,"$N revokes %s's tells.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}

	return;
}



void do_peace(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *rch;

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (rch->fighting != NULL)
		    stop_fighting(rch, TRUE);
		if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
		    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
	}

	char_nputs(OK, ch);
	return;
}

void do_wizlock(CHAR_DATA *ch, const char *argument)
{
	extern bool wizlock;
	wizlock = !wizlock;

	if (wizlock)
	{
		wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
		char_puts("Game wizlocked.\n\r", ch);
	}
	else
	{
		wiznet("$N removes wizlock.",ch,NULL,0,0,0);
		char_puts("Game un-wizlocked.\n\r", ch);
	}

	return;
}

/* RT anti-newbie code */

void do_newlock(CHAR_DATA *ch, const char *argument)
{
	extern bool newlock;
	newlock = !newlock;
	
	if (newlock)
	{
		wiznet("$N locks out new characters.",ch,NULL,0,0,0);
	    char_puts("New characters have been locked out.\n\r", ch);
	}
	else
	{
		wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
	    char_puts("Newlock removed.\n\r", ch);
	}
	
	return;
}


void do_slookup(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int sn;

	one_argument(argument, arg);
	if (arg[0] == '\0')
	{
		char_puts("Lookup which skill or spell?\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "all"))
	{
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
		    if (skill_table[sn].name == NULL)
			break;
		    sprintf(buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
			sn, skill_table[sn].slot, skill_table[sn].name);
		    char_puts(buf, ch);
		}
	}
	else
	{
		if ((sn = skill_lookup(arg)) < 0)
		{
		    char_puts("No such skill or spell.\n\r", ch);
		    return;
		}

		sprintf(buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		    sn, skill_table[sn].slot, skill_table[sn].name);
		char_puts(buf, ch);
	}

	return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument,arg);

	if (arg[0] == '\0')
	{
		char_puts("Syntax:\n\r",ch);
		char_puts("  set mob   <name> <field> <value>\n\r",ch);
		char_puts("  set obj   <name> <field> <value>\n\r",ch);
		char_puts("  set room  <room> <field> <value>\n\r",ch);
	    char_puts("  set skill <name> <spell or skill> <value>\n\r",ch);
		return;
	}

	if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
	{
		do_mset(ch,argument);
		return;
	}

	if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
	{
		do_sset(ch,argument);
		return;
	}

	if (!str_prefix(arg,"object"))
	{
		do_oset(ch,argument);
		return;

	}

	if (!str_prefix(arg,"room"))
	{
		do_rset(ch,argument);
		return;
	}
	/* echo syntax */
	do_set(ch,"");
}


void do_sset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value;
	int sn;
	bool fAll;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		char_puts("Syntax:\n\r",ch);
		char_puts("  set skill <name> <spell or skill> <value>\n\r", ch);
		char_puts("  set skill <name> all <value>\n\r",ch);  
		char_puts("   (use the name of the skill, not the number)\n\r",ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		char_puts("Not on NPC's.\n\r", ch);
		return;
	}

	fAll = !str_cmp(arg2, "all");
	sn   = 0;
	if (!fAll && (sn = skill_lookup(arg2)) < 0)
	{
		char_puts("No such skill or spell.\n\r", ch);
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3))
	{
		char_puts("Value must be numeric.\n\r", ch);
		return;
	}

	value = atoi(arg3);
	if (value < 0 || value > 100)
	{
		char_puts("Value range is 0 to 100.\n\r", ch);
		return;
	}

	if (fAll)
	{
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
		    if ((skill_table[sn].name != NULL) 
			&& ((victim->clan == skill_table[sn].clan) 
			|| (skill_table[sn].clan == CLAN_NONE)) 
			&& (SKILL_RACE_OK(victim,sn)) 
			)
			victim->pcdata->learned[sn]	= value;
		}
	}
	else
	{
		victim->pcdata->learned[sn] = value;
	}

	return;
}


void do_string(CHAR_DATA *ch, const char *argument)
{
	char type [MAX_INPUT_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	argument = one_argument(argument, type);
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	strcpy(arg3, argument);
	smash_tilde(arg3);

	if (type[0] == '\0' || arg1[0] == '\0'
	||  arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n\r",ch);
		char_puts("  string char <name> <field> <string>\n\r",ch);
		char_puts("    fields: name short long desc title spec\n\r",ch);
		char_puts("  string obj  <name> <field> <string>\n\r",ch);
		char_puts("    fields: name short long extended\n\r",ch);
		return;
	}
	
	if (!str_prefix(type, "character") || !str_prefix(type, "mobile")) {
		if ((victim = get_char_world(ch, arg1)) == NULL) {
			char_puts("They aren't here.\n\r", ch);
			return;
		}

		/* clear zone for mobs */
		victim->zone = NULL;

		/* string something */

	 	if (!str_prefix(arg2, "name")) {
			if (!IS_NPC(victim)) {
				char_puts("Not on PC's.\n\r", ch);
				return;
			}
			free_string(victim->name);
			victim->name = str_dup(arg3);
			return;
		}
		
		if (!str_prefix(arg2, "short")) {
			free_string(victim->short_descr);
			victim->short_descr = str_dup(arg3);
			return;
		}

		if (!str_prefix(arg2, "desc")) {
			char_puts("Sorry, this option is temporarily "
				  "disabled\n\r", ch);
			return;
		}

		if (!str_prefix(arg2, "long")) {
			if (!IS_NPC(victim)) {
				char_puts("Not on PC's.\n\r", ch);
				return;
			}
			mlstr_change_desc(victim->long_descr, arg3);
			return;
		}

		if (!str_prefix(arg2, "title")) {
			if (IS_NPC(victim)) {
				char_puts("Not on NPC's.\n\r", ch);
				return;
			}

			set_title(victim, arg3);
			return;
		}

		if (!str_prefix(arg2, "spec")) {
			if (!IS_NPC(victim)) {
				char_puts("Not on PC's.\n\r", ch);
				return;
			}

			if ((victim->spec_fun = spec_lookup(arg3)) == 0) {
				char_puts("No such spec fun.\n\r", ch);
				return;
			}

			return;
		}
	}
	
	if (!str_prefix(type,"object")) {
		/* string an obj */
		
	 	if ((obj = get_obj_world(ch, arg1)) == NULL) {
			char_puts("Nothing like that in heaven or earth.\n\r",
				  ch);
			return;
		}
		
		if (!str_prefix(arg2, "name")) {
			free_string(obj->name);
			obj->name = str_dup(arg3);
			return;
		}

		if (!str_prefix(arg2, "short")) {
			free_string(obj->short_descr);
			obj->short_descr = str_dup(arg3);
			return;
		}

		if (!str_prefix(arg2, "long")) {
			free_string(obj->description);
			obj->description = str_dup(arg3);
			return;
		}

		if (!str_prefix(arg2, "ed") || !str_prefix(arg2, "extended")) {
			EXTRA_DESCR_DATA *ed;

			argument = one_argument(argument, arg3);
			if (argument == NULL) {
				char_puts("Syntax: oset <object> ed <keyword> <string>\n\r",
				ch);
				return;
			}


			ed = new_extra_descr();
			ed->keyword		= str_dup(arg3    );
			ed->description	= str_add(argument, "\n\r", NULL);
			ed->next		= obj->extra_descr;
			obj->extra_descr	= ed;
			return;
		}
	}
	
		
	/* echo bad use message */
	do_string(ch,"");
}


void do_oset(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int value;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
		   one_argument(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n\r",ch);
		char_puts("  set obj <object> <field> <value>\n\r",ch);
		char_puts("  Field being one of:\n\r",				ch);
		char_puts("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch);
		char_puts("    extra wear level weight cost timer\n\r",		ch);
		return;
	}

	if ((obj = get_obj_world(ch, arg1)) == NULL) {
		char_puts("Nothing like that in heaven or earth.\n\r", ch);
		return;
	}

	value = atoi(arg3);

	/*
	 * Set something.
	 */
	if (!str_cmp(arg2, "value0") || !str_cmp(arg2, "v0")) {
		obj->value[0] = UMIN(50,value);
		return;
	}

	if (!str_cmp(arg2, "value1") || !str_cmp(arg2, "v1")) {
		obj->value[1] = value;
		return;
	}

	if (!str_cmp(arg2, "value2") || !str_cmp(arg2, "v2")) {
		obj->value[2] = value;
		return;
	}

	if (!str_cmp(arg2, "value3") || !str_cmp(arg2, "v3")) {
		obj->value[3] = value;
		return;
	}

	if (!str_cmp(arg2, "value4") || !str_cmp(arg2, "v4")) {
		obj->value[4] = value;
		return;
	}

	if (!str_prefix(arg2, "extra")) {
		obj->extra_flags = value;
		return;
	}

	if (!str_prefix(arg2, "wear")) {
		obj->wear_flags = value;
		return;
	}

	if (!str_prefix(arg2, "level")) {
		obj->level = value;
		return;
	}
		
	if (!str_prefix(arg2, "weight")) {
		obj->weight = value;
		return;
	}

	if (!str_prefix(arg2, "cost")) {
		obj->cost = value;
		return;
	}

	if (!str_prefix(arg2, "timer")) {
		obj->timer = value;
		return;
	}
		
	/*
	 * Generate usage message.
	 */
	do_oset(ch, "");
	return;
}



void do_rset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	int value;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
		   one_argument(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n\r",ch);
		char_puts("  set room <location> <field> <value>\n\r",ch);
		char_puts("  Field being one of:\n\r",			ch);
		char_puts("    flags sector\n\r",				ch);
		return;
	}

	if ((location = find_location(ch, arg1)) == NULL)
	{
		char_puts("No such location.\n\r", ch);
		return;
	}

/*    if (!is_room_owner(ch,location) && ch->in_room != location  */
	if (ch->in_room != location 
	&&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR))
	{
	    char_puts("That room is private right now.\n\r",ch);
	    return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3)) {
		char_puts("Value must be numeric.\n\r", ch);
		return;
	}
	value = atoi(arg3);

	/*
	 * Set something.
	 */
	if (!str_prefix(arg2, "flags"))
	{
		location->room_flags	= value;
		return;
	}

	if (!str_prefix(arg2, "sector"))
	{
		location->sector_type	= value;
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_rset(ch, "");
	return;
}



void do_sockets(CHAR_DATA *ch, const char *argument)
{
	char buf[2 * MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	int count;

	count	= 0;
	buf[0]	= '\0';

	one_argument(argument,arg);
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->character != NULL && can_see(ch, d->character) 
		&& (arg[0] == '\0' || is_name(arg,d->character->name)
				   || (d->original && is_name(arg,d->original->name))))
		{
		    count++;
		    sprintf(buf + strlen(buf), "[%3d %2d] %s@%s\n\r",
			d->descriptor,
			d->connected,
			d->original  ? d->original->name  :
			d->character ? d->character->name : "(none)",
			d->host
			);
		}
	}
	if (count == 0)
	{
		char_puts("No one by that name is connected.\n\r",ch);
		return;
	}

	sprintf(buf2, "%d user%s\n\r", count, count == 1 ? "" : "s");
	strcat(buf,buf2);
	page_to_char(buf, ch);
	return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		char_puts("Force whom to do what?\n\r", ch);
		return;
	}

	one_argument(argument,arg2);
	
	if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob")) {
		char_puts("That will NOT be done.\n\r",ch);
		return;
	}

	sprintf(buf, "$n forces you to '%s'.", argument);

	if (!str_cmp(arg, "all"))
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		if (get_trust(ch) < MAX_LEVEL - 3)
		{
		    char_puts("Not at your level!\n\r",ch);
		    return;
		}

		for (vch = char_list; vch != NULL; vch = vch_next)
		{
		    vch_next = vch->next;

		    if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch))
		    {
			act(buf, ch, NULL, vch, TO_VICT);
			interpret_raw(vch, argument, TRUE);
		    }
		}
	}
	else if (!str_cmp(arg,"players"))
	{
	    CHAR_DATA *vch;
	    CHAR_DATA *vch_next;
	
	    if (get_trust(ch) < MAX_LEVEL - 2)
	    {
	        char_puts("Not at your level!\n\r",ch);
	        return;
	    }
	
	    for (vch = char_list; vch != NULL; vch = vch_next)
	    {
	        vch_next = vch->next;
	
	        if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch) 
		    &&	 vch->level < LEVEL_HERO)
	        {
	            act(buf, ch, NULL, vch, TO_VICT);
	            interpret(vch, argument);
	        }
	    }
	}
	else if (!str_cmp(arg,"gods"))
	{
	    CHAR_DATA *vch;
	    CHAR_DATA *vch_next;
	
	    if (get_trust(ch) < MAX_LEVEL - 2)
	    {
	        char_puts("Not at your level!\n\r",ch);
	        return;
	    }
	
	    for (vch = char_list; vch != NULL; vch = vch_next)
	    {
	        vch_next = vch->next;
	
	        if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch)
	        &&   vch->level >= LEVEL_HERO)
	        {
	            act(buf, ch, NULL, vch, TO_VICT);
	            interpret(vch, argument);
	        }
	    }
	}
	else
	{
		CHAR_DATA *victim;

		if ((victim = get_char_world(ch, arg)) == NULL)
		{
		    char_puts("They aren't here.\n\r", ch);
		    return;
		}

		if (victim == ch)
		{
		    char_puts("Aye aye, right away!\n\r", ch);
		    return;
		}

		if (!is_room_owner(ch,victim->in_room) 
		&&  ch->in_room != victim->in_room 
	    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
		{
	        char_puts("That character is in a private room.\n\r",ch);
	        return;
	    }

		if (get_trust(victim) >= get_trust(ch))
		{
		    char_puts("Do it yourself!\n\r", ch);
		    return;
		}

		if (!IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
		{
		    char_puts("Not at your level!\n\r",ch);
		    return;
		}

		act(buf, ch, NULL, victim, TO_VICT);
		interpret(victim, argument);
	}

	char_nputs(OK, ch);
	return;
}



/*
 * New routines by Dionysos.
 */
void do_invis(CHAR_DATA *ch, const char *argument)
{
	int level;
	char arg[MAX_STRING_LENGTH];

	/* RT code for taking a level argument */
	one_argument(argument, arg);

	if (arg[0] == '\0') 
	/* take the default path */

	  if (ch->invis_level)
	  {
		  ch->invis_level = 0;
		  act("$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
		  char_puts("You slowly fade back into existence.\n\r", ch);
	  }
	  else
	  {
		  ch->invis_level = get_trust(ch);
		  act("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
		  char_puts("You slowly vanish into thin air.\n\r", ch);
	  }
	else
	/* do the level thing */
	{
	  level = atoi(arg);
	  if (level < 2 || level > get_trust(ch))
	  {
		char_puts("Invis level must be between 2 and your level.\n\r",ch);
	    return;
	  }
	  else
	  {
		  ch->reply = NULL;
	      ch->invis_level = level;
	      act("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
	      char_puts("You slowly vanish into thin air.\n\r", ch);
	  }
	}

	return;
}


void do_incognito(CHAR_DATA *ch, const char *argument)
{
	int level;
	char arg[MAX_STRING_LENGTH];
	
	/* RT code for taking a level argument */
	one_argument(argument, arg);
	
	if (arg[0] == '\0')
	/* take the default path */
	
	  if (ch->incog_level)
	  {
	      ch->incog_level = 0;
	      act("$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM);
	      char_puts("You are no longer cloaked.\n\r", ch);
	  }
	  else
	  {
	      ch->incog_level = get_trust(ch);
	      act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	      char_puts("You cloak your presence.\n\r", ch);
	  }
	else
	/* do the level thing */
	{
	  level = atoi(arg);
	  if (level < 2 || level > get_trust(ch))
	  {
	    char_puts("Incog level must be between 2 and your level.\n\r",ch);
	    return;
	  }
	  else
	  {
	      ch->reply = NULL;
	      ch->incog_level = level;
	      act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	      char_puts("You cloak your presence.\n\r", ch);
	  }
	}
	
	return;
}



void do_holylight(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
		char_puts("Holy light mode off.\n\r", ch);
	}
	else
	{
		SET_BIT(ch->act, PLR_HOLYLIGHT);
		char_puts("Holy light mode on.\n\r", ch);
	}

	return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, const char *argument)
{
	char_puts("You cannot abbreviate the prefix command.\r\n",ch);
	return;
}

void do_prefix (CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_INPUT_LENGTH];

	if (argument[0] == '\0')
	{
		if (ch->prefix[0] == '\0')
		{
		    char_puts("You have no prefix to clear.\r\n",ch);
		    return;
		}

		char_puts("Prefix removed.\r\n",ch);
		free_string(ch->prefix);
		ch->prefix = str_dup("");
		return;
	}

	if (ch->prefix[0] != '\0')
	{
		sprintf(buf,"Prefix changed to %s.\r\n",argument);
		free_string(ch->prefix);
	}
	else
	{
		sprintf(buf,"Prefix set to %s.\r\n",argument);
	}

	ch->prefix = str_dup(argument);
}


void do_advance(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;
	int iLevel;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
		char_puts("Syntax: advance <char> <level>.\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL) {
		char_puts("That player is not here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n\r", ch);
		return;
	}

	if ((level = atoi(arg2)) < 1 || level > MAX_LEVEL) {
		char_puts("Level must be 1 to 100.\n\r", ch);
		return;
	}

	if (level > get_trust(ch)) {
		char_puts("Limited to your trust level.\n\r", ch);
		return;
	}


	/* Level counting */
	if (ch->level <= 5 || ch->level > LEVEL_HERO) 
	{ 
	  if (5 < level && level <= LEVEL_HERO) 
		total_levels += level - 5;
	}
	else 
	{
	  if (5 < level && level <= LEVEL_HERO) 
		total_levels += level - ch->level;
	  else total_levels -= (ch->level - 5);
	}

	/*
	 * Lower level:
	 *   Reset to level 1.
	 *   Then raise again.
	 *   Currently, an imp can lower another imp.
	 *   -- Swiftest
	 */
	if (level <= victim->level)
	{
	    int temp_prac;

		char_puts("Lowering a player's level!\n\r", ch);
		char_puts("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim);
		temp_prac = victim->practice;
		victim->level    = 1;
		victim->exp      = exp_to_level(victim,victim->pcdata->points);
		victim->max_hit  = 10;
		victim->max_mana = 100;
		victim->max_move = 100;
		victim->practice = 0;
		victim->hit      = victim->max_hit;
		victim->mana     = victim->max_mana;
		victim->move     = victim->max_move;
		advance_level(victim);
		victim->practice = temp_prac;
	}
	else
	{
		char_puts("Raising a player's level!\n\r", ch);
		char_puts("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim);
	}

	for (iLevel = victim->level ; iLevel < level; iLevel++)
	{
		char_puts(msg(YOU_RAISE_A_LEVEL, victim), victim);
		victim->exp += exp_to_level(victim,victim->pcdata->points);
		victim->level += 1;
		advance_level(victim);
	}
	victim->trust = 0;
	save_char_obj(victim, FALSE);
}

void do_mset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value,sn;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
		   one_argument(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n\r",ch);
		char_puts("  set char <name> <field> <value>\n\r",ch); 
		char_puts("  Field being one of:\n\r",			ch);
		char_puts("    str int wis dex con cha sex class level\n\r",ch);
		char_puts("    race gold hp mana move practice align\n\r",	ch);
		char_puts("    train thirst drunk full hometown ethos\n\r",	ch);
		char_puts("    pumped noghost\n\r", ch);

/*** Added By KIO ***/
		char_puts("    questp questt relig bloodlust desire security\n\r",	ch);
/*** Added By KIO ***/
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	/*
	 * Snarf the value (which need not be numeric).
	 */
	value = is_number(arg3) ? atoi(arg3) : -1;

	/*
	 * Set something.
	 */
	if (!str_cmp(arg2, "str")) {
		if (value < 3 || value > get_max_train(victim,STAT_STR))
		{
		    char_printf(ch,
			"Strength range is 3 to %d\n\r.",
			get_max_train(victim,STAT_STR));
		    return;
		}

		victim->perm_stat[STAT_STR] = value;
		return;
	}

	if (!str_cmp(arg2, "pumped")) {
		if (value < 0)
			return;
		victim->pumped = value != 0;
		return;
	}

	if (!str_cmp(arg2, "security"))	{ /* OLC */
		if (IS_NPC(ch)) {
			char_puts("Si, claro.\n\r", ch);
			return;
		}

		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n\r", ch);
			return;
		}

		if (value > ch->pcdata->security || value < 0) {
			if (ch->pcdata->security != 0)
				char_printf(ch, "Valid security is 0-%d.\n\r",
					    ch->pcdata->security);
			else
				char_puts("Valid security is 0 only.\n\r", ch);
			return;
		}
		victim->pcdata->security = value;
		return;
	}

	if (!str_cmp(arg2, "int"))
	{
	    if (value < 3 || value > get_max_train(victim,STAT_INT))
	    {
	        char_printf(ch, "Intelligence range is 3 to %d.\n\r",
			get_max_train(victim,STAT_INT));
	        return;
	    }
	
	    victim->perm_stat[STAT_INT] = value;
	    return;
	}

	if (!str_cmp(arg2, "wis"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_WIS))
		{
		    char_printf(ch,
			"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
		    return;
		}

		victim->perm_stat[STAT_WIS] = value;
		return;
	}
/*** Added By KIO  ***/
	if (!str_cmp(arg2, "questp"))
	{
		 if (value == -1) value = 0;
		 if (!IS_NPC(victim)) victim->pcdata->questpoints = value;
		return;
	}
	if (!str_cmp(arg2, "questt"))
	{
		 if (value == -1) value = 30;
		 if (!IS_NPC(victim)) victim->pcdata->questtime = value;
		return;
	}
	if (!str_cmp(arg2, "relig"))
	{
		 if (value == -1) value = 0;
		 victim->religion = value;
		return;
	}
/*** Added By KIO ***/



	if (!str_cmp(arg2, "dex"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_DEX))
		{
		    char_printf(ch,
			"Dexterity ranges is 3 to %d.\n\r",
			get_max_train(victim,STAT_DEX));
		    return;
		}

		victim->perm_stat[STAT_DEX] = value;
		return;
	}

	if (!str_cmp(arg2, "con"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_CON))
		{
		    char_printf(ch,
			"Constitution range is 3 to %d.\n\r",
			get_max_train(victim,STAT_CON));
		    return;
		}

		victim->perm_stat[STAT_CON] = value;
		return;
	}
	if (!str_cmp(arg2, "cha"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_CHA))
		{
		    char_printf(ch,
			"Constitution range is 3 to %d.\n\r",
			get_max_train(victim,STAT_CHA));
		    return;
		}

		victim->perm_stat[STAT_CHA] = value;
		return;
	}

	if (!str_prefix(arg2, "sex"))
	{
		if (value < 0 || value > 2)
		{
		    char_puts("Sex range is 0 to 2.\n\r", ch);
		    return;
		}
		if ((victim->class == 0) || (victim->class == 8))
		{
		    char_puts("You can't change their sex.\n\r", ch);
		    return;
		}
		victim->sex = value;
		if (!IS_NPC(victim))
		    victim->pcdata->true_sex = value;
		return;
	}

	if (!str_prefix(arg2, "class"))
	{
		int class;

		if (IS_NPC(victim))
		{
		    char_puts("Mobiles have no class.\n\r",ch);
		    return;
		}

		class = class_lookup(arg3);
		if (class == -1) {
			BUFFER *output;

			output = buf_new(0);

			buf_add(output, "Possible classes are: ");
	    		for (class = 0; class < MAX_CLASS; class++) {
	        		if (class > 0)
	                		buf_add(output, " ");
	        		buf_add(output, class_table[class].name);
	    		}
	        	buf_add(output, ".\n\r");

			char_puts(buf_string(output), ch);
			buf_free(output);
			return;
		}

		victim->class = class;
		return;
	}

	if (!str_prefix(arg2, "level"))
	{
		if (!IS_NPC(victim))
		{
		    char_puts("Not on PC's.\n\r", ch);
		    return;
		}

		if (value < 0 || value > 100)
		{
		    char_puts("Level range is 0 to 100.\n\r", ch);
		    return;
		}
		victim->level = value;
		return;
	}

	if (!str_prefix(arg2, "gold"))
	{
		victim->gold = value;
		return;
	}

	if (!str_prefix(arg2, "hp"))
	{
		if (value < -10 || value > 30000)
		{
		    char_puts("Hp range is -10 to 30,000 hit points.\n\r", ch);
		    return;
		}
		victim->max_hit = value;
	    if (!IS_NPC(victim))
	        victim->pcdata->perm_hit = value;
		return;
	}

	if (!str_prefix(arg2, "mana"))
	{
		if (value < 0 || value > 60000)
		{
		    char_puts("Mana range is 0 to 60,000 mana points.\n\r", ch);
		    return;
		}
		victim->max_mana = value;
	    if (!IS_NPC(victim))
	        victim->pcdata->perm_mana = value;
		return;
	}

	if (!str_prefix(arg2, "move"))
	{
		if (value < 0 || value > 60000)
		{
		    char_puts("Move range is 0 to 60,000 move points.\n\r", ch);
		    return;
		}
		victim->max_move = value;
	    if (!IS_NPC(victim))
	        victim->pcdata->perm_move = value;
		return;
	}

	if (!str_prefix(arg2, "practice"))
	{
		if (value < 0 || value > 250)
		{
		    char_puts("Practice range is 0 to 250 sessions.\n\r", ch);
		    return;
		}
		victim->practice = value;
		return;
	}

	if (!str_prefix(arg2, "train"))
	{
		if (value < 0 || value > 50)
		{
		    char_puts("Training session range is 0 to 50 sessions.\n\r",ch);
		    return;
		}
		victim->train = value;
		return;
	}

	if (!str_prefix(arg2, "align"))
	{
		if (value < -1000 || value > 1000)
		{
		    char_puts("Alignment range is -1000 to 1000.\n\r", ch);
		    return;
		}
		victim->alignment = value;
		char_puts("Remember to check their hometown.\n\r", ch);
		return;
	}

	if (!str_prefix(arg2, "ethos"))
	  {
		if (IS_NPC(victim))
		  {
		    char_puts("Mobiles don't have an ethos.\n\r", ch);
		    return;
		  }
		if (value < 0 || value > 3)
		  {
		char_puts("The values are Lawful - 1, Neutral - 2, Chaotic - 3\n\r",
				ch);
		    return;
		  }

		victim->ethos = value;
		return;
	  }

	if (!str_prefix(arg2, "hometown"))
	{
	    if (IS_NPC(victim))
		{
		    char_puts("Mobiles don't have hometowns.\n\r", ch);
		    return;
		}
	    if (value < 0 || value > 4)
	    { 
	        char_puts("Please choose one of the following :.\n\r", ch);
	        char_puts("Town        Alignment       Value\n\r", ch);
	        char_puts("----        ---------       -----\n\r", ch);
	        char_puts("Midgaard     Any              0\n\r", ch);
	        char_puts("New Thalos   Any              1\n\r", ch);
	        char_puts("Titan        Any              2\n\r", ch);
	        char_puts("Ofcol        Neutral          3\n\r", ch);
	        char_puts("Old Midgaard Evil             4\n\r", ch);
	        return;
	    }

	    if ((value == 2 && !IS_GOOD(victim)) || (value == 3 &&
		!IS_NEUTRAL(victim)) || (value == 4 && !IS_EVIL(victim)))
	    { 
	        char_puts("The hometown doesn't match this character's alignment.\n\r", ch);
	        return;
	    }    
	    
	    victim->hometown = value;
	    return;
	}

	if (!str_prefix(arg2, "thirst"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n\r", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Thirst range is -1 to 100.\n\r", ch);
		    return;
		}

		victim->pcdata->condition[COND_THIRST] = value;
		return;
	}

	if (!str_prefix(arg2, "drunk"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n\r", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Drunk range is -1 to 100.\n\r", ch);
		    return;
		}

		victim->pcdata->condition[COND_DRUNK] = value;
		return;
	}

	if (!str_prefix(arg2, "full"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n\r", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n\r", ch);
		    return;
		}

		victim->pcdata->condition[COND_FULL] = value;
		return;
	}

	if (!str_prefix(arg2, "bloodlust"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n\r", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n\r", ch);
		    return;
		}

		victim->pcdata->condition[COND_BLOODLUST] = value;
		return;
	}

	if (!str_prefix(arg2, "desire"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n\r", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n\r", ch);
		    return;
		}

		victim->pcdata->condition[COND_DESIRE] = value;
		return;
	}

	if (!str_prefix(arg2, "race"))
	{
		int race;

		race = race_lookup(arg3);

		if (race == 0)
		{
		    char_puts("That is not a valid race.\n\r",ch);
		    return;
		}

		if (!IS_NPC(victim) && !race_table[race].pc_race)
		{
		    char_puts("That is not a valid player race.\n\r",ch);
		    return;
		}

		if (!IS_NPC(victim))
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
		    if ((skill_table[sn].name != NULL) 
			&& !SKILL_RACE_OK(victim,sn)	)
			victim->pcdata->learned[sn]	= 0;

		    if ((skill_table[sn].name != NULL) 
			&&  (ORG_RACE(victim) == skill_table[sn].race) 
			)
			victim->pcdata->learned[sn]	= 70;
		}

		if (ORG_RACE(victim) == RACE(victim)) RACE(victim) = race;
		ORG_RACE(victim) = race;
		return;
	}

	if (!str_prefix(arg2, "noghost")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n\r", ch);
			return;
		}
		REMOVE_BIT(victim->act, PLR_GHOST);
		char_nputs(OK, ch);
	}

	/*
	 * Generate usage message.
	 */
	do_mset(ch, "");
}


void do_desocket(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;
	int socket;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg);

	if (!is_number(arg))

	{
	  char_puts("The argument must be a number.\n\r", ch);
	  return;
	}

	if (arg[0] == '\0')
	{
	  char_puts("Disconnect which socket?\n\r", ch);
	  return;
	}

	else
	{
	  socket = atoi(arg);
	  for (d = descriptor_list; d != NULL; d = d->next)      
		{
		  if (d->descriptor == socket)
		    {
		      if (d->character == ch)
			{
			  char_puts("It would be foolish to disconnect yourself.\n\r", ch);
			  return;
			}
		      if (d->connected == CON_PLAYING)
			{
			  char_puts("Why don't you just use disconnect?\n\r", ch);
			  return;
			}
		      write_to_descriptor(d->descriptor,
					  "You are being disconnected by an immortal.",
					  0);
		      close_socket(d);
		      char_puts("Done.\n\r", ch);
		      return;
		    }
		}
	  char_puts("No such socket is connected.\n\r", ch);
	  return;
	}
}

void do_smite(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;

	if (argument[0] == '\0') {
	  char_puts("You are so frustrated you smite yourself!  OWW!\n\r", 
			ch);
	  return;
	}

	if ((victim = get_char_world(ch, argument)) == NULL) {
	  char_puts("You'll have to smite them some other day.\n\r", ch);
	  return;
	}

	if (IS_NPC(victim)) {
	  char_puts("That poor mob never did anything to you.\n\r", ch);
	  return;
	}

	if (get_trust(victim) > get_trust(ch)) {
	  char_puts("How dare you!\n\r", ch);
	  return;
	}

	if (victim->position < POS_SLEEPING) {
	  char_puts("Take pity on the poor thing.\n\r", ch);
	  return;
	}

	act("A bolt comes down out of the heavens and smites you!", victim, NULL,
		ch, TO_CHAR);
	act("You reach down and smite $n!", victim, NULL, ch, TO_VICT);
	act("A bolt from the heavens smites $n!", victim, NULL, ch, TO_NOTVICT);
	victim->hit = victim->hit / 2;
	return;
}

void do_popularity(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;
	AREA_DATA *area;
	extern AREA_DATA *area_first;
	int i;

	output = buf_new(0);
	buf_add(output, "Area popularity statistics (in char * ticks)\n\r");

	for (area = area_first,i=0; area != NULL; area = area->next,i++) {
		if (i % 2 == 0) 
			buf_add(output, "\n\r");
		buf_printf(output, "%-20s %-8lu       ",
			   area->name, area->count);
	}
	buf_add(output, "\n\r\n\r");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_ititle(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg);

	if (arg[0] == '\0')  {
		char_puts("Change whose title to what?\n\r", ch);
		return;
	}

	victim = get_char_world(ch, arg);
	if (victim == NULL)  {
		char_puts("Nobody is playing with that name.\n\r", ch);
		return;
	}

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0') {
		char_puts("Change the title to what?\n\r", ch);
		return;
	}

	set_title(victim, argument);
	char_nputs(OK, ch);
}

/*
 * .gz files are checked for too, just in case.
 */

bool check_parse_name (char* name);  

void do_rename (CHAR_DATA* ch, const char *argument)
{
	char old_name[MAX_INPUT_LENGTH], 
	     new_name[MAX_INPUT_LENGTH],
	     strsave [MAX_INPUT_LENGTH];

	CHAR_DATA* victim;
	FILE* file;
		
	argument = one_argument(argument, old_name); 
	one_argument (argument, new_name);
		
	if (!old_name[0]) {
		char_puts ("Rename who?\n\r",ch);
		return;
	}
		
	victim = get_char_world (ch, old_name);
		
	if (!victim) {
		char_puts ("There is no such a person online.\n\r",ch);
		return;
	}
		
	if (IS_NPC(victim)) {   
		char_puts ("You cannot use Rename on NPCs.\n\r",ch);
		return;
	}

	if ((victim != ch) && (get_trust (victim) >= get_trust (ch))) {
		char_puts ("You failed.\n\r",ch);
		return;
	}
		
	if (!victim->desc || (victim->desc->connected != CON_PLAYING)) {
		char_puts ("This player has lost his link or is inside a pager or the like.\n\r",ch);
		return;
	}

	if (!new_name[0]) {
		char_puts ("Rename to what new name?\n\r",ch);
		return;
	}
		
/*
	if (victim->clan) {
		char_puts ("This player is member of a clan, remove him from there first.\n\r",ch);
		return;
	}
*/

	new_name[0] = UPPER(new_name[0]);
	if (!check_parse_name(new_name)) {
		char_puts ("The new name is illegal.\n\r",ch);
		return;
	}

	sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(new_name)); 

	fclose (fpReserve); 
	file = fopen (strsave, "r"); 
	if (file) {
		char_puts ("A player with that name already exists!\n\r",ch);
		fclose (file);
		fpReserve = fopen(NULL_FILE, "r"); 
		return;		
	}
	fpReserve = fopen(NULL_FILE, "r");  

		/* Check .gz file ! */
	sprintf(strsave, "%s%s.gz", PLAYER_DIR, capitalize(new_name));

	fclose (fpReserve); 
	file = fopen (strsave, "r"); 
	if (file) {
		char_puts ("A player with that name already exists in a compressed file!\n\r",ch);
		fclose (file);
		fpReserve = fopen(NULL_FILE, "r"); 
		return;		
	}
	fpReserve = fopen(NULL_FILE, "r");  

	if (get_char_world(ch, new_name)) {
		char_puts ("A player with the name you specified already exists!\n\r",ch);
		return;
	}

	sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(victim->name));

/*
 * NOTE: Players who are level 1 do NOT get saved under a new name 
 */
		free_string (victim->name);
		victim->name = str_dup (capitalize(new_name));
		
		save_char_obj (victim, FALSE);
		
		unlink (strsave); 
		char_puts ("Character renamed.\n\r",ch);
		victim->position = POS_STANDING; 
		act ("$n has renamed you to $N!",ch,NULL,victim,TO_VICT);
				
} 

void do_notitle(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!IS_IMMORTAL(ch))
	    return;

	argument = one_argument(argument,arg);

	if ((victim = get_char_world(ch ,arg)) == NULL) {
		char_puts("He is not currently playing.\n\r", ch);
		return;
	}
	 
	if (IS_SET(victim->act, PLR_NOTITLE)) {
	 	REMOVE_BIT(victim->act,PLR_NOTITLE);
	 	char_puts("You can change your title again.\n\r",victim);
		char_nputs(OK, ch);
	}
	else {		       
		SET_BIT(victim->act,PLR_NOTITLE);
		char_puts("You won't be able to change your title anymore.\n\r",victim);
		char_nputs(OK, ch);
	}
}
	   

void do_noaffect(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA *paf,*paf_next;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!IS_IMMORTAL(ch))
		return;

	argument = one_argument(argument,arg);

	if ((victim = get_char_world(ch ,arg)) == NULL) {
		char_puts("He is not currently playing.\n\r", ch);
		return;
	}
	 
	
	for (paf = victim->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		if (paf->duration >= 0) {
	        	if (paf->type > 0 && skill_table[paf->type].msg_off) {
				char_puts(skill_table[paf->type].msg_off,
					  victim);
				char_puts("\n\r", victim);
			}
		  
			affect_remove(victim, paf);
		}
	}
}

void do_affrooms(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *room_next;
	int count = 0;

	if (!top_affected_room) 
		char_puts("No affected room.\n\r",ch);

	for (room = top_affected_room; room ; room = room_next) {
		room_next = room->aff_next;
		count++;
		char_printf(ch, "%d) [Vnum : %5d] %s\n\r",
			count, room->vnum , mlstr_val(ch, room->name));
	}
}

void do_find(CHAR_DATA *ch, const char *argument)
{
	char* path;
	ROOM_INDEX_DATA *location;

	if (argument[0] == '\0') {
		char_puts("Ok. But what I should find?\n\r", ch);
		return;
	}

	if ((location = find_location(ch, argument)) == NULL) {
		char_puts("No such location.\n\r", ch);
		return;
	}

	path = find_way(ch, ch->in_room, location);
	char_printf(ch, "%s.\n\r", path);
	log_printf("From %d to %d: %s.\n\r",
		   ch->in_room->vnum, location->vnum, path);
	return;
}


void do_reboot(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg);    

	if (arg[0] == '\0') {
		char_puts("Usage: reboot now\n\r"
			  "Usage: reboot <ticks to reboot>\n\r"
			  "Usage: reboot cancel\n\r"
			  "Usage: reboot status\n\r", ch);
		return;
	}

	if (is_name(arg,"cancel")) {
		reboot_counter = -1;
		char_puts("Reboot canceled.\n\r", ch);
		return;
	}

	if (is_name(arg, "now")) {
		reboot_muddy();
		return;
	}

	if (is_name(arg, "status")) {
		if (reboot_counter == -1) 
			char_printf(ch, "Automatic rebooting is inactive.\n\r");
		else
			char_printf(ch, "Reboot in %i minutes.\n\r",
				    reboot_counter);
		return;
	}

	if (is_number(arg)) {
		reboot_counter = atoi(arg);
		rebooter = 1;
		char_printf(ch, "Muddy will reboot in %i ticks.\n\r",
			    reboot_counter);
		return;
	}

	if (is_name(arg, "auto") && get_trust(ch) == MAX_LEVEL) {
		rebooter = 0;
		char_printf(ch, "Reboot will now be shown as automatic.\n\r");
		do_reboot(ch, "status");
		return;
	}

	do_reboot(ch, "");   
}


void reboot_muddy(void)
{
	extern bool merc_down;
	DESCRIPTOR_DATA *d,*d_next;

	log("Rebooting ANATOLIA.");
	for (d = descriptor_list; d != NULL; d = d_next) {
		d_next = d->next;
		write_to_buffer(d,"Muddy is going down for rebooting NOW!",0);
		if (d->character != NULL)
			save_char_obj(d->character, TRUE);
		close_socket(d);
	}
	merc_down = TRUE;    
}

