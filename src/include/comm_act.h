/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: comm_act.h,v 1.27 2001-06-22 07:13:32 avn Exp $
 */

#ifndef _COMM_ACT_H_
#define _COMM_ACT_H_

/* the following 5 act target flags are exclusive */
#define TO_ROOM		(A)
#define TO_NOTVICT	(B)
#define TO_VICT		(C)
#define TO_CHAR		(D)
#define TO_ALL		(E)

#define ACT_TOBUF	(F)	/* append to replay buffer if link-dead	    */
#define ACT_NOTRIG	(G)	/* do not pull act triggers		    */
#define ACT_NOTWIT	(H)	/* do not perform twit list checking	    */
#define ACT_NOTRANS	(I)	/* do not perform $t, $T, $u and $U transl. */
#define ACT_NODEAF	(J)	/* skip is_affected(to, "deafen") chars     */
#define ACT_STRANS	(K)	/* do $t and $T slang translation (from ch) */
#define ACT_NOMORTAL	(L)	/* skip mortals				    */
#define ACT_VERBOSE	(M)	/* skip if (!IS_SET(to->comm, COMM_VERBOSE))*/
#define ACT_NOLF	(N)	/* do not append lf			    */
#define ACT_NOUCASE	(O)	/* do not uppercase first letter	    */
#define ACT_FORMSH	(P)	/* call format_short for short descrs	    */
#define ACT_SEDIT	(Q)	/* string editor message		    */
				/* (do not buffer it)			    */
#define ACT_NOFIXSH	(Z)	/* do not fix char/obj short descrs	    */
				/* (used internally in comm_act.c for not   */
				/* stripping '~' when short descrs are      */
				/* used inside $xx{}			    */
#define ACT_SPEECH(ch)	(ACT_NODEAF | ACT_STRANS | 			\
		  	 (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) ?	\
				ACT_NOTRANS : 0))
/*
 * formatting stuff
 */

#define format_short(mshort, name, to)	\
		_format_short((mshort), (name), (to), GET_LANG(to), 0)
const char *_format_short(mlstring *mshort, const char *name,
			  CHAR_DATA *to, int to_lang, int act_flags);

#define format_long(desc, to) (_format_long((desc), (to), GET_LANG(to)))
const char *_format_long(mlstring *desc, CHAR_DATA *to, int to_lang);

#define PERS(ch, to) PERS2(ch, to, GET_LANG(to), ACT_FORMSH)
const char* PERS2(CHAR_DATA *ch, CHAR_DATA *to, int to_lang, int act_flags);

/*
 * act stuff
 */

typedef struct actopt_t {
	size_t to_lang;
	int to_sex;
	int act_flags;
} actopt_t;

#define act(format, ch, arg1, arg2, type) \
		act_puts((format), (ch), (arg1), (arg2), (type), POS_RESTING)
#define act_char(format, ch)					\
		act_puts((format), (ch), NULL, NULL,		\
			 TO_CHAR | ACT_NOUCASE, POS_DEAD)
#define act_puts(format, ch, arg1, arg2, type, min_pos)		\
		act_puts3((format), (ch), (arg1), (arg2), NULL,	\
			  (type), (min_pos))
#define act_mlputs(mlformat, ch, arg1, arg2, type, min_pos)		\
		act_mlputs3((mlformat), (ch), (arg1), (arg2), NULL,	\
			    (type), (min_pos))

/*
 * ->to must not be NULL for all char/obj formatting or if ACT_STRANS is set
 * other formatting functions use opt->to_lang/opt->to_sex instead
 */
void	act_buf		(const char *format, CHAR_DATA *ch, CHAR_DATA *to,
			 const void *arg1, const void *arg2, const void *arg3,
			 actopt_t *opt, char *buf, size_t buf_len);
void    act_puts3	(const char *format, CHAR_DATA *ch,
			 const void *arg1, const void *arg2, const void *arg3,
			 int act_flags, int min_pos);
void	act_mlputs3	(mlstring *mlformat, CHAR_DATA *ch,
			 const void *arg1, const void *arg2, const void *arg3,
			 int act_flags, int min_pos);

/*
 * misc comm act-like functions
 */
const char *	act_speech(CHAR_DATA *ch, CHAR_DATA *vch,
			   const char *text, const void *arg);

void	act_yell(CHAR_DATA *ch, const char *text, const void *arg,
		 const char *format);
void	act_clan(CHAR_DATA *ch, const char *text, const void *arg);
void	act_say(CHAR_DATA *ch, const char *text, const void *arg);

/*
 * common declarations for act_xxx and mob progs
 */
extern const char *he_she[];
extern const char *him_her[];
extern const char *his_her[];

int GET_SEX(mlstring *ml, int to_lang);

#endif
