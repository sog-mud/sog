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
 * $Id: trig.h,v 1.11 2001-09-13 16:21:54 fjoe Exp $
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
#define TRIG_MOB_GREET		6		/* arg: probability	*/
#define TRIG_MOB_KILL		7		/* arg: probability	*/
#define TRIG_MOB_HPCNT		8		/* arg: probability	*/
#define TRIG_MOB_RANDOM		9		/* arg: probability	*/
#define TRIG_MOB_SPEECH		10		/* arg: text		*/
#define TRIG_MOB_EXIT		11		/* arg: dir (or "all")	*/
#define TRIG_MOB_DELAY		12		/* arg: probability	*/
#define TRIG_MOB_SURR		13		/* arg: probability	*/
#define TRIG_MOB_LOOK		14		/* arg: probability	*/
#define TRIG_MOB_OPEN		15		/* arg: obj vnum/name	*/
#define TRIG_MOB_GET		16		/* arg: obj vnum/name	*/
#define TRIG_MOB_TELL		17		/* arg: text		*/
#define TRIG_MOB_CMD		18		/* arg: cmd		*/
#define TRIG_MOB_YELL		19		/* arg: text		*/

/* obj triggers */
#define TRIG_OBJ_WEAR		100		/* arg: probability	*/
#define TRIG_OBJ_REMOVE		101		/* arg: probability	*/
#define TRIG_OBJ_DROP		102		/* arg: probability	*/
#define TRIG_OBJ_SAC		103		/* arg: probability	*/
#define TRIG_OBJ_GIVE		104		/* arg: probability	*/
#define TRIG_OBJ_GREET		105		/* arg: probability	*/
#define TRIG_OBJ_FIGHT		106		/* arg: probability	*/
#define TRIG_OBJ_DEATH		107		/* arg: probability	*/
#define TRIG_OBJ_SPEECH		108		/* arg: text		*/
#define TRIG_OBJ_ENTRY		109		/* arg: probability	*/
#define TRIG_OBJ_GET		110		/* arg: probability	*/
#define TRIG_OBJ_RANDOM		111		/* arg: probability	*/

/* spec triggers */
#define TRIG_SPEC		300

#define HAS_TEXT_ARG(trig)	(trig->trig_type == TRIG_MOB_ACT ||	\
				 trig->trig_type == TRIG_MOB_SPEECH ||	\
				 trig->trig_type == TRIG_MOB_TELL ||	\
				 trig->trig_type == TRIG_MOB_YELL ||	\
				 trig->trig_type == TRIG_OBJ_SPEECH)
#define HAS_OBJ_ARG(trig)	(trig->trig_type == TRIG_MOB_GIVE ||	\
				 trig->trig_type == TRIG_MOB_OPEN ||	\
				 trig->trig_type == TRIG_MOB_GET)
#define HAS_CMD_ARG(trig)	(trig->trig_type == TRIG_MOB_CMD)

#define TRIG_F_CASEDEP	(A)	/* text arg case-dependent	*/
#define TRIG_F_REGEXP	(B)	/* text arg is regexp		*/

struct trig_t {
	int		trig_type;	/**< trigger type	*/
	const char *	trig_prog;	/**< program to execute	*/
	const char *	trig_arg;	/**< trigger argument	*/
	flag_t		trig_flags;	/**< trigger flags	*/
	void *		trig_extra;	/**< trigger extra data	*/
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
void trig_fread(trig_t *trig, rfile_t *fp);

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
void trig_fread_list(varr *v, rfile_t *fp);

/**
 * Write list of triggers to file
 */
void trig_fwrite_list(const char *pre, varr *v, FILE *fp);

/**
 * Dump trigger list to buffer
 */
void trig_dump_list(varr *v, BUFFER *buf);

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
 * Pull trigger
 */
int pull_spec_trigger(spec_t *spec,
		      CHAR_DATA *ch, const char *spn_rm, const char *spn_add);

#endif /* _TRIG_H_ */
