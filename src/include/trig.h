/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: trig.h,v 1.21 2004-02-11 21:44:11 fjoe Exp $
 */

#ifndef _TRIG_H_
#define _TRIG_H_

#define TRIG_NONE		-1

/* mob triggers */
#define TRIG_MOB_ACT		0		/* arg: text		*/
#define TRIG_MOB_BRIBE		1		/* arg: silver		*/
#define TRIG_MOB_DEATH		2		/* arg: probability	*/
#define TRIG_MOB_ENTRY		3		/* arg: probability	*/
#define TRIG_MOB_FIGHT		4		/* arg: probability	*/
#define TRIG_MOB_GIVE		5		/* arg: obj vnum/name	*/
#define TRIG_MOB_GREET		6		/* arg: dir (or "all")	*/
#define TRIG_MOB_KILL		7		/* arg: probability	*/
#define TRIG_MOB_RANDOM		8		/* arg: probability	*/
#define TRIG_MOB_SPEECH		9		/* arg: text		*/
#define TRIG_MOB_EXIT		10		/* arg: dir (or "all")	*/
#define TRIG_MOB_DELAY		11		/* arg: probability	*/
#define TRIG_MOB_LOOK		12		/* arg: probability	*/
#define TRIG_MOB_OPEN		13		/* arg: obj vnum/name	*/
#define TRIG_MOB_GET		14		/* arg: obj vnum/name	*/
#define TRIG_MOB_TELL		15		/* arg: text		*/
#define TRIG_MOB_CMD		16		/* arg: cmd		*/
#define TRIG_MOB_YELL		17		/* arg: text		*/

/* obj triggers */
#define TRIG_OBJ_WEAR		100		/* arg: probability	*/
#define TRIG_OBJ_REMOVE		101		/* arg: probability	*/
#define TRIG_OBJ_DROP		102		/* arg: probability	*/
#define TRIG_OBJ_SAC		103		/* arg: probability	*/
#define TRIG_OBJ_GIVE		104		/* arg: probability	*/
#define TRIG_OBJ_GREET		105		/* arg: dir (or "all")	*/
#define TRIG_OBJ_FIGHT		106		/* arg: probability	*/
#define TRIG_OBJ_DEATH		107		/* arg: probability	*/
#define TRIG_OBJ_SPEECH		108		/* arg: text		*/
#define TRIG_OBJ_ENTRY		109		/* arg: probability	*/
#define TRIG_OBJ_GET		110		/* arg: probability	*/
#define TRIG_OBJ_RANDOM		111		/* arg: probability	*/
#define TRIG_OBJ_ENTER		112		/* arg: probability	*/
#define TRIG_OBJ_LOOK		113		/* arg: probability	*/
#define TRIG_OBJ_OPEN		114		/* arg: probability	*/
#define TRIG_OBJ_CLOSE		115		/* arg: probability	*/
#define TRIG_OBJ_LOCK		116		/* arg: probability	*/
#define TRIG_OBJ_UNLOCK		117		/* arg: probability	*/
#define TRIG_OBJ_DAMAGE		118		/* arg: probability	*/
#define TRIG_OBJ_REPAIR		119		/* arg: probability	*/
#define TRIG_OBJ_CMD		120		/* arg: cmd		*/
#define TRIG_OBJ_USE		121		/* arg: probability	*/

/* room triggers */
#define TRIG_ROOM_RANDOM	200		/* arg: probability	*/
#define TRIG_ROOM_CMD		201		/* arg: cmd		*/
#define TRIG_ROOM_GREET		202		/* arg: dir (or "all")	*/
#define TRIG_ROOM_DOPEN		203		/* arg: dir (or name)	*/
#define TRIG_ROOM_DCLOSE	204		/* arg: dir (or name)	*/
#define TRIG_ROOM_DLOCK		204		/* arg: dir (or name)	*/
#define TRIG_ROOM_DUNLOCK	204		/* arg: dir (or name)	*/

/* spec triggers */
#define TRIG_SPEC		300

#define HAS_TEXT_ARG(trig)	((trig)->trig_type == TRIG_MOB_ACT ||	\
				 (trig)->trig_type == TRIG_MOB_SPEECH ||\
				 (trig)->trig_type == TRIG_MOB_TELL ||	\
				 (trig)->trig_type == TRIG_MOB_YELL ||	\
				 (trig)->trig_type == TRIG_OBJ_SPEECH)
#define HAS_OBJ_ARG(trig)	((trig)->trig_type == TRIG_MOB_GIVE ||	\
				 (trig)->trig_type == TRIG_MOB_OPEN ||	\
				 (trig)->trig_type == TRIG_MOB_GET)
#define HAS_CMD_ARG(trig)	((trig)->trig_type == TRIG_MOB_CMD ||	\
				 (trig)->trig_type == TRIG_ROOM_CMD ||	\
				 (trig)->trig_type == TRIG_OBJ_CMD)
#define HAS_EXIT_ARG(trig)	((trig)->trig_type == TRIG_MOB_GREET ||	\
				 (trig)->trig_type == TRIG_MOB_EXIT ||	\
				 (trig)->trig_type == TRIG_OBJ_GREET ||	\
				 (trig)->trig_type == TRIG_ROOM_GREET || \
				 (trig)->trig_type == TRIG_ROOM_DOPEN || \
				 (trig)->trig_type == TRIG_ROOM_DCLOSE || \
				 (trig)->trig_type == TRIG_ROOM_DLOCK || \
				 (trig)->trig_type == TRIG_ROOM_DUNLOCK)

#define TRIG_F_CASEDEP	(A)	/* text arg case-dependent	*/
#define TRIG_F_REGEXP	(B)	/* text arg is regexp		*/

struct trig_t {
	int		trig_type;	/**< trigger type	*/
	const char *	trig_prog;	/**< program to execute	*/
	const char *	trig_arg;	/**< trigger argument	*/
	flag_t		trig_flags;	/**< trigger flags	*/
	void *		trig_extra;	/**< trigger extra data	*/
	AFFECT_DATA *	trig_paf;	/**< trigger affect	*/
};

/**
 * Initialize trigger
 */
void trig_init(trig_t *trig);

/**
 * Destroy trigger
 */
void trig_destroy(trig_t *trig);

/**
 * Read trigger from file
 */
void trig_fread(trig_t *trig, int mp_type, const char *mp_name, rfile_t *fp);

/**
 * Read inline prog for trigger from file
 */
bool trig_fread_inline_prog(trig_t *trig, int mp_type, const char *mp_name,
			    rfile_t *fp);

/**
 * Write trigger to file
 */
void trig_fwrite(const char *pre, trig_t *trig, FILE *fp);

/**
 * Initialize list of triggers
 */
void trig_init_list(varr *v);

/**
 * Destroy list of triggers
 */
void trig_destroy_list(varr *v);

/**
 * Read trigger from file and insert it into trigger list
 */
bool trig_fread_list(varr *v, int mp_type, const char *mp_name, rfile_t *fp);

/**
 * Write list of triggers to file
 */
void trig_fwrite_list(const char *pre, varr *v, FILE *fp);

/**
 * Dump trigger list to buffer
 */
void trig_dump_list(varr *v, BUFFER *buf);

/**
 * Allocate new trigger
 *
 * Preserves order of old elements
 */
trig_t *trig_new(varr *v, int trig_type);

/**
 * Set trigger arg
 *
 * @param arg dynamically allocated (via str_[q]dup) string
 */
void trig_set_arg(trig_t *trig, const char *arg);

/**
 * Pull mob trigger of specified type
 */
int pull_mob_trigger(int trig_type,
		     CHAR_DATA *ch, CHAR_DATA *victim, void *arg);

/**
 * Pull obj trigger of specified type
 */
int pull_obj_trigger(int trig_type,
		     OBJ_DATA *obj, CHAR_DATA *ch, void *arg);

/**
 * Pull room trigger of specified type
 */
int pull_room_trigger(int trig_type,
		      ROOM_INDEX_DATA *room, CHAR_DATA *ch, void *arg);

/**
 * Pull trigger
 */
int pull_spec_trigger(spec_t *spec,
		      CHAR_DATA *ch, const char *spn_rm, const char *spn_add);

/**
 * Check if trigger is in list
 */
bool has_trigger(varr *v, int trig_type);

#define MOB_HAS_TRIGGER(mob, trig)					\
	(IS_NPC(mob) && has_trigger(&(mob)->pMobIndex->mp_trigs, (trig)))
#define OBJ_HAS_TRIGGER(obj, trig)					\
	has_trigger(&(obj)->pObjIndex->mp_trigs, (trig))
#define ROOM_HAS_TRIGGER(room, trig)					\
	has_trigger(&(room)->mp_trigs, (trig))

/**
 * Generate inline mp name using string
 */
const char *genmpname_str(int mp_type, const char *str);

/**
 * Generate inline mp name for objects with `vnum' and trigger with index `num'
 */
const char *genmpname_vnum(int mp_type, int vnum, int num);

/**
 * Format mprog name
 */
const char *format_mpname(mprog_t *mp);

#endif /* _TRIG_H_ */
