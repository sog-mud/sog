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
 * $Id: dynafun_decl.h,v 1.4 2000-06-05 12:06:16 fjoe Exp $
 */

/* no #ifdef _XXX_H_/#define _XXX_H_/#endif */

/*
 * dynamic function call "blood SUGAR sex MAGIK"
 *
 * main dynafun module header should use this header in the following way:
 *
 *	#undef MODULE_NAME
 *	#define MODULE_NAME MOD_XXX
 *	#include "dynafun_decl.h"
 *
 *	__MODULE_START_DECL
 *	DECLARE_FUNX(...)
 *	...
 *	__MODULE_END_DECL
 *
 * MOD_XXX should be defined in "module_decl.h"
 * do not use ';' after DECLARE_FUNX(...) "declarations"
 *
 * additionally module implementation .c files (where dynafuns are
 * actually implemented) should #define MODULE to `xxx'
 * (the same as MODULE_NAME in main header, this is done
 *  via Makefile's now)
 * and module init .c file should #define MODULE_INIT to `xxx'
 * before including main dynafun header
 *
 * macro __mod_tab(MOD_XXX) can be used in module init .c file
 */

#include "dynafun.h"
#include "module_decl.h"

#undef __tag
#define __tag(name) name##_tag

#if (MODULE_INIT == MODULE_NAME)
#	undef __mod_tab_name
#	define __mod_tab_name(name) __mod_tab_##name

#	undef __mod_tab
#	define __mod_tab(name) __mod_tab_name(name)

#	undef __MODULE_START_DECL
#	define __MODULE_START_DECL \
		static dynafun_data_t __mod_tab(MODULE_NAME)[] = {

#	undef __MODULE_END_DECL
#	define __MODULE_END_DECL	{ NULL } };

#	undef void_tag
#	define void_tag MT_VOID

#	undef int_tag
#	define int_tag	MT_INT

#	undef bool_tag
#	define bool_tag MT_INT

#	undef cchar_t_tag
#	define cchar_t_tag MT_STR

#	undef va_list_tag
#	define va_list_tag MT_VA_LIST

#	undef CHAR_DATA_tag
#	define CHAR_DATA_tag MT_CHAR

#	undef OBJ_DATA_tag
#	define OBJ_DATA_tag MT_OBJ

#	undef AFFECT_DATA_tag
#	define AFFECT_DATA_tag MT_AFFECT

#	undef BUFFER_tag
#	define BUFFER_tag MT_BUFFER

#	undef OBJ_INDEX_DATA_tag
#	define OBJ_INDEX_DATA_tag MT_OBJ_INDEX

#	undef MOB_INDEX_DATA_tag
#	define MOB_INDEX_DATA_tag MT_MOB_INDEX

#	undef ROOM_INDEX_DATA_tag
#	define ROOM_INDEX_DATA_tag MT_ROOM

#else

#	undef __MODULE_START_DECL
#	define __MODULE_START_DECL

#	undef __MODULE_END_DECL
#	define __MODULE_END_DECL

#	undef void_tag
#	define void_tag void *

#	undef int_tag
#	define int_tag	int

#	undef bool_tag
#	define bool_tag bool

#	undef cchar_t_tag
#	define cchar_t_tag cchar_t

#	undef va_list_tag
#	define va_list_tag va_list

#	undef CHAR_DATA_tag
#	define CHAR_DATA_tag CHAR_DATA *

#	undef OBJ_DATA_tag
#	define OBJ_DATA_tag OBJ_DATA *

#	undef AFFECT_DATA_tag
#	define AFFECT_DATA_tag AFFECT_DATA *

#	undef BUFFER_tag
#	define BUFFER_tag BUFFER *

#	undef OBJ_INDEX_DATA_tag
#	define OBJ_INDEX_DATA_tag OBJ_INDEX_DATA *

#	undef MOB_INDEX_DATA_tag
#	define MOB_INDEX_DATA_tag MOB_INDEX_DATA *

#	undef ROOM_INDEX_DATA_tag
#	define ROOM_INDEX_DATA_tag ROOM_INDEX_DATA *

#endif	/* MODULE_INIT */

#if (MODULE_INIT == MODULE_NAME)

/*
 * module init declarations
 */

#	undef DECLARE_FUN0
#	define DECLARE_FUN0(ret, name)	\
	{ \
	  #name, __tag(ret), 0 \
	},

#	undef DECLARE_FUN1
#	define DECLARE_FUN1(ret, name, a1, n1)	\
	{ \
	  #name, __tag(ret), 1, \
	  { __tag(a1) } \
	},

#	undef DECLARE_FUN2
#	define DECLARE_FUN2(ret, name, a1, n1, a2, n2)	\
	{ \
	  #name, __tag(ret), 2, \
	  { __tag(a1), __tag(a2) } \
	},

#	undef DECLARE_FUN3
#	define DECLARE_FUN3(ret, name, a1, n1, a2, n2, a3, n3)	\
	{ \
	  #name, __tag(ret), 3, \
	  { __tag(a1), __tag(a2), __tag(a3) } \
	},

#	undef DECLARE_FUN4
#	define DECLARE_FUN4(ret, name, a1, n1, a2, n2, a3, n3, a4, n4)	\
	{ \
	  #name, __tag(ret), 4, \
	  { __tag(a1), __tag(a2), __tag(a3), __tag(a4) } \
	},

#	undef DECLARE_FUN5
#	define DECLARE_FUN5(ret, name, a1, n1, a2, n2, a3, n3, a4, n4, a5, n5)	\
	{ \
	  #name, __tag(ret), 5, \
	  { __tag(a1), __tag(a2), __tag(a3), __tag(a4), __tag(a5) } \
	},

#	undef DECLARE_FUN6
#	define DECLARE_FUN6(ret, name, a1, n1, a2, n2, a3, n3, a4, n4, a5, n5, a6, n6)	\
	{ \
	  #name, __tag(ret), 6, \
	  { __tag(a1), __tag(a2), __tag(a3), __tag(a4), __tag(a5), __tag(a6) } \
	},

#elif (MODULE == MODULE_NAME)

/*
 * module implementation declarations
 */

#	undef DECLARE_FUN0
#	define DECLARE_FUN0(ret, name)	\
	__tag(ret) name(void);

#	undef DECLARE_FUN1
#	define DECLARE_FUN1(ret, name, a1, n1)	\
	__tag(ret) name(__tag(a1));

#	undef DECLARE_FUN2
#	define DECLARE_FUN2(ret, name, a1, n1, a2, n2)	\
	__tag(ret) name(__tag(a1), __tag(a2));

#	undef DECLARE_FUN3
#	define DECLARE_FUN3(ret, name, a1, n1, a2, n2, a3, n3)	\
	__tag(ret) name(__tag(a1), __tag(a2), __tag(a3));

#	undef DECLARE_FUN4
#	define DECLARE_FUN4(ret, name, a1, n1, a2, n2, a3, n3, a4, n4)	\
	__tag(ret) name(__tag(a1), __tag(a2), __tag(a3), __tag(a4));

#	undef DECLARE_FUN5
#	define DECLARE_FUN5(ret, name, a1, n1, a2, n2, a3, n3, a4, n4, a5, n5)	\
	__tag(ret) name(__tag(a1), __tag(a2), __tag(a3), __tag(a4), __tag(a5));

#	undef DECLARE_FUN6
#	define DECLARE_FUN6(ret, name, a1, n1, a2, n2, a3, n3, a4, n4, a5, n5, a6, n6)	\
	__tag(ret) name(__tag(a1), __tag(a2), __tag(a3), __tag(a4), __tag(a5), __tag(a6));

#else

/*
 * common declarations
 */

#	undef DECLARE_FUN0
#	define DECLARE_FUN0(ret, name)					\
		static inline __tag(ret)				\
		name(void)						\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 0);	\
			return rv;					\
		}

#	undef DECLARE_FUN1
#	define DECLARE_FUN1(ret, name, a1, n1)				\
		static inline __tag(ret)				\
		name(__tag(a1) n1)					\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 1,	\
						n1);			\
			return rv;					\
		}

#	undef DECLARE_FUN2
#	define DECLARE_FUN2(ret, name, a1, n1, a2, n2)			\
		static inline __tag(ret)				\
		name(__tag(a1) n1, __tag(a2) n2)			\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 2,	\
						n1, n2);		\
			return rv;					\
		}

#	undef DECLARE_FUN3
#	define DECLARE_FUN3(ret, name, a1, n1, a2, n2, a3, n3)		\
		static inline __tag(ret)				\
		name(__tag(a1) n1, __tag(a2) n2, __tag(a3) n3)		\
		{							\
			__tag(ret) rv;					\
			rv = (__tag(ret)) dynafun_call(#name, 3,	\
						n1, n2, n3);		\
			return rv;					\
		}

#	undef DECLARE_FUN4
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

#	undef DECLARE_FUN5
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

#	undef DECLARE_FUN6
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
