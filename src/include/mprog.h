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
 * $Id: mprog.h,v 1.9 2004-02-11 22:56:16 fjoe Exp $
 */

#ifndef _MPROG_H_
#define _MPROG_H_

/**
 * Program types
 */
enum {
	MP_T_NONE = -1,
	MP_T_MOB,
	MP_T_OBJ,
	MP_T_ROOM,
	MP_T_SPEC,
};

/**
 * Program status
 */
enum {
	MP_S_DIRTY,		/**< program was not compiled */
	MP_S_READY,		/**< program compiled ok */
};

#define MP_F_TRACE		(A)	/**< mprog should be traced	*/
#define MP_F_CHANGED		(Z)	/**< mprog is changed (for OLC)	*/

/**
 * Program
 */
struct mprog_t {
	const char *name;	/**< program name			*/
	int type;		/**< program type			*/
	int status;		/**< program status			*/
	int flags;		/**< program flags			*/
	const char *text;	/**< program text			*/
	BUFFER *errbuf;		/**< buffer for error messages		*/
};

extern avltree_t mprogs;
extern avltree_info_t c_info_mprogs;

#define mprog_lookup(name)	((mprog_t *) c_lookup(&mprogs, (name)))
#define mprog_search(name)	((mprog_t *) c_strkey_search(&mprogs, (name)))

extern int (*mprog_compile)(mprog_t *mp);
extern int (*mprog_execute)(mprog_t *mp,
			    void *arg1, void *arg2, void *arg3, void *arg4,
			    void *arg5);

#define MPROG_COMPILE(ch, mp)						\
	do {								\
		if (mprog_compile == NULL)				\
			act_char("Module mod_mpc is not loaded.", ch);	\
		else if (mprog_compile(mp) < 0)				\
			page_to_char(buf_string(mp->errbuf), ch);	\
		else							\
			act_char("Compile ok.", ch);			\
	} while(0)

/**
 * mprog_execute error codes
 */
#define MPC_ERR_INTERNAL	(-1)	/* mpc internal error		*/
#define MPC_ERR_COMPILE		(-2)	/* compile error		*/
#define MPC_ERR_RUNTIME		(-3)	/* runtime error		*/
#define MPC_ERR_DIRTY		(-4)	/* program is not compiled	*/
#define MPC_ERR_NOTFOUND	(-5)	/* trigger/program not found	*/
#define MPC_ERR_UNLOADED	(-6)	/* mod_mpc is not loaded	*/
#define MPC_ERR_TYPE_MISMATCH	(-7)	/* mprog type mismatch		*/
#define MPC_ERR_COND_FAILED	(-8)	/* trig arg condition failed	*/

#endif /* _MPROG_H_ */
