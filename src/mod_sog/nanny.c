/*
 * $Id: nanny.c,v 1.2 2001-08-20 16:47:44 fjoe Exp $
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
 *  Merc Diku vMud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <unistd.h>
#include <stdio.h>

#include <merc.h>
#include <resolver.h>

#include <sog.h>
#include <quest.h>

#include "handler_impl.h"
#include "comm.h"

static bool	check_reconnect(DESCRIPTOR_DATA *d, bool fConn);
static bool	check_playing(DESCRIPTOR_DATA *d, const char *name);

static bool	class_ok(CHAR_DATA *ch , class_t *cl);
static void	print_cb(const char *s, CHAR_DATA *ch, int *pcol);
static void *	print_class_cb(void *p, va_list ap);
static void *	print_race_cb(void *p, va_list ap);

static int	align_restrict(CHAR_DATA *ch);
static int	ethos_check(CHAR_DATA *ch);
static void	adjust_hmv(CHAR_DATA *ch, int percent);
static void	print_hometown(CHAR_DATA *ch);

#ifdef NO_PLAYING_TWICE
static int	search_sockets(DESCRIPTOR_DATA *d);
#endif

#define OBJ_VNUM_MAP			3162
#define OBJ_VNUM_NMAP1			3385
#define OBJ_VNUM_NMAP2			3386

#define ROOM_VNUM_CHAT			1200
#define ROOM_VNUM_SCHOOL		3700

static const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
static const char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };

static int iNumPlayers = 0;

/*
 * Deal with sockets that haven't logged in yet.
 */
void
nanny(DESCRIPTOR_DATA *d, const char *argument)
{
	DESCRIPTOR_DATA *d_old, *d_next;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ch, *pet;
	char *pwdnew;
	int i;
	int nextquest = 0;
	int size;
	race_t *r;
	class_t *cl;
	int col;

	while (isspace(*argument))
		argument++;

	ch = d->character;

	switch (d->connected) {
	default:
		log(LOG_BUG, "Nanny: bad d->connected %d.", d->connected);
		close_descriptor(d, SAVE_F_NONE);
		return;

	case CON_GET_CODEPAGE: {
		size_t num;

		if (argument[0] == '\0') {
			close_descriptor(d, SAVE_F_NONE);
			return;
		}

		if (argument[0] < '1'
		|| (num = argument[0] - '1') >= codepages_sz) {
			charset_print(d);
			break;
		}

		d->codepage = num;
		d->connected = CON_GET_NAME;
		write_to_buffer(d, "By which name do you wish to be known? ", 0);
		break;
	}

	case CON_GET_NAME:
		if (argument[0] == '\0') {
			close_descriptor(d, SAVE_F_NONE);
			return;
		}

		if (!pc_name_ok(argument)) {
			write_to_buffer(d, "Illegal name, try another.\n\r"
					   "Name: ", 0);
			return;
		}

		ch = char_load(argument, 0);
		d->character = ch;
		ch->desc = d;
		dvdata_free(d->dvdata);
		d->dvdata = dvdata_dup(PC(ch)->dvdata);

		if (d->host == NULL) {
			struct sockaddr_in sock;

			size = sizeof(sock);
			if (getpeername(d->descriptor,
					(struct sockaddr *) &sock, &size) < 0) {
				d->ip = str_dup("(unknown)");	// notrans
				d->host = str_qdup(d->ip);
			} else {
				d->ip = str_dup(inet_ntoa(sock.sin_addr));
#if defined (WIN32)
				printf("%s@%s\n",		// notrans
#else
				fprintf(rfout, "%s@%s\n",	// notrans
#endif
				    ch->name, d->ip);
				d->connected = CON_RESOLV;
/* wait until sock.sin_addr gets resolved */
				break;
			}
		}

		/* FALLTHRU */

	case CON_RESOLV:
		if (d->host == NULL)
			break;

		/*
		 * Swiftest: I added the following to ban sites.  I don't
		 * endorse banning of sites, but Copper has few descriptors now
		 * and some people from certain sites keep abusing access by
		 * using automated 'autodialers' and leaving connections
		 * hanging.
		 *
		 * Furey: added suffix check by request of Nickel of
		 *	HiddenWorlds.
		 * fjoe: replaced suffix/prefix checks with fnmatch check
		 */
		if (check_ban(d, BCL_ALL))
			return;

		if (!IS_IMMORTAL(ch)) {
			if (check_ban(d, BCL_PLAYERS))
				return;

#ifdef NO_PLAYING_TWICE
			if (search_sockets(d)) {
				act_char("Playing twice is restricted...", ch);
				close_descriptor(d, SAVE_F_NONE);
				return;
			}
#endif
			if (iNumPlayers > MAX_OLDIES
			&&  !IS_SET(PC(ch)->plr_flags, PLR_NEW)) {
				act_puts3("\nThere are currently $j "
					  "$qj{players} mudding out "
					  "of a maximum of $J.",
					   ch, (const void*)(iNumPlayers - 1),
					   NULL, (const void*) MAX_OLDIES,
					   TO_CHAR, POS_DEAD);
				act_puts("Please try again soon.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				close_descriptor(d, SAVE_F_NONE);
				return;
			}

			if (iNumPlayers > MAX_NEWBIES
			&&  IS_SET(PC(ch)->plr_flags, PLR_NEW)) {
				act_puts("\nThere are currently $j players "
					 "mudding.\n"
					 "New player creation is limited to "
					 "when there are",
					 ch, (const void*)(iNumPlayers - 1),
					 NULL, TO_CHAR, POS_DEAD);
				act_puts("less than $j players. Please try "
					 "again soon.",
					 ch, (const void*) MAX_NEWBIES, NULL,
					 TO_CHAR | ACT_NOUCASE, POS_DEAD);
				close_descriptor(d, SAVE_F_NONE);
				return;
			}
		}

		if (check_reconnect(d, FALSE))
			REMOVE_BIT(PC(ch)->plr_flags, PLR_NEW);
		else if (wizlock && !IS_HERO(ch)) {
			act_char("The game is wizlocked.", ch);
			close_descriptor(d, SAVE_F_NONE);
			return;
		}

		if (!IS_SET(PC(ch)->plr_flags, PLR_NEW)) {
			/* Old player */
			write_to_descriptor(d->descriptor, echo_off_str, 0);
			act_puts("Password: ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		} else {
			/* New player */
			if (newlock) {
				act_char("The game is newlocked.", ch);
				close_descriptor(d, SAVE_F_NONE);
				return;
			}

			if (check_ban(d, BCL_NEWBIES))
				return;

			dofun("help", ch, "NAME");
			act_puts("Do you accept? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			d->connected = CON_CONFIRM_NEW_NAME;
			return;
		}
		break;

/* RT code for breaking link */
	case CON_BREAK_CONNECT:
		switch(*argument) {
		case 'y' : case 'Y':
			for (d_old = descriptor_list; d_old; d_old = d_next) {
				CHAR_DATA *rch;

				d_next = d_old->next;
				if (d_old == d || d_old->character == NULL)
					continue;

				rch = d_old->original ? d_old->original :
							d_old->character;
				if (str_cmp(ch->name, rch->name))
					continue;

				if (d_old->original) {
					dofun("return", d_old->character,
					      str_empty);
				}
				close_descriptor(d_old, SAVE_F_NORMAL);
			}

			if (check_reconnect(d, TRUE))
				return;
			write_to_buffer(d, "Reconnect attempt failed.\n\r", 0);

			/* FALLTHRU */

		case 'n' : case 'N':
			write_to_buffer(d, "Name: ", 0);
			if (d->character != NULL) {
				char_nuke(d->character);
				d->character = NULL;
			}
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer(d, "Please type Y or N? ", 0);
			break;
		}
		break;

	case CON_CONFIRM_NEW_NAME:
		switch (*argument) {
		case 'y': case 'Y':
			act_char("New character.", ch);
			act_puts("Give me a password for $n: ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			write_to_descriptor(d->descriptor, echo_off_str, 0);
			d->connected = CON_GET_NEW_PASSWORD;
			break;

		case 'n': case 'N':
			act_puts("Ok, what IS it, then? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			char_nuke(d->character);
			d->character = NULL;
			d->connected = CON_GET_NAME;
			break;

		default:
			act_puts("Please type Yes or No? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			break;
		}
		break;

	case CON_GET_NEW_PASSWORD:
		send_to_char("\n", ch);
		if (strlen(argument) < 5) {
			act_char("Password must be at least five characters long.", ch);
			act_puts("Password: ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			return;
		}

		pwdnew = crypt(argument, ch->name);
		free_string(PC(ch)->pwd);
		PC(ch)->pwd	= str_dup(pwdnew);
		act_puts("Please retype password: ",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		d->connected = CON_CONFIRM_NEW_PASSWORD;
		break;

	case CON_CONFIRM_NEW_PASSWORD:
		send_to_char("\n", ch);
		if (strcmp(crypt(argument, PC(ch)->pwd), PC(ch)->pwd)) {
			act_char("Passwords don't match.", ch);
			act_puts("\nRetype password: ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			d->connected = CON_GET_NEW_PASSWORD;
			return;
		}

		write_to_descriptor(d->descriptor, echo_on_str, 0);
		send_to_char("\n", ch);
		dofun("help", ch, "RACETABLE");
		act_puts("What is your race ('help <race>' for more information)? ",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		d->connected = CON_GET_NEW_RACE;
		break;

	case CON_GET_NEW_RACE:
		one_argument(argument, arg, sizeof(arg));

		if (!str_cmp(arg, "help")) {
			argument = one_argument(argument, arg, sizeof(arg));
			send_to_char("\n", ch);
			if (argument[0] == '\0')
				dofun("help", ch, "RACETABLE");
			else
				dofun("help", ch, argument);
			act_puts("What is your race ('help <race>' for more information)? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			break;
		}

		r = race_search(argument);
		if (r == NULL
		||  !r->race_pcdata
		||  r->race_pcdata->classes.nused == 0) {
			act_char("That is not a valid race.", ch);
			act_char("The following races are available:", ch);
			send_to_char("  ", ch);			// notrans
			col = 0;
			hash_foreach(&races, print_race_cb, ch, &col);
			send_to_char("\n", ch);
			act_puts("What is your race ('help <race>' for more information)? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			break;
		}

		free_string(PC(ch)->race);
		PC(ch)->race = str_qdup(r->name);
		free_string(ch->race);
		ch->race = str_qdup(r->name);

		/* Add race modifiers */
		SET_HIT(ch, ch->perm_hit + r->race_pcdata->hp_bonus);
		SET_MANA(ch, ch->perm_mana + r->race_pcdata->mana_bonus);
		PC(ch)->practice = r->race_pcdata->prac_bonus;
		for (i = 0; i < MAX_STAT; i++)
			ch->perm_stat[i] += r->race_pcdata->mod_stat[i];
		race_resetstats(ch);

		act_puts("What is your sex (M/F)? ",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		d->connected = CON_GET_NEW_SEX;
		break;

	case CON_GET_NEW_SEX:
		switch (argument[0]) {
		case 'm': case 'M':
			mlstr_init2(&ch->gender, flag_string(gender_table, SEX_MALE));
			break;
		case 'f': case 'F':
			mlstr_init2(&ch->gender, flag_string(gender_table, SEX_FEMALE));
			break;
		default:
			act_char("That's not a sex.", ch);
			act_puts("What IS your sex? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			return;
		}

		dofun("help", ch, "'CLASS HELP'");

		act_char("The following classes are available:", ch);
		col = 0;
		hash_foreach(&classes, print_class_cb, ch, &col);
		send_to_char("\n", ch);
		act_puts("What is your class ('help <class>' for more information)? ",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		d->connected = CON_GET_NEW_CLASS;
		break;

	case CON_GET_NEW_CLASS:
		argument = one_argument(argument, arg, sizeof(arg));

		if (!str_prefix(arg, "help")) {
			if (argument[0] == '\0')
				dofun("help", ch, "'CLASS HELP'");
			else
				dofun("help", ch, argument);
			act_puts("What is your class ('help <class>' for more information)? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			return;
		}

		if ((cl = class_search(arg)) == NULL) {
			act_char("That's not a class.", ch);
			act_puts("What IS your class? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			return;
		}

		if (IS_SET(cl->class_flags, CLASS_CLOSED)) {
			act_char("This class isn't available yet.", ch);
			act_puts("What IS your class? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			return;
		}

		if (!class_ok(ch, cl)) {
			act_char("That class is not available for your race or sex.", ch);
			act_puts("Choose again: ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			return;
		}

		ch->class = str_qdup(cl->name);
		for (i = 0; i < MAX_STAT; i++)
			ch->perm_stat[i] += cl->mod_stat[i];
		act("You are now $t.", ch, cl->name, NULL, TO_CHAR);

		if (!align_restrict(ch)) {
			act_char("You may be good, neutral, or evil.", ch);
			act_puts("Which alignment (G/N/E)? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			d->connected = CON_GET_ALIGNMENT;
		} else {
			act_puts("[Hit Return to continue]",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			print_hometown(ch);
		}
		break;

	case CON_GET_ALIGNMENT:
		switch(argument[0]) {
		case 'g': case 'G':
			ch->alignment = 1000;
			break;
		case 'n': case 'N':
			ch->alignment = 0;
			break;
		case 'e': case 'E':
			ch->alignment = -1000;
			break;
		default:
			act_char("That's not a valid alignment.", ch);
			act_puts("Which alignment (G/N/E)? ",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			return;
		}
		act_puts("Now your character is $t.",
			 ch, flag_string(align_names, NALIGN(ch)), NULL,
			 TO_CHAR, POS_DEAD);
		act_puts("[Hit Return to continue]",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		print_hometown(ch);
		break;

	case CON_PICK_HOMETOWN: {
		int htn;

		if (argument[0] == '\0'
		||  (htn = htn_lookup(argument)) < 0
		||  hometown_restrict(HOMETOWN(htn), ch)) {
			act_char("That's not a valid hometown.", ch);
			print_hometown(ch);
			return;
		}

		PC(ch)->hometown = htn; 
		act_puts("\nNow your hometown is $t.",
			 ch, hometown_name(htn), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_char("[Hit Return to continue]", ch);
		ch->endur = 100;
		d->connected = CON_GET_ETHOS;
		break;
	}

	  case CON_GET_ETHOS:
		if (!ch->endur) {
			switch(argument[0]) {
			case 'H': case 'h': case '?':
				dofun("help", ch, "ALIGNMENT");
				return;
				/* NOTREACHED */

			case 'L': case 'l':
				ch->ethos = ETHOS_LAWFUL;
				break;
			case 'N': case 'n':
				ch->ethos = ETHOS_NEUTRAL;
				break;
			case 'C': case 'c':
				ch->ethos = ETHOS_CHAOTIC;
				break;
			default:
				send_to_char("\n", ch);
				act_char("That is not a valid ethos.", ch);
				act_puts("What ethos do you want, (L/N/C) (type 'help' for more info)? ",
					 ch, NULL, NULL,
					 TO_CHAR | ACT_NOLF, POS_DEAD);
				return;
			}
			act_puts("Now you are $t-$T.",
				 ch, flag_string(ethos_table, ch->ethos),
				 flag_string(align_names, NALIGN(ch)),
				 TO_CHAR, POS_DEAD);
		} else {
			ch->endur = 0;
			if (!ethos_check(ch)) {
				act_puts("What ethos do you want, (L/N/C) (type 'help' for more info)? ",
					 ch, NULL, NULL,
					 TO_CHAR | ACT_NOLF, POS_DEAD);
				d->connected = CON_GET_ETHOS;
				return;
			} else {
				ch->ethos = 1;
			}
		}
		act_puts("[Hit Return to continue]",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		d->connected = CON_CREATE_DONE;
		break;

	case CON_CREATE_DONE:
		log(LOG_INFO, "%s@%s new player.", ch->name, d->host);
		dofun("help", ch, "MOTD");
		act_puts("[Hit Return to continue]",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		d->connected = CON_READ_MOTD;
		break;

	case CON_GET_OLD_PASSWORD:
		send_to_char("\n", ch);
		if (strcmp(crypt(argument, PC(ch)->pwd), PC(ch)->pwd)) {
			act_char("Wrong password.", ch);
			log(LOG_INFO, "Wrong password by %s@%s", ch->name, d->host);
			if (ch->endur == 2)
				close_descriptor(d, SAVE_F_NONE);
			else {
				write_to_descriptor(
				    d->descriptor, echo_off_str, 0);
				act_puts("Password: ",
					 ch, NULL, NULL,
					 TO_CHAR | ACT_NOLF, POS_DEAD);
				d->connected = CON_GET_OLD_PASSWORD;
				ch->endur++;
			}
			return;
		}

		if (PC(ch)->pwd[0] == '\0') {
			act_char("Warning! Null password!", ch);
			act_char("Type 'password null <new password>' to fix.", ch);
		}

		write_to_descriptor(d->descriptor, echo_on_str, 0);

		if (check_playing(d, ch->name)
		||  check_reconnect(d, TRUE))
			return;

		log(LOG_INFO, "%s@%s has connected.", ch->name, d->host);
		d->connected = CON_READ_IMOTD;

		/* FALL THRU */

	case CON_READ_IMOTD:
		if (IS_IMMORTAL(ch))
			dofun("help", ch, "IMOTD");
		dofun("help", ch, "MOTD");
		d->connected = CON_READ_MOTD;

		/* FALLTHRU */

	case CON_READ_MOTD:
		act_char("Welcome to Shades of Gray! Enjoy!", ch);
		send_to_char("\n", ch);

		ch->next	= char_list;
		char_list	= ch;
		if (!char_list_lastpc)
			char_list_lastpc = ch;
		if ((pet = GET_PET(ch)) != NULL) {
			/* we know that char_list_lastpc isn't NULL */
			pet->next = char_list_lastpc->next;
			char_list_lastpc->next = pet;
		}
		d->connected	= CON_PLAYING;
		{
			int count;
			FILE *fp;
			int tmp = 0;
			count = 0;
			for (d = descriptor_list; d != NULL; d = d->next)
				if (d->connected == CON_PLAYING)
					count++;
			top_player = UMAX(count, top_player);
			if ((fp = dfopen(TMP_PATH, MAXON_FILE, "r"))) {
				fscanf(fp, "%d", &tmp);
				fclose(fp);
			}
			if (tmp < top_player
			&&  (fp = dfopen(TMP_PATH, MAXON_FILE, "w"))) {
				fprintf(fp, "%d", top_player);
				fclose(fp);
			}
		}

		/* quest code */
		nextquest = -abs(PC(ch)->questtime);
		quest_cancel(ch);
		PC(ch)->questtime = nextquest;
		/* !quest code */

		wiznet("{W$N{x has left real life behind.",
			ch, NULL, WIZ_LOGINS, 0, ch->level);

		for (i = 0; i < MAX_STAT; i++) {
			int max_stat = get_max_train(ch, i);

			if (ch->perm_stat[i] > max_stat) {
				PC(ch)->train +=
					ch->perm_stat[i] - max_stat;
				ch->perm_stat[i] = max_stat;
			}
		}

		if (ch->gold > 6000 && !IS_IMMORTAL(ch)) {
			int tax = (ch->gold - 6000) / 2;
			act_puts("You are taxed $j gold to pay for the Mayor's bar.",
				 ch, (const void *) tax, NULL,
				 TO_CHAR, POS_DEAD);
			ch->gold -= tax;
		}

		if (ch->level == 0) {
			OBJ_INDEX_DATA *map;
			OBJ_DATA *obj;

			ch->level = 1;
			PC(ch)->exp = 0;
			PC(ch)->train = 5;
			PC(ch)->practice += 5;
			PC(ch)->death = 0;

			spec_update(ch);

			dofun("outfit", ch, str_empty);

			if ((obj = create_obj(OBJ_VNUM_MAP, 0)) != NULL)
				obj_to_char(obj, ch);
			if ((obj = create_obj(OBJ_VNUM_NMAP1, 0)) != NULL)
				obj_to_char(obj, ch);
			if ((obj = create_obj(OBJ_VNUM_NMAP2, 0)) != NULL)
				obj_to_char(obj, ch);

			if ((map = get_map(ch)) != NULL
			&&  (obj = create_obj(map->vnum, 0)) != NULL)
				obj_to_char(obj, ch);

			if ((obj = get_eq_char(ch, WEAR_WIELD)))
				_set_skill(ch, get_weapon_sn(obj), 40, FALSE);

			dofun("help", ch, "NEWBIE INFO");
			char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
		} else {
			ROOM_INDEX_DATA *to_room;
			int logoff = PC(ch)->logoff;
			int percent;

			if (!logoff)
				logoff = current_time;

			/*
			 * adjust hp/mana/move up
			 */
			percent = (current_time - logoff) * 25 / (2 * 60 * 60);
			percent = UMIN(percent, 100);
			adjust_hmv(ch, percent);

			if (ch->in_room
			&&  (room_is_private(ch->in_room) ||
			     (!IS_NULLSTR(ch->in_room->area->clan) &&
			      !IS_CLAN(ch->in_room->area->clan, ch->clan))))
				ch->in_room = NULL;

			if (ch->in_room)
				to_room = ch->in_room;
			else if (IS_IMMORTAL(ch))
				to_room = get_room_index(ROOM_VNUM_CHAT);
			else
				to_room = get_room_index(ROOM_VNUM_TEMPLE);

			pet = GET_PET(ch);
			act("$N has entered the game.",
			    to_room->people, NULL, ch, TO_ALL);
			char_to_room(ch, to_room);

			if (pet) {
				adjust_hmv(pet, percent);
				act("$N has entered the game.",
				    to_room->people, NULL, pet, TO_ROOM);
				char_to_room(pet, to_room);
			}
		}

		if (!IS_EXTRACTED(ch)) {
			dofun("look", ch, "auto");
			dofun("unread", ch, "login");		// notrans
		}

		break;
	}
}

/*--------------------------------------------------------------------
 * local functions
 */

/*
 * look for link-dead player to reconnect.
 *
 * when fConn == FALSE then
 * simple copy password for newly [re]connected character
 * authentication
 *
 * otherwise reconnect attempt is made
 */
static bool
check_reconnect(DESCRIPTOR_DATA *d, bool fConn)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d2;

	if (!fConn) {
		for (d2 = descriptor_list; d2; d2 = d2->next) {
			if (d2 == d)
				continue;
			ch = d2->original ? d2->original : d2->character;
			if (ch && !str_cmp(d->character->name, ch->name)) {
				free_string(PC(d->character)->pwd);
				PC(d->character)->pwd = str_qdup(PC(ch)->pwd);
				return TRUE;
			}
		}
	}

	for (ch = char_list; ch && !IS_NPC(ch); ch = ch->next) {
		if ((!fConn || ch->desc == NULL)
		&&  !str_cmp(d->character->name, ch->name)) {
			if (!fConn) {
				free_string(PC(d->character)->pwd);
				PC(d->character)->pwd = str_qdup(PC(ch)->pwd);
			} else {
				char_nuke(d->character);
				d->character		= ch;
				ch->desc		= d;
				PC(ch)->idle_timer	= 0;
				dvdata_free(d->dvdata);
				d->dvdata = dvdata_dup(PC(ch)->dvdata);
				act_char("Reconnecting. Type replay to see missed tells.", ch);
				act("$n has reconnected.",
				    ch, NULL, NULL, TO_ROOM);

				log(LOG_INFO, "%s@%s reconnected.",
					   ch->name, d->host);
				wiznet("$N groks the fullness of $S link.",
				       ch, NULL, WIZ_LINKS, 0, 0);
				d->connected = CON_PLAYING;
			}
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Check if already playing.
 */
static bool
check_playing(DESCRIPTOR_DATA *d, const char *name)
{
	DESCRIPTOR_DATA *dold;

	for (dold = descriptor_list; dold; dold = dold->next) {
		if (dold != d
		&&  dold->character != NULL
		&&  dold->connected != CON_GET_CODEPAGE
		&&  dold->connected != CON_GET_NAME
		&&  dold->connected != CON_RESOLV
		&&  dold->connected != CON_GET_OLD_PASSWORD
		&&  !str_cmp(name, dold->original ?  dold->original->name :
						     dold->character->name)) {
			write_to_buffer(d, "That character is already playing.\n\r", 0);
			write_to_buffer(d, "Do you wish to connect anyway (Y/N)? ",0);
			d->connected = CON_BREAK_CONNECT;
			return TRUE;
		}
	}

	return FALSE;
}

static bool
class_ok(CHAR_DATA *ch, class_t *cl)
{
	race_t *r;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->race_pcdata)
		return FALSE;

	if (rclass_lookup(r, cl->name) == NULL
	||  is_name(mlstr_mval(&ch->gender), cl->restrict_sex))
		return FALSE;

	return TRUE;
}

static void *
print_class_cb(void *p, va_list ap)
{
	class_t *cl = (class_t *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int *pcol = va_arg(ap, int *);

	if (!class_ok(ch, cl))
		return NULL;

	print_cb(cl->name, ch, pcol);
	return NULL;
}

static void
print_cb(const char *s, CHAR_DATA *ch, int *pcol)
{
	if (*pcol > 60) {
		send_to_char("\n  ", ch);			// notrans
		*pcol = 0;
	}

	act_puts("($t) ", ch, s, NULL,				// notrans
		 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
	*pcol += strlen(s) + 3;
}

static void *
print_race_cb(void *p, va_list ap)
{
	race_t *r = (race_t *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int *pcol = va_arg(ap, int *);

        if (!r->race_pcdata
	||  r->race_pcdata->classes.nused == 0)
		return NULL;

	print_cb(r->name, ch, pcol);
	return NULL;
}

static int
align_restrict(CHAR_DATA *ch)
{
	class_t *cl;
	race_t *r;

	if ((cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->race_pcdata)
		return RA_NONE;

	if (r->race_pcdata->restrict_align == RA_GOOD
	||  cl->restrict_align == RA_GOOD) {
		act_char("Your character has good tendencies.", ch);
		ch->alignment = 1000;
		return RA_GOOD;
	}

	if (r->race_pcdata->restrict_align == RA_NEUTRAL
	||  cl->restrict_align == RA_NEUTRAL) {
		act_char("Your character has neutral tendencies.", ch);
		ch->alignment = 0;
		return RA_NEUTRAL;
	}

	if (r->race_pcdata->restrict_align == RA_EVIL
	||  cl->restrict_align == RA_EVIL) {
		act_char("Your character has evil tendencies.", ch);
		ch->alignment = -1000;
		return RA_EVIL;
	}

	return RA_NONE;
}

static int
ethos_check(CHAR_DATA *ch)
{
	class_t *cl;

	if ((cl = class_lookup(ch->class))) {
		/*
		 * temporary workaround for paladins
		 */
		if (IS_SET(cl->restrict_ethos, ETHOS_LAWFUL)) {
			act_char("You are Lawful.", ch);
			return 1;
		}
	}
	return 0;
}

static void
adjust_hmv(CHAR_DATA *ch, int percent)
{
	if (percent > 0
	&&  !IS_AFFECTED(ch, AFF_POISON | AFF_PLAGUE)) {
		ch->hit += (ch->max_hit - ch->hit) * percent / 100;
		ch->mana += (ch->max_mana - ch->mana) * percent / 100;
		ch->move += (ch->max_move - ch->move) * percent / 100;

		if (!IS_NPC(ch)) {
			PC(ch)->questtime = -abs(PC(ch)->questtime *
				(100 - UMIN(5 * percent, 100)) / 100);
		}
	}
}

static void
print_hometown(CHAR_DATA *ch)
{
	race_t *r;
	class_t *cl;
	int htn;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->race_pcdata
	||  (cl = class_lookup(ch->class)) == NULL) {
		act_char("You should create your character anew.", ch);
		close_descriptor(ch->desc, SAVE_F_NONE);
		return;
	}

	if ((htn = hometown_permanent(ch)) >= 0) {
		PC(ch)->hometown = htn;
		act_puts("\nYour hometown is $t, permanently.",
			 ch, hometown_name(htn), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("[Hit Return to continue]",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);

/* XXX */
		ch->endur = 100;
		ch->desc->connected = CON_GET_ETHOS;
		return;
	}

	send_to_char("\n", ch);
	dofun("help", ch, "'CREATECHAR HOMETOWN'");
	hometown_print_avail(ch);
	send_to_char("? ", ch);					// notrans
	ch->desc->connected = CON_PICK_HOMETOWN;
}

#ifdef NO_PLAYING_TWICE
static int
search_sockets(DESCRIPTOR_DATA *d)
{
	DESCRIPTOR_DATA *d2;

	if (IS_IMMORTAL(d->character))
		return 0;

	for (d2 = descriptor_list; d2 != NULL; d2 = d2->next) {
		if(!strcmp(d->host, d2->host)) {
			if (d2->character && d->character
			&&  !strcmp(d->character->name, d2->character->name))
				continue;
			return 1;
		}
	}
	return 0;
}
#endif
