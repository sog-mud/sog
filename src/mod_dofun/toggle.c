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
 * $Id: toggle.c,v 1.1.2.1 2002-11-23 18:54:05 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

#include "toggle.h"

static toggle_t *toggle_lookup(toggle_t *tbl, const char *name);
static flag64_t* toggle_bits(CHAR_DATA *ch, toggle_t *t, bool *wide);

void
toggle(CHAR_DATA *ch, const char *argument, toggle_t *tbl)
{
	toggle_t *t;
	char arg[MAX_INPUT_LENGTH];
	flag64_t* bits;
	bool wide;

	argument = one_argument(argument, arg, sizeof(arg));
	if ((t = toggle_lookup(tbl, arg)) == NULL
	||  (bits = toggle_bits(ch, t, &wide)) == NULL) {
		act_puts("$t: no such flag.",
			 ch, arg, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (!str_cmp(arg, "on")) {
		if (wide)
			SET_BIT(*bits, t->bit);
		else
			SET_BIT(*(flag32_t *) bits, t->bit);
	} else if (!str_cmp(arg, "off")) {
		if (wide)
			REMOVE_BIT(*bits, t->bit);
		else
			REMOVE_BIT(*(flag32_t *) bits, t->bit);
	} else {
		if (wide)
			TOGGLE_BIT(*bits, t->bit);
		else
			TOGGLE_BIT(*(flag32_t *) bits, t->bit);
	}

	act_char((wide ? IS_SET(*bits, t->bit) :
			 IS_SET(*(flag32_t *) bits, t->bit)) ?
	    t->msg_on : t->msg_off, ch);
}

void
print_toggles(CHAR_DATA *ch, toggle_t *t)
{
	for (; t->name != NULL; t++) {
		char buf[MAX_STRING_LENGTH];
		flag64_t *bits;
		bool wide;

		if ((bits = toggle_bits(ch, t, &wide)) == 0)
			return;

		snprintf(buf, sizeof(buf),
		     "  %-11.11s - %-3.3s ($t)",		// notrans
		     t->name,
		     (wide ? IS_SET(*bits, t->bit) :
			    IS_SET(*(flag32_t *) bits, t->bit)) ? "ON" : "OFF");
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

static flag64_t*
toggle_bits(CHAR_DATA *ch, toggle_t *t, bool *wide)
{
	if (t->f == comm_flags) {
		*wide = TRUE;
		return &ch->comm;
	}
	if (t->f == plr_flags && !IS_NPC(ch)) {
		*wide = FALSE;
		return (flag64_t *) &PC(ch)->plr_flags;
	}
	return NULL;
}
