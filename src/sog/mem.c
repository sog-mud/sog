/*
 * $Id: mem.c,v 1.8 1998-08-14 22:33:05 fjoe Exp $
 */

/***************************************************************************
 *  File: mem.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "db.h"
#include "lookup.h"
#include "recycle.h"
#include "mlstring.h"

/*
 * Globals
 */
extern          int                     top_reset;
extern          int                     top_area;
extern          int                     top_exit;
extern          int                     top_room;
extern		int			top_mprog_index;

AREA_DATA		*	area_free;
EXIT_DATA		*	exit_free;
ROOM_INDEX_DATA		*	room_index_free;
OBJ_INDEX_DATA		*	obj_index_free;
SHOP_DATA		*	shop_free;
MOB_INDEX_DATA		*	mob_index_free;
RESET_DATA		*	reset_free;
HELP_DATA		*	help_free;

HELP_DATA		*	help_last;

void	ed_free	args((ED_DATA *pExtra));
void	free_affect		args((AFFECT_DATA *af));
void	free_mprog              args ((MPROG_LIST *mp));


RESET_DATA *new_reset_data(void)
{
    RESET_DATA *pReset;

    if (!reset_free)
    {
        pReset          =   alloc_perm(sizeof(*pReset));
        top_reset++;
    }
    else
    {
        pReset          =   reset_free;
        reset_free      =   reset_free->next;
    }

    pReset->next        =   NULL;
    pReset->command     =   'X';
    pReset->arg1        =   0;
    pReset->arg2        =   0;
    pReset->arg3        =   0;
    pReset->arg4	=   0;

    return pReset;
}



void free_reset_data(RESET_DATA *pReset)
{
    pReset->next            = reset_free;
    reset_free              = pReset;
    return;
}



AREA_DATA *new_area(void)
{
	AREA_DATA *pArea;

	if (!area_free) {
		pArea		= alloc_perm(sizeof(*pArea));
		top_area++;
	}
	else {
		pArea		= area_free;
		area_free	= area_free->next;
	}

	pArea->next		= NULL;
	pArea->reset_first	= NULL;
	pArea->reset_last	= NULL;
	pArea->help_first	= NULL;
	pArea->help_last	= NULL;
	pArea->file_name	= str_printf("area%d.are", pArea->vnum);
	pArea->name		= str_dup("New area");
	pArea->writer		= NULL;
	pArea->credits		= NULL;
	pArea->age		= 0;
	pArea->nplayer		= 0;
	pArea->low_range	= 0;
	pArea->high_range	= 0;
	pArea->min_vnum		= 0;
	pArea->max_vnum		= 0;
	pArea->empty		= TRUE;              /* ROM patch */
	pArea->builders		= str_dup("None");
	pArea->vnum		= top_area-1;
	pArea->area_flags	= AREA_ADDED;
	pArea->security		= 1;
	pArea->count		= 0;
	pArea->resetmsg		= NULL;
	pArea->area_flag	= 0;
/*    pArea->recall		= ROOM_VNUM_TEMPLE;      ROM OLC */

	return pArea;
}



void free_area(AREA_DATA *pArea)
{
    free_string(pArea->name);
    free_string(pArea->file_name);
    free_string(pArea->builders);
    free_string(pArea->credits);

    pArea->next         =   area_free->next;
    area_free           =   pArea;
    return;
}



EXIT_DATA *new_exit(void)
{
    EXIT_DATA *pExit;

    if (!exit_free)
    {
        pExit           =   alloc_perm(sizeof(*pExit));
    	pExit->description  =   NULL;
        top_exit++;
    }
    else
    {
        pExit           =   exit_free;
        exit_free       =   exit_free->next;
    }

    pExit->u1.to_room   =   NULL;                  /* ROM OLC */
    pExit->next         =   NULL;
/*  pExit->vnum         =   0;                        ROM OLC */
    pExit->exit_info    =   0;
    pExit->key          =   0;
    pExit->keyword      =   &str_empty[0];
    pExit->rs_flags     =   0;

    return pExit;
}



void free_exit(EXIT_DATA *pExit)
{
    free_string(pExit->keyword);
    mlstr_free(pExit->description);

    pExit->next         =   exit_free;
    exit_free           =   pExit;
    return;
}


ROOM_INDEX_DATA *new_room_index(void)
{
    ROOM_INDEX_DATA *pRoom;
    int door;

    if (!room_index_free)
    {
        pRoom           =   alloc_perm(sizeof(*pRoom));
    	pRoom->name             =   NULL;
    	pRoom->description      =   NULL;
        top_room++;
    }
    else
    {
        pRoom           =   room_index_free;
        room_index_free =   room_index_free->next;
    }

    pRoom->next             =   NULL;
    pRoom->people           =   NULL;
    pRoom->contents         =   NULL;
    pRoom->ed      =   NULL;
    pRoom->area             =   NULL;

    for (door=0; door < MAX_DIR; door++)
        pRoom->exit[door]   =   NULL;

    pRoom->owner	    =	&str_empty[0];
    pRoom->vnum             =   0;
    pRoom->room_flags       =   0;
    pRoom->light            =   0;
    pRoom->sector_type      =   0;
    pRoom->clan		    =	0;
    pRoom->heal_rate	    =   100;
    pRoom->mana_rate	    =   100;

    return pRoom;
}



void free_room_index(ROOM_INDEX_DATA *pRoom)
{
    int door;
    ED_DATA *pExtra;
    RESET_DATA *pReset;

    mlstr_free(pRoom->name);
    mlstr_free(pRoom->description);
    free_string(pRoom->owner);

    for (door = 0; door < MAX_DIR; door++)
    {
        if (pRoom->exit[door])
            free_exit(pRoom->exit[door]);
    }

    for (pExtra = pRoom->ed; pExtra; pExtra = pExtra->next)
    {
        ed_free(pExtra);
    }

    for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
    {
        free_reset_data(pReset);
    }

    pRoom->next     =   room_index_free;
    room_index_free =   pRoom;
    return;
}

extern AFFECT_DATA *affect_free;


SHOP_DATA *new_shop(void)
{
    SHOP_DATA *pShop;
    int buy;

    if (!shop_free)
    {
        pShop           =   alloc_perm(sizeof(*pShop));
        top_shop++;
    }
    else
    {
        pShop           =   shop_free;
        shop_free       =   shop_free->next;
    }

    pShop->next         =   NULL;
    pShop->keeper       =   0;

    for (buy=0; buy<MAX_TRADE; buy++)
        pShop->buy_type[buy]    =   0;

    pShop->profit_buy   =   100;
    pShop->profit_sell  =   100;
    pShop->open_hour    =   0;
    pShop->close_hour   =   23;

    return pShop;
}



void free_shop(SHOP_DATA *pShop)
{
    pShop->next = shop_free;
    shop_free   = pShop;
    return;
}



OBJ_INDEX_DATA *new_obj_index(void)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (!obj_index_free) {
        pObj           =   alloc_perm(sizeof(*pObj));
        top_obj_index++;
    }
    else {
        pObj            =   obj_index_free;
        obj_index_free  =   obj_index_free->next;
    }

    pObj->short_descr   =   NULL;
    pObj->description   =   NULL;
    pObj->next          =   NULL;
    pObj->ed		=   NULL;
    pObj->affected      =   NULL;
    pObj->area          =   NULL;
    pObj->name          =   str_dup("no name");
    pObj->vnum          =   0;
    pObj->item_type     =   ITEM_TRASH;
    pObj->extra_flags   =   0;
    pObj->wear_flags    =   0;
    pObj->count         =   0;
    pObj->weight        =   0;
    pObj->cost          =   0;
    pObj->material      =   str_dup("unknown");      /* ROM */
    pObj->condition     =   100;                        /* ROM */
    pObj->limit		=   -1;
    pObj->oprogs	= NULL;
    for (value = 0; value < 5; value++)               /* 5 - ROM */
        pObj->value[value]  =   0;

    pObj->new_format    = TRUE; /* ROM */

    return pObj;
}



void free_obj_index(OBJ_INDEX_DATA *pObj)
{
    ED_DATA *pExtra;
    AFFECT_DATA *pAf;

    free_string(pObj->name);
    mlstr_free(pObj->short_descr);
    mlstr_free(pObj->description);

    for (pAf = pObj->affected; pAf; pAf = pAf->next)
        free_affect(pAf);

    for (pExtra = pObj->ed; pExtra; pExtra = pExtra->next)
        ed_free(pExtra);
    
    pObj->next              = obj_index_free;
    obj_index_free          = pObj;
    return;
}



MOB_INDEX_DATA *new_mob_index(void)
{
    MOB_INDEX_DATA *pMob;

    if (!mob_index_free)
    {
        pMob           =   alloc_perm(sizeof(*pMob));
        top_mob_index++;
    }
    else
    {
        pMob            =   mob_index_free;
        mob_index_free  =   mob_index_free->next;
    }

    pMob->short_descr   =   NULL;
    pMob->long_descr    =   NULL;
    pMob->description   =   NULL;
    pMob->next          =   NULL;
    pMob->spec_fun      =   NULL;
    pMob->pShop         =   NULL;
    pMob->area          =   NULL;
    pMob->name   =   str_dup("no name");
    pMob->vnum          =   0;
    pMob->count         =   0;
    pMob->killed        =   0;
    pMob->sex           =   0;
    pMob->level         =   0;
    pMob->act           =   ACT_NPC;
    pMob->affected_by   =   0;
    pMob->alignment     =   0;
    pMob->hitroll	=   0;
    pMob->race          =   race_lookup("human"); /* - Hugin */
    pMob->form          =   0;           /* ROM patch -- Hugin */
    pMob->parts         =   0;           /* ROM patch -- Hugin */
    pMob->imm_flags     =   0;           /* ROM patch -- Hugin */
    pMob->res_flags     =   0;           /* ROM patch -- Hugin */
    pMob->vuln_flags    =   0;           /* ROM patch -- Hugin */
    pMob->material      =   str_dup("unknown"); /* -- Hugin */
    pMob->off_flags     =   0;           /* ROM patch -- Hugin */
    pMob->size          =   SIZE_MEDIUM; /* ROM patch -- Hugin */
    pMob->ac[AC_PIERCE]	=   0;           /* ROM patch -- Hugin */
    pMob->ac[AC_BASH]	=   0;           /* ROM patch -- Hugin */
    pMob->ac[AC_SLASH]	=   0;           /* ROM patch -- Hugin */
    pMob->ac[AC_EXOTIC]	=   0;           /* ROM patch -- Hugin */
    pMob->hit[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->hit[DICE_TYPE]	=   0;   /* ROM patch -- Hugin */
    pMob->hit[DICE_BONUS]	=   0;   /* ROM patch -- Hugin */
    pMob->mana[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->mana[DICE_TYPE]	=   0;   /* ROM patch -- Hugin */
    pMob->mana[DICE_BONUS]	=   0;   /* ROM patch -- Hugin */
    pMob->damage[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->damage[DICE_TYPE]	=   0;   /* ROM patch -- Hugin */
    pMob->damage[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->start_pos             =   POS_STANDING; /*  -- Hugin */
    pMob->default_pos           =   POS_STANDING; /*  -- Hugin */
    pMob->wealth                =   0;

    pMob->new_format            = TRUE;  /* ROM */

    return pMob;
}



void free_mob_index(MOB_INDEX_DATA *pMob)
{
    free_string(pMob->name);
    mlstr_free(pMob->short_descr);
    mlstr_free(pMob->long_descr);
    mlstr_free(pMob->description);
    free_mprog(pMob->mprogs);

    free_shop(pMob->pShop);

    pMob->next		= mob_index_free;
    mob_index_free	= pMob;
    return;
}

MPROG_CODE              *       mpcode_free;

MPROG_CODE *new_mpcode(void)
{
     MPROG_CODE *NewCode;

     if (!mpcode_free)
     {
         NewCode = alloc_perm(sizeof(*NewCode));
         top_mprog_index++;
     }
     else
     {
         NewCode     = mpcode_free;
         mpcode_free = mpcode_free->next;
     }

     NewCode->vnum    = 0;
     NewCode->code    = str_dup("");
     NewCode->next    = NULL;

     return NewCode;
}

void free_mpcode(MPROG_CODE *pMcode)
{
    free_string(pMcode->code);
    pMcode->next = mpcode_free;
    mpcode_free  = pMcode;
    return;
}
