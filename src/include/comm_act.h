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
 * $Id: comm_act.h,v 1.5 1999-03-03 13:50:43 fjoe Exp $
 */

#ifndef _COMM_ACT_H_
#define _COMM_ACT_H_

/* the following 5 act target flags are exclusive */
#define TO_ROOM		(A)
#define TO_NOTVICT	(B)
#define TO_VICT		(C)
#define TO_CHAR		(D)
#define TO_ALL		(E)

#define ACT_TOBUF	(F)
#define ACT_NOTRIG	(G)
#define ACT_NOTWIT	(H)
#define ACT_TRANS	(I)	/* do $t and $T translation		    */
#define ACT_NODEAF	(J)	/* skip is_affected(to, gsn_deafen) chars   */
#define ACT_STRANS	(K)	/* do $t and $T slang translation (from ch) */
#define ACT_NOMORTAL	(L)	/* skip mortals */
#define ACT_VERBOSE	(M)	/* skip if (!IS_SET(ch->comm, COMM_VERBOSE)) */
#define ACT_NOLF	(N)	/* do not append lf */
#define ACT_FORMSH	(O)	/* call format_short for short descrs */
#define ACT_FIXSH	(P)	/* call fix_short (used only in PERS2) */

#define act(format, ch, arg1, arg2, type) \
		act_puts((format), (ch), (arg1), (arg2), (type), POS_RESTING)
#define act_puts(format, ch, arg1, arg2, type, min_pos)		\
		act_puts3((format), (ch), (arg1), (arg2), NULL,	\
			  (type), (min_pos))
void    act_puts3(const char *format, CHAR_DATA *ch,
		  const void *arg1, const void *arg2, const void *arg3,
		  int flags, int min_pos);

#endif
