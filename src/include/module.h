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
 * $Id: module.h,v 1.11 2001-07-08 17:18:46 fjoe Exp $
 */

#ifndef _MODULE_H_
#define _MODULE_H_

#include <module_decl.h>

struct module_t {
	const char *name;	/* module name				*/
	const char *file_name;	/* module filename			*/
	int mod_id;		/* mod id				*/
	int mod_prio;		/* module priority			*/
				/* modules with higher priority will be	*/
				/* [re]loaded first on `mod_load' (and	*/
				/* will be unloaded last)		*/
				/* load/unload order of modules with	*/
				/* equal priority undefined		*/
				/* module `a' priority is higher than	*/
				/* module `b' priority if		*/
				/* a.mod_prio > b.mod_prio		*/

	const char *mod_deps;	/* modules this mod depends on		*/

	void *dlh;		/* module handle			*/
	time_t last_reload;	/* last [re]load time			*/
};

int		mod_load	(module_t *m, time_t curr_time);
module_t *	mod_lookup	(const char *name);

extern varr modules;

#endif
