/*-
 * Copyright (c) 1998, 99 SoG Development Team
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
 * 3. Follow all guidelines in the diku license (contained in the file
 *    license.doc)
 * 4. Follow all guidelines in the Merc license (contained in the file
 *    license.txt)
 * 5. Follow all guidelines in the ROM license (contained in the file
 *    rom.license)
 * 6. Follow all guidelines in the Anatolia license (contained in the file
 *    anatolia.license)
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
 * $Id: act_update.c,v 1.15 2001-12-03 22:28:44 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>

#include <sog.h>
#include <update.h>
#include "update_impl.h"

DECLARE_DO_FUN(do_settick);
DECLARE_DO_FUN(do_tick);

DO_FUN(do_settick, ch, argument)
{
	uhandler_t *hdlr;
	char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	int val;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		BUFFER *buf;

		buf = buf_new(0);
		buf_append(buf, "    Name       Module     Iterator    Max   Cur     Function\n");	// notrans
		buf_append(buf, "----------- ----------- ------------ ----- ----- ---------------\n");	// notrans
		C_FOREACH(hdlr, &uhandlers) {
			buf_printf(buf, BUF_END,
			    "[%9s] [%9s] [%10s] %5d %5d %c%s\n", // notrans
			    hdlr->name,
			    flag_string(module_names, hdlr->mod),
			    hdlr->iter != NULL ?
			        flag_string(iterator_names, (flag_t) hdlr->iter) : "none",
			    hdlr->ticks,
			    hdlr->cnt,
			    hdlr->fun != NULL ? ' ' : '*',
			    hdlr->fun_name);
		}
		page_to_char(buf_string(buf), ch);
		buf_free(buf);
		return;
	}

	one_argument(argument, arg2, sizeof(arg2));
	val = atoi(arg2);
	if (!val) {
		act_char("Non-zero, please.", ch);
		return;
	}

	C_FOREACH(hdlr, &uhandlers) {
		if (!str_cmp(arg, hdlr->name)) {
			hdlr->ticks = val;
			return;
		}
	}

	dofun("help", ch, "'WIZ SETTICK'");
}

DO_FUN(do_tick, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')  {
		dofun("help", ch, "'WIZ TICK'");
		return;
	}

	if (update_one(arg))
		return;
	do_tick(ch, str_empty);
}
