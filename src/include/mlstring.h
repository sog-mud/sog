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
 * $Id: mlstring.h,v 1.14 1999-02-20 12:54:30 fjoe Exp $
 */

#ifndef _MLSTRING_H_
#define _MLSTRING_H_

mlstring *	mlstr_new	(const char *mval);
mlstring * 	mlstr_fread	(FILE *fp);
void		mlstr_fwrite	(FILE *fp, const char* name,
				 const mlstring *ml);
void		mlstr_free	(mlstring *ml);
mlstring *	mlstr_dup	(mlstring *ml);
mlstring *	mlstr_printf	(mlstring *ml, ...);

int		mlstr_nlang	(const mlstring *ml);
const char *	mlstr_val	(const mlstring *ml, int lang);
#define		mlstr_mval(ml)		mlstr_val(ml, 0)
#define		mlstr_cval(ml, ch)	mlstr_val(ml, ch->lang)
bool		mlstr_null	(const mlstring *ml);
int		mlstr_cmp	(const mlstring *ml1, const mlstring *ml2);

const char**	mlstr_convert	(mlstring **mlp, int newlang);

bool	mlstr_append	(CHAR_DATA *ch, mlstring **mlp, const char *arg);
bool	mlstr_edit	(mlstring **mlp, const char *arg);
bool	mlstr_editnl	(mlstring **mlp, const char *arg);

void	mlstr_dump	(BUFFER *buf, const char *name, const mlstring *ml);

void	mlstr_for_each	(mlstring **ml, void *arg,
			 void (*cb)(int lang, const char **p, void *arg));

#endif
