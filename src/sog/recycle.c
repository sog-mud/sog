/*
 * $Id: recycle.c,v 1.41 1999-02-21 19:19:27 fjoe Exp $
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
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "hometown.h"
#include "db/db.h"

/* stuff for recycling extended descs */
extern int top_ed;

ED_DATA *ed_new(void)
{
	ED_DATA *ed;
	ed = calloc(1, sizeof(*ed));
	top_ed++;
	return ed;
}

ED_DATA *ed_new2(const ED_DATA *ed, const char* name)
{
	ED_DATA *ed2		= ed_new();
	ed2->keyword		= str_qdup(ed->keyword);
	ed2->description	= mlstr_printf(ed->description, name);
	return ed2;
}

ED_DATA *ed_dup(const ED_DATA *ed)
{
	ED_DATA *ned = ed_new();
	ned->keyword		= str_qdup(ed->keyword);
	ned->description	= mlstr_dup(ed->description);
	return ned;
}

void ed_free(ED_DATA *ed)
{
	if (!ed)
		return;

	free_string(ed->keyword);
	mlstr_free(ed->description);
	free(ed);
	top_ed--;
}

void ed_fread(FILE *fp, ED_DATA **edp)
{
	ED_DATA *ed	= ed_new();
	ed->keyword	= fread_string(fp);
	ed->description	= mlstr_fread(fp);
	SLIST_ADD(ED_DATA, *edp, ed);
}

void ed_fwrite(FILE *fp, ED_DATA *ed)
{
       	fprintf(fp, "E\n%s~\n", fix_string(ed->keyword));
	mlstr_fwrite(fp, NULL, ed->description);
}

AFFECT_DATA *aff_new(void)
{
	return calloc(1, sizeof(AFFECT_DATA));
}

AFFECT_DATA *aff_dup(const AFFECT_DATA *paf)
{
	AFFECT_DATA *naf = aff_new();
	naf->where	= paf->where;
	naf->type	= paf->type;
	naf->level	= paf->level;
	naf->duration	= paf->duration;
	naf->location	= paf->location;
	naf->modifier	= paf->modifier;
	naf->bitvector	= paf->bitvector;
	return naf;
}

void aff_free(AFFECT_DATA *af)
{
	free(af);
}

OBJ_DATA *free_obj_list;

extern int obj_count;
extern int obj_free_count;

OBJ_DATA *new_obj(void)
{
	OBJ_DATA *obj;

	if (free_obj_list) {
		obj = free_obj_list;
		free_obj_list = free_obj_list->next;
		memset(obj, '\0', sizeof(*obj));
		obj_free_count--;
	}
	else {
		obj = calloc(1, sizeof(*obj));
		obj_count++;
	}

	obj->altar = ROOM_VNUM_ALTAR;
	obj->pit = OBJ_VNUM_PIT;

	return obj;
}

void free_obj(OBJ_DATA *obj)
{
	AFFECT_DATA *paf, *paf_next;
	ED_DATA *ed, *ed_next;

	if (!obj)
		return;

	for (paf = obj->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
    	}
	obj->affected = NULL;

	for (ed = obj->ed; ed != NULL; ed = ed_next ) {
		ed_next = ed->next;
		ed_free(ed);
    	}
	obj->ed = NULL;
   
	free_string(obj->name);
	obj->name = NULL;

	mlstr_free(obj->description);
	obj->description = NULL;

	mlstr_free(obj->short_descr);
	obj->short_descr = NULL;

	mlstr_free(obj->owner);
	obj->owner = NULL;

	free_string(obj->material);
	obj->material = NULL;

	obj->next = free_obj_list;
	free_obj_list = obj;

	obj_free_count++;
}

CHAR_DATA *free_char_list;

extern int mob_count;
extern int mob_free_count;

CHAR_DATA *new_char (void)
{
	CHAR_DATA *ch;
	int i;

	if (free_char_list) {
		ch = free_char_list;
		free_char_list = free_char_list->next;
		memset(ch, '\0', sizeof(*ch));
		mob_free_count--;
	}
	else {
		ch = calloc(1, sizeof(*ch));
		mob_count++;
	}

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

void free_char(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	if (!ch)
		return;

	nuke_pets(ch);

	for (obj = ch->carrying; obj; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj_nocount(obj);
	}

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		affect_remove(ch,paf);
	}
	ch->affected = NULL;

	free_string(ch->name);
	ch->name = NULL;

	mlstr_free(ch->short_descr);
	ch->short_descr = NULL;

	mlstr_free(ch->long_descr);
	ch->long_descr = NULL;

	mlstr_free(ch->description);
	ch->description = NULL;

	free_string(ch->prompt);
	ch->prompt = NULL;

	free_string(ch->prefix);
	ch->prefix = NULL;

	free_string(ch->material);
	ch->material = NULL;

	free_string(ch->in_mind);
	ch->in_mind = NULL;

	free_pcdata(ch->pcdata);
	ch->pcdata = NULL;

	ch->next = free_char_list;
	free_char_list = ch;

	mob_free_count++;
}

PC_DATA *new_pcdata(void)
{
	PC_DATA *pcdata;
	pcdata = calloc(1, sizeof(*pcdata));
	pcdata->buffer = buf_new(-1);
	pcdata->learned.nsize = sizeof(PC_SKILL);
	pcdata->learned.nstep = 8;
	pcdata->pwd = str_empty;
	pcdata->bamfin = str_empty;
	pcdata->bamfout = str_empty;
	pcdata->title = str_empty;
	pcdata->twitlist = str_empty;
	pcdata->granted = str_empty;
	return pcdata;
}
	
void free_pcdata(PC_DATA *pcdata)
{
	int alias;

	if (!pcdata)
		return;

	varr_free(&pcdata->learned);
	free_string(pcdata->pwd);
	free_string(pcdata->bamfin);
	free_string(pcdata->bamfout);
	free_string(pcdata->title);
	free_string(pcdata->twitlist);
	free_string(pcdata->granted);
	buf_free(pcdata->buffer);
    
	for (alias = 0; alias < MAX_ALIAS; alias++) {
		free_string(pcdata->alias[alias]);
		free_string(pcdata->alias_sub[alias]);
	}
	free(pcdata);
}

/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
	return last_pc_id = (current_time <= last_pc_id) ?
			    last_pc_id + 1 : current_time;
}

long get_mob_id(void)
{
	last_mob_id++;
	return last_mob_id;
}
    
MPTRIG *mptrig_new(int type, const char *phrase, int vnum)
{
	const char *p;
	MPTRIG *mptrig;

	mptrig = calloc(1, sizeof(*mptrig));
	mptrig->type	= type;
	mptrig->vnum	= vnum;
	mptrig->phrase	= str_dup(phrase);

	for (p = mptrig->phrase; *p; p++)
		if (ISUPPER(*p)) {
			SET_BIT(mptrig->flags, TRIG_CASEDEP);
			break;
		}

	if ((type == TRIG_ACT || type == TRIG_SPEECH) && phrase[0] == '*') {
		int errcode;
		int cflags = REG_EXTENDED | REG_NOSUB;

		SET_BIT(mptrig->flags, TRIG_REGEXP);
		if (!IS_SET(mptrig->flags, TRIG_CASEDEP))
			cflags |= REG_ICASE;

		mptrig->extra = malloc(sizeof(regex_t));
		errcode = regcomp(mptrig->extra, phrase+1, cflags);
		if (errcode) {
			char buf[MAX_STRING_LENGTH];

			regerror(errcode, mptrig->extra, buf, sizeof(buf));
			log_printf("bad trigger for vnum %d (phrase '%s'): %s",
				   vnum, phrase, buf);
		}
	}
		
	return mptrig;
}

void mptrig_add(MOB_INDEX_DATA *mob, MPTRIG *mptrig)
{
	SET_BIT(mob->mptrig_types, mptrig->type);
	SLIST_ADD(MPTRIG, mob->mptrig_list, mptrig);
}

void mptrig_fix(MOB_INDEX_DATA *mob)
{
	MPTRIG *mptrig;

	for (mptrig = mob->mptrig_list; mptrig; mptrig = mptrig->next)
		SET_BIT(mob->mptrig_types, mptrig->type);
}

void mptrig_free(MPTRIG *mp)
{
	if (!mp)
		return;

	if (IS_SET(mp->flags, TRIG_REGEXP)) {
		regfree(mp->extra);
		free(mp->extra);
	}

	free_string(mp->phrase);
	free(mp);
}
