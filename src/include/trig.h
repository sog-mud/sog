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
 * $Id: trig.h,v 1.1 2001-08-22 20:45:44 fjoe Exp $
 */

#ifndef _TRIG_H_
#define _TRIG_H_

#if 0
/*
 * MOBprog definitions
 */
#define TRIG_ACT	(A)
#define TRIG_BRIBE	(B)
#define TRIG_DEATH	(C)
#define TRIG_ENTRY	(D)
#define TRIG_FIGHT	(E)
#define TRIG_GIVE	(F)
#define TRIG_GREET	(G)
#define TRIG_GRALL	(H)
#define TRIG_KILL	(I)
#define TRIG_HPCNT	(J)
#define TRIG_RANDOM	(K)
#define TRIG_SPEECH	(L)
#define TRIG_EXIT	(M)
#define TRIG_EXALL	(N)
#define TRIG_DELAY	(O)
#define TRIG_SURR	(P)

enum {
	OPROG_WEAR,
	OPROG_REMOVE,
	OPROG_DROP,
	OPROG_SAC,
	OPROG_GIVE,
	OPROG_GREET,
	OPROG_FIGHT,
	OPROG_DEATH,
	OPROG_SPEECH,
	OPROG_ENTRY,
	OPROG_GET,
	OPROG_AREA,

	OPROG_MAX
};
#endif

#define TRIG_NONE	-1

/* spec triggers */
#define TRIG_SPEC	0

/* mob triggers */
#define TRIG_MOB_FIGHT	1
#define TRIG_MOB_DEATH	2

/* obj triggers */
#define TRIG_OBJ_FIGHT	101

#define HAS_TEXT_ARG(trig)	FALSE

#define TRIG_F_CASEDEP	(A)
#define TRIG_F_REGEXP	(B)

struct trig_t {
	int		trig_type;	/**< trigger type */
	const char *	trig_prog;	/**< program to execute */
	const char *	trig_arg;	/**< trigger argument */
	flag_t		trig_flags;	/**< trigger flags */
	void *		trig_extra;	/**< trigger extra data */
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
 * Copy trigger
 */
trig_t *trig_cpy(trig_t *dst, const trig_t *src);

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
 * Pull one trigger
 */
int pull_one_trigger(trig_t *trig, ...);

/**
 * Pull mob trigger of specified type
 */
int pull_mob_trigger(int trig_type, CHAR_DATA *ch, ...);

/**
 * Pull obj trigger of specified type
 */
int pull_obj_trigger(int trig_type, OBJ_DATA *obj, ...);

#endif /* _TRIG_H_ */
