/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: dynafun.h,v 1.1 2000-05-24 21:17:47 fjoe Exp $
 */

#ifndef _DYNAFUN_H_
#define _DYNAFUN_H_

#define DYNAFUN_NARGS 6

typedef struct dynafun_args_t {
	char p[DYNAFUN_NARGS * sizeof(void *)];
} dynafun_args_t;
typedef void *(*dynafun_t)(dynafun_args_t);

typedef struct dynafun_data_t dynafun_data_t;
struct dynafun_data_t {
	const char *name;
	int rv_type;
	int nargs;
	int argtype[DYNAFUN_NARGS];
	dynafun_t fun;
};

void	init_dynafuns(void);

void *	dynafun_call(cchar_t, int nargs, ...);
void	dynafun_tab_register(dynafun_data_t *, module_t *m);
void	dynafun_tab_unregister(dynafun_data_t *);

/*
 * dynamic function call "blood SUGAR sex MAGIK"
 */

#define __tag(name) name##_tag

#if defined(MODULE_INIT) && (MODULE_INIT == MODULE_NAME)
#	define __MODULE_START_DECL(name)	\
		static dynafun_data_t dynafun_tab##name[] = {
#	define __MODULE_END_DECL	{ NULL } };

#	define void_tag MT_VOID
#	define int_tag	MT_INT
#	define bool_tag MT_INT
#	define cchar_t_tag MT_STR
#	define CHAR_DATA_tag MT_CHAR
#	define OBJ_DATA_tag MT_OBJ
#	define AFFECT_DATA_tag MT_AFFECT
#else
#	define __MODULE_START_DECL(name)
#	define __MODULE_END_DECL

#	define void_tag void *
#	define int_tag	int
#	define bool_tag bool
#	define cchar_t_tag cchar_t
#	define CHAR_DATA_tag CHAR_DATA *
#	define OBJ_DATA_tag OBJ_DATA *
#	define AFFECT_DATA_tag AFFECT_DATA *
#endif	/* MODULE_INIT */

#if defined(MODULE_INIT) && (MODULE_INIT == MODULE_NUM)
/*
 * init module declarations
 */

#	define DECLARE_FUN1(ret, name, a1, n1)	\
	{ \
	  #name, __tag(ret), 1, \
	  { __tag(a1) } \
	},
#	define DECLARE_FUN2(ret, name, a1, n1, a2, n2)	\
	{ \
	  #name, __tag(ret), 2, \
	  { __tag(a1), __tag(a2) } \
	},
#	define DECLARE_FUN3(ret, name, a1, n1, a2, n2, a3, n3)	\
	{ \
	  #name, __tag(ret), 3, \
	  { __tag(a1), __tag(a2), __tag(a3) } \
	},
#	define DECLARE_FUN4(ret, name, a1, n1, a2, n2, a3, n3, a4, n4)	\
	{ \
	  #name, __tag(ret), 4, \
	  { __tag(a1), __tag(a2), __tag(a3), __tag(a4) } \
	},
#	define DECLARE_FUN5(ret, name, a1, n1, a2, n2, a3, n3, a4, n4, a5, n5)	\
	{ \
	  #name, __tag(ret), 5, \
	  { __tag(a1), __tag(a2), __tag(a3), __tag(a4), __tag(a5) } \
	},
#	define DECLARE_FUN6(ret, name, a1, n1, a2, n2, a3, n3, a4, n4, a5, n5, a6, n6)	\
	{ \
	  #name, __tag(ret), 6, \
	  { __tag(a1), __tag(a2), __tag(a3), __tag(a4), __tag(a5), __tag(a6) } \
	},

#elif defined(MODULE_IMPL) && (MODULE_IMPL == MODULE_NUM)
/*
 * module implementation declarations
 */

#	define DECLARE_FUN1(ret, name, a1, n1)	\
	__tag(ret) name(__tag(a1));
#	define DECLARE_FUN2(ret, name, a1, n1, a2, n2)	\
	__tag(ret) name(__tag(a1), __tag(a2));
#	define DECLARE_FUN3(ret, name, a1, n1, a2, n2, a3, n3)	\
	__tag(ret) name(__tag(a1), __tag(a2), __tag(a3));
#	define DECLARE_FUN4(ret, name, a1, n1, a2, n2, a3, n3, a4, n4)	\
	__tag(ret) name(__tag(a1), __tag(a2), __tag(a3), __tag(a4));
#	define DECLARE_FUN5(ret, name, a1, n1, a2, n2, a3, n3, a4, n4, a5, n5)	\
	__tag(ret) name(__tag(a1), __tag(a2), __tag(a3), __tag(a4), __tag(a5));
#	define DECLARE_FUN6(ret, name, a1, n1, a2, n2, a3, n3, a4, n4, a5, n5, a6, n6)	\
	__tag(ret) name(__tag(a1), __tag(a2), __tag(a3), __tag(a4), __tag(a5), __tag(a6));
#else
/*
 * common declarations
 */

#	define DECLARE_FUN1(ret, name, a1, n1)				\
		static inline __tag(ret)				\
		name(__tag(a1) n1)					\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 1,	\
						n1);			\
			return rv;					\
		}
#	define DECLARE_FUN2(ret, name, a1, n1, a2, n2)			\
		static inline __tag(ret)				\
		name(__tag(a1) n1, __tag(a2) n2)			\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 2,	\
						n1, n2);		\
			return rv;					\
		}
#	define DECLARE_FUN3(ret, name, a1, n1, a2, n2, a3, n3)		\
		static inline __tag(ret)				\
		name(__tag(a1) n1, __tag(a2) n2, __tag(a3) n3)		\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 3,	\
						n1, n2, n3);		\
			return rv;					\
		}
#	define DECLARE_FUN4(ret, name, a1, n1, a2, n2, a3, n3,		\
				       a4, n4)				\
		static inline __tag(ret)				\
		name(__tag(a1) n1, __tag(a2) n2, __tag(a3) n3,		\
		     __tag(a4) n4)					\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 4,	\
						n1, n2, n3, n4);	\
			return rv;					\
		}
#	define DECLARE_FUN5(ret, name, a1, n1, a2, n2, a3, n3,		\
				       a4, n4, a5, n5)			\
		static inline __tag(ret)				\
		name(__tag(a1) n1, __tag(a2) n2, __tag(a3) n3,		\
		     __tag(a4) n4, __tag(a5) n5)			\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 5,	\
						n1, n2, n3, n4, n5);	\
			return rv;					\
		}
#	define DECLARE_FUN6(ret, name, a1, n1, a2, n2, a3, n3,		\
				       a4, n4, a5, n5, a6, n6)		\
		static inline __tag(ret)				\
		name(__tag(a1) n1, __tag(a2) n2, __tag(a3) n3,		\
		     __tag(a4) n4, __tag(a5) n5, __tag(a6) n6)		\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 6,	\
						n1, n2, n3, n4, n5, n6);\
			return rv;					\
		}
#endif

#endif /* _DYNAFUN_H_ */
