/*
 * $Id: olc_save.c,v 1.24 1998-09-01 18:38:09 fjoe Exp $
 */

/**************************************************************************
 *  File: olc_save.c                                                       *
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
/* OLC_SAVE.C
 * This takes care of saving all the .are information.
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "obj_prog.h"
#include "interp.h"

#define DIF(a,b) (~((~a)|(b)))

/*
 *  Verbose writes reset data in plain english into the comments
 *  section of the resets.  It makes areas considerably larger but
 *  may aid in debugging.
 */
#define VERBOSE

static void save_print(CHAR_DATA *ch, const char *format, ...);

/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list()
{
	FILE *fp;
	AREA_DATA *pArea;

	if ((fp = dfopen(AREA_PATH, AREA_LIST, "w")) == NULL) {
		bug("Save_area_list: fopen", 0);
		perror(AREA_LIST);
	}
	else {
		/*
		 * Add any help files that need to be loaded at
		 * startup to this section.
		 */
		fprintf(fp, "social.are\n");    /* ROM OLC */

		for (pArea = area_first; pArea; pArea = pArea->next)
			fprintf(fp, "%s\n", pArea->file_name);

		fprintf(fp, "$\n");
		fclose(fp);
	}
}

void save_mobprogs(FILE *fp, AREA_DATA *pArea)
{
	MPCODE *mpcode;
        int i;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++) {
        	if ((mpcode = mpcode_lookup(i)) != NULL) {
			if (!found) {
        			fprintf(fp, "#MOBPROGS\n");
				found = TRUE;
			}
			fprintf(fp, "#%d\n", i);
			fprintf(fp, "%s~\n", fix_string(mpcode->code));
		}
        }

	if (found)
        	fprintf(fp,"#0\n\n");
}

/*****************************************************************************
 Name:		save_mobile
 Purpose:	Save one mobile to file, new format -- Hugin
 Called by:	save_mobiles (below).
 ****************************************************************************/
void save_mobile(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
    int race = pMobIndex->race;
    MPTRIG *mptrig;
    flag_t temp;

    fprintf(fp, "#%d\n",	pMobIndex->vnum);
    fprintf(fp, "%s~\n",	pMobIndex->name);
    mlstr_fwrite(fp, NULL,	pMobIndex->short_descr);
    mlstr_fwrite(fp, NULL,	pMobIndex->long_descr);
    mlstr_fwrite(fp, NULL,	pMobIndex->description);
    fprintf(fp, "%s~\n",	race_table[race].name);
    fprintf(fp, "%s ",		format_flags(pMobIndex->act &
					    ~race_table[pMobIndex->race].act));
    fprintf(fp, "%s ",		format_flags(pMobIndex->affected_by &
					    ~race_table[pMobIndex->race].aff));
    fprintf(fp, "%d %d\n",	pMobIndex->alignment , pMobIndex->group);
    fprintf(fp, "%d ",		pMobIndex->level);
    fprintf(fp, "%d ",		pMobIndex->hitroll);
    fprintf(fp, "%dd%d+%d ",	pMobIndex->hit[DICE_NUMBER], 
				pMobIndex->hit[DICE_TYPE], 
				pMobIndex->hit[DICE_BONUS]);
    fprintf(fp, "%dd%d+%d ",	pMobIndex->mana[DICE_NUMBER], 
				pMobIndex->mana[DICE_TYPE], 
				pMobIndex->mana[DICE_BONUS]);
    fprintf(fp, "%dd%d+%d ",	pMobIndex->damage[DICE_NUMBER], 
				pMobIndex->damage[DICE_TYPE], 
				pMobIndex->damage[DICE_BONUS]);
    fprintf(fp, "%s\n",	attack_table[pMobIndex->dam_type].name);
    fprintf(fp, "%d %d %d %d\n",
				pMobIndex->ac[AC_PIERCE] / 10, 
				pMobIndex->ac[AC_BASH]   / 10, 
				pMobIndex->ac[AC_SLASH]  / 10, 
				pMobIndex->ac[AC_EXOTIC] / 10);
    fprintf(fp, "%s ",		format_flags(pMobIndex->off_flags &
					    ~race_table[pMobIndex->race].off));
    fprintf(fp, "%s ",		format_flags(pMobIndex->imm_flags &
					    ~race_table[pMobIndex->race].imm));
    fprintf(fp, "%s ",		format_flags(pMobIndex->res_flags &
					    ~race_table[pMobIndex->race].res));
    fprintf(fp, "%s\n",		format_flags(pMobIndex->vuln_flags &
					    ~race_table[pMobIndex->race].vuln));
    fprintf(fp, "%s %s %s %d\n",
			flag_string(position_table, pMobIndex->start_pos),
			flag_string(position_table, pMobIndex->default_pos),
			flag_string(sex_table, pMobIndex->sex),
			pMobIndex->wealth);
    fprintf(fp, "%s ",		format_flags(pMobIndex->form &
					    ~race_table[pMobIndex->race].form));
    fprintf(fp, "%s ",		format_flags(pMobIndex->parts &
					   ~race_table[pMobIndex->race].parts));

    fprintf(fp, "%s ",		size_table[pMobIndex->size].name);
    fprintf(fp, "%s\n",	IS_NULLSTR(pMobIndex->material) ? pMobIndex->material : "unknown");

/* save diffs */
    if ((temp = DIF(race_table[race].act,pMobIndex->act)))
     	fprintf(fp, "F act %s\n", format_flags(temp));

    if ((temp = DIF(race_table[race].aff,pMobIndex->affected_by)))
     	fprintf(fp, "F aff %s\n", format_flags(temp));

    if ((temp = DIF(race_table[race].off,pMobIndex->off_flags)))
     	fprintf(fp, "F off %s\n", format_flags(temp));

    if ((temp = DIF(race_table[race].imm,pMobIndex->imm_flags)))
     	fprintf(fp, "F imm %s\n", format_flags(temp));

    if ((temp = DIF(race_table[race].res,pMobIndex->res_flags)))
     	fprintf(fp, "F res %s\n", format_flags(temp));

    if ((temp = DIF(race_table[race].vuln,pMobIndex->vuln_flags)))
     	fprintf(fp, "F vul %s\n", format_flags(temp));

    if ((temp = DIF(race_table[race].form,pMobIndex->form)))
     	fprintf(fp, "F for %s\n", format_flags(temp));

    if ((temp = DIF(race_table[race].parts,pMobIndex->parts)))
    	fprintf(fp, "F par %s\n", format_flags(temp));

    for (mptrig = pMobIndex->mptrig_list; mptrig; mptrig = mptrig->next)
    {
        fprintf(fp, "M %s %d %s~\n",
        flag_string(mptrig_types, mptrig->type), mptrig->vnum,
                mptrig->phrase);
    }

    if (pMobIndex->clan)
		fprintf(fp, "C %s~\n", clan_name(pMobIndex->clan));
}


/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_mobiles(FILE *fp, AREA_DATA *pArea)
{
	int i;
	MOB_INDEX_DATA *pMob;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMob = get_mob_index(i))) {
			if (!found) {
				fprintf(fp, "#MOBILES\n");
				found = TRUE;
			}
			save_mobile(fp, pMob);
		}

	if (found)
		fprintf(fp, "#0\n\n");
}





/*****************************************************************************
 Name:		save_object
 Purpose:	Save one object to file.
                new ROM format saving -- Hugin
 Called by:	save_objects (below).
 ****************************************************************************/
void save_object(FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
    char letter;
    AFFECT_DATA *pAf;
    ED_DATA *pEd;

    fprintf(fp, "#%d\n",	pObjIndex->vnum);
    fprintf(fp, "%s~\n",	pObjIndex->name);
    mlstr_fwrite(fp, NULL,	pObjIndex->short_descr);
    mlstr_fwrite(fp, NULL,	pObjIndex->description);
    fprintf(fp, "%s~\n",	pObjIndex->material);
    fprintf(fp, "%s ",		flag_string(item_types, pObjIndex->item_type));
    fprintf(fp, "%s ",		format_flags(pObjIndex->extra_flags));
    fprintf(fp, "%s\n",		format_flags(pObjIndex->wear_flags));

/*
 *  Using format_flags to write most values gives a strange
 *  looking area file, consider making a case for each
 *  item type later.
 */

    switch (pObjIndex->item_type)
    {
        default:
	    fprintf(fp, "%s %s %s %s %s\n",
			format_flags(pObjIndex->value[0]),
	    		format_flags(pObjIndex->value[1]),
	    		format_flags(pObjIndex->value[2]),
	    		format_flags(pObjIndex->value[3]),
	    		format_flags(pObjIndex->value[4]));
	    break;

	case ITEM_MONEY:
	case ITEM_ARMOR:
	    fprintf(fp, "%d %d %d %d %d\n",
			pObjIndex->value[0],
	    		pObjIndex->value[1],
	    		pObjIndex->value[2],
	    		pObjIndex->value[3],
	    		pObjIndex->value[4]);
		break;

        case ITEM_DRINK_CON:
        case ITEM_FOUNTAIN:
            fprintf(fp, "%d %d '%s' %d %d\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     liq_table[pObjIndex->value[2]].liq_name,
		     pObjIndex->value[3],
		     pObjIndex->value[4]);
            break;

        case ITEM_CONTAINER:
            fprintf(fp, "%d %s %d %d %d\n",
                     pObjIndex->value[0],
                     format_flags(pObjIndex->value[1]),
                     pObjIndex->value[2],
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;

        case ITEM_WEAPON:
            fprintf(fp, "%s %d %d %s %s\n",
                     flag_string(weapon_class, pObjIndex->value[0]),
                     pObjIndex->value[1],
                     pObjIndex->value[2],
                     attack_table[pObjIndex->value[3]].name,
                     format_flags(pObjIndex->value[4]));
            break;
            
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
	    fprintf(fp, "%d '%s' '%s' '%s' '%s'\n",
/* no negative numbers */
		     pObjIndex->value[0] > 0 ? pObjIndex->value[0] : 0,
		     pObjIndex->value[1] != -1 ?
			skill_name(pObjIndex->value[1]) : "",
		     pObjIndex->value[2] != -1 ?
		     	skill_name(pObjIndex->value[2]) : "",
		     pObjIndex->value[3] != -1 ?
		     	skill_name(pObjIndex->value[3]) : "",
		     pObjIndex->value[4] != -1 ?
		     	skill_name(pObjIndex->value[4]) : "");
	    break;

        case ITEM_STAFF:
        case ITEM_WAND:
	    fprintf(fp, "%d %d %d '%s' %d\n",
	    			pObjIndex->value[0],
	    			pObjIndex->value[1],
	    			pObjIndex->value[2],
	    			pObjIndex->value[3] != -1 ?
	    				skill_name(pObjIndex->value[3]) : "",
	    			pObjIndex->value[4]);
	    break;
	case ITEM_PORTAL:
	    fprintf(fp, "%s %s %s %d %s\n",
			format_flags(pObjIndex->value[0]),
	    		format_flags(pObjIndex->value[1]),
	    		format_flags(pObjIndex->value[2]),
	    		pObjIndex->value[3],
	    		format_flags(pObjIndex->value[4]));
	    break;
	case ITEM_LIGHT:
	case ITEM_TATTOO:
	case ITEM_TREASURE:
	    fprintf(fp, "%s %s %d %s %s\n",
			format_flags(pObjIndex->value[0]),
	    		format_flags(pObjIndex->value[1]),
	    		pObjIndex->value[2],
	    		format_flags(pObjIndex->value[3]),
	    		format_flags(pObjIndex->value[4]));
	    break;
    }

    fprintf(fp, "%d ", pObjIndex->level);
    fprintf(fp, "%d ", pObjIndex->weight);
    fprintf(fp, "%d ", pObjIndex->cost);

         if (pObjIndex->condition >  90) letter = 'P';
    else if (pObjIndex->condition >  75) letter = 'G';
    else if (pObjIndex->condition >  50) letter = 'A';
    else if (pObjIndex->condition >  25) letter = 'W';
    else if (pObjIndex->condition >  10) letter = 'D';
    else if (pObjIndex->condition >   0) letter = 'B';
    else                                   letter = 'R';

    fprintf(fp, "%c\n", letter);

    for (pAf = pObjIndex->affected; pAf; pAf = pAf->next)
    {
	if (pAf->where == TO_OBJECT || pAf->bitvector == 0)
	        fprintf(fp, "A\n%d %d\n",  pAf->location, pAf->modifier);
	else
	{
		fprintf(fp, "F\n");

		switch(pAf->where)
		{
			case TO_AFFECTS:
				fprintf(fp, "A ");
				break;
			case TO_IMMUNE:
				fprintf(fp, "I ");
				break;
			case TO_RESIST:
				fprintf(fp, "R ");
				break;
			case TO_VULN:
				fprintf(fp, "V ");
				break;
			default:
				log_printf("olc_save: vnum %d: "
					   "invalid affect->where: %d",
					   pObjIndex->vnum, pAf->where);
				break;
		}
		
		fprintf(fp, "%d %d %s\n", pAf->location, pAf->modifier,
				format_flags(pAf->bitvector));
	}
    }

    for (pEd = pObjIndex->ed; pEd; pEd = pEd->next) {
        fprintf(fp, "E\n%s~\n", pEd->keyword);
	mlstr_fwrite(fp, NULL, pEd->description);
    }

    if (pObjIndex->clan)
	fprintf(fp, "C %s~\n", clan_name(pObjIndex->clan));
}
 



/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_objects(FILE *fp, AREA_DATA *pArea)
{
	int i;
	OBJ_INDEX_DATA *pObj;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pObj = get_obj_index(i))) {
			if (!found) {
    				fprintf(fp, "#OBJECTS\n");
				found = TRUE;
			}
			save_object(fp, pObj);
    		}

	if (found)
		fprintf(fp, "#0\n\n");
}

static int exitcmp(const void *p1, const void *p2)
{
	return (*(EXIT_DATA**)p1)->orig_door - (*(EXIT_DATA**)p2)->orig_door;
}

void save_room(FILE *fp, ROOM_INDEX_DATA *pRoomIndex)
{
	int door;
	ED_DATA *pEd;
	EXIT_DATA *pExit;
	EXIT_DATA *exit[MAX_DIR];
	int max_door;

        fprintf(fp, "#%d\n",	pRoomIndex->vnum);
	mlstr_fwrite(fp, NULL,	pRoomIndex->name);
	mlstr_fwrite(fp, NULL,	pRoomIndex->description);
	fprintf(fp, "0 ");
        fprintf(fp, "%s ",	format_flags(pRoomIndex->room_flags));
        fprintf(fp, "%s\n",	flag_string(sector_types,
					    pRoomIndex->sector_type));

        for (pEd = pRoomIndex->ed; pEd; pEd = pEd->next) {
        	fprintf(fp, "E\n%s~\n", pEd->keyword);
		mlstr_fwrite(fp, NULL, pEd->description);
	}

	/* sort exits (to minimize diffs) */
	for (max_door = 0, door = 0; door < MAX_DIR; door++)
		if ((pExit = pRoomIndex->exit[door]))
			exit[max_door++] = pExit;
	qsort(exit, max_door, sizeof(*exit), exitcmp);

	for (door = 0; door < max_door; door++) {
		pExit = exit[door];
		if (pExit->u1.to_room) {
 
	 		/* HACK : TO PREVENT EX_LOCKED etc without EX_ISDOOR
 			   to stop booting the mud */
 			if (IS_SET(pExit->rs_flags, EX_CLOSED)
 			||  IS_SET(pExit->rs_flags, EX_LOCKED)
 			||  IS_SET(pExit->rs_flags, EX_PICKPROOF)
 			||  IS_SET(pExit->rs_flags, EX_NOPASS)
 			||  IS_SET(pExit->rs_flags, EX_EASY)
 			||  IS_SET(pExit->rs_flags, EX_HARD)
 			||  IS_SET(pExit->rs_flags, EX_INFURIATING)
 			||  IS_SET(pExit->rs_flags, EX_NOCLOSE)
 			||  IS_SET(pExit->rs_flags, EX_NOLOCK) )
 				SET_BIT(pExit->rs_flags, EX_ISDOOR);
 			else
 				REMOVE_BIT(pExit->rs_flags, EX_ISDOOR);
 
			fprintf(fp, "D%d\n",      pExit->orig_door);
			mlstr_fwrite(fp, NULL,	  pExit->description);
			fprintf(fp, "%s~\n",      pExit->keyword);
			fprintf(fp, "%s %d %d\n",
				format_flags(pExit->rs_flags | EX_BITVAL),
				pExit->key,
				pExit->u1.to_room->vnum);
		}
	}

	if (pRoomIndex->mana_rate != 100 || pRoomIndex->heal_rate != 100)
		fprintf (fp, "M %d H %d\n", pRoomIndex->mana_rate,
					    pRoomIndex->heal_rate);
		 			     
	if (!IS_NULLSTR(pRoomIndex->owner))
		fprintf (fp, "O %s~\n" , pRoomIndex->owner);

	if (pRoomIndex->clan)
		fprintf(fp, "C %s~\n", clan_name(pRoomIndex->clan));

	fprintf(fp, "S\n");
}

/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_rooms(FILE *fp, AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoomIndex;
	bool found = FALSE;
	int i;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pRoomIndex = get_room_index(i))) {
			if (!found) {
				fprintf(fp, "#ROOMS\n");
				found = TRUE;
			}
			save_room(fp, pRoomIndex);
		}

	if (found)
		fprintf(fp, "#0\n\n");
}

void save_special(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
#if defined(VERBOSE)
	fprintf(fp, "M %d %s\t* %s\n",
		pMobIndex->vnum,
		spec_name(pMobIndex->spec_fun),
		mlstr_mval(pMobIndex->short_descr));
#else
	fprintf(fp, "M %d %s\n",
		pMobIndex->vnum, spec_name(pMobIndex->spec_fun));
#endif
}

/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_specials(FILE *fp, AREA_DATA *pArea)
{
	int i;
	MOB_INDEX_DATA *pMobIndex;
	bool found = FALSE;
    
	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMobIndex = get_mob_index(i))
		&&  pMobIndex->spec_fun) {
			if (!found) {
				fprintf(fp, "#SPECIALS\n");
				found = TRUE;
			}
			save_special(fp, pMobIndex);
		}

	if (found)
		fprintf(fp, "S\n\n");
}



/*
 * This function is obsolete.  It it not needed but has been left here
 * for historical reasons.  It is used currently for the same reason.
 *
 * I don't think it's obsolete in ROM -- Hugin.
 */
void save_door_reset(FILE *fp, ROOM_INDEX_DATA *pRoomIndex, EXIT_DATA *pExit)
{
#if defined(VERBOSE)
	fprintf(fp,
		"D 0 %d %d %d\t* %s: door to the %s: %s\n", 
		pRoomIndex->vnum,
		pExit->orig_door,
		IS_SET(pExit->rs_flags, EX_LOCKED) ? 2 : 1,
		mlstr_mval(pRoomIndex->name),
		dir_name[pExit->orig_door],
		IS_SET(pExit->rs_flags, EX_LOCKED) ?
			"closed and locked" : "closed");
#else
	fprintf(fp, "D 0 %d %d %d\n", 
		pRoomIndex->vnum,
		pExit->orig_door,
		IS_SET(pExit->rs_flags, EX_LOCKED) ? 2 : 1);
#endif
}

void save_reset(FILE *fp, AREA_DATA *pArea,
		ROOM_INDEX_DATA *pRoomIndex, RESET_DATA *pReset)
{
	switch (pReset->command) {
	default:
		bug("Save_resets: bad command %c.", pReset->command);
		break;

#if defined(VERBOSE)
	case 'M':
		fprintf(fp, "M 0 %d %d %d %d\t* %s (%s)\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4,
			mlstr_mval(get_mob_index(pReset->arg1)->short_descr),
			mlstr_mval(get_room_index(pReset->arg3)->name));
		break;

	case 'O':
		fprintf(fp, "O 0 %d 0 %d\t* %s (%s)\n", 
			pReset->arg1,
			pReset->arg3,
			mlstr_mval(get_obj_index(pReset->arg1)->short_descr),
			mlstr_mval(get_room_index(pReset->arg3)->name));
		break;

	case 'P':
		fprintf(fp, "P 0 %d %d %d %d\t* %s: %s\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4,
			mlstr_mval(get_obj_index(pReset->arg3)->short_descr),
			mlstr_mval(get_obj_index(pReset->arg1)->short_descr));
		break;

	case 'G':
		fprintf(fp, "G 0 %d 0\t\t*\t%s\n",
			pReset->arg1,
			mlstr_mval(get_obj_index(pReset->arg1)->short_descr));
		break;

	case 'E':
		fprintf(fp, "E 0 %d 0 %d\t\t*\t%s: %s\n",
			pReset->arg1,
			pReset->arg3,
			mlstr_mval(get_obj_index(pReset->arg1)->short_descr),
			flag_string(wear_loc_strings, pReset->arg3));
		break;

	case 'D':
		break;

	case 'R':
		pRoomIndex = get_room_index(pReset->arg1);
		fprintf(fp, "R 0 %d %d\t* %s: randomize\n", 
			pReset->arg1,
			pReset->arg2,
			mlstr_mval(pRoomIndex->name));
		break;
#else
	case 'M':
		fprintf(fp, "M 0 %d %d %d %d\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4);
		break;

	case 'O':
		fprintf(fp, "O 0 %d 0 %d\n", 
			pReset->arg1,
			pReset->arg3);
		break;

	case 'P':
		fprintf(fp, "P 0 %d %d %d %d\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4);
		break;

	case 'G':
		fprintf(fp, "G 0 %d 0\n", pReset->arg1);
		break;

	case 'E':
		fprintf(fp, "E 0 %d 0 %d\n",
			pReset->arg1,
			pReset->arg3);
		break;

	case 'D':
		break;

	case 'R':
		fprintf(fp, "R 0 %d %d\n", 
			pReset->arg1,
			pReset->arg2);
		break;
#endif
	}
}


/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_resets(FILE *fp, AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoomIndex;
	RESET_DATA *pReset;
	EXIT_DATA *pExit;
	int door;
	bool found = FALSE;
	int i;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pRoomIndex = get_room_index(i)))
			for (door = 0; door < MAX_DIR; door++)
				if ((pExit = pRoomIndex->exit[door])
				&&  pExit->u1.to_room 
				&&  (IS_SET(pExit->rs_flags, EX_CLOSED) ||
				     IS_SET(pExit->rs_flags, EX_LOCKED))) {
					if (!found) {
						fprintf(fp, "#RESETS\n");
						found = TRUE;
					}
    					save_door_reset(fp, pRoomIndex, pExit);
				}

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pRoomIndex = get_room_index(i)))
    			for (pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next) {
				if (!found) {
					fprintf(fp, "#RESETS\n");
					found = TRUE;
				}
				save_reset(fp, pArea, pRoomIndex, pReset);
			}

	if (found)
		fprintf(fp, "S\n\n");
}


void save_shop(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
	SHOP_DATA *pShopIndex;
	int iTrade;

	pShopIndex = pMobIndex->pShop;

	fprintf(fp, "%d ", pShopIndex->keeper);
	for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
		if (pShopIndex->buy_type[iTrade] != 0)
			fprintf(fp, "%d ", pShopIndex->buy_type[iTrade]);
		else
			fprintf(fp, "0 ");
	}
	fprintf(fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell);
	fprintf(fp, "%d %d\n", pShopIndex->open_hour, pShopIndex->close_hour);
}

/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_shops(FILE *fp, AREA_DATA *pArea)
{
	MOB_INDEX_DATA *pMobIndex;
	int i;
	bool found = FALSE;
    
	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMobIndex = get_mob_index(i))
		&&  pMobIndex->pShop) {
			if (!found) {
				fprintf(fp, "#SHOPS\n");
				found = TRUE;
			}
			save_shop(fp, pMobIndex);
		}

	if (found)
		fprintf(fp, "0\n\n");
}


void save_olimits(FILE *fp, AREA_DATA *pArea)
{
	int i;
	OBJ_INDEX_DATA *pObj;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pObj = get_obj_index(i)) != NULL
		&&  pObj->limit != -1) {
			if (!found) {
				fprintf(fp, "#OLIMITS\n");
				found = TRUE;
			}
			fprintf(fp, "O %d %d\t* %s\n",
				i, pObj->limit, mlstr_mval(pObj->short_descr));
		}

	if (found)
		fprintf(fp, "S\n\n");
}


void save_omprog(FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
	int i;

	for (i = 0; i < OPROG_MAX; i++)
		if (pObjIndex->oprogs[i] != NULL)
			fprintf(fp, "O %d %s %s\t* `%s'\n",
				pObjIndex->vnum,
				optype_table[i],
				oprog_name_lookup(pObjIndex->oprogs[i]),
				mlstr_mval(pObjIndex->short_descr));
}


void save_omprogs(FILE *fp, AREA_DATA *pArea)
{
	int i;
	OBJ_INDEX_DATA *pObjIndex;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pObjIndex = get_obj_index(i)) != NULL
		&&  pObjIndex->oprogs) {
			if (!found) {
				fprintf(fp, "#OMPROGS\n");
				found = TRUE;
			}
			save_omprog(fp, pObjIndex);
		}

	if (found)
		fprintf(fp, "S\n\n");
}


void save_practicer(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
	const FLAG *f;

	for (f = skill_groups; f->name != NULL; f++)
		if (IS_SET(pMobIndex->practicer, f->bit))
    			fprintf(fp, "M %d %s\t* %s\n",
				pMobIndex->vnum,
				f->name,
				mlstr_mval(pMobIndex->short_descr));
}


void save_practicers(FILE *fp, AREA_DATA *pArea)
{
	int i;
	MOB_INDEX_DATA *pMobIndex;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMobIndex = get_mob_index(i)) != NULL
		&&  pMobIndex->practicer != 0) {
			if (!found) {
				fprintf(fp, "#PRACTICERS\n");
				found = TRUE;
			}
			save_practicer(fp, pMobIndex);
		}

	if (found)
		fprintf(fp, "S\n\n");
}


void save_helps(FILE *fp, AREA_DATA *pArea)
{
	HELP_DATA *pHelp = pArea->help_first;

	if (pHelp == NULL)
		return;
		
	fprintf(fp, "#HELPS\n");

	for (; pHelp; pHelp = pHelp->next_in_area) {
		fprintf(fp, "%d %s~\n", pHelp->level, pHelp->keyword);
		mlstr_fwrite(fp, NULL, pHelp->text);
	}

	fprintf(fp, "-1 $~\n\n");
}

/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area(AREA_DATA *pArea)
{
	FILE *fp;
	int flags;

	fclose(fpReserve);
	if (!(fp = dfopen(AREA_PATH, pArea->file_name, "w"))) {
		bug("Open_area: fopen", 0);
		perror(pArea->file_name);
	}

	fprintf(fp, "#AREADATA\n");
	fprintf(fp, "Name %s~\n",	pArea->name);
	fprintf(fp, "Builders %s~\n",	fix_string(pArea->builders));
	fprintf(fp, "VNUMs %d %d\n",	pArea->min_vnum, pArea->max_vnum);
	fprintf(fp, "Credits %s~\n",	pArea->credits);
	fprintf(fp, "Security %d\n",	pArea->security);
	fprintf(fp, "LevelRange %d %d\n",	pArea->min_level, pArea->max_level);
	if (!mlstr_null(pArea->resetmsg))
		mlstr_fwrite(fp, "ResetMessage", pArea->resetmsg);
	flags = pArea->flags & ~AREA_CHANGED;
	if (flags)
		fprintf(fp, "Flags %s\n", flag_string(area_flags, flags));
	fprintf(fp, "End\n\n");

	if (pArea->min_vnum && pArea->max_vnum) {
		save_mobiles(fp, pArea);
		save_objects(fp, pArea);
		save_rooms(fp, pArea);
		save_specials(fp, pArea);
		save_resets(fp, pArea);
		save_shops(fp, pArea);
		save_olimits(fp, pArea);
		save_mobprogs(fp, pArea);
		save_practicers(fp, pArea);
		save_omprogs(fp, pArea);
	}
	save_helps(fp, pArea);

	fprintf(fp, "#$\n");

	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
}

void save_skills()
{
}

void save_clan(CHAR_DATA *ch, CLAN_DATA *clan)
{
	int i;
	FILE *fp;

	fp = dfopen(CLANS_PATH, clan->file_name, "w");
	if (fp == NULL) {
		save_print(ch, "%s: %s", clan->file_name, strerror(errno));
		return;
	}
		
	fprintf(fp, "#CLAN\n");

	fprintf(fp, "Name %s~\n", clan->name);
	fprintf(fp, "Filename %s~\n", clan->file_name);
	if (clan->recall_vnum)
		fprintf(fp, "Recall %d\n", clan->recall_vnum);
	if (!mlstr_null(clan->msg_prays))
		mlstr_fwrite(fp, "MsgPrays", clan->msg_prays);
	if (!mlstr_null(clan->msg_vanishes))
		mlstr_fwrite(fp, "MsgVanishes", clan->msg_vanishes);

	REMOVE_BIT(clan->flags, CLAN_CHANGED);
	if (clan->flags)
		fprintf(fp, "Flags %s~\n",
			flag_string(clan_flags, clan->flags));

	for (i = 0; i < clan->skills->nused; i++) {
		CLAN_SKILL *cs = VARR_GET(clan->skills, i);

		if (cs->sn > 0) 
			fprintf(fp, "Skill '%s' %d\n",
				skill_name(cs->sn), cs->level);
	}

	fprintf(fp, "End\n\n"
		    "#$\n");
	fclose(fp);

	save_print(ch, "    %s (%s)", clan->name, clan->file_name);
}

void save_clans(CHAR_DATA *ch)
{
	int i;
	FILE *fp;
	bool found = FALSE;
	int sec = ch ? (IS_NPC(ch) ? 0 : ch->pcdata->security) : 9;

	if (sec < SECURITY_CLAN) {
		save_print(ch, "Insufficient security to save clans.");
		return;
	}

	fp = dfopen(CLANS_PATH, CLAN_LIST, "w");
	if (fp == NULL) {
		save_print(ch, "%s: %s", CLAN_LIST, strerror(errno));
		return;
	}

	save_print(ch, "Saved clans:");

	for (i = 0; i < clans->nused; i++) {
		fprintf(fp, "%s\n", CLAN(i)->file_name);
		if (IS_SET(CLAN(i)->flags, CLAN_CHANGED)) {
			save_clan(ch, CLAN(i));
			found = TRUE;
		}
	}

	fprintf(fp, "$\n");
	fclose(fp);

	if (!found)
		save_print(ch, "    None.");
}

void do_asave_raw(CHAR_DATA *ch, int flags)
{
	AREA_DATA *pArea;
	bool found = FALSE;
	int sec;

	if (!ch)       /* Do an autosave */
		sec = 9;
	else if (!IS_NPC(ch))
    		sec = ch->pcdata->security;
	else
    		sec = 0;

	save_area_list();

	if (ch)
		send_to_char("Saved zones:\n\r", ch);
	else
		log("Saved zones:");

	for (pArea = area_first; pArea; pArea = pArea->next) {
		/* Builder must be assigned this area. */
		if (ch && !IS_BUILDER(ch, pArea))
			continue;

		if (flags && !IS_SET(pArea->flags, flags))
			continue;

		found = TRUE;
		save_area(pArea);

		if (ch)
			char_printf(ch, "    %s (%s)\n\r",
				    pArea->name, pArea->file_name);
		else
			log_printf("    %s (%s)",
				   pArea->name, pArea->file_name);
		REMOVE_BIT(pArea->flags, flags);
	}

	if (!found) {
		if (ch)
			char_puts("    None.\n\r", ch);
		else
			log("    None.");
	}
}


/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asave(CHAR_DATA *ch, const char *argument)
{
	if (argument[0] == '\0') {
		if (ch)
			do_help(ch, "'OLC ASAVE'");
		return;
	}

	/* Save the world, only authorized areas. */
	/* -------------------------------------- */
	if (!str_cmp("world", argument)) {
		do_asave_raw(ch, 0);
		if (ch)
			send_to_char("You saved the world.\n\r", ch);
		else
			log("Saved the world");
		return;
	}

	/* Save changed areas, only authorized areas. */
	/* ------------------------------------------ */
	if (!str_cmp("changed", argument)) {
		do_asave_raw(ch, AREA_CHANGED);
		if (ch)
			send_to_char("You saved changed areas.\n\r", ch);
		else
			log("Saved changed areas");
		return;
	}

	if (!str_cmp("skills", argument)) {
		save_skills(ch);
		if (ch)
			send_to_char("You saved skills table.\n\r", ch);
		else
			log("Saved skills table");
		return;
	}

	if (!str_cmp("clans", argument)) {
		save_clans(ch);
		return;
	}

	/* Show correct syntax. */
	/* -------------------- */
	if (ch)
		do_asave(ch, "");
}

static void save_print(CHAR_DATA *ch, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	if (ch)
		char_printf(ch, "%s\n\r", buf);
	else
		log(buf);
}
