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
 * $Id: mpc_impl.h,v 1.14 2001-08-05 16:36:49 fjoe Exp $
 */

#ifndef _MPC_IMPL_H_
#define _MPC_IMPL_H_

#include <stdarg.h>
#include <stdlib.h>

#include <typedef.h>
#include <varr.h>
#include <hash.h>
#include <str.h>

/**
 * Symbol types
 */
enum symtype_t {
	SYM_KEYWORD,		/**< 'if', 'else', 'continue' etc. */
	SYM_FUNC,		/**< functions */
	SYM_VAR,		/**< variables */
	SYM_LABEL,		/**< labels */
};
typedef enum symtype_t symtype_t;

/**
 * Symbol
 */
struct sym_t {
	const char *name;
	symtype_t type;
	union {
		struct {
			int type_tag;
			vo_t data;
			bool is_const;
			int block;
		} var;

		struct {
			int addr;
		} label;
	} s;
};
typedef struct sym_t sym_t;

void	sym_init(sym_t *sym);
void	sym_destroy(sym_t *sym);
sym_t *	sym_cpy(sym_t *dst, const sym_t *src);

extern hash_t glob_syms;		/* (sym_t) */
extern hashdata_t h_syms;
extern flaginfo_t mpc_types[];

/**
 * Switch jump table
 */
struct swjump_t {
	vo_t val;
	int addr;
};
typedef struct swjump_t swjump_t;

/**
 * Program types
 */
enum {
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

struct prog_t {
	const char *name;	/**< program name			*/
	int type;		/**< program type			*/
	int status;		/**< program status			*/

	const char *text;	/**< program text			*/
	size_t textlen;		/**< program text len			*/

	const char *cp;		/**< current position (for lexer)	*/
	BUFFER *errbuf;		/**< error msg buffer			*/
	int lineno;		/**< current line number		*/

	hash_t strings;		/**< (const char *) string space	*/
	hash_t syms;		/**< (sym_t) symbols			*/

	/* compiler data */
	varr cstack;		/**< (void *) compiler stack		*/
	varr args;		/**< (int) argument type stack		*/
	int curr_block;		/**< current block depth		*/

	int curr_jumptab;	/**< current jumptab			*/
	int curr_break_addr;	/**< current 'break' info		*/
	int curr_continue_addr;	/**< current 'continue' info		*/

	/* runtime data */
	int ip;			/**< program instruction pointer	*/
	varr code;		/**< (void *) program code		*/

	varr jumptabs;		/**< (varr ) 'switch' jump tables	*/
	varr iterdata;		/**< (iterdata_t) 'foreach' iter data	*/

	jmp_buf jmpbuf;		/**< jmp buf				*/
	varr data;		/**< data stack				*/
};
typedef struct prog_t prog_t;

extern hash_t progs;

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
prog_execute(prog_t *prog, int *errcode);

/**
 * Initialize mpc
 */
void
mpc_init();

/**
 * Destroy mpc environment
 */
void
mpc_fini();

/**
 * Lexer
 */
int mpc_lex(prog_t *prog);

/**
 * Lookup symbol by name
 */
sym_t *
sym_lookup(prog_t *prog, const char *name);

/**
 * Remove symbols with block >= specified
 */
void
cleanup_syms(prog_t *prog, int block);

/**
 * Handle compilation error
 */
void
compile_error(prog_t *prog, const char *fmt, ...)
	__attribute__ ((format(printf, 2, 3)));

/**
 * Make sure the string is allocated in program string space
 */
const char *alloc_string(prog_t *prog, const char *s);

/**
 * Execute program from specified position
 */
void execute(prog_t *prog, int ip);

/*--------------------------------------------------------------------
 * direct data stack manipulation opcodes
 */

typedef void (*c_fun)(prog_t *prog);

/**
 * Pop value from data stack
 */
void	c_pop(prog_t *prog);

/**
 * Push constant on data stack
 *
 * Expects next opcode to be const value to push.
 */
void	c_push_const(prog_t *prog);

/**
 * Push variable on data stack
 *
 * Expects next opcode to be variable name.
 */
void	c_push_var(prog_t *prog);

/**
 * Push function return value on data stack
 *
 * Expects next opcode to be 'func_t *'.
 */
void	c_push_retval(prog_t *prog);

#define c_stop 0
#define INVALID_ADDR -1

void	c_jmp(prog_t *prog);		/* jmp */
void	c_jmp_addr(prog_t *prog);	/* jmp addr */
void	c_if(prog_t *prog);		/* if */
void	c_switch(prog_t *prog);		/* switch */
void	c_quecolon(prog_t *prog);	/* ?: */
void	c_foreach(prog_t *prog);	/* foreach */
void	c_foreach_next(prog_t *prog);	/* foreach_next */
void	c_declare(prog_t *prog);	/* declare variable */
void	c_declare_assign(prog_t *prog);	/* declare variable and assign */
					/* initial value */
void	c_cleanup_syms(prog_t *prog);	/* cleanup symbols */
void	c_return(prog_t *prog);		/* return */

/*--------------------------------------------------------------------
 * binary operations
 *
 * all of them pop two arguments from data stack and push the result back.
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

/*--------------------------------------------------------------------
 * unary operations
 *
 * all of them pop one argument from data stack and push the result back.
 */
void	c_uop_not(prog_t *prog);	/* ! */
void	c_uop_compl(prog_t *prog);	/* ~ */
void	c_uop_minus(prog_t *prog);	/* - (unary) */

/*--------------------------------------------------------------------
 * incdec operations
 *
 * all of them expect next opcode to be variable name.
 * push the result on data stack.
 */
void	c_postinc(prog_t *prog);	/* post ++ */
void	c_postdec(prog_t *prog);	/* post -- */
void	c_preinc(prog_t *prog);		/* pre ++ */
void	c_predec(prog_t *prog);		/* pre -- */

/*--------------------------------------------------------------------
 * assign operations
 *
 * all of them pop one argument from data stack and push the result back.
 * next op is expected to be variable name.
 */

void	c_assign(prog_t *prog);		/* = */
void	c_add_eq(prog_t *prog);		/* += */
void	c_sub_eq(prog_t *prog);		/* -= */
void	c_div_eq(prog_t *prog);		/* /= */
void	c_mul_eq(prog_t *prog);		/* *= */
void	c_mod_eq(prog_t *prog);		/* %= */
void	c_and_eq(prog_t *prog);		/* &= */
void	c_or_eq(prog_t *prog);		/* |= */
void	c_xor_eq(prog_t *prog);		/* ^= */
void	c_shl_eq(prog_t *prog);		/* <<= */
void	c_shr_eq(prog_t *prog);		/* >>= */

#endif /* _MPC_IMPL_H_ */