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
 * $Id: container.c,v 1.2 2001-09-13 12:03:08 fjoe Exp $
 */

#include <stdio.h>

#include <typedef.h>
#include <buffer.h>
#include <container.h>
#include <log.h>
#include <mlstring.h>
#include <rwfile.h>
#include <str.h>
#include <util.h>

static DECLARE_FOREACH_CB_FUN(str_search_cb);
static DECLARE_FOREACH_CB_FUN(mlstr_search_cb);
static DECLARE_FOREACH_CB_FUN(str_dump_cb);
static DECLARE_FOREACH_CB_FUN(mlstr_dump_cb);

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
c_strkey_lookup(void *c, const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;
	return c_lookup(c, name);
}

void *
c_strkey_search(void *c, const char *name)
{
	void *p;

	if (IS_NULLSTR(name))
		return NULL;

	/*
	 * try exact match first
	 */
	if ((p = c_lookup(c, name)) != NULL)
		return p;

	/*
	 * search by prefix
	 */
	return c_foreach(c, str_search_cb, name);
}

void *
c_mlstrkey_search(void *c, const char *name)
{
	void *p;

	if (IS_NULLSTR(name))
		return NULL;

	/*
	 * try exact match first
	 */
	if ((p = c_lookup(c, name)) != NULL)
		return p;

	/*
	 * search by prefix
	 */
	return c_foreach(c, mlstr_search_cb, name);
}

void
c_strkey_dump(void *c, BUFFER *buf)
{
	int col = 0;
	c_foreach(c, str_dump_cb, buf, &col);
	if (col % 4 != 0)
		buf_append(buf, "\n");
}

void
c_mlstrkey_dump(void *c, BUFFER *buf)
{
	int col = 0;
	c_foreach(c, mlstr_dump_cb, buf, &col);
	if (col % 4 != 0)
		buf_append(buf, "\n");
}

const char *
c_fread_strkey(const char *ctx, rfile_t *fp, void *c)
{
	const char *name = fread_sword(fp);
	C_STRKEY_CHECK(ctx, c, name);
	return name;
}

static
FOREACH_CB_FUN(get_nth_elem_cb, p, ap)
{
	int *pnum = va_arg(ap, int *);

	if (!--*pnum)
		return p;

	return NULL;
}

void *
c_random_elem_foreach(void *c)
{
	size_t size = c_size(c);
	int num;

	if (!size)
		return NULL;

	num = number_range(1, size);
	return c_foreach(c, get_nth_elem_cb, &num);
}

/*--------------------------------------------------------------------
 * static functions
 */

FOREACH_CB_FUN(str_search_cb, p, ap)
{
	const char *name = va_arg(ap, const char *);

	if (!str_prefix(name, *(const char **) p))
		return p;

	return NULL;
}

FOREACH_CB_FUN(mlstr_search_cb, p, ap)
{
	const char *key = va_arg(ap, const char *);
	if (!str_prefix(key, mlstr_mval((mlstring *) p)))
		return p;
	return NULL;
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
