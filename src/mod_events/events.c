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
 * $Id: events.c,v 1.1 1999-10-12 13:56:16 avn Exp $
 */

#include <stdio.h>
#include <dlfcn.h>

#include "const.h"
#include "typedef.h"
#include "varr.h"
#include "hash.h"
#include "log.h"
#include "skills.h"
#include "module.h"

static void *load_cb(void *p, void *d);
static void *unload_cb(void *p, void *d);

int _module_load(module_t* m)
{
	hash_foreach(&skills, load_cb, m);
	return 0;
}

int _module_unload(module_t *m)
{
	hash_foreach(&skills, unload_cb, NULL);
	return 0;
}

static void *
load_cb(void *p, void *d)
{
	skill_t *sk = (skill_t*) p;
	module_t *m = (module_t *) d;
	event_fun_t *evf;

        for (evf = sk->eventlist; evf; evf = evf->next) {
		evf->fun = dlsym(m->dlh, evf->fun_name);
		if (evf->fun == NULL)
			wizlog("_module_load(events): %s", dlerror());
	}
	
	return NULL;
}

static void *
unload_cb(void *p, void *d)
{
	skill_t *sk = (skill_t*) p;
	event_fun_t *evf;

	for (evf = sk->eventlist; evf; evf = evf->next)
		evf->fun = NULL;

	return NULL;
}
