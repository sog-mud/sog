/*-
 * Copyright (c) 2000 SoG Development Team
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
 * $Id: update.h,v 1.2 2000-02-28 18:21:54 avn Exp $
 */

#ifndef _UPDATE_H_
#define _UPDATE_H_

typedef void (*update_fun)(void);

typedef struct update_info_t {
	int cnt;
	int max;
	const char *name;
	const char *notify;
	const char *fun_name;
	void *fun;
	vo_iter_t *iter;
} update_info_t;

void	update_init(update_info_t *ui);
void	update_destroy(update_info_t *ui);
void	*update_load_cb(void *p, va_list ap);
void	*update_unload_cb(void *p, va_list ap);

void	update_handler	(void);
void	*update_one(const char *what);
void	update_reset(const char *what);
int	get_pulse(const char *what);

extern varr	updates;

void gain_condition(CHAR_DATA *ch, int iCond, int value);

#endif
