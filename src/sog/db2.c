/*
 * $Id: db2.c,v 1.4 1998-04-26 17:08:05 efdi Exp $
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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include "merc.h"
#include "db.h"
#include "lookup.h"


/* values for db2.c */
struct		social_type	social_table		[MAX_SOCIALS];
int		social_count;

/* snarf a socials file */
void load_socials( FILE *fp)
{
    for ( ; ; ) 
    {
    	struct social_type social;
    	char *temp;
        /* clear social */
	social.char_no_arg = NULL;
	social.others_no_arg = NULL;
	social.char_found = NULL;
	social.others_found = NULL;
	social.vict_found = NULL; 
	social.char_not_found = NULL;
	social.char_auto = NULL;
	social.others_auto = NULL;

    	temp = fread_word(fp);
    	if (!strcmp(temp,"#0"))
	    return;  /* done */
#if defined(social_debug) 
	else 
	    fprintf(stderr,"%s\n\r",temp);
#endif

    	strcpy(social.name,temp);
    	fread_to_eol(fp);

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.char_no_arg = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue; 
	}
        else
	    social.char_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_no_arg = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.others_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
       	else
	    social.char_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.others_found = temp; 

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.vict_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.vict_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_not_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.char_not_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.char_auto = temp;
         
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.others_auto = temp; 
	
	social_table[social_count] = social;
    	social_count++;
   }
   return;
}
    





/*
 * Snarf a mob section.  new style
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
 
    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_mob_index( vnum ) != NULL )
        {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pMobIndex                       = alloc_perm( sizeof(*pMobIndex) );
        pMobIndex->vnum                 = vnum;
	pMobIndex->new_format		= TRUE;
	newmobs++;
        pMobIndex->player_name          = fread_string( fp );
        pMobIndex->short_descr          = fread_string( fp );
        pMobIndex->long_descr           = fread_string( fp );
        pMobIndex->description          = fread_string( fp );
	pMobIndex->race		 	= race_lookup(fread_string( fp ));
 
        pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
 
        pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC
					| race_table[pMobIndex->race].act;

        pMobIndex->affected_by          = fread_flag( fp )
					| race_table[pMobIndex->race].aff;
	pMobIndex->practicer		= 0;
	pMobIndex->detection		= race_table[pMobIndex->race].det;

/* chronos smashed affection of ROM and created detection of ANATOLIA */
	if (IS_AFFECTED(pMobIndex,C))	/* detect evil */
		 SET_BIT(pMobIndex->detection,C);
	if (IS_AFFECTED(pMobIndex,D))	/* detect invis */
		 SET_BIT(pMobIndex->detection,D);
	if (IS_AFFECTED(pMobIndex,E))	/* detect magic */
		 SET_BIT(pMobIndex->detection,E);
	if (IS_AFFECTED(pMobIndex,F))	/* detect hidden */
		 SET_BIT(pMobIndex->detection,F);
	if (IS_AFFECTED(pMobIndex,G))	/* detect good */
		 SET_BIT(pMobIndex->detection,G);
	if (IS_AFFECTED(pMobIndex,Z))	/* dark vision */
		 SET_BIT(pMobIndex->detection,Z);
	if (IS_AFFECTED(pMobIndex,ee))	/* acute vision */
		 SET_BIT(pMobIndex->detection,ee);
	REMOVE_BIT(pMobIndex->affected_by,(C|D|E|F|G|Z|ee));

        pMobIndex->pShop                = NULL;
        pMobIndex->alignment            = fread_number( fp );
        pMobIndex->group                = fread_number( fp );

        pMobIndex->level                = fread_number( fp );
        pMobIndex->hitroll              = fread_number( fp );  

	/* read hit dice */
        pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
        /* 'd'          */                fread_letter( fp ); 
        pMobIndex->hit[DICE_TYPE]   	= fread_number( fp );
        /* '+'          */                fread_letter( fp );   
        pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

 	/* read mana dice */
	pMobIndex->mana[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->mana[DICE_BONUS]	= fread_number( fp );

	/* read damage dice */
	pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_BONUS]	= fread_number( fp );
	pMobIndex->dam_type		= attack_lookup(fread_word(fp));

	/* read armor class */
	pMobIndex->ac[AC_PIERCE]	= fread_number( fp ) * 10;
	pMobIndex->ac[AC_BASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_SLASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_EXOTIC]	= fread_number( fp ) * 10;

	/* read flags and add in data from the race table */
	pMobIndex->off_flags		= fread_flag( fp ) 
					| race_table[pMobIndex->race].off;
	pMobIndex->imm_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].imm;
	pMobIndex->res_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].res;
	pMobIndex->vuln_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].vuln;

	/* vital statistics */
	pMobIndex->start_pos		= position_lookup(fread_word(fp));
	pMobIndex->default_pos		= position_lookup(fread_word(fp));
	pMobIndex->sex			= sex_lookup(fread_word(fp));

	pMobIndex->wealth		= fread_number( fp );

	pMobIndex->form			= fread_flag( fp )
					| race_table[pMobIndex->race].form;
	pMobIndex->parts		= fread_flag( fp )
					| race_table[pMobIndex->race].parts;
	/* size */
	pMobIndex->size			= size_lookup(fread_word(fp));
	pMobIndex->material		= str_dup(fread_word( fp ));
	pMobIndex->mprogs		= NULL;
	pMobIndex->progtypes		= 0;
 
	for ( ; ; )
        {
            letter = fread_letter( fp );

            if (letter == 'F')
            {
		char *word;
		long vector;

                word                    = fread_word(fp);
		vector			= fread_flag(fp);

		if (!str_prefix(word,"act"))
		    REMOVE_BIT(pMobIndex->act,vector);
                else if (!str_prefix(word,"aff"))
		    REMOVE_BIT(pMobIndex->affected_by,vector);
		else if (!str_prefix(word,"off"))
		    REMOVE_BIT(pMobIndex->affected_by,vector);
		else if (!str_prefix(word,"imm"))
		    REMOVE_BIT(pMobIndex->imm_flags,vector);
		else if (!str_prefix(word,"res"))
		    REMOVE_BIT(pMobIndex->res_flags,vector);
		else if (!str_prefix(word,"vul"))
		    REMOVE_BIT(pMobIndex->vuln_flags,vector);
		else if (!str_prefix(word,"for"))
		    REMOVE_BIT(pMobIndex->form,vector);
		else if (!str_prefix(word,"par"))
		    REMOVE_BIT(pMobIndex->parts,vector);
		else
		{
		    bug("Flag remove: flag not found.",0);
		    exit(1);
		}
	     }
	     else
	     {
		ungetc(letter,fp);
		break;
	     }
	}

        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;
        top_mob_index++;
        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }
 
    return;
}

/*
 * Snarf an obj section. new style
 */
void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
 
    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_obj_index( vnum ) != NULL )
        {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pObjIndex                       = alloc_perm( sizeof(*pObjIndex) );
        pObjIndex->vnum                 = vnum;
        pObjIndex->new_format           = TRUE;
	pObjIndex->reset_num		= 0;
	newobjs++;
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );
        pObjIndex->material		= fread_string( fp );
 
        pObjIndex->item_type            = item_lookup(fread_word( fp ));
        pObjIndex->extra_flags          = fread_flag( fp );
        pObjIndex->wear_flags           = fread_flag( fp );
	switch(pObjIndex->item_type)
	{
	case ITEM_WEAPON:
	    pObjIndex->value[0]		= weapon_type(fread_word(fp));
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= attack_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_flag(fp);
	    break;
	case ITEM_CONTAINER:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_flag(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= fread_number(fp);
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
        case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
            pObjIndex->value[0]         = fread_number(fp);
            pObjIndex->value[1]         = fread_number(fp);
            pObjIndex->value[2]         = liq_lookup(fread_word(fp));
            pObjIndex->value[3]         = fread_number(fp);
            pObjIndex->value[4]         = fread_number(fp);
            break;
	case ITEM_WAND:
	case ITEM_STAFF:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
 	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[2]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= skill_lookup(fread_word(fp));
	    break;
	default:
            pObjIndex->value[0]             = fread_flag( fp );
            pObjIndex->value[1]             = fread_flag( fp );
            pObjIndex->value[2]             = fread_flag( fp );
            pObjIndex->value[3]             = fread_flag( fp );
	    pObjIndex->value[4]		    = fread_flag( fp );
	    break;
	}
	pObjIndex->level		= fread_number( fp );
        pObjIndex->weight               = fread_number( fp );
        pObjIndex->cost                 = fread_number( fp ); 
        pObjIndex->progtypes            = 0;
        pObjIndex->oprogs               = NULL; 
        pObjIndex->limit                = -1; 

        /* condition */
        letter 				= fread_letter( fp );
	switch (letter)
 	{
	    case ('P') :		pObjIndex->condition = 100; break;
	    case ('G') :		pObjIndex->condition =  90; break;
	    case ('A') :		pObjIndex->condition =  75; break;
	    case ('W') :		pObjIndex->condition =  50; break;
	    case ('D') :		pObjIndex->condition =  25; break;
	    case ('B') :		pObjIndex->condition =  10; break;
	    case ('R') :		pObjIndex->condition =   0; break;
	    default:			pObjIndex->condition = 100; break;
	}
 
        for ( ; ; )
        {
            char letter;
 
            letter = fread_letter( fp );
 
            if ( letter == 'A' )
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm( sizeof(*paf) );
		paf->where		= TO_OBJECT;
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number( fp );
                paf->modifier           = fread_number( fp );
                paf->bitvector          = 0;
                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
                top_affect++;
            }

	    else if (letter == 'F')
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm( sizeof(*paf) );
		letter 			= fread_letter(fp);
		switch (letter)
	 	{
		case 'A':
                    paf->where          = TO_AFFECTS;
		    break;
		case 'I':
		    paf->where		= TO_IMMUNE;
		    break;
		case 'R':
		    paf->where		= TO_RESIST;
		    break;
		case 'V':
		    paf->where		= TO_VULN;
		    break;
		case 'D':
		    paf->where		= TO_DETECTS;
		    break;
		default:
            	    bug( "Load_objects: Bad where on flag set.", 0 );
            	   exit( 1 );
		}
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number(fp);
                paf->modifier           = fread_number(fp);
                paf->bitvector          = fread_flag(fp);
                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
                top_affect++;
            }
 
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;
 
                ed                      = alloc_perm( sizeof(*ed) );
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pObjIndex->extra_descr;
                pObjIndex->extra_descr  = ed;
                top_ed++;
            }
 
            else
            {
                ungetc( letter, fp );
                break;
            }
        }
 
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        top_obj_index++;
    }
 
    return;
}

/*
 * Snarf a mprog section
 */

void load_omprogs( FILE *fp )
{
  char progtype[MAX_INPUT_LENGTH];
  char progname[MAX_INPUT_LENGTH];
  
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	char letter;
	

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_omprogs: letter '%c' not *IMS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;
	    
        case 'O':
	    pObjIndex = get_obj_index ( fread_number ( fp ) );
	    if (pObjIndex->oprogs == NULL)
	      pObjIndex->oprogs = alloc_perm(sizeof(OPROG_DATA));
	    
	    strcpy(progtype, fread_word(fp));
	    strcpy(progname, fread_word(fp));
	    oprog_set( pObjIndex, progtype, progname);
	    break;
	
	case 'M':
	    pMobIndex		= get_mob_index	( fread_number ( fp ) );
	    if (pMobIndex->mprogs == NULL)
	      pMobIndex->mprogs = alloc_perm(sizeof(MPROG_DATA));

	    strcpy(progtype,fread_word(fp));
	    strcpy(progname,fread_word(fp));
	    mprog_set( pMobIndex,progtype,progname);
	    break;  
	}

	fread_to_eol( fp );
    }
}




