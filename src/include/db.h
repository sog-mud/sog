/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: db.h,v 1.27 1998-10-02 04:48:41 fjoe Exp $
 */

#ifndef _DB_H_
#define _DB_H_

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

typedef void DBLOAD_FUN(FILE *fp);
#define DECLARE_DBLOAD_FUN(fun) DBLOAD_FUN fun
#define DBLOAD_FUN(fun) void fun(FILE *fp)

struct dbfun {
	char *		name;
	DBLOAD_FUN *	fun;
};
typedef struct dbfun DBFUN;

typedef void DBINIT_FUN(void);
#define DECLARE_DBINIT_FUN(fun) DBINIT_FUN fun
#define DBINIT_FUN(fun) void fun(void)

extern DBFUN db_load_areas[];
extern DBFUN db_load_clans[];
extern DBFUN db_load_classes[];
extern DBFUN db_load_langs[];
extern DBFUN db_load_skills[];

DECLARE_DBINIT_FUN(init_area);
DECLARE_DBINIT_FUN(init_class);

extern int fBootDb;

void	load_oldmsgdb	(void);
void	load_msgdb	(void);
void	load_notes	(void);
void	load_bans	(void);

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

void db_load_file(const char *path, const char *file,
		  DBFUN *dbfun_table, DBINIT_FUN *dbinit);

extern char	filename	[PATH_MAX];
extern int	line_number;
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

