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
 * $Id: container.c,v 1.4 2001-11-30 21:18:03 fjoe Exp $
 */

#include <stdio.h>

#include <ctype.h>
#include <typedef.h>
#include <buffer.h>
#include <container.h>
#include <log.h>
#include <mlstring.h>
#include <rwfile.h>
#include <str.h>
#include <util.h>

void *
c_foreach(void *c, foreach_cb_t cb, ...)
{
	va_list ap;
	void *rv;

	va_start(ap, cb);
	rv = C_OPS(c)->c_foreach(c, cb, ap);
	va_end(ap);

	return rv;
}

void *
c_random_elem_foreach(void *c)
{
	size_t size = c_size(c);
	int num;
	void *elem;

	if (!size)
		return NULL;

	num = number_range(1, size);
	C_FOREACH(elem, c) {
		if (!--num)
			return elem;
	}

	return NULL;
}

void
c_dump(void *c, BUFFER *buf, foreach_cb_t cb)
{
	int col = 0;
	c_foreach(c, cb, buf, &col);
	if (col % 4 != 0)
		buf_append(buf, "\n");
}

int
vnum_ke_cmp(const void *k, const void *e)
{
	return *(const int *) k - *(const int *) e;
}

void
strkey_init(void *p)
{
	*(const char **) p = str_empty;
}

#if !defined(HASHTEST)
void
strkey_destroy(void *p)
{
	free_string(*(const char **) p);
}
#endif

int
ke_cmp_str(const void *k, const void *e)
{
	return str_cmp((const char *) k, *(const char * const *) e);
}

int
ke_cmp_csstr(const void *k, const void *e)
{
	return str_cscmp((const char *) k, *(const char * const *) e);
}

#if !defined(HASHTEST) && !defined(MPC)
int
ke_cmp_mlstr(const void *k, const void *e)
{
	return str_cmp((const char *) k, mlstr_mval((const mlstring *) e));
}

int
ke_cmp_csmlstr(const void *k, const void *e)
{
	return str_cscmp((const char *) k, mlstr_mval((const mlstring *) e));
}
#endif

void *
c_strkey_lookup(void *c, const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;
	return c_lookup(c, name);
}

void *
c_strkey_search(void *c, const char *name)
{
	void *elem;

	if (IS_NULLSTR(name))
		return NULL;

	/*
	 * try exact match first
	 */
	if ((elem = c_lookup(c, name)) != NULL)
		return elem;

	/*
	 * search by prefix
	 */
	C_FOREACH(elem, c) {
		if (!str_prefix(name, *(const char **) elem))
			return elem;
	}

	return NULL;
}

void *
c_mlstrkey_search(void *c, const char *name)
{
	void *elem;

	if (IS_NULLSTR(name))
		return NULL;

	/*
	 * try exact match first
	 */
	if ((elem = c_lookup(c, name)) != NULL)
		return elem;

	/*
	 * search by prefix
	 */
	C_FOREACH(elem, c) {
		if (!str_prefix(name, mlstr_mval((mlstring *) elem)))
			return elem;
	}

	return NULL;
}

void
c_strkey_dump(void *c, BUFFER *buf)
{
	c_dump(c, buf, str_dump_cb);
}

void
c_mlstrkey_dump(void *c, BUFFER *buf)
{
	c_dump(c, buf, mlstr_dump_cb);
}

FOREACH_CB_FUN(str_dump_cb, p, ap)
{
	BUFFER *buf = va_arg(ap, BUFFER *);
	int *pcol = va_arg(ap, int *);

	buf_printf(buf, BUF_END, "%-19.18s",			// notrans
		   *(const char**) p);
	if (++(*pcol) % 4 == 0)
		buf_append(buf, "\n");
	return NULL;
}

FOREACH_CB_FUN(mlstr_dump_cb, p, ap)
{
	BUFFER *buf = va_arg(ap, BUFFER *);
	int *pcol = va_arg(ap, int *);

	buf_printf(buf, BUF_END, "%-19.18s",			// notrans
		   mlstr_mval((mlstring *) p));
	if (++(*pcol) % 4 == 0)
		buf_append(buf, "\n");
	return NULL;
}

const char *
c_fread_strkey(const char *ctx, rfile_t *fp, void *c)
{
	const char *name = fread_sword(fp);
	C_STRKEY_CHECK(ctx, c, name);
	return name;
}

char *
strkey_filename(const char *name, const char *ext)
{
	static char buf[2][MAX_STRING_LENGTH];
	static int ind = 0;
	char *p;

	if (IS_NULLSTR(name))
		return str_empty;

	ind = (ind + 1) % 2;
	for (p = buf[ind]; *name && p < buf[ind] + sizeof(buf[0]) - 1;
				p++, name++) {
		switch (*name) {
		case ' ':
		case '\t':
			*p = '_';
			break;

		default:
			*p = LOWER(*name);
			break;
		}
	}

	*p = '\0';
	if (!IS_NULLSTR(ext))
		strnzcat(buf[ind], sizeof(buf[ind]), ext);
	return buf[ind];
}
