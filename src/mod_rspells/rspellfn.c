/*
 * Copyright (c) 1999 Arborn <avn@org.chem.msu.su>
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
 * $Id: rspellfn.c,v 1.2 1999-09-25 11:29:31 fjoe Exp $
 */

#include <stdio.h>
#include <dlfcn.h>

#include "typedef.h"
#include "varr.h"
#include "log.h"
#include "raffect.h"

#include "module.h"

int _module_load(module_t* m)
{
	int rsn;

	for (rsn = 0; rsn < rspells.nused; rsn++) {
		rspell_t *rsp = RSPELL(rsn);

		if (IS_SET(rsp->events, EVENT_ENTER)) {
			rsp->enter_fun = dlsym(m->dlh, rsp->enter_fun_name);
			if (rsp->enter_fun == NULL) 
				wizlog("_module_load(rspells): %s", dlerror());
		}
		if (IS_SET(rsp->events, EVENT_LEAVE)) {
			rsp->leave_fun = dlsym(m->dlh, rsp->leave_fun_name);
			if (rsp->leave_fun == NULL) 
				wizlog("_module_load(rspells): %s", dlerror());
		}
		if (IS_SET(rsp->events, EVENT_UPDATE)) {
			rsp->update_fun = dlsym(m->dlh, rsp->update_fun_name);
			if (rsp->update_fun == NULL) 
				wizlog("_module_load(rspells): %s", dlerror());
		}
	}

	return 0;
}

int _module_unload(module_t *m)
{
	int rsn;

	for (rsn = 0; rsn < rspells.nused; rsn++) {
		rspell_t *rsp = RSPELL(rsn);

		rsp->enter_fun = NULL;
		rsp->leave_fun = NULL;
		rsp->update_fun = NULL;
	}
	return 0;
}
