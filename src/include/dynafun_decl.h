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
 * $Id: dynafun_decl.h,v 1.16 2001-08-13 18:23:15 fjoe Exp $
 */

/* no #ifdef _XXX_H_/#define _XXX_H_/#endif */

/*
 * dynamic function call "blood SUGAR sex MAGIK"
 *
 * main dynafun module header should use this header in the following way:
 *
 *	#undef MODULE_NAME
 *	#define MODULE_NAME MOD_XXX
 *	#include <dynafun_decl.h>
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

#include <dynafun.h>
#include <module_decl.h>

#if (MODULE_INIT == MODULE_NAME)

#	undef ARG
#	define ARG(a)		{ __tag(a), FALSE }
#	undef NULLABLE_ARG
#	define NULLABLE_ARG(a)	{ __tag(a), TRUE }

#	undef __mod_tab_name
#	define __mod_tab_name(name) __mod_tab_##name

#	undef __mod_tab
#	define __mod_tab(name) __mod_tab_name(name)

#	undef __MODULE_START_DECL
#	define __MODULE_START_DECL \
		static dynafun_data_t __mod_tab(MODULE_NAME)[] = {

#	define NULL_ARG		{ 0, FALSE }

#	undef __MODULE_END_DECL
#	define __MODULE_END_DECL					\
		{ NULL, 0, 0,						\
		  { NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG,	\
		    NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG },		\
		  NULL							\
		}							\
	};

#else

#	undef ARG
#	define ARG(a)		__tag_t(a)
#	undef NULLABLE_ARG
#	define NULLABLE_ARG(a)	ARG(a)

#	undef __MODULE_START_DECL
#	define __MODULE_START_DECL

#	undef __MODULE_END_DECL
#	define __MODULE_END_DECL

#endif	/* MODULE_INIT */

#undef __tag
#define __tag(name) name##_tag

#undef __tag_t
#define __tag_t(name) name##_tag_t

#undef void_tag
#define void_tag MT_VOID
#undef void_tag_t
#define void_tag_t void

/* (void *) */
#undef pvoid_t_tag
#define pvoid_t_tag MT_PVOID
#undef pvoid_t_tag_t
#define pvoid_t_tag_t void *

/* (const void *) */
#undef pcvoid_t_tag
#define pcvoid_t_tag MT_PCVOID
#undef pcvoid_t_tag_t
#define pcvoid_t_tag_t const void *

#undef int_tag
#define int_tag	MT_INT
#undef int_tag_t
#define int_tag_t int

#undef actopt_t_tag
#define actopt_t_tag MT_ACTOPT
#undef actopt_t_tag_t
#define actopt_t_tag_t actopt_t *

#undef bool_tag
#define bool_tag MT_BOOL
#undef bool_tag_t
#define bool_tag_t bool

/* (const char *) */
#undef cchar_t_tag
#define cchar_t_tag MT_STR
#undef cchar_t_tag_t
#define cchar_t_tag_t const char *

/* (const char **) */
#undef pcchar_t_tag
#define pcchar_t_tag MT_PCCHAR
#undef pcchar_t_tag_t
#define pcchar_t_tag_t const char **

/* (char *) */
#undef pchar_t_tag
#define pchar_t_tag MT_PCHAR
#undef pchar_t_tag_t
#define pchar_t_tag_t char *

#undef gmlstr_t_tag
#define gmlstr_t_tag MT_GMLSTR
#undef gmlstr_t_tag_t
#define gmlstr_t_tag_t gmlstr_t *

#undef flaginfo_t_tag
#define flaginfo_t_tag MT_FLAGINFO
#undef flaginfo_t_tag_t
#define flaginfo_t_tag_t flaginfo_t *

#undef mlstring_tag
#define mlstring_tag MT_MLSTRING
#undef mlstring_tag_t
#define mlstring_tag_t mlstring *

#undef pint_t_tag
#define pint_t_tag MT_PINT
#undef pint_t_tag_t
#define pint_t_tag_t int *

#undef size_t_tag
#define size_t_tag MT_SIZE_T
#undef size_t_tag_t
#define size_t_tag_t size_t

#undef spec_skill_t_tag
#define spec_skill_t_tag MT_SPEC_SKILL
#undef spec_skill_t_tag_t
#define spec_skill_t_tag_t spec_skill_t *

#undef uint_tag
#define uint_tag MT_UINT
#undef uint_tag_t
#define uint_tag_t uint

#undef va_list_tag
#define va_list_tag MT_VA_LIST
#undef va_list_tag_t
#define va_list_tag_t va_list

#undef flag_t_tag
#define flag_t_tag MT_INT
#undef flag_t_tag_t
#define flag_t_tag_t flag_t

#undef CHAR_DATA_tag
#define CHAR_DATA_tag MT_CHAR
#undef CHAR_DATA_tag_t
#define CHAR_DATA_tag_t CHAR_DATA *

#undef OBJ_DATA_tag
#define OBJ_DATA_tag MT_OBJ
#undef OBJ_DATA_tag_t
#define OBJ_DATA_tag_t OBJ_DATA *

#undef AFFECT_DATA_tag
#define AFFECT_DATA_tag MT_AFFECT
#undef AFFECT_DATA_tag_t
#define AFFECT_DATA_tag_t AFFECT_DATA *

#undef BUFFER_tag
#define BUFFER_tag MT_BUFFER
#undef BUFFER_tag_t
#define BUFFER_tag_t BUFFER *

#undef OBJ_INDEX_DATA_tag
#define OBJ_INDEX_DATA_tag MT_OBJ_INDEX
#undef OBJ_INDEX_DATA_tag_t
#define OBJ_INDEX_DATA_tag_t OBJ_INDEX_DATA *

#undef MOB_INDEX_DATA_tag
#define MOB_INDEX_DATA_tag MT_MOB_INDEX
#undef MOB_INDEX_DATA_tag_t
#define MOB_INDEX_DATA_tag_t MOB_INDEX_DATA *

#undef ROOM_INDEX_DATA_tag
#define ROOM_INDEX_DATA_tag MT_ROOM
#undef ROOM_INDEX_DATA_tag_t
#define ROOM_INDEX_DATA_tag_t ROOM_INDEX_DATA *

#undef AREA_DATA_tag
#define AREA_DATA_tag MT_AREA
#undef AREA_DATA_tag_t
#define AREA_DATA_tag_t AREA_DATA *

#undef DESCRIPTOR_DATA_tag
#define DESCRIPTOR_DATA_tag MT_DESCRIPTOR
#undef DESCRIPTOR_DATA_tag_t
#define DESCRIPTOR_DATA_tag_t DESCRIPTOR_DATA *

#if (MODULE_INIT == MODULE_NAME)

/*
 * module init declarations
 */

#	undef DECLARE_FUN0
#	define DECLARE_FUN0(ret, name)					\
	{								\
	  #name, __tag(ret), 0,						\
	  { NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG,		\
	    NULL_ARG, NULL_ARG, NULL_ARG },				\
	  NULL,								\
	},

#	undef DECLARE_PROC0
#	define DECLARE_PROC0(name)					\
	{								\
	  #name, __tag(void), 0,					\
	  { NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG,		\
	    NULL_ARG, NULL_ARG, NULL_ARG },				\
	  NULL,								\
	},

#	undef DECLARE_FUN1
#	define DECLARE_FUN1(ret, name, a1, n1)				\
	{								\
	  #name, __tag(ret), 1,						\
	  { a1, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG,	\
	    NULL_ARG, NULL_ARG, NULL_ARG },				\
	  NULL,								\
	},

#	undef DECLARE_PROC1
#	define DECLARE_PROC1(name, a1, n1)				\
	{								\
	  #name, __tag(void), 1,					\
	  { a1, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG,	\
	    NULL_ARG, NULL_ARG, NULL_ARG },				\
	  NULL,								\
	},

#	undef DECLARE_FUN2
#	define DECLARE_FUN2(ret, name, a1, n1, a2, n2)			\
	{								\
	  #name, __tag(ret), 2,						\
	  { a1, a2, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG,	\
	    NULL_ARG, NULL_ARG },					\
	  NULL,								\
	},

#	undef DECLARE_PROC2
#	define DECLARE_PROC2(name, a1, n1, a2, n2)			\
	{								\
	  #name, __tag(void), 2,					\
	  { a1, a2, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG,	\
	    NULL_ARG, NULL_ARG },					\
	  NULL,								\
	},

#	undef DECLARE_FUN3
#	define DECLARE_FUN3(ret, name, a1, n1, a2, n2, a3, n3)		\
	{								\
	  #name, __tag(ret), 3,						\
	  { a1, a2, a3, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG,		\
	    NULL_ARG, NULL_ARG },					\
	  NULL,								\
	},

#	undef DECLARE_PROC3
#	define DECLARE_PROC3(name, a1, n1, a2, n2, a3, n3)		\
	{								\
	  #name, __tag(void), 3,					\
	  { a1, a2, a3, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG,		\
	    NULL_ARG, NULL_ARG },					\
	  NULL,								\
	},

#	undef DECLARE_FUN4
#	define DECLARE_FUN4(ret, name, a1, n1, a2, n2, a3, n3, a4, n4)	\
	{								\
	  #name, __tag(ret), 4,						\
	  { a1, a2, a3, a4,						\
	    NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG },		\
	  NULL,								\
	},

#	undef DECLARE_PROC4
#	define DECLARE_PROC4(name, a1, n1, a2, n2, a3, n3, a4, n4)	\
	{								\
	  #name, __tag(void), 4,					\
	  { a1, a2, a3, a4,						\
	    NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG },		\
	  NULL,								\
	},

#	undef DECLARE_FUN5
#	define DECLARE_FUN5(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5)					\
	{								\
	  #name, __tag(ret), 5,						\
	  { a1, a2, a3, a4, a5,						\
	    NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG },			\
	  NULL,								\
	},

#	undef DECLARE_PROC5
#	define DECLARE_PROC5(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5)					\
	{								\
	  #name, __tag(void), 5,					\
	  { a1, a2, a3, a4, a5,						\
	    NULL_ARG, NULL_ARG, NULL_ARG, NULL_ARG },			\
	  NULL,								\
	},

#	undef DECLARE_FUN6
#	define DECLARE_FUN6(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6)				\
	{								\
	  #name, __tag(ret), 6,						\
	  { a1, a2, a3, a4, a5, a6, NULL_ARG, NULL_ARG, NULL_ARG },	\
	  NULL,								\
	},

#	undef DECLARE_PROC6
#	define DECLARE_PROC6(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6)				\
	{								\
	  #name, __tag(void), 6,					\
	  { a1, a2, a3, a4, a5, a6, NULL_ARG, NULL_ARG, NULL_ARG },	\
	  NULL,								\
	},

#	undef DECLARE_FUN7
#	define DECLARE_FUN7(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6, a7, n7)			\
	{								\
	  #name, __tag(ret), 6,						\
	  { a1, a2, a3, a4, a5, a6, a7, NULL_ARG, NULL_ARG },		\
	  NULL,								\
	},

#	undef DECLARE_PROC7
#	define DECLARE_PROC7(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6, a7, n7)			\
	{								\
	  #name, __tag(void), 6,					\
	  { a1, a2, a3, a4, a5, a6, a7, NULL_ARG, NULL_ARG },		\
	  NULL,								\
	},

#	undef DECLARE_FUN8
#	define DECLARE_FUN8(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6, a7, n7, a8, n8)		\
	{								\
	  #name, __tag(ret), 6,						\
	  { a1, a2, a3, a4, a5, a6, a7, a8, NULL_ARG },			\
	  NULL,								\
	},

#	undef DECLARE_PROC8
#	define DECLARE_PROC8(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6, a7, n7, a8, n8)		\
	{								\
	  #name, __tag(void), 6,					\
	  { a1, a2, a3, a4, a5, a6, a7, a8, NULL_ARG },			\
	  NULL,								\
	},

#	undef DECLARE_FUN9
#	define DECLARE_FUN9(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6, a7, n7, a8, n8, a9, n9)	\
	{								\
	  #name, __tag(ret), 6,						\
	  { a1, a2, a3, a4, a5, a6, a7, a8, a9 },			\
	  NULL,								\
	},

#	undef DECLARE_PROC9
#	define DECLARE_PROC9(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6, a7, n7, a8, n8, a9, n9)	\
	{								\
	  #name, __tag(void), 6,					\
	  { a1, a2, a3, a4, a5, a6, a7, a8, a9 },			\
	  NULL,								\
	},

#elif (MODULE == MODULE_NAME)

/*
 * module implementation declarations
 */

#	undef DECLARE_FUN0
#	define DECLARE_FUN0(ret, name)	\
	__tag_t(ret) name(void);

#	undef DECLARE_PROC0
#	define DECLARE_PROC0(name)	\
		DECLARE_FUN0(void, name)

#	undef DECLARE_FUN1
#	define DECLARE_FUN1(ret, name, a1, n1)	\
	__tag_t(ret) name(a1);

#	undef DECLARE_PROC1
#	define DECLARE_PROC1(name, a1, n1)	\
		DECLARE_FUN1(void, name, a1, n1)

#	undef DECLARE_FUN2
#	define DECLARE_FUN2(ret, name, a1, n1, a2, n2)	\
	__tag_t(ret) name(a1, a2);

#	undef DECLARE_PROC2
#	define DECLARE_PROC2(name, a1, n1, a2, n2)	\
		DECLARE_FUN2(void, name, a1, n1, a2, n2)

#	undef DECLARE_FUN3
#	define DECLARE_FUN3(ret, name, a1, n1, a2, n2, a3, n3)	\
	__tag_t(ret) name(a1, a2, a3);

#	undef DECLARE_PROC3
#	define DECLARE_PROC3(name, a1, n1, a2, n2, a3, n3)	\
		DECLARE_FUN3(void, name, a1, n1, a2, n2, a3, n3)

#	undef DECLARE_FUN4
#	define DECLARE_FUN4(ret, name, a1, n1, a2, n2, a3, n3, a4, n4)	\
	__tag_t(ret) name(a1, a2, a3, a4);

#	undef DECLARE_PROC4
#	define DECLARE_PROC4(name, a1, n1, a2, n2, a3, n3, a4, n4)	\
		DECLARE_FUN4(void, name, a1, n1, a2, n2, a3, n3, a4, n4)

#	undef DECLARE_FUN5
#	define DECLARE_FUN5(ret, name, a1, n1, a2, n2, a3, n3, a4, n4, a5, n5)	\
	__tag_t(ret) name(a1, a2, a3, a4, a5);

#	undef DECLARE_PROC5
#	define DECLARE_PROC5(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5)					\
		DECLARE_FUN5(void, name, a1, n1, a2, n2, a3, n3, a4, n4,\
			     a5, n5)

#	undef DECLARE_FUN6
#	define DECLARE_FUN6(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6)				\
	__tag_t(ret) name(a1, a2, a3, a4, a5, a6);

#	undef DECLARE_PROC6
#	define DECLARE_PROC6(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6)				\
		DECLARE_FUN6(void, name, a1, n1, a2, n2, a3, n3, a4, n4,\
			     a5, n5, a6, n6)

#	undef DECLARE_FUN7
#	define DECLARE_FUN7(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6, a7, n7)			\
	__tag_t(ret) name(a1, a2, a3, a4, a5, a6, a7);

#	undef DECLARE_PROC7
#	define DECLARE_PROC7(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6, a7, n7)			\
		DECLARE_FUN7(void, name, a1, n1, a2, n2, a3, n3, a4, n4,\
			     a5, n5, a6, n6, a7, n7)

#	undef DECLARE_FUN8
#	define DECLARE_FUN8(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6, a7, n7, a8, n8)		\
	__tag_t(ret) name(a1, a2, a3, a4, a5, a6, a7, a8);

#	undef DECLARE_PROC8
#	define DECLARE_PROC8(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6, a7, n7, a8, n8)		\
		DECLARE_FUN8(void, name, a1, n1, a2, n2, a3, n3, a4, n4,\
			     a5, n5, a6, n6, a7, n7, a8, n8)

#	undef DECLARE_FUN9
#	define DECLARE_FUN9(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6, a7, n7, a8, n8, a9, n9)	\
	__tag_t(ret) name(a1, a2, a3, a4, a5, a6, a7, a8, a9);

#	undef DECLARE_PROC9
#	define DECLARE_PROC9(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6, a7, n7, a8, n8, a9, n9)	\
		DECLARE_FUN9(void, name, a1, n1, a2, n2, a3, n3, a4, n4,\
			     a5, n5, a6, n6, a7, n7, a8, n8, a9, n9)

#else

/*
 * common declarations
 */

#	undef DECLARE_FUN0
#	define DECLARE_FUN0(ret, name)					\
		__tag_t(ret) name(void);				\
		extern inline __tag_t(ret)				\
		name(void)						\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 0);					\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC0
#	define DECLARE_PROC0(name)					\
		void name(void);					\
		extern inline void					\
		name(void)						\
		{							\
			dynaproc_call(#name, 0);			\
		}

#	undef DECLARE_FUN1
#	define DECLARE_FUN1(ret, name, a1, n1)				\
		__tag_t(ret) name(a1);					\
		extern inline __tag_t(ret)				\
		name(a1 n1)						\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 1, n1);				\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC1
#	define DECLARE_PROC1(name, a1, n1)				\
		void name(a1);						\
		extern inline void					\
		name(a1 n1)						\
		{							\
			dynaproc_call(#name, 1,				\
			    n1);					\
		}

#	undef DECLARE_FUN2
#	define DECLARE_FUN2(ret, name, a1, n1, a2, n2)			\
		__tag_t(ret) name(a1, a2);				\
		extern inline __tag_t(ret)				\
		name(a1 n1, a2 n2)					\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 2, n1, n2);				\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC2
#	define DECLARE_PROC2(name, a1, n1, a2, n2)			\
		void name(a1, a2);					\
		extern inline void					\
		name(a1 n1, a2 n2)					\
		{							\
			dynaproc_call(#name, 2,				\
			    n1, n2);					\
		}

#	undef DECLARE_FUN3
#	define DECLARE_FUN3(ret, name, a1, n1, a2, n2, a3, n3)		\
		__tag_t(ret) name(a1, a2, a3);				\
		extern inline __tag_t(ret)				\
		name(a1 n1, a2 n2, a3 n3)				\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 3, n1, n2, n3);			\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC3
#	define DECLARE_PROC3(name, a1, n1, a2, n2, a3, n3)		\
		void name(a1, a2, a3);					\
		extern inline void					\
		name(a1 n1, a2 n2, a3 n3)				\
		{							\
			dynaproc_call(#name, 3,				\
			    n1, n2, n3);				\
		}

#	undef DECLARE_FUN4
#	define DECLARE_FUN4(ret, name, a1, n1, a2, n2, a3, n3, a4, n4)	\
		__tag_t(ret) name(a1, a2, a3, a4);			\
		extern inline __tag_t(ret)				\
		name(a1 n1, a2 n2, a3 n3, a4 n4)			\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 4, n1, n2, n3, n4);			\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC4
#	define DECLARE_PROC4(name, a1, n1, a2, n2, a3, n3, a4, n4)	\
		void name(a1, a2, a3, a4);				\
		extern inline void					\
		name(a1 n1, a2 n2, a3 n3, a4 n4)			\
		{							\
			dynaproc_call(#name, 4,				\
			    n1, n2, n3, n4);				\
		}

#	undef DECLARE_FUN5
#	define DECLARE_FUN5(ret, name, a1, n1, a2, n2, a3, n3,		\
				       a4, n4, a5, n5)			\
		__tag_t(ret) name(a1, a2, a3, a4, a5);			\
		extern inline __tag_t(ret)				\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5)			\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 5, n1, n2, n3, n4, n5);		\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC5
#	define DECLARE_PROC5(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
				   a5, n5)				\
		void name(a1, a2, a3, a4, a5);				\
		extern inline void					\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5)			\
		{							\
			dynaproc_call(#name, 5,				\
			    n1, n2, n3, n4, n5);			\
		}

#	undef DECLARE_FUN6
#	define DECLARE_FUN6(ret, name, a1, n1, a2, n2, a3, n3,		\
				       a4, n4, a5, n5, a6, n6)		\
		__tag_t(ret) name(a1, a2, a3, a4, a5, a6);		\
		extern inline __tag_t(ret)				\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5, a6 n6)		\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 6, n1, n2, n3, n4, n5, n6);		\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC6
#	define DECLARE_PROC6(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
				   a5, n5, a6, n6)			\
		void name(a1, a2, a3, a4, a5, a6);			\
		extern inline void					\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5, a6 n6)		\
		{							\
			dynaproc_call(#name, 6,				\
			    n1, n2, n3, n4, n5, n6);			\
		}

#	undef DECLARE_FUN7
#	define DECLARE_FUN7(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6, a7, n7)			\
		__tag_t(ret) name(a1, a2, a3, a4, a5, a6, a7);		\
		extern inline __tag_t(ret)				\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5, a6 n6, a7 n7)	\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 6, n1, n2, n3, n4, n5, n6, n7);	\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC7
#	define DECLARE_PROC7(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6, a7, n7)			\
		void name(a1, a2, a3, a4, a5, a6, a7);			\
		extern inline void					\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5, a6 n6, a7 n7)	\
		{							\
			dynaproc_call(#name, 6,				\
			    n1, n2, n3, n4, n5, n6, n7);		\
		}

#	undef DECLARE_FUN8
#	define DECLARE_FUN8(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6, a7, n7, a8, n8)		\
		__tag_t(ret) name(a1, a2, a3, a4, a5, a6, a7, a8);	\
		extern inline __tag_t(ret)				\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5, a6 n6, a7 n7,	\
		     a8 n8)						\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 6, n1, n2, n3, n4, n5, n6, n7, n8);	\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC8
#	define DECLARE_PROC8(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6, a7, n7, a8, n8)		\
		void name(a1, a2, a3, a4, a5, a6, a7, a8);		\
		extern inline void					\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5, a6 n6, a7 n7,	\
		     a8 n8)						\
		{							\
			dynaproc_call(#name, 6,				\
			    n1, n2, n3, n4, n5, n6, n7, n8);		\
		}

#	undef DECLARE_FUN9
#	define DECLARE_FUN9(ret, name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			    a5, n5, a6, n6, a7, n7, a8, n8, a9, n9)	\
		__tag_t(ret) name(a1, a2, a3, a4, a5, a6, a7, a8, a9);	\
		extern inline __tag_t(ret)				\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5, a6 n6, a7 n7,	\
		     a8 n8, a9 n9)					\
		{							\
			__tag_t(ret) rv;				\
			void *v;					\
			v = dynafun_call(__tag(ret),			\
			    #name, 6, n1, n2, n3, n4, n5, n6, n7, n8, n9);\
			rv = (__tag_t(ret)) v;				\
			return rv;					\
		}

#	undef DECLARE_PROC9
#	define DECLARE_PROC9(name, a1, n1, a2, n2, a3, n3, a4, n4,	\
			     a5, n5, a6, n6, a7, n7, a8, n8, a9, n9)	\
		void name(a1, a2, a3, a4, a5, a6, a7, a8, a9);		\
		extern inline void					\
		name(a1 n1, a2 n2, a3 n3, a4 n4, a5 n5, a6 n6, a7 n7,	\
		     a8 n8, a9 n9)					\
		{							\
			dynaproc_call(#name, 6,				\
			    n1, n2, n3, n4, n5, n6, n7, n8, n9);	\
		}

#endif
