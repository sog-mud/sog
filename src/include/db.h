#ifndef _DB_H_
#define _DB_H_

/*
 * $Id: db.h,v 1.22 1998-09-01 18:37:57 fjoe Exp $
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

typedef void DBLOAD_FUN(FILE*);
#define DECLARE_DBLOAD_FUN(fun) DBLOAD_FUN fun

struct dbfun {
	char *		name;
	DBLOAD_FUN *	fun;
};
typedef struct dbfun DBFUN;

typedef void DBINIT_FUN(void);
#define DECLARE_DBINIT_FUN(fun) DBINIT_FUN fun
#define DBINIT(fun) void fun(void)

extern DBFUN db_load_skills[];
extern DBFUN db_load_classes[];
extern DBFUN db_load_clans[];
extern DBFUN db_load_areas[];

DECLARE_DBINIT_FUN(init_area);
DECLARE_DBINIT_FUN(init_skills);
DECLARE_DBINIT_FUN(init_classes);
DECLARE_DBINIT_FUN(init_clans);

extern int fBootDb;

char		fread_letter	(FILE *fp);
int		fread_number	(FILE *fp);
flag_t 		fread_flags	(FILE *fp);
char *		fread_string	(FILE *fp);
void		fread_to_eol	(FILE *fp);
char *		fread_word	(FILE *fp);
flag_t		fread_fword	(const FLAG *table, FILE *fp); 
flag_t		fread_fstring	(const FLAG *table, FILE *fp);
void *		fread_namedp	(NAMEDP *table, FILE *fp);
int		fread_clan	(FILE *fp);

#include <sys/syslimits.h>
char		filename	[PATH_MAX];
void		db_error	(const char* fn, const char* fmt, ...);

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

#endif

