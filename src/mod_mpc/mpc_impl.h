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
 * $Id: mpc_impl.h,v 1.31 2004-02-13 14:48:15 fjoe Exp $
 */

#ifndef _MPC_IMPL_H_
#define _MPC_IMPL_H_

#include <stdlib.h>

#include <typedef.h>
#include <varr.h>
#include <str.h>

/**
 * Symbol types
 */
enum symtype_t {
	SYM_KEYWORD,		/**< 'if', 'else', 'continue' etc. */
	SYM_FUNC,		/**< functions */
	SYM_VAR,		/**< variables */
	SYM_LABEL,		/**< labels */
	SYM_ITER,		/**< iterator data */
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
			const char *name;
		} func;

		struct {
			int addr;
		} label;
	} s;
};
typedef struct sym_t sym_t;

/*
 * Static variable holder
 */
struct svh_t {
	const char *sym_name;
	avltree_t svars;		/* svar_t */
};
typedef struct svh_t svh_t;

/*
 * Static variable
 */
struct svar_t {
	const char *name;
	int type_tag;
	int var_flags;
};
typedef struct svar_t svar_t;

struct mpc_iter_t {
	dynafun_data_t d;			/* init data */
	dynafun_t init;				/* init */
	void	(*destroy)(vo_iter_t *);	/* destroy */
	bool	(*cond)(vo_iter_t *, vo_t *);	/* loop condition */
	void	(*next)(vo_iter_t *, vo_t *);	/* get next */
	vo_iter_class_t *cl;
};

mpc_iter_t *mpc_iter_lookup(const char *name);

extern avltree_t glob_syms;		/* (sym_t) */
extern avltree_info_t c_info_syms;
extern flaginfo_t mpc_types[];

#define IS_PTR_TYPE(type_tag)						\
	((type_tag) == MT_STR ||					\
	 (type_tag) == MT_CHAR ||					\
	 (type_tag) == MT_OBJ ||					\
	 (type_tag) == MT_ROOM)						\

#define IS_INT_TYPE(type_tag)						\
	((type_tag) == MT_BOOL ||					\
	 (type_tag) == MT_SIZE_T ||					\
	 (type_tag) == MT_UINT)

/*
 * check that type_tag1 can be casted to type_tag2
 */
#define TYPE_IS(type_tag1, type_tag2)					\
	 ((type_tag1) == (type_tag2) ||					\
	  (IS_PTR_TYPE(type_tag1) && (type_tag2) == MT_PVOID) ||	\
	  (IS_INT_TYPE(type_tag1) && (type_tag2) == MT_INT))

/**
 * Switch jump table
 */
struct swjump_t {
	vo_t val;
	int addr;
};
typedef struct swjump_t swjump_t;

struct mpcode_t {
	const char *name;	/**< program name			*/

	mprog_t *mp;		/**< mprog (text, errbuf)		*/

	const char *cp;		/**< current position (for lexer)	*/
	int lineno;		/**< current line number		*/

	avltree_t strings;	/**< (const char *) string space	*/
	avltree_t syms;		/**< (sym_t) symbols			*/
	avltree_t svhs;		/**< (svh_t) svar_t holders		*/

	/* compiler data */
	varr cstack;		/**< (void *) compiler stack		*/
	varr args;		/**< (int) argument type stack		*/
	int curr_block;		/**< current block depth		*/

	int curr_jumptab;	/**< current jumptab			*/
	int curr_break_addr;	/**< current 'break' info		*/
	int curr_continue_addr;	/**< current 'continue' info		*/

	/* runtime data */
	int retval;		/**< program return value		*/
	int ip;			/**< program instruction pointer	*/
	varr code;		/**< (void *) program code		*/

	varr jumptabs;		/**< (varr) 'switch' jump tables	*/
	varr iters;		/**< (iterdata_t) iterators		*/
	varr affects;		/**< (AFFECT_DATA *) affects		*/

	jmp_buf jmpbuf;		/**< jmp buf				*/
	varr data;		/**< data stack				*/
};
typedef struct mpcode_t mpcode_t;

extern avltree_t mpcodes;

#define mpcode_lookup(name)	((mpcode_t *) c_lookup(&mpcodes, (name)))

/**
 * Currently executed mpc
 */
extern mpcode_t *current_mpc;

struct mpc_accessor_t {
	const char *name;
	int type_tag;
	int val_type_tag;
	vo_t (*get)(vo_t *vo);
	vo_t (*set)(vo_t *vo, vo_t val);
};
typedef struct mpc_accessor_t mpc_accessor_t;

mpc_accessor_t *mpc_accessor_lookup(int type_tag, const char *name);

/**
 * Dump program
 */
void
mpcode_dump(mpcode_t *mpc);

/**
 * Initialize mpc
 */
int
mpc_init(void);

/**
 * Destroy mpc environment
 */
void
mpc_fini(void);

/**
 * Lexer
 */
int mpc_lex(mpcode_t *mpc);

/**
 * Lookup symbol by name
 */
sym_t *
sym_lookup(mpcode_t *mpc, const char *name);

/**
 * Remove symbols with block >= specified
 */
void
cleanup_syms(mpcode_t *mpc, int block);

/**
 * Handle compilation error
 */
void
compile_error(mpcode_t *mpc, const char *fmt, ...)
	__attribute__ ((format(printf, 2, 3)));

/**
 * Make sure the string is allocated in program string space
 */
const char *alloc_string(mpcode_t *mpc, const char *s);

/**
 * Execute program from specified position
 */
void execute(mpcode_t *mpc, int ip);

/*--------------------------------------------------------------------
 * direct data stack manipulation opcodes
 */

typedef void (*c_fun)(mpcode_t *mpc);

/**
 * Pop value from data stack
 */
void	c_pop(mpcode_t *mpc);

/**
 * Push lvalue on data stack
 *
 * Expects lvalue on data stack
 */
void	c_push_lvalue(mpcode_t *mpc);

/**
 * Push constant on data stack
 *
 * Expects next opcode to be const value to push.
 */
void	c_push_const(mpcode_t *mpc);

/**
 * Push variable on data stack
 *
 * Expects next opcode to be variable name.
 */
void	c_push_var(mpcode_t *mpc);

/**
 * Push static variable on data stack
 */
void	c_push_svar(mpcode_t *mpc);

/**
 * Push function return value on data stack
 *
 * Expects next opcode to be 'func_t *'.
 */
void	c_push_retval(mpcode_t *mpc);

#define c_stop 0
#define INVALID_ADDR -1

void	c_jmp(mpcode_t *mpc);		/* jmp */
void	c_jmp_addr(mpcode_t *mpc);	/* jmp addr */
void	c_if(mpcode_t *mpc);		/* if */
void	c_switch(mpcode_t *mpc);	/* switch */
void	c_quecolon(mpcode_t *mpc);	/* ?: */
void	c_foreach(mpcode_t *mpc);	/* foreach */
void	c_foreach_next(mpcode_t *mpc);	/* foreach_next */
void	c_declare(mpcode_t *mpc);	/* declare variable */
void	c_declare_assign(mpcode_t *mpc);/* declare variable and assign */
					/* initial value */
void	c_cleanup_syms(mpcode_t *mpc);	/* cleanup symbols */
void	c_return(mpcode_t *mpc);	/* return expr */
void	c_push_accessor(mpcode_t *mpc);	/* accessor */

/*--------------------------------------------------------------------
 * binary operations
 *
 * all of them pop two arguments from data stack and push the result back.
 */

void	c_bop_lor(mpcode_t *mpc);	/* || */
void	c_bop_land(mpcode_t *mpc);	/* && */
void	c_bop_or(mpcode_t *mpc);	/* | */
void	c_bop_xor(mpcode_t *mpc);	/* ^ */
void	c_bop_and(mpcode_t *mpc);	/* & */

void	c_bop_ne(mpcode_t *mpc);	/* == */
void	c_bop_eq(mpcode_t *mpc);	/* != */

void	c_bop_gt(mpcode_t *mpc);	/* > */
void	c_bop_ge(mpcode_t *mpc);	/* >= */
void	c_bop_lt(mpcode_t *mpc);	/* < */
void	c_bop_le(mpcode_t *mpc);	/* <= */

void	c_bop_shl(mpcode_t *mpc);	/* << */
void	c_bop_shr(mpcode_t *mpc);	/* >> */
void	c_bop_add(mpcode_t *mpc);	/* + */
void	c_bop_sub(mpcode_t *mpc);	/* - */
void	c_bop_mul(mpcode_t *mpc);	/* * */
void	c_bop_mod(mpcode_t *mpc);	/* % */
void	c_bop_div(mpcode_t *mpc);	/* / */

void	c_bop_ne_string(mpcode_t *mpc);	/* == for strings (case-sensitive) */
void	c_bop_eq_string(mpcode_t *mpc);	/* != for strings (case-sensitive) */

/*--------------------------------------------------------------------
 * unary operations
 *
 * all of them pop one argument from data stack and push the result back.
 */
void	c_uop_not(mpcode_t *mpc);	/* ! */
void	c_uop_compl(mpcode_t *mpc);	/* ~ */
void	c_uop_minus(mpcode_t *mpc);	/* - (unary) */

/*--------------------------------------------------------------------
 * incdec operations
 *
 * all of them expect next opcode to be variable name.
 * push the result on data stack.
 */
void	c_postinc(mpcode_t *mpc);	/* post ++ */
void	c_postdec(mpcode_t *mpc);	/* post -- */
void	c_preinc(mpcode_t *mpc);	/* pre ++ */
void	c_predec(mpcode_t *mpc);	/* pre -- */

/*--------------------------------------------------------------------
 * assign operations
 *
 * all of them pop one argument from data stack and push the result back.
 * next op is expected to be variable name.
 */

void	c_assign(mpcode_t *mpc);	/* = */
void	c_add_eq(mpcode_t *mpc);	/* += */
void	c_sub_eq(mpcode_t *mpc);	/* -= */
void	c_div_eq(mpcode_t *mpc);	/* /= */
void	c_mul_eq(mpcode_t *mpc);	/* *= */
void	c_mod_eq(mpcode_t *mpc);	/* %= */
void	c_and_eq(mpcode_t *mpc);	/* &= */
void	c_or_eq(mpcode_t *mpc);		/* |= */
void	c_xor_eq(mpcode_t *mpc);	/* ^= */
void	c_shl_eq(mpcode_t *mpc);	/* <<= */
void	c_shr_eq(mpcode_t *mpc);	/* >>= */

#endif /* _MPC_IMPL_H_ */
