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
 * $Id: update.c,v 1.189 2000-03-02 17:14:12 avn Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>

#include "merc.h"
#include "module.h"

varr updates;
void (*gain_cond)(CHAR_DATA *ch, int iCond, int value);

void
inline gain_condition(CHAR_DATA *ch, int iCond, int value)
{
	if (gain_cond)
		gain_cond(ch, iCond, value);
}

void
update_init(update_info_t *ui)
{
	ui->name = str_empty;
	ui->cnt = 0;
	ui->max = 0;
	ui->fun_name = str_empty;
	ui->fun = NULL;
	ui->iter = NULL;
}

void
update_destroy(update_info_t *ui)
{
	free_string(ui->name);
	free_string(ui->fun_name);
	ui->fun = NULL;
}

void *
update_load_cb(void *p, va_list ap)
{
	update_info_t *ui = (update_info_t *) p;
	module_t *m = va_arg(ap, module_t*);

	ui->fun = dlsym(m->dlh, ui->fun_name);

	if (ui->fun == NULL)
		log(LOG_ERROR, "update_load: %s", dlerror());

	return NULL;
}

void *
update_unload_cb(void *p, va_list ap)
{
	update_info_t *ui = (update_info_t *) p;

	ui->fun = NULL;
	return NULL;
}


void *
update_check_cb(void *p, va_list ap)
{
	update_info_t *ui = (update_info_t *) p;

	if (ui->fun == NULL) {
		log(LOG_ERROR, "update_check: NULL update fun for '%s'", ui->name);
		return NULL;
	}

	if (--ui->cnt == 0) {
		ui->cnt = ui->max;

		if (ui->iter == NULL)
			((update_fun)ui->fun)();
		else
			vo_foreach(NULL, ui->iter, ui->fun);

		if (!IS_NULLSTR(ui->notify))
			wiznet(ui->notify, NULL, NULL, WIZ_TICKS, 0, 0);
	}
	return NULL;
}

void *
update_find_cb(void *p, va_list ap)
{
	update_info_t *ui = (update_info_t *) p;
	const char *s = va_arg(ap, const char *);

	if (s != NULL && str_cmp(s, ui->name))
		return NULL;

	if (ui->fun == NULL) {
		log(LOG_INFO, "update_check: NULL update fun for '%s'", s);
		return NULL;
	}

	if (ui->iter == NULL)
		((update_fun)ui->fun)();
	else
		vo_foreach(NULL, ui->iter, ui->fun);

	if (!IS_NULLSTR(ui->notify))
		wiznet(ui->notify, NULL, NULL, WIZ_TICKS, 0, 0);
	return ui;
}

void *
update_reset_cb(void *p, va_list ap)
{
	update_info_t *ui = (update_info_t *) p;
	const char *s = va_arg(ap, const char *);

	if (s != NULL && str_cmp(s, ui->name))
		return NULL;

	ui->cnt = ui->max;
	return ui;
}

void *
update_get_cb(void *p, va_list ap)
{
	update_info_t *ui = (update_info_t *) p;
	const char *s = va_arg(ap, const char *);

	if (s != NULL && str_cmp(s, ui->name))
		return NULL;

	return &(ui->max);
}

void update_handler(void)
{
	varr_foreach(&updates, update_check_cb);
}

void *update_one(const char *what)
{
	return varr_foreach(&updates, update_find_cb, what);
}

void update_reset(const char *what)
{
	varr_foreach(&updates, update_reset_cb, what);
}

int get_pulse(const char *what)
{
	void *rv = varr_foreach(&updates, update_get_cb, what);
	if (!rv) {
		log(LOG_ERROR, "get_pulse: unknown pulse type '%s'", what);
		return 0;
	}
	return *((int*)rv);
}
