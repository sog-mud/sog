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
 * $Id: comm_act.h,v 1.10 1999-06-22 13:50:46 fjoe Exp $
 */

#ifndef _COMM_ACT_H_
#define _COMM_ACT_H_

/* the following 5 act target flags are exclusive */
#define TO_ROOM		(A)
#define TO_NOTVICT	(B)
#define TO_VICT		(C)
#define TO_CHAR		(D)
#define TO_ALL		(E)

#define ACT_TOBUF	(F)	/* append to replay buffer if link-dead */
#define ACT_NOTRIG	(G)	/* do not pull act triggers */
#define ACT_NOTWIT	(H)	/* do not perform twit list checking */
#define ACT_NOTRANS	(I)	/* do not perform $t, $T, $u and $U transl. */
#define ACT_NODEAF	(J)	/* skip is_affected(to, gsn_deafen) chars   */
#define ACT_STRANS	(K)	/* do $t and $T slang translation (from ch) */
#define ACT_NOMORTAL	(L)	/* skip mortals */
#define ACT_VERBOSE	(M)	/* skip if (!IS_SET(ch->comm, COMM_VERBOSE)) */
#define ACT_NOLF	(N)	/* do not append lf */
#define ACT_NOUCASE	(O)	/* do not uppercase first letter */
#define ACT_FORMSH	(P)	/* call format_short for short descrs */
#define ACT_NOFIXSH	(Q)	/* do not call fix_short */

#define ACT_SPEECH	(ACT_NODEAF | ACT_STRANS | ACT_NOFIXSH)

/*
 * formatting stuff
 */

const char *fix_short	(const char *short_descr);
const char *format_short(mlstring *mshort, const char *name, CHAR_DATA *to);
const char *format_long(mlstring *desc, CHAR_DATA *to);

#define PERS(ch, looker) PERS2(ch, looker, 0)
const char* PERS2(CHAR_DATA *ch, CHAR_DATA *looker, int act_flags);

/*
 * act stuff
 */

typedef struct actopt_t {
	int to_lang;
	int to_sex;
	int act_flags;
} actopt_t;

#define act(format, ch, arg1, arg2, type) \
		act_puts((format), (ch), (arg1), (arg2), (type), POS_RESTING)
#define act_puts(format, ch, arg1, arg2, type, min_pos)		\
		act_puts3((format), (ch), (arg1), (arg2), NULL,	\
			  (type), (min_pos))
/*
 * ->to must not be NULL for all char/obj formatting or if ACT_STRANS is set
 * other formatting functions use opt->to_lang/opt->to_sex instead
 */
void	act_buf(const char *format, CHAR_DATA *ch, CHAR_DATA *to,
		const void *arg1, const void *arg2, const void *arg3,
		actopt_t *opt, char *buf, size_t buf_len);
void    act_puts3(const char *format, CHAR_DATA *ch,
		  const void *arg1, const void *arg2, const void *arg3,
		  int act_flags, int min_pos);

/*
 * misc comm act-like functions
 */
void	act_yell(const char *format, CHAR_DATA *ch,
		 const void *arg1, const void *arg3);
void	act_clan(const char *format, CHAR_DATA *ch,
		 const void *arg1, const void *arg3);

void	act_say(const char *format_self, const char *format_others,
		CHAR_DATA *ch,
		const void *arg1, const void *arg2, const void *arg3);

#endif
