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
 * $Id: events.c,v 1.17 2004-06-28 19:21:01 tatyana Exp $
 */

#include <stdio.h>
#include <dlfcn.h>

#include <merc.h>
#include <module.h>

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);

MODINIT_FUN(_module_load, m)
{
	skill_t *sk;

	C_FOREACH (skill_t *, sk, &skills) {
		evf_t *evf;

		C_FOREACH (evf_t *, evf, &sk->events) {
			evf->fun = dlsym(m->dlh, evf->fun_name);
			if (evf->fun == NULL) {
				printlog(LOG_INFO,
				    "_module_load(events): skill %s: %s",
				    gmlstr_mval(&sk->sk_name), dlerror());
			}
		}
	}
	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	skill_t *sk;

	C_FOREACH (skill_t *, sk, &skills) {
		evf_t *evf;

		C_FOREACH (evf_t *, evf, &sk->events)
			evf->fun = NULL;
	}
	return 0;
}
