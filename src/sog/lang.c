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
 * $Id: lang.c,v 1.7 1999-02-16 20:26:04 fjoe Exp $
 */

#if	defined (LINUX) || defined (WIN32)
#include <string.h>
#endif

#include <limits.h>
#include <stdio.h>

#include "const.h"
#include "typedef.h"
#include "varr.h"
#include "word.h"
#include "lang.h"
#include "db.h"
#include "str.h"
#include "varr.h"

varr langs = { sizeof(LANG_DATA), 2 };

LANG_DATA *lang_new(void)
{
	int i;
	LANG_DATA *lang = varr_enew(&langs);

	lang->slang_of = -1;
	lang->vnum = langs.nused-1;

	for (i = 0; i < MAX_WORD_HASH; i++) {
		lang->hash_genders[i].nsize = sizeof(WORD_DATA);
		lang->hash_genders[i].nstep = 4;
		
		lang->hash_cases[i].nsize = sizeof(WORD_DATA);
		lang->hash_cases[i].nstep = 4;

		lang->hash_qtys[i].nsize = sizeof(WORD_DATA);
		lang->hash_qtys[i].nstep = 4;
	}

	return lang;
}

int lang_lookup(const char *name)
{
	return lang_nlookup(name, strlen(name));
}

int lang_nlookup(const char *name, size_t len)
{
	unsigned int lang;

	if (IS_NULLSTR(name))
		return -1;

	for (lang = 0; lang < langs.nused; lang++) {
		LANG_DATA *l = VARR_GET(&langs, lang);
		if (str_ncmp(l->name, name, len) == 0)
			return lang;
	}

	if (fBootDb)
		db_error("lang_lookup", "%s: unknown language", name);
	return -1;
}

