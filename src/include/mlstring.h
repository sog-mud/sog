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
 * $Id: mlstring.h,v 1.31 2001-08-03 11:27:28 fjoe Exp $
 */

#ifndef _MLSTRING_H_
#define _MLSTRING_H_

/*
 * multi-language string
 * if nlang == 0 the value is stored in u.str
 * otherwise the value is stored in array of strings u.lstr
 * the size of array is equal to 'nlang'
 * 'ref' = number of references (COW semantics)
 */

typedef const char *(mlstr_foreach_cb_t)(size_t, const char **, va_list);
#define DECLARE_MLSTR_FOREACH(fun) mlstr_foreach_cb_t
#define MLSTR_FOREACH_FUN(fun, lang, p, ap)				\
		const char *fun(size_t lang __attribute__((unused)),	\
		const char **p __attribute__((unused)),			\
		va_list ap __attribute__((unused)))
typedef struct mlstring {
	union {
		const char* str;
		const char** lstr;
	} u;
	uint nlang;
} mlstring;

void		mlstr_init	(mlstring *ml);
void		mlstr_init2	(mlstring *ml, const char *mval);
void		mlstr_destroy	(mlstring *ml);
void		mlstr_clear	(mlstring *ml);

void		mlstr_fread	(rfile_t *fp, mlstring *ml);
void		mlstr_fwrite	(FILE *fp, const char* name,
				 const mlstring *ml);
mlstring *	mlstr_cpy	(mlstring *dst, const mlstring *src);
void		mlstr_printf	(mlstring *dst, const mlstring *format, ...);

int		mlstr_nlang	(const mlstring *ml);
const char *	mlstr_val	(const mlstring *ml, size_t lang);
#define		mlstr_mval(ml)		mlstr_val(ml, 0)
#define		mlstr_cval(ml, ch)	mlstr_val(ml, GET_LANG(ch))
bool		mlstr_null	(const mlstring *ml);
bool		mlstr_valid	(const mlstring *ml);
int		mlstr_cmp	(const mlstring *ml1, const mlstring *ml2);

const char**	mlstr_convert	(mlstring *mlp, lang_t *newlang);

bool	mlstr_edit	(mlstring *mlp, const char *arg);
bool	mlstr_editnl	(mlstring *mlp, const char *arg);

void	mlstr_dump	(BUFFER *buf, const char *name, const mlstring *ml,
			 int dump_level);

const char *mlstr_foreach(mlstring *mlp, mlstr_foreach_cb_t *cb, ...);
bool	mlstr_addnl	(mlstring *mlp);
bool	mlstr_stripnl	(mlstring *mlp);

/*
 * mlstring with gender
 */
typedef struct gmlstr_t {
	mlstring ml;
	mlstring gender;
} gmlstr_t;

void		gmlstr_init	(gmlstr_t *);
gmlstr_t *	gmlstr_cpy	(gmlstr_t *dst, const gmlstr_t *src);
void		gmlstr_destroy	(gmlstr_t *);

#define gmlstr_mval(gml)	(mlstr_mval(&(gml)->ml))

#endif
