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
 * $Id: db.h,v 1.54 1999-10-20 11:10:37 fjoe Exp $
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

typedef struct dbdata DBDATA;

typedef void DBLOAD_FUN(DBDATA *dbdata, FILE *fp, void *arg);
#define DECLARE_DBLOAD_FUN(fun) DBLOAD_FUN fun
#define DBLOAD_FUN(fun) void fun(DBDATA *dbdata, FILE *fp, void *arg)

typedef void DBINIT_FUN(DBDATA *dbdata);
#define DECLARE_DBINIT_FUN(fun) DBINIT_FUN fun
#define DBINIT_FUN(fun) void fun(DBDATA *dbdata)

struct dbfun {
	char *		name;
	DBLOAD_FUN *	fun;
	void *		arg;
};
typedef struct dbfun DBFUN;

struct dbdata {
	DBFUN *		fun_tab;	/* table of parsing functions	*/
	DBINIT_FUN *	dbinit;		/* init function (called before	*/
					/* parsing each directive and	*/
					/* before parsing)		*/
	size_t		tab_sz;		/* table size			*/
};

#define DBDATA_VALID(dbdata) (!!(dbdata)->tab_sz)

extern DBDATA db_areas;
extern DBDATA db_clans;
extern DBDATA db_classes;
extern DBDATA db_cmd;
extern DBDATA db_damtype;
extern DBDATA db_hometowns;
extern DBDATA db_langs;
extern DBDATA db_liquids;
extern DBDATA db_materials;
extern DBDATA db_msg;
extern DBDATA db_races;
extern DBDATA db_skills;
extern DBDATA db_events;
extern DBDATA db_socials;
extern DBDATA db_spec;
extern DBDATA db_system;

void db_load_file(DBDATA *, const char *path, const char *file);
void db_set_arg(DBDATA *, const char* name, void *arg);

extern int fBootDb;

int	xgetc	(FILE *fp);
void	xungetc	(int c, FILE *fp);

void	vnum_check	(AREA_DATA *area, int vnum);

void	convert_objects	(void);
void	convert_object	(OBJ_INDEX_DATA *pObjIndex);

void	reset_area      (AREA_DATA * pArea);

#define RESET_F_NOPCHECK (A)
void	reset_room	(ROOM_INDEX_DATA *pRoom, int flags);

char *		fix_string	(const char *s);

const char *	fread_string	(FILE *fp);
char		fread_letter	(FILE *fp);
int		fread_number	(FILE *fp);
flag64_t 	fread_flags	(FILE *fp);
void		fread_to_eol	(FILE *fp);
char *		fread_word	(FILE *fp);
flag64_t	fread_fword	(const flag_t *table, FILE *fp); 
void		fwrite_ival	(FILE *fp, const flag_t *table,
				 const char *name, int);
flag64_t	fread_fstring	(const flag_t *table, FILE *fp);
int		fread_clan	(FILE *fp);

void		fwrite_string	(FILE *fp, const char *name, const char *str);
void		fwrite_word	(FILE *fp, const char *name, const char *w);
void		fwrite_number	(FILE *fp, const char *name, int num);

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

#define SKEY(string, field, value)			\
		if (!str_cmp(word, string)) {		\
			free_string(field);		\
			field = value;			\
			fMatch = TRUE;			\
			break;				\
		}

#define MLSKEY(string, field)				\
		if (!str_cmp(word, string)) {		\
			mlstr_fread(fp, &field);	\
			fMatch = TRUE;			\
			break;				\
		}

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
extern int		social_count;
extern int		str_count;
extern int		str_real_count;
extern int		top_player;
extern int		rebooter;
extern AREA_DATA *	area_first;
extern AREA_DATA *	area_last;
extern AREA_DATA *	area_current;
extern SHOP_DATA *	shop_last;

/*
 * the following path/file name consts are defined in db.c
 */
extern const char TMP_PATH	[];
extern const char PLAYER_PATH	[];
extern const char GODS_PATH	[];
extern const char NOTES_PATH	[];
extern const char ETC_PATH	[];
extern const char CLANS_PATH	[];
extern const char AREA_PATH	[];
extern const char LANG_PATH	[];
extern const char PLISTS_PATH	[];
extern const char MODULES_PATH	[];

extern const char CLASSES_PATH	[];
extern const char RACES_PATH	[];
extern const char SPEC_PATH	[];

extern const char CLASS_EXT	[];
extern const char RACE_EXT	[];
extern const char SPEC_EXT	[];

extern const char TMP_FILE	[];
extern const char NULL_FILE	[];

extern const char HOMETOWNS_CONF[];
extern const char SKILLS_CONF	[];
extern const char SOCIALS_CONF	[];
extern const char SYSTEM_CONF	[];
extern const char LANG_CONF	[];
extern const char MSGDB_CONF	[];
extern const char CMD_CONF	[];
extern const char DAMTYPE_CONF	[];
extern const char MATERIALS_CONF[];
extern const char LIQUIDS_CONF	[];

extern const char AREA_LIST	[];
extern const char CLAN_LIST	[];
extern const char CLASS_LIST	[];
extern const char LANG_LIST	[];

extern const char NOTE_FILE	[];
extern const char IDEA_FILE	[];
extern const char PENALTY_FILE	[];
extern const char NEWS_FILE	[];
extern const char CHANGES_FILE	[];
extern const char SHUTDOWN_FILE	[];
extern const char EQCHECK_FILE	[];
extern const char BAN_FILE	[];
extern const char MAXON_FILE	[];
extern const char AREASTAT_FILE	[];
extern const char IMMLOG_FILE	[];

#endif

