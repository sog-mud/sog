/*-
 * Copyright (c) 2002 SoG Development Team
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
 * $Id: toggle.c,v 1.1 2002-11-23 18:02:31 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>
#include <sog.h>

#include "toggle.h"

static toggle_t *toggle_lookup(toggle_t *tbl, const char *name);
static flag_t* toggle_bits(CHAR_DATA *ch, toggle_t *t);
static bool toggle_enabled(CHAR_DATA *ch, toggle_t *t);

void
toggle(CHAR_DATA *ch, const char *argument, toggle_t *tbl)
{
	toggle_t *t;
	char arg[MAX_INPUT_LENGTH];
	flag_t* bits;

	argument = one_argument(argument, arg, sizeof(arg));
	if ((t = toggle_lookup(tbl, arg)) == NULL
	||  (bits = toggle_bits(ch, t)) == NULL) {
		act_puts("$t: no such flag.",
			 ch, arg, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (!str_cmp(arg, "on"))
		SET_BIT(*bits, t->bit);
	else if (!str_cmp(arg, "off"))
		REMOVE_BIT(*bits, t->bit);
	else
		TOGGLE_BIT(*bits, t->bit);

	act_char(IS_SET(*bits, t->bit) ? t->msg_on : t->msg_off, ch);
}

void
print_toggles(CHAR_DATA *ch, toggle_t *t)
{
	for (; t->name != NULL; t++) {
		char buf[MAX_STRING_LENGTH];
		flag_t *bits;

		if ((bits = toggle_bits(ch, t)) == 0)
			return;

		snprintf(buf, sizeof(buf),
		     "  %-11.11s - %-3.3s ($t)",		// notrans
		     t->name, IS_SET(*bits, t->bit) ? "ON" : "OFF");
		act_puts(buf, ch, t->desc, NULL, TO_CHAR, POS_DEAD);
	}
}

static toggle_t *
toggle_lookup(toggle_t *tbl, const char *name)
{
	for (; tbl->name != NULL; tbl++) {
		if (!str_prefix(name, tbl->name))
			return tbl;
	}

	return NULL;
}

static flag_t*
toggle_bits(CHAR_DATA *ch, toggle_t *t)
{
	if (!toggle_enabled(ch, t))
		return NULL;

	if (t->f == comm_flags)
		return &ch->comm;
	if (t->f == olc_flags && ch->desc)
		return &ch->desc->dvdata->olc_flags;
	if (t->f == plr_flags && !IS_NPC(ch))
		return &PC(ch)->plr_flags;
	return NULL;
}

static bool
toggle_enabled(CHAR_DATA *ch, toggle_t *t)
{
	if (t->cmds == NULL)
		return TRUE;

	if (!IS_NPC(ch)) {
		char cmdname[MAX_STRING_LENGTH];
		const char *str = t->cmds;
		PC_DATA *pc = PC(ch);

		for (;;) {
			str = one_argument(str, cmdname, sizeof(cmdname));

			if (cmdname[0] == '\0')
				break;
			if (is_name_strict(cmdname, pc->granted))
				return TRUE;
		}
	}

	return FALSE;
}
