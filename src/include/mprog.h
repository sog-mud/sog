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
 * $Id: mprog.h,v 1.2 2001-08-26 05:49:07 fjoe Exp $
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

#define MP_F_CHANGED		(Z)

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

extern hash_t mprogs;
extern hashdata_t h_mprogs;

void mprog_init(mprog_t *);
void mprog_destroy(mprog_t *);
mprog_t *mprog_cpy(mprog_t *dst, const mprog_t *src);

#define mprog_lookup(name)	((mprog_t *) hash_lookup(&mprogs, (name)))
#define mprog_search(name)	((mprog_t *) strkey_search(&mprogs, (name)))

extern int (*mprog_compile)(mprog_t *mp);
extern int (*mprog_execute)(mprog_t *mp, va_list ap);

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

#endif /* _MPROG_H_ */
