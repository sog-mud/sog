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
 * $Id: core.c,v 1.8 1999-12-16 12:24:41 fjoe Exp $
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "db.h"
#include "module.h"

int _module_load(module_t *m)
{
	varr_foreach(&commands, cmd_load_cb, CC_CORE, m);
	return 0;
}

int _module_unload(module_t *m)
{
	wizlog("_module_unload: core dofuns module could not be unloaded");
	return -1;
}

void do_modules(CHAR_DATA *ch, const char *argument)
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

		one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0') {
			do_modules(ch, str_empty);
			return;
		}

		if ((m = mod_lookup(arg)) == NULL) {
			char_printf(ch, "%s: unknown module name.\n",
				    arg);
			return;
		}

		if (mod_load(m) == 0)
			char_puts("Ok.\n", ch);

		return;
	}

	if (!str_prefix(arg, "list")
	||  !str_prefix(arg, "status")) {
		int i;

		if (modules.nused == 0) {
			char_puts("No modules found.\n", ch);
			return;
		}

		for (i = 0; i < modules.nused; i++) {
			module_t *m = VARR_GET(&modules, i);
			char_printf(ch, "Module: %s, Loaded: %s\n",
				    m->name, m->load_time ?
					strtime(m->load_time) : "never");
		}
		return;
	}

	do_modules(ch, str_empty);
}

void do_shutdow(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to SHUTDOWN, spell it out.\n", ch);
}

void do_shutdown(CHAR_DATA *ch, const char *argument)
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
		char_printf(ch, "Shutdown status: %s\n",
			    active ? "active" : "inactive");
		return;
	}

	if (!str_prefix(arg, "activate")) {
		if (!active) {
			FILE *fp = dfopen(TMP_PATH, SHUTDOWN_FILE, "w");
			if (!fp) {
				char_printf(ch, "Error: %s.\n",
					    strerror(errno));
				return;
			}
			fclose(fp);
			wiznet("$N has activated shutdown", ch, NULL, 0, 0, 0);
			char_puts("Shutdown activated.\n", ch);
		}
		else
			char_puts("Shutdown already activated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "deactivate") || !str_prefix(arg, "cancel")) {
		if (!active)
			char_puts("Shutdown already inactive.\n", ch);
		else {
			if (dunlink(TMP_PATH, SHUTDOWN_FILE) < 0) {
				char_printf(ch, "Error: %s.\n",
					    strerror(errno));
				return;
			}
			wiznet("$N has deactivated shutdown",
				ch, NULL, 0, 0, 0);
			char_puts("Shutdown deactivated.\n", ch);
		}
		return;
	}

	do_shutdown(ch, str_empty);
}

void do_reboo(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to REBOOT, spell it out.\n", ch);
}

void do_reboot(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));    

	if (arg[0] == '\0') {
		dofun("help", ch, "'WIZ REBOOT'");
		return;
	}

	if (is_name(arg, "cancel")) {
		reboot_counter = -1;
		char_puts("Reboot canceled.\n", ch);
		return;
	}

	if (is_name(arg, "now")) {
		rebooter = 1;
		reboot_mud();
		return;
	}

	if (is_name(arg, "status")) {
		if (reboot_counter == -1) 
			char_printf(ch, "Automatic rebooting is inactive.\n");
		else
			char_printf(ch, "Reboot in %i minutes.\n",
				    reboot_counter);
		return;
	}

	if (is_number(arg)) {
		reboot_counter = atoi(arg);
		rebooter = 1;
		char_printf(ch, "SoG will reboot in %i ticks.\n",
			    reboot_counter);
		return;
	}

	do_reboot(ch, "");   
}

