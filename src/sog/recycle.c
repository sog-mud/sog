/*
 * $Id: recycle.c,v 1.64.2.8 2002-02-19 20:43:12 tatyana Exp $
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
#include "db.h"

/*
 * Globals
 */
extern int	top_reset;
extern int	top_area;
extern int	top_exit;
extern int	top_room;
extern int	top_mprog_index;
extern int	top_ed;

void	aff_free(AFFECT_DATA *af);

ED_DATA *ed_new(void)
{
	ED_DATA *ed;
	ed = calloc(1, sizeof(*ed));
	top_ed++;
	return ed;
}

ED_DATA *ed_new2(const ED_DATA *ed, const char* name)
{
	ED_DATA *ed2 = ed_new();
	ed2->keyword = str_qdup(ed->keyword);
	mlstr_printf(&ed2->description, &ed->description, name);
	return ed2;
}

ED_DATA *ed_dup(const ED_DATA *ed)
{
	ED_DATA *ned = NULL;
	ED_DATA **ped = &ned;

	for (; ed; ed = ed->next) {
		*ped = ed_new();
		(*ped)->keyword = str_qdup(ed->keyword);
		mlstr_cpy(&(*ped)->description, &ed->description);
		ped = &(*ped)->next;
	}

	return ned;
}

void ed_free(ED_DATA *ed)
{
	ED_DATA *ed_next;

	for (; ed; ed = ed_next) {
		ed_next = ed->next;

		free_string(ed->keyword);
		mlstr_destroy(&ed->description);
		free(ed);
		top_ed--;
	}
}

void ed_fread(FILE *fp, ED_DATA **edp)
{
	ED_DATA *ed	= ed_new();
	ed->keyword	= fread_string(fp);
	mlstr_fread(fp, &ed->description);
	SLIST_ADD(ED_DATA, *edp, ed);
}

void ed_fwrite(FILE *fp, ED_DATA *ed)
{
       	fprintf(fp, "E\n%s~\n", fix_string(ed->keyword));
	mlstr_fwrite(fp, NULL, &ed->description);
}

AFFECT_DATA *aff_new(void)
{
	top_affect++;
	return calloc(1, sizeof(AFFECT_DATA));
}

ROOM_AFFECT_DATA *raff_new(void)
{
	top_raffect++;
	return calloc(1, sizeof(ROOM_AFFECT_DATA));
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
	top_affect--;
}

void raff_free(ROOM_AFFECT_DATA *raf)
{
	free(raf);
	top_raffect--;
}

OBJ_DATA *free_obj_list;

int obj_count;
int obj_free_count;

OBJ_DATA *new_obj(void)
{
	OBJ_DATA *obj;

	if (free_obj_list) {
		obj = free_obj_list;
		free_obj_list = free_obj_list->next;
		obj_free_count--;
		mem_validate(obj);
	}
	else {
		obj = mem_alloc(MT_OBJ, sizeof(*obj));
		obj_count++;
	}

	memset(obj, '\0', sizeof(*obj));
	return obj;
}

void free_obj(OBJ_DATA *obj)
{
	AFFECT_DATA *paf, *paf_next;

	if (!obj)
		return;

	if (!mem_is(obj, MT_OBJ)) {
		bug("free_obj: obj is not MT_OBJ");
		return;
	}
	mem_invalidate(obj);

	for (paf = obj->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
    	}
	obj->affected = NULL;

	ed_free(obj->ed);
	obj->ed = NULL;
   
	free_string(obj->name);
	obj->name = NULL;

	mlstr_destroy(&obj->description);
	mlstr_destroy(&obj->short_descr);
	mlstr_destroy(&obj->owner);

	free_string(obj->material);
	obj->material = NULL;

	obj->next = free_obj_list;
	free_obj_list = obj;

	obj_free_count++;
}

/*
 * PC/NPC recycling
 */
CHAR_DATA *free_npc_list;
int npc_free_count;

CHAR_DATA *free_pc_list;
int pc_free_count;

int npc_count;
int pc_count;

CHAR_DATA *char_new(MOB_INDEX_DATA *pMobIndex)
{
	CHAR_DATA *ch;
	int i;

	CHAR_DATA **free_list;
	int *free_count;
	int *count;
	size_t size;

	if (pMobIndex) {
		size = sizeof(*ch) + sizeof(NPC_DATA);
		count = &npc_count;
		free_count = &npc_free_count;
		free_list = &free_npc_list;
	} else {
		size = sizeof(*ch) + sizeof(PC_DATA);
		count = &pc_count;
		free_count = &pc_free_count;
		free_list = &free_pc_list;
	}

	if (*free_list) {
		ch = *free_list;
		*free_list = (*free_list)->next;
		(*free_count)--;
		mem_validate(ch);
	}
	else {
		ch = mem_alloc(MT_CHAR, size);
		(*count)++;
	}

	memset(ch, 0, size);

	ch->last_death_time	= -1;
	SET_HIT(ch, 20);
	SET_MANA(ch, 100);
	SET_MOVE(ch, 100);
	ch->position		= POS_STANDING;
	ch->hitroll		= 0;
	ch->damroll		= 0;
	ch->saving_throw	= 0;
	ch->add_level		= 0;

	for (i = 0; i < 4; i++)
		ch->armor[i] = 100;
	for (i = 0; i < MAX_STATS; i++) {
		ch->perm_stat[i] = 13;
		ch->mod_stat[i] = 0;
	}

	varr_init(&ch->sk_affected, sizeof(saff_t), 1);

	if (pMobIndex) {
		ch->pMobIndex = pMobIndex;
		ch->comm = COMM_NOSHOUT | COMM_NOMUSIC;
		NPC(ch)->timer = -1;
	} else {
		PC_DATA *pc = PC(ch);
		pc->add_age = 0;
		pc->logon = current_time;
		pc->version = PFILE_VERSION;
		pc->buffer = buf_new(-1);
		varr_init(&pc->learned, sizeof(pcskill_t), 8);
		pc->pwd = str_empty;
		pc->bamfin = str_empty;
		pc->bamfout = str_empty;
		pc->title = str_empty;
		pc->twitlist = str_empty;
		pc->granted = str_empty;
		pc->form_name = str_empty;
		pc->wanted_by = str_empty;
		pc->dvdata = dvdata_new();
		pc->condition[COND_FULL] = 48;
		pc->condition[COND_THIRST] = 48;
		pc->condition[COND_HUNGER] = 48;
		pc->condition[COND_BLOODLUST] = 48;
		pc->condition[COND_DESIRE] = 48;
		pc->race = rn_lookup("human");
		pc->clan_status = CLAN_COMMONER;
		pc->plr_flags = PLR_NOSUMMON;
		pc->bank_s = 0;
		pc->bank_g = 0;
		ch->comm = COMM_COMBINE | COMM_PROMPT;
		pc->ll_time = current_time;
		pc->ll_host = str_empty;
		pc->ll_ip = str_empty;
	}
	RESET_FIGHT_TIME(ch);
	return ch;
}

void char_free(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	CHAR_DATA **free_list;
	int *free_count;

	if (!mem_is(ch, MT_CHAR)) {
		bug("free_char: ch is not MT_CHAR");
		return;
	}
	mem_invalidate(ch);

	if (IS_NPC(ch)) {
		NPC_DATA *npc = NPC(ch);

		free_count = &npc_free_count;
		free_list = &free_npc_list;

		free_string(npc->in_mind);
		npc->in_mind = NULL;
	} else {
		PC_DATA *pc = PC(ch);

		free_count = &pc_free_count;
		free_list = &free_pc_list;

		/* free pc stuff */
		varr_destroy(&pc->learned);
		free_string(pc->pwd);
		free_string(pc->bamfin);
		free_string(pc->bamfout);
		free_string(pc->title);
		free_string(pc->twitlist);
		free_string(pc->form_name);
		free_string(pc->granted);
		free_string(pc->wanted_by);
		buf_free(pc->buffer);
		free_string(pc->ll_host);
		free_string(pc->ll_ip);
		dvdata_free(pc->dvdata);

		nuke_pets(ch);
	}

	for (obj = ch->carrying; obj; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj, XO_F_NOCOUNT | XO_F_NUKE);
	}
	ch->carrying = NULL;

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
	}
	ch->affected = NULL;

	free_string(ch->name);
	ch->name = NULL;

	mlstr_destroy(&ch->short_descr);
	mlstr_destroy(&ch->long_descr);
	mlstr_destroy(&ch->description);

	free_string(ch->material);
	ch->material = NULL;

	ch->next = *free_list;
	*free_list = ch;

	(*free_count)++;
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
			SET_BIT(mptrig->mptrig_flags, TRIG_CASEDEP);
			break;
		}

	if ((type == TRIG_ACT || type == TRIG_SPEECH) && phrase[0] == '*') {
		int errcode;
		int cflags = REG_EXTENDED | REG_NOSUB;

		SET_BIT(mptrig->mptrig_flags, TRIG_REGEXP);
		if (!IS_SET(mptrig->mptrig_flags, TRIG_CASEDEP))
			cflags |= REG_ICASE;

		mptrig->extra = malloc(sizeof(regex_t));
		errcode = regcomp(mptrig->extra, phrase+1, cflags);
		if (errcode) {
			char buf[MAX_STRING_LENGTH];

			regerror(errcode, mptrig->extra, buf, sizeof(buf));
			log("bad trigger for vnum %d (phrase '%s'): %s",
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

	if (IS_SET(mp->mptrig_flags, TRIG_REGEXP)) {
		regfree(mp->extra);
		free(mp->extra);
	}

	free_string(mp->phrase);
	free(mp);
}

RESET_DATA *reset_new(void)
{
	RESET_DATA *pReset;

	pReset = calloc(1, sizeof(*pReset));
	pReset->command = 'X';

	top_reset++;
	return pReset;
}

void reset_free(RESET_DATA *pReset)
{
	if (!pReset)
		return;
	top_reset--;
	free(pReset);
}

void reset_add(ROOM_INDEX_DATA *room, RESET_DATA *reset, RESET_DATA *after)
{
	RESET_DATA *r;

	if (after == NULL) {
		reset->next = NULL;
		if (room->reset_first == NULL)
			room->reset_first = reset;
		if (room->reset_last == NULL)
			room->reset_last = reset;
		else
			room->reset_last = room->reset_last->next = reset;
		return;
	}

	for (r = room->reset_first; r != NULL; r = r->next) {
		if (r == after)
			break;
	}

	if (r == NULL) {
		bug("reset_add: `after' reset not found");
		return;
	}

	reset->next = r->next;
	r->next = reset;
	if (reset->next == NULL)
		room->reset_last = reset;
}

void reset_del(ROOM_INDEX_DATA *room, RESET_DATA *reset)
{
	RESET_DATA *r;
	RESET_DATA *prev = NULL;

	for (r = room->reset_first; r != NULL; r = r->next) {
		if (r == reset)
			break;
		prev = r;
	}

	if (r == NULL)
		return;

	if (prev == NULL)
		room->reset_first = r->next;
	else
		prev->next = r->next;
	if (r->next == NULL)
		room->reset_last = prev;
}

RESET_DATA *reset_lookup(ROOM_INDEX_DATA *room, int rnum)
{
	RESET_DATA *r;

	for (r = room->reset_first; r != NULL; r = r->next) {
		if (!--rnum)
			break;
	}

	return r;
}


AREA_DATA *new_area(void)
{
	AREA_DATA *pArea;

	pArea = calloc(1, sizeof(*pArea));
	pArea->vnum		= top_area;
	pArea->file_name	= str_printf("area%d.are", pArea->vnum);
	pArea->builders		= str_empty;
	pArea->name		= str_dup("New area");
	pArea->empty		= TRUE;              /* ROM patch */
	pArea->security		= 1;

	top_area++;
	return pArea;
}

/*****************************************************************************
 Name:		area_lookup
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *area_lookup(int vnum)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next)
		if (pArea->vnum == vnum)
			return pArea;

	return 0;
}

AREA_DATA *area_vnum_lookup(int vnum)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next) {
		 if (vnum >= pArea->min_vnum
		 &&  vnum <= pArea->max_vnum)
		     return pArea;
	}

	return 0;
}

void free_area(AREA_DATA *pArea)
{
	free_string(pArea->name);
	free_string(pArea->file_name);
	free_string(pArea->builders);
	free_string(pArea->credits);
	top_area--;
	free(pArea);
}

EXIT_DATA *new_exit(void)
{
	EXIT_DATA *pExit;

        pExit = calloc(1, sizeof(*pExit));
	pExit->keyword = str_empty;

        top_exit++;
	return pExit;
}

void free_exit(EXIT_DATA *pExit)
{
	if (!pExit)
		return;

	free_string(pExit->keyword);
	mlstr_destroy(&pExit->description);

	top_exit--;
	free(pExit);
}

ROOM_INDEX_DATA *new_room_index(void)
{
	ROOM_INDEX_DATA *pRoom;

        pRoom = mem_alloc(MT_ROOM, sizeof(*pRoom));
	memset(pRoom, 0, sizeof(*pRoom));
	pRoom->heal_rate = 100;
	pRoom->mana_rate = 100;

        top_room++;
	return pRoom;
}

void free_room_index(ROOM_INDEX_DATA *pRoom)
{
	int door;
	RESET_DATA *pReset;

	mlstr_destroy(&pRoom->name);
	mlstr_destroy(&pRoom->description);

	for (door = 0; door < MAX_DIR; door++)
        	if (pRoom->exit[door])
			free_exit(pRoom->exit[door]);

	ed_free(pRoom->ed);

	for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
		reset_free(pReset);

	top_room--;
	mem_free(pRoom);
}

SHOP_DATA *new_shop(void)
{
	SHOP_DATA *pShop;

        pShop = calloc(1, sizeof(*pShop));
	pShop->profit_buy   =   100;
	pShop->profit_sell  =   100;
	pShop->close_hour   =   23;

        top_shop++;
	return pShop;
}

void free_shop(SHOP_DATA *pShop)
{
	if (!pShop)
		return;
	top_shop--;
	free(pShop);
}

OBJ_INDEX_DATA *new_obj_index(void)
{
	OBJ_INDEX_DATA *pObj;

        pObj = calloc(1, sizeof(*pObj));

	pObj->name		= str_dup(str_empty);
	pObj->item_type		= ITEM_TRASH;
	pObj->material		= str_dup("unknown");
	pObj->condition		= 100;
	pObj->limit		= -1;

        top_obj_index++;
	return pObj;
}

void free_obj_index(OBJ_INDEX_DATA *pObj)
{
	AFFECT_DATA *paf, *paf_next;

	if (!pObj)
		return;

	free_string(pObj->name);
	free_string(pObj->material);
	mlstr_destroy(&pObj->short_descr);
	mlstr_destroy(&pObj->description);

	for (paf = pObj->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
	}

	ed_free(pObj->ed);
    
	top_obj_index--;
	free(pObj);
}

MOB_INDEX_DATA *new_mob_index(void)
{
	MOB_INDEX_DATA *pMob;

        pMob = calloc(1, sizeof(*pMob));

	pMob->name		= str_dup(str_empty);
	pMob->act		= ACT_NPC;
	pMob->race		= rn_lookup("human");
	pMob->material		= str_dup("unknown");
	pMob->size		= SIZE_MEDIUM;
	pMob->start_pos		= POS_STANDING;
	pMob->default_pos	= POS_STANDING;

	top_mob_index++;
	return pMob;
}

void free_mob_index(MOB_INDEX_DATA *pMob)
{
	if (!pMob)
		return;

	free_string(pMob->name);
	free_string(pMob->material);
	mlstr_destroy(&pMob->short_descr);
	mlstr_destroy(&pMob->long_descr);
	mlstr_destroy(&pMob->description);
	mptrig_free(pMob->mptrig_list);
	free_shop(pMob->pShop);

	top_mob_index--;
	free(pMob);
}

MPCODE *mpcode_list;

MPCODE *mpcode_new(void)
{
	MPCODE *mpcode;

        mpcode = calloc(1, sizeof(*mpcode));
	mpcode->code = str_empty;

	top_mprog_index++;
	return mpcode;
}

void mpcode_add(MPCODE *mpcode)
{
	if (mpcode_list == NULL)
		mpcode_list = mpcode;
	else {
		mpcode->next = mpcode_list;
		mpcode_list = mpcode;
	}
}

MPCODE *mpcode_lookup(int vnum)
{
	MPCODE *mpcode;
	for (mpcode = mpcode_list; mpcode; mpcode = mpcode->next)
	    	if (mpcode->vnum == vnum)
        		return mpcode;
	return NULL;
}    
 
void mpcode_free(MPCODE *mpcode)
{
	if (!mpcode)
		return;

	free_string(mpcode->code);

	top_mprog_index--;
	free(mpcode);
}
