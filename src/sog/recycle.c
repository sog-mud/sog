/*
 * $Id: recycle.c,v 1.24 1998-10-02 04:48:27 fjoe Exp $
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
#include "merc.h"
#include "hometown.h"
#include "db/db.h"

/* stuff for recycling extended descs */
ED_DATA *ed_free_list;
extern int top_ed;

ED_DATA *ed_new(void)
{
	ED_DATA *ed;

	if (ed_free_list == NULL) {
		ed = alloc_perm(sizeof(*ed));
		ed->description = NULL;
		top_ed++;
	}
	else {
		ed = ed_free_list;
		ed_free_list = ed_free_list->next;
	}

	VALIDATE(ed);
	return ed;
}

ED_DATA * ed_new2(const ED_DATA *ed, const char* name)
{
	ED_DATA *ed2		= ed_new();
	ed2->keyword		= str_dup(ed->keyword);
	ed2->description	= mlstr_printf(ed->description, name);
	ed2->next		= NULL;
	return ed2;
}

void ed_free(ED_DATA *ed)
{
    if (!IS_VALID(ed))
	return;

    free_string(ed->keyword);
    mlstr_free(ed->description);
    INVALIDATE(ed);
    
    ed->next = ed_free_list;
    ed_free_list = ed;
}


void ed_fread(FILE *fp, ED_DATA **edp)
{
	ED_DATA *ed;

	ed		= ed_new();
	ed->keyword	= fread_string(fp);
	ed->description	= mlstr_fread(fp);
	SLIST_ADD(ED_DATA, *edp, ed);
}


/* stuff for recycling affects */
AFFECT_DATA *affect_free;

AFFECT_DATA *new_affect(void)
{
    static AFFECT_DATA af_zero;
    AFFECT_DATA *af;

    if (affect_free == NULL)
	af = alloc_perm(sizeof(*af));
    else
    {
	af = affect_free;
	affect_free = affect_free->next;
    }

    *af = af_zero;


    VALIDATE(af);
    return af;
}

void free_affect(AFFECT_DATA *af)
{
    if (!IS_VALID(af))
	return;

    INVALIDATE(af);
    af->next = affect_free;
    affect_free = af;
}

/* stuff for recycling objects */
OBJ_DATA *obj_free;

OBJ_DATA *new_obj(void)
{
	int i;
	OBJ_DATA *obj;

	if (obj_free == NULL)
		obj = alloc_perm(sizeof(*obj));
	else {
		obj = obj_free;
		obj_free = obj_free->next;
	}
	VALIDATE(obj);

	obj->short_descr	= NULL;
	obj->description	= NULL;
	obj->next		= NULL;
	obj->next_content	= NULL;
	obj->contains		= NULL;
	obj->in_obj		= NULL;
	obj->on			= NULL;
	obj->carried_by		= NULL;
	obj->ed			= NULL;
	obj->affected		= NULL;
	obj->pIndexData		= NULL;
	obj->in_room		= NULL;
	obj->enchanted		= FALSE;
	obj->name		= NULL;
	obj->item_type		= 0;
	obj->extra_flags	= 0;
	obj->wear_flags		= 0;
	obj->wear_loc		= 0;
	obj->weight		= 0;
	obj->cost		= 0;
	obj->level		= 0;
	obj->condition		= 0;
	obj->material		= NULL;
	obj->timer		= 0;
	for (i = 0; i < 5; i++)
		obj->value[i]	= 0;
	obj->progtypes		= 0;
	obj->from		= NULL;
	obj->altar		= ROOM_VNUM_ALTAR;
	obj->pit		= OBJ_VNUM_PIT;
	obj->extracted		= FALSE;
	obj->water_float	= 0;
	return obj;
}

void free_obj(OBJ_DATA *obj)
{
    AFFECT_DATA *paf, *paf_next;
    ED_DATA *ed, *ed_next;


    if (!IS_VALID(obj))
	return;

    for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
	paf_next = paf->next;
	free_affect(paf);
    }
    obj->affected = NULL;

    for (ed = obj->ed; ed != NULL; ed = ed_next ) {
	ed_next = ed->next;
	ed_free(ed);
    }
    obj->ed = NULL;
   
	free_string(obj->name);
	mlstr_free(obj->description);
	mlstr_free(obj->short_descr);
	free_string(obj->from);
	free_string(obj->material);

    INVALIDATE(obj);

    obj->next   = obj_free;
    obj_free    = obj; 
}


/* stuff for recyling characters */
CHAR_DATA *char_free;

CHAR_DATA *new_char (void)
{
	CHAR_DATA *ch;
	int i;

	if (char_free == NULL) 
		ch = calloc(1, sizeof(*ch));
	else {
		ch = char_free;
		char_free = char_free->next;
		memset(ch, 0, sizeof(*ch));
	}

	VALIDATE(ch);

	RESET_FIGHT_TIME(ch);
	ch->last_death_time	= -1;
	ch->prefix		= str_empty;
	ch->lines		= PAGELEN;
	ch->logon		= current_time;
	ch->hit			= 20;
	ch->max_hit		= 20;
	ch->mana		= 100;
	ch->max_mana		= 100;
	ch->move		= 100;
	ch->max_move		= 100;
	ch->position		= POS_STANDING;
	for (i = 0; i < 4; i++)
		ch->armor[i]	= 100;
	for (i = 0; i < MAX_STATS; i ++)
		ch->perm_stat[i] = 13;
	return ch;
}

void free_char (CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    if (!IS_VALID(ch))
	return;

    if (IS_NPC(ch))
	mobile_count--;

    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	extract_obj_nocount(obj);
    }

    for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
	paf_next = paf->next;
	affect_remove(ch,paf);
    }

    free_string(ch->name);
    mlstr_free(ch->short_descr);
    mlstr_free(ch->long_descr);
    mlstr_free(ch->description);
    free_string(ch->prompt);
    free_string(ch->prefix);
    free_string(ch->material);
    free_string(ch->in_mind);

    if (ch->pcdata != NULL)
    	free_pcdata(ch->pcdata);

    ch->next = char_free;
    char_free  = ch;

    INVALIDATE(ch);
    return;
}

PC_DATA *new_pcdata(void)
{
	PC_DATA *pcdata;
	pcdata = calloc(1, sizeof(*pcdata));
	pcdata->buffer = buf_new(0);
	pcdata->learned.nsize = sizeof(PC_SKILL);
	pcdata->learned.nstep = 8;
	pcdata->pwd = str_empty;
	pcdata->bamfin = str_empty;
	pcdata->bamfout = str_empty;
	pcdata->title = str_empty;
	pcdata->twitlist = str_empty;
	return pcdata;
}
	
void free_pcdata(PC_DATA *pcdata)
{
	int alias;

	varr_free(&pcdata->learned);
	free_string(pcdata->pwd);
	free_string(pcdata->bamfin);
	free_string(pcdata->bamfout);
	free_string(pcdata->title);
	free_string(pcdata->twitlist);
	buf_free(pcdata->buffer);
    
	for (alias = 0; alias < MAX_ALIAS; alias++) {
		free_string(pcdata->alias[alias]);
		free_string(pcdata->alias_sub[alias]);
	}
}

/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
    int val;

    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

long get_mob_id(void)
{
    last_mob_id++;
    return last_mob_id;
}

    
/* stuff for recycling mobprograms */
MPTRIG *mptrig_free_list;
 
MPTRIG *mptrig_new(int type, char *phrase, int vnum)
{
	char *p;
	MPTRIG *mptrig;

	if (mptrig_free_list == NULL)
		mptrig = alloc_perm(sizeof(*mptrig));
	else {
		mptrig = mptrig_free_list;
		mptrig_free_list = mptrig_free_list->next;
	}

	mptrig->type	= type;
	mptrig->phrase	= str_dup(phrase);
	mptrig->vnum	= vnum;
	mptrig->flags	= 0;
	for (p = mptrig->phrase; *p; p++)
		if (ISUPPER(*p)) {
			SET_BIT(mptrig->flags, TRIG_CASEDEP);
			break;
		}
	VALIDATE(mptrig);
	return mptrig;
}

void mptrig_add(MOB_INDEX_DATA *mob, MPTRIG *mptrig)
{
	SET_BIT(mob->mptrig_types, mptrig->type);
	SLIST_ADD(MPTRIG, mob->mptrig_list, mptrig);
}

void mptrig_free(MPTRIG *mp)
{
	if (!IS_VALID(mp))
		return;

	INVALIDATE(mp);
	mp->next = mptrig_free_list;
	mptrig_free_list = mp;
}
