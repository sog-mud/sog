/*
 * $Id: ban.c,v 1.52 2003-04-17 05:58:58 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if	!defined (WIN32)
#include <unistd.h>
#endif
#include <stdio.h>

#if defined(BSD44)
#	include <fnmatch.h>
#else
#	include <compat/fnmatch.h>
#endif

#include <merc.h>

#include <sog.h>

#include "comm.h"
#include "handler_impl.h"

static const char *format_ban(ban_t *);
static void save_bans(void);

/*
 * ban_add must work properly if ch == NULL
 */

#define BAN_ERROR(ch, arg)			\
{						\
	if (ch)					\
		dofun("help", ch, "'WIZ BAN'");	\
	else					\
		log arg;			\
}

void
ban_add(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	int ban_num;
	int ban_action;
	int ban_class;

	ban_t *b_prev;
	ban_t *b;
	ban_t *bnew;

	/*
	 * parse arguments in form "num action class mask"
	 */
	argument = one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg)) {
		BAN_ERROR(ch, (LOG_WARN, "ban: 'num' argument must be an integer"));	// notrans
		return;
	}
	ban_num = atoi(arg);

	argument = one_argument(argument, arg, sizeof(arg));
	if ((ban_action = flag_value(ban_actions, arg)) < 0) {
		BAN_ERROR(ch, (LOG_WARN, "ban: %s: unknown ban action", arg)); // notrans
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if ((ban_class = flag_value(ban_classes, arg)) < 0) {
		BAN_ERROR(ch, (LOG_WARN, "ban: %s: unknown ban class", arg)); // notrans
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		BAN_ERROR(ch, (LOG_WARN, "ban: no ban mask specified")); // notrans
		return;
	}

	b_prev = NULL;
	for (b = ban_list; b; b_prev = b, b = b->next) {
		if (b->ban_num == ban_num) {
			if (ch) {
				act_puts("ban: rule $j already exists.",
					 ch, (const void *) ban_num, NULL,
					 TO_CHAR | ACT_NOUCASE, POS_DEAD);
			} else
				log(LOG_INFO, "ban: rule %d already exists.\n", ban_num);
			return;
		}
		if (b->ban_num > ban_num)
			break;
	}

	bnew = malloc(sizeof(ban_t));
	bnew->ban_num = ban_num;
	bnew->ban_action = ban_action;
	bnew->ban_class = ban_class;
	bnew->ban_mask = str_dup(arg);

	if (!b_prev) {
		bnew->next = ban_list;
		ban_list = bnew;
	} else {
		bnew->next = b;
		b_prev->next = bnew;
	}

	if (ch) {
		act_char("ban: rule added.", ch);
		log(LOG_INFO, "Log %s: ban add %s", ch->name, format_ban(bnew));
		save_bans();
	}
}

void
ban_delete(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int ban_num;

	ban_t *prev;
	ban_t *curr;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg)) {
		dofun("help", ch, "'WIZ BAN'");
		return;
	}

	ban_num = atoi(arg);
	prev = NULL;
	for (curr = ban_list; curr; prev = curr, curr = curr->next)
		if (curr->ban_num == ban_num)
			break;

	if (!curr) {
		act_puts("ban: rule $j not found.",
			 ch, (const void *) ban_num, NULL,
			 TO_CHAR | ACT_NOUCASE, POS_DEAD);
		return;
	}

	if (prev == NULL)
		ban_list = ban_list->next;
	else
		prev->next = curr->next;

	free_string(curr->ban_mask);
	free(curr);

	act_puts("ban: rule $j deleted.",
		 ch, (const void *) ban_num, NULL,
		 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	log(LOG_INFO, "Log %s: ban delete %d", ch->name, ban_num);
	save_bans();
}

int
check_ban(DESCRIPTOR_DATA *d, int ban_class)
{
	ban_t *pban;
	int ban_action = BA_ALLOW;

	const char *m1 = str_printf("%s@%s", d->character->name, d->host);
	const char *m2 = str_printf("%s@%s", d->character->name, d->ip);

	for (pban = ban_list; pban; pban = pban->next) {
		if (pban->ban_class != ban_class)
			continue;

		if (!fnmatch(pban->ban_mask, m1, FNM_CASEFOLD)
		||  !fnmatch(pban->ban_mask, m2, FNM_CASEFOLD)) {
			ban_action = pban->ban_action;
			break;
		}
	}

	free_string(m1);
	free_string(m2);

	if (ban_action == BA_DENY) {
		write_to_buffer(d, "You are banned from this mud.\n\r", 0);
		close_descriptor(d, SAVE_F_NONE);
	}

	return ban_action;
}

void
dump_bans(BUFFER *output)
{
	ban_t *pban;

	if (ban_list == NULL) {
		buf_append(output, "No ban rules defined.\n");
		return;
	}

	buf_append(output, "Ban rules:\n");

	for (pban = ban_list; pban != NULL; pban = pban->next)
		buf_printf(output, BUF_END, "%s\n", format_ban(pban));
}

/*--------------------------------------------------------------------
 * semi-locals
 */

void
load_bans(void)
{
	FILE *fp;
	char buf[MAX_INPUT_LENGTH];

	if (!dfexist(ETC_PATH, BAN_FILE))
		return;

	if ((fp = dfopen(ETC_PATH, BAN_FILE, "r")) == NULL)
		return;

	while (fgets(buf, sizeof(buf), fp)) {
		char arg[MAX_INPUT_LENGTH];
		char *p;
		const char *argument = buf;

		if ((p = strchr(buf, '\n'))) 
			*p = '\0';

		one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0')
			continue;

		ban_add(NULL, buf);
	}

	fclose(fp);
}

/*--------------------------------------------------------------------
 * local functions
 */

static void
save_bans(void)
{
	ban_t *pban;
	FILE *fp;

	if (!ban_list) {
		dunlink(ETC_PATH, BAN_FILE);
		return;
	}

	if ((fp = dfopen(ETC_PATH, BAN_FILE, "w")) == NULL)
		return;

	for (pban = ban_list; pban; pban = pban->next)
		fprintf(fp,"%s\n", format_ban(pban));

	fclose(fp);
}

static const char *
format_ban(ban_t *pban)
{
	static char buf[MAX_STRING_LENGTH];

	snprintf(buf, sizeof(buf), "%9d %5s %7s %s",		// notrans
		 pban->ban_num,
		 flag_string(ban_actions, pban->ban_action),
		 flag_string(ban_classes, pban->ban_class),
		 pban->ban_mask);
	return buf;
}
