/*
 * $Id: affects.h,v 1.3 1999-10-18 18:08:03 avn Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos} 	bulut@rorqual.cc.metu.edu.tr	   *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the	   *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence		   *
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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

#ifndef _AFFECT_H_
#define _AFFECT_H_

/*-------------------------------------------------------------------
 * affects stuff
 */

struct affect_data
{
	AFFECT_DATA *	next;
	flag32_t	where;
	const char *	type;
	int		level;
	int		duration;
	vo_t		location;
	int		modifier;
	flag64_t 	bitvector;
	CHAR_DATA *	owner;
	flag32_t	events;
};

/* where definitions */
#define TO_AFFECTS	0
#define TO_OBJECT	1
#define TO_IMMUNE	2
#define TO_RESIST	3
#define TO_VULN 	4
#define TO_WEAPON	5
#define TO_SKILLS	6

typedef struct where_t where_t;
struct where_t
{
	flag32_t	where;
	flag_t *	table;
	const char *	format;
};

where_t *where_lookup(flag32_t where);

void affect_modify(CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd);
AFFECT_DATA	*affect_find (AFFECT_DATA *paf, const char *sn);
void	affect_check	(CHAR_DATA *ch, int where, flag64_t vector);
void	affect_enchant	(OBJ_DATA *obj);

void	affect_to_char	(CHAR_DATA *ch, AFFECT_DATA *paf);
void	affect_to_obj	(OBJ_DATA *obj, AFFECT_DATA *paf);
void	affect_remove	(CHAR_DATA *ch, AFFECT_DATA *paf);
void	affect_remove_obj (OBJ_DATA *obj, AFFECT_DATA *paf);
void	affect_strip	(CHAR_DATA *ch, const char *sn);
void	affect_bit_strip(CHAR_DATA *ch, int where, flag64_t bits);
void	affect_join(CHAR_DATA *ch, AFFECT_DATA *paf);
#define is_affected(ch, sn) affect_find((ch)->affected, (sn))
AFFECT_DATA *is_bit_affected	(CHAR_DATA *ch, int where, flag64_t bits);
int	has_obj_affect	(CHAR_DATA *ch, int vector);

/* room affects */
void	affect_to_room		(ROOM_INDEX_DATA *room, AFFECT_DATA *paf);
void	affect_remove_room	(ROOM_INDEX_DATA *room, AFFECT_DATA *paf);
void	affect_strip_room	(ROOM_INDEX_DATA *ch, const char *sn);
bool	is_affected_room	(ROOM_INDEX_DATA *ch, const char *sn);
void	strip_raff_owner(CHAR_DATA *ch);

/* format_obj_affects flags */
#define FOA_F_NODURATION	(A)	/* do not show duration		*/
#define FOA_F_NOAFFECTS		(B)	/* do not show bit affects	*/

void format_obj(BUFFER *output, OBJ_DATA *obj);
void format_obj_affects(BUFFER *output, AFFECT_DATA *paf, int flags);

void fwrite_affect(AFFECT_DATA *paf, FILE *fp);
AFFECT_DATA *fread_affect(FILE *fp);

#endif
