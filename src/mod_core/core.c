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
 * $Id: core.c,v 1.22 2001-08-02 18:20:01 fjoe Exp $
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <merc.h>
#include <module.h>

#include "core.h"

void
do_modules(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'WIZ MODULES'");
		return;
	}

	if (!str_prefix(arg, "reload")
	||  !str_prefix(arg, "load")) {
		module_t *m;
		time_t curr_time;

		one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0') {
			do_modules(ch, str_empty);
			return;
		}

		if ((m = mod_lookup(arg)) == NULL) {
			act_puts("$t: unknown module name.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOUCASE | ACT_NOTRANS, POS_DEAD);
			return;
		}

		log(LOG_INFO, "do_modules: reloading module '%s'", m->name);
		act_puts("Reloading module '$t'.",
			 ch, m->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);

		log_setchar(ch);
		time(&curr_time);
		if (!mod_load(m, curr_time))
			act_char("Ok.", ch);
		log_unsetchar();
		return;
	}

	if (!str_prefix(arg, "list")
	||  !str_prefix(arg, "status")) {
		int i;
		BUFFER *buf;

		if (modules.nused == 0) {
			act_char("No modules found.", ch);
			return;
		}

		buf = buf_new(GET_LANG(ch));
		buf_append(buf, "  Module  Prio          Load time         Deps\n");
		buf_append(buf, "--------- ---- -------------------------- -----------------------------------\n");	// notrans
		for (i = 0; i < modules.nused; i++) {
			module_t *m = VARR_GET(&modules, i);
			buf_printf(buf, BUF_END, "%9s %4d [%24s] %s\n", // notrans
				   m->name,
				   m->mod_prio,
				   m->last_reload ?
					strtime(m->last_reload) : "never",
				   m->mod_deps);
		}

		page_to_char(buf_string(buf), ch);
		buf_free(buf);

		return;
	}

	do_modules(ch, str_empty);
}

void
do_shutdow(CHAR_DATA *ch, const char *argument)
{
	act_char("If you want to SHUTDOWN, spell it out.", ch);
}

void
do_shutdown(CHAR_DATA *ch, const char *argument)
{
	bool active;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'WIZ SHUTDOWN'");
		return;
	}

	active = dfexist(TMP_PATH, SHUTDOWN_FILE);

	if (!str_prefix(arg, "status")) {
		act_puts(active ? "Shutdown status: active" :
				  "Shutdown status: inactive",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (!str_prefix(arg, "activate")) {
		if (!active) {
			FILE *fp = dfopen(TMP_PATH, SHUTDOWN_FILE, "w");
			if (!fp) {
				act_puts("Error: $t",
					 ch, strerror(errno), NULL,
					 TO_CHAR | ACT_NOTRANS, POS_DEAD);
				return;
			}
			fclose(fp);
			wiznet("$N has activated shutdown.", ch, NULL, 0, 0, 0);
			act_char("Shutdown activated.", ch);
		}
		else
			act_char("Shutdown already activated.", ch);
		return;
	}

	if (!str_prefix(arg, "deactivate") || !str_prefix(arg, "cancel")) {
		if (!active)
			act_char("Shutdown already inactive.", ch);
		else {
			if (dunlink(TMP_PATH, SHUTDOWN_FILE) < 0) {
				act_puts("Error: $t",
					 ch, strerror(errno), NULL,
					 TO_CHAR | ACT_NOTRANS, POS_DEAD);
				return;
			}
			wiznet("$N has deactivated shutdown.",
				ch, NULL, 0, 0, 0);
			act_char("Shutdown deactivated.", ch);
		}
		return;
	}

	do_shutdown(ch, str_empty);
}

void
do_reboo(CHAR_DATA *ch, const char *argument)
{
	act_char("If you want to REBOOT, spell it out.", ch);
}

void
do_reboot(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		dofun("help", ch, "'WIZ REBOOT'");
		return;
	}

	if (is_name(arg, "cancel")) {
		reboot_counter = -1;
		act_char("Reboot canceled.", ch);
		return;
	}

	if (is_name(arg, "now")) {
		rebooter = 1;
		reboot_mud();
		return;
	}

	if (is_name(arg, "status")) {
		if (reboot_counter == -1)
			act_char("Automatic rebooting is inactive.", ch);
		else {
			act_puts("Reboot in $j $qj{minutes}.",
				 ch, (const void *) reboot_counter, NULL,
				 TO_CHAR, POS_DEAD);
		}
		return;
	}

	if (is_number(arg)) {
		reboot_counter = atoi(arg);
		rebooter = 1;
		act_puts("SoG will reboot in $j $qj{ticks}.",
			 ch, (const void *) reboot_counter, NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	do_reboot(ch, "");
}

void
reboot_mud(void)
{
	DESCRIPTOR_DATA *d,*d_next;

	log(LOG_INFO, "Rebooting SoG");
	for (d = descriptor_list; d != NULL; d = d_next) {
		d_next = d->next;
		write_to_buffer(d,"SoG is going down for rebooting NOW!\n\r",0);
		close_descriptor(d, SAVE_F_REBOOT);
	}

	/*
	 * activate eqcheck on next boot
	 */
	if (!rebooter) {
		FILE *fp = dfopen(TMP_PATH, EQCHECK_FILE, "w");
		if (!fp) {
			log(LOG_ERROR,
			    "reboot_mud: unable to activate eqcheck");  // notrans
		} else {
			log(LOG_INFO, "reboot_mud: eqcheck activated"); // notrans
			fclose(fp);
		}
	}

	merc_down = TRUE;
}
