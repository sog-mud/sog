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
 * $Id: mpc_impl.h,v 1.1 2001-06-16 18:20:37 fjoe Exp $
 */

#ifndef _MPC_CODE_H_
#define _MPC_CODE_H_

#include <stdarg.h>
#include <stdlib.h>

#include <typedef.h>
#include <varr.h>
#include <hash.h>
#include <str.h>

struct prog_t {
	const char *name;	/**< program name			*/

	const char *text;	/**< program text			*/
	size_t textlen;		/**< program text len			*/

	const char *cp;		/**< current position (for lexer)	*/
	BUFFER *errbuf;		/**< error msg buffer			*/
	int lineno;		/**< current line number		*/

	int ip;			/**< program instruction pointer	*/
	varr code;		/**< (void **) program code		*/
	hash_t strings;		/**< (const char *) string space	*/
	hash_t syms;		/**< (sym_t) symbols			*/
	varr args;		/**< (int) argument type stack		*/

	varr data;		/**< data stack				*/
};
typedef struct prog_t prog_t;

/**
 * Initialize program
 */
void
prog_init(prog_t *prog);

/**
 * Destroy program
 */
void
prog_destroy(prog_t *prog);

/**
 * Compile program
 */
int
prog_compile(prog_t *prog);

/**
 * Execute program
 */
int
prog_execute(prog_t *prog);

/**
 * Symbol types
 */
enum symtype_t {
	SYM_KEYWORD,		/**< 'if', 'else', 'continue' etc. */
	SYM_FUNC,		/**< functions */
	SYM_VAR			/**< variables */
};
typedef enum symtype_t symtype_t;

/**
 * Symbol
 */
struct sym_t {
	symtype_t type;
	const char *name;
	union {
		struct {
			int type_tag;
			vo_t data;
			bool is_const;
		} var;
	} s;
};
typedef struct sym_t sym_t;

/**
 * Lookup symbol by name
 */
sym_t *
sym_lookup(prog_t *prog, const char *name);

/**
 * Handle compilation error
 */
void
compile_error(prog_t *prog, const char *fmt, ...)
	__attribute__ ((format(printf, 2, 3)));

/*--------------------------------------------------------------------
 * direct data stack manipulation opcodes
 */

typedef void (*c_fun)(prog_t *prog);

/**
 * Pop value from data stack
 */
void
c_pop(prog_t *prog);

/**
 * Push constant on data stack
 *
 * Expects next opcode to be const value to push.
 */
void
c_push_const(prog_t *prog);

/**
 * Push variable on data stack
 *
 * Expects next opcode to be 'sym_t *'.
 */
void
c_push_var(prog_t *prog);

/**
 * Push function return value on data stack
 *
 * Expects next opcode to be 'func_t *'.
 */
void
c_push_retval(prog_t *prog);

/*--------------------------------------------------------------------
 * binary operations
 *
 * all of them pop two arguments from data stack and push the result back
 */

void	c_bop_lor(prog_t *prog);	/* || */
void	c_bop_land(prog_t *prog);	/* && */
void	c_bop_or(prog_t *prog);		/* | */
void	c_bop_xor(prog_t *prog);	/* ^ */
void	c_bop_and(prog_t *prog);	/* & */

void	c_bop_ne(prog_t *prog);		/* == */
void	c_bop_eq(prog_t *prog);		/* != */

void	c_bop_gt(prog_t *prog);		/* > */
void	c_bop_ge(prog_t *prog);		/* >= */
void	c_bop_lt(prog_t *prog);		/* < */
void	c_bop_le(prog_t *prog);		/* <= */

void	c_bop_shl(prog_t *prog);	/* << */
void	c_bop_shr(prog_t *prog);	/* >> */
void	c_bop_add(prog_t *prog);	/* + */
void	c_bop_sub(prog_t *prog);	/* - */
void	c_bop_mul(prog_t *prog);	/* * */
void	c_bop_mod(prog_t *prog);	/* % */
void	c_bop_div(prog_t *prog);	/* / */

void	c_bop_ne_string(prog_t *prog);	/* == for strings (case-sensitive) */
void	c_bop_eq_string(prog_t *prog);	/* != for strings (case-sensitive) */

#endif /* _MPC_OP_H_ */
