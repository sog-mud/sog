#ifndef _DB_H_
#define _DB_H_

/*
 * $Id: db.h,v 1.20 1998-08-18 18:05:39 fjoe Exp $
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

extern bool fBootDb;
extern int		newmobs;
extern int		newobjs;
extern MOB_INDEX_DATA *	mob_index_hash	[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *	obj_index_hash	[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash [MAX_KEY_HASH];
extern int		top_mob_index;
extern int		top_obj_index;
extern int		top_vnum_mob;
extern int		top_vnum_obj;
extern int		top_vnum_room;
extern int  		top_affect;
extern int		top_ed; 
extern int		top_area;
extern int		top_exit;
extern int		top_help;
extern int		top_reset;
extern int		top_room;
extern int		top_shop;
extern AREA_DATA *	area_first;
extern AREA_DATA *	area_last;
extern AREA_DATA *	area_current;
extern HELP_DATA *	help_first;
extern SHOP_DATA *	shop_last;

extern		char			str_empty[1];

void	reset_area      (AREA_DATA * pArea);		/* OLC */
void	reset_room	(ROOM_INDEX_DATA *pRoom);	/* OLC */

void		boot_db		(void);
CHAR_DATA *	create_mob	(MOB_INDEX_DATA *pMobIndex);
CHAR_DATA *	create_named_mob(MOB_INDEX_DATA *pMobIndex, const char *name);
void		clone_mob	(CHAR_DATA *parent, CHAR_DATA *clone);
OBJ_DATA *	create_obj	(OBJ_INDEX_DATA *pObjIndex, int level);
OBJ_DATA *	create_named_obj(OBJ_INDEX_DATA *pObjIndex, int level,
				 const char *name);
OBJ_DATA *	create_obj_nocount (OBJ_INDEX_DATA *pObjIndex, int level);
void	clone_obj	(OBJ_DATA *parent, OBJ_DATA *clone); 
void	clear_char	(CHAR_DATA *ch);
ED_DATA * ed_lookup(const char *name, ED_DATA *ed);
MOB_INDEX_DATA *	get_mob_index	(int vnum);
OBJ_INDEX_DATA *	get_obj_index	(int vnum);
ROOM_INDEX_DATA *	get_room_index	(int vnum);
char	fread_letter	(FILE *fp);
int	fread_number	(FILE *fp);
long 	fread_flags	(FILE *fp);
char *	fread_string	(FILE *fp);
char *  fread_string_eol(FILE *fp);
void	fread_to_eol	(FILE *fp);
char *	fread_word	(FILE *fp);
long	flag_convert	(char letter);
void *	alloc_mem	(int sMem);
void *	alloc_perm	(int sMem);
void	free_mem	(void *pMem, int sMem);
char *	str_dup		(const char *str);
char *	str_add		(const char *str,...);
void	free_string	(char *str);
char *	str_printf	(const char *format,...);
int	number_fuzzy	(int number);
int	number_range	(int from, int to);
int	number_percent	(void);
int	number_door	(void);
int	number_bits	(int width);
long     number_mm      (void);
int	dice		(int number, int size);
int	interpolate	(int level, int value_00, int value_32);
void	smash_tilde	(char *str);
bool	str_cmp		(const char *astr, const char *bstr);
bool	str_prefix	(const char *astr, const char *bstr);
bool	str_infix	(const char *astr, const char *bstr);
bool	str_suffix	(const char *astr, const char *bstr);
char *	capitalize	(const char *str);
void	append_file	(CHAR_DATA *ch, const char *file, const char *str);
void	tail_chain	(void);
void	db_error	(const char* fn, const char* fmt, ...);
char *format_flags(int flags);

#define chance(num) (number_range(1, 100) <= num)

/* from db2.c */
extern int	social_count;

/* conversion from db.h */
void	convert_mob(MOB_INDEX_DATA *mob);
void	convert_obj(OBJ_INDEX_DATA *obj);

/* macro for flag swapping */
#define GET_UNSET(flag1,flag2)	(~(flag1)&((flag1)|(flag2)))

extern void vnum_check(AREA_DATA *area, int vnum);	/* OLC */

void convert_mobile(MOB_INDEX_DATA *pMobIndex);            /* OLC ROM */
void convert_objects(void);                                /* OLC ROM */
void convert_object(OBJ_INDEX_DATA *pObjIndex);            /* OLC ROM */

#define SLIST_ADD(type, list, item)					\
	{								\
		if ((list) == NULL)					\
			(list) = (item);				\
		else {							\
			type *p;					\
									\
			for (p = (list); p->next != NULL; p = p->next)	\
				;					\
			p->next = (item);				\
		}							\
		(item)->next = NULL;					\
	}

#define KEY(literal, field, value)			\
		if (!str_cmp(word, literal)) {		\
			field  = value;			\
			fMatch = TRUE;			\
			break;				\
		}

#define SKEY(string, field)				\
		if (!str_cmp(word, string)) {		\
			free_string(field);		\
			field = fread_string(fp);	\
			fMatch = TRUE;			\
			break;				\
		}

#define MLSKEY(string, field)				\
		if (!str_cmp(word, string)) {		\
			mlstr_free(field);		\
			field = mlstr_fread(fp);	\
			fMatch = TRUE;			\
			break;				\
		}

char * fix_string(const char *);

#endif

