/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: act.c,v 1.15 1999-02-27 04:44:12 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "comm_colors.h"
#include "mob_prog.h"
#include "db/word.h"

/*
 * static functions declarations
 */
static char *		translate(CHAR_DATA *ch, CHAR_DATA *victim,
				  const char *i);
static CHAR_DATA *	act_args(CHAR_DATA *ch, CHAR_DATA *vch,
				 int flags, const char *format);
static bool		act_skip(CHAR_DATA *ch, CHAR_DATA *vch, CHAR_DATA *to,
				 int flags, int min_pos);
static void		act_raw(CHAR_DATA *ch, CHAR_DATA *to,
				const void *arg1,
				const void *arg2,
				const void *arg3,
				const char *str, int flags);

void act_puts3(const char *format, CHAR_DATA *ch,
	       const void *arg1, const void *arg2, const void *arg3,
	       int flags, int min_pos)
{
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;

	if ((to = act_args(ch, vch, flags, format)) == NULL)
		return;

	if (IS_SET(flags, TO_CHAR | TO_VICT)) {
		if (!act_skip(ch, vch, to, flags, min_pos)) {
			act_raw(ch, to, arg1, arg2, arg3,
				GETMSG(format, to->lang), flags);
		}
		return;
	}
		
	for(; to; to = to->next_in_room) {
		if (act_skip(ch, vch, to, flags, min_pos)) 
			continue;
		act_raw(ch, to, arg1, arg2, arg3,
			GETMSG(format, to->lang), flags);
	}
}

/*----------------------------------------------------------------------------
 * static functions
 */

/* common and slang should have the same size */
char common[] =
	"aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"
	"áÁâÂ÷×çÇäÄåÅ³£öÖúÚéÉêÊëËìÌíÍîÎïÏðÐòÒóÓôÔõÕæÆèÈãÃþÞûÛýÝÿßùÙøØüÜàÀñÑ";
char slang[] =
	"eEcCdDfFiIgGhHjJoOkKlLmMnNpPuUqQrRsStTvVyYwWxXzZaAbB"
	"õÕâÂâÂïÏâÂáÁùÙâÂâÂõÕâÂâÂâÂâÂéÉáÁâÂâÂòÒâÂùÙâÂâÂâÂâÂâÂâÂÿßõÕøØáÁõÕùÙ";

/* ch says, victim hears */
static char *translate(CHAR_DATA *ch, CHAR_DATA *victim, const char *i)
{
	static char trans[MAX_STRING_LENGTH];
	char *o;
	RACE_DATA *r;

	if (*i == '\0'
	||  (ch == NULL) || (victim == NULL)
	||  IS_NPC(ch) || IS_NPC(victim)
	||  IS_IMMORTAL(ch) || IS_IMMORTAL(victim)
	||  ch->slang == SLANG_COMMON
	||  ((r = race_lookup(ORG_RACE(victim))) &&
	     r->pcdata &&
	     ch->slang == r->pcdata->slang)) {
		if (IS_IMMORTAL(victim))
			snprintf(trans, sizeof(trans), "[%s] %s",
				 flag_string(slang_table, ch->slang), i);
		else
			strnzcpy(trans, sizeof(trans), i);
		return trans;
	}

	snprintf(trans, sizeof(trans), "[%s] ",
		 flag_string(slang_table, ch->slang));
	o = strchr(trans, '\0');
	for (; *i && o-trans < sizeof(trans)-1; i++, o++) {
		char *p = strchr(common, *i);
		*o = p ? slang[p-common] : *i;
	}
	*o = '\0';

	return trans;
}

static CHAR_DATA *act_args(CHAR_DATA *ch, CHAR_DATA *vch,
			   int flags, const char *format)
{
	if (format == NULL)
		return NULL;

	if (IS_SET(flags, TO_CHAR))
		return ch;

	if (IS_SET(flags, TO_VICT))
		return vch;

	if (!ch || !ch->in_room)
		return NULL;

	return ch->in_room->people;
}
 
static bool act_skip(CHAR_DATA *ch, CHAR_DATA *vch, CHAR_DATA *to,
		     int flags, int min_pos)
{
	if (to->position < min_pos)
		return TRUE;

	if (IS_SET(flags, TO_CHAR) && to != ch)
		return TRUE;
	if (IS_SET(flags, TO_VICT) && (to != vch || to == ch))
		return TRUE;
	if (IS_SET(flags, TO_ROOM) && to == ch)
		return TRUE;
	if (IS_SET(flags, TO_NOTVICT) && (to == ch || to == vch))
		return TRUE;

	if (IS_NPC(to)
	&&  to->desc == NULL
	&&  !HAS_TRIGGER(to, TRIG_ACT))
		return TRUE;
 
	if (IS_SET(flags, ACT_NOMORTAL) && !IS_NPC(to) && !IS_IMMORTAL(to))
		return TRUE;

/* twitlist handling */
	if (IS_SET(flags, ACT_NOTWIT)
	&&  !IS_NPC(to) && !IS_IMMORTAL(to)
	&&  !IS_NPC(ch) && !IS_IMMORTAL(ch)
	&&  is_name(ch->name, to->pcdata->twitlist))
		return TRUE;

/* check "deaf dumb blind" chars */
	if (IS_SET(flags, ACT_NODEAF) && is_affected(to, gsn_deafen))
		return TRUE;

/* skip verbose messages */
	if (IS_SET(flags, ACT_VERBOSE)
	&&  IS_SET(to->comm, COMM_NOVERBOSE))
		return TRUE;

	return FALSE;
}

static char * const he_she  [] = { "it",  "he",  "she" };
static char * const him_her [] = { "it",  "him", "her" };
static char * const his_her [] = { "its", "his", "her" };
 
struct tdata {
	char	type;
	int	arg;
	char *	p;
};

#define TSTACK_SZ 4

static int SEX(CHAR_DATA *ch, CHAR_DATA *looker)
{
	if (is_affected(ch, gsn_doppelganger)
	&&  (IS_NPC(looker) || !IS_SET(looker->plr_flags, PLR_HOLYLIGHT)))
		ch = ch->doppel;
	return URANGE(0, ch->sex, SEX_MAX-1);
}

/*
 * vch is (CHAR_DATA*) arg2
 * vch1 is (CHAR_DATA*) arg1
 * obj1 is (OBJ_DATA*) arg1
 * obj2 is (OBJ_DATA*) arg2
 *
 * Known act_xxx format codes are:
 *
 * a
 * A
 * b
 * B
 * c - $cn{...} - case number ``n''
 * C
 * d - door name (arg2)
 * D
 * e - he_she(ch)
 * E - he_she(vch)
 * f
 * F
 * g - $gx{...} - gender form depending on sex of ``x'', where x is:
 *	c - char
 *	v - vch ((CHAR_DATA*) arg2)
 *	t - to (char the message is being printed to)
 * G
 * h
 * H
 * i - name(vch1)
 * I - name(vch3)
 * j - num(arg1)
 * J - num(arg3)
 * k
 * K
 * l
 * L
 * m - him_her(ch)
 * M - him_her(vch)
 * n - name(ch)
 * N - name(vch)
 * o
 * O
 * p - name(obj1)
 * P - name(obj2)
 * q - $qx{...} - numeric form depending on ``x'' where x is:
 *	j - num(arg1)
 *	J - num(arg2)
 * Q
 * r - room name (arg1)
 * R - room name (arg3)
 * s - his_her(ch)
 * S - his_her(vch)
 * t - text(arg1)
 * T - text(arg2)
 * u - text(arg1)
 * U - text(arg3)
 * v
 * V
 * w
 * W
 * x
 * X
 * y
 * Y
 * z
 * Z
 *
 */
static void act_raw(CHAR_DATA *ch, CHAR_DATA *to,
		    const void *arg1, const void *arg2, const void *arg3,
		    const char *str, int flags)
{
	CHAR_DATA *	vch = (CHAR_DATA*) arg2;
	CHAR_DATA *	vch1 = (CHAR_DATA*) arg1;
	CHAR_DATA *	vch3 = (CHAR_DATA*) arg3;
	int		num1 = (int) arg1;
	int		num3 = (int) arg3;
	ROOM_INDEX_DATA *room1 = (ROOM_INDEX_DATA*) arg1;
	ROOM_INDEX_DATA *room3 = (ROOM_INDEX_DATA*) arg3;
	OBJ_DATA *	obj1 = (OBJ_DATA*) arg1;
	OBJ_DATA *	obj2 = (OBJ_DATA*) arg2;
	char 		buf	[MAX_STRING_LENGTH];
	char 		tmp	[MAX_STRING_LENGTH];

	char *		point = buf;
	const char *	s = str;

	struct tdata	tstack[TSTACK_SZ];
	int		sp = -1;

	while(*s) {
		char		code;
		char		subcode;
		const char *	i;

		switch (*s) {
		default:
			*point++ = *s++;
			break;

		case '}':
			if (sp < 0) {
				*point++ = *s++;
				continue;
			}

			if (sp < TSTACK_SZ) {
				const char *tr = str_empty;

				*point = '\0';

				switch (tstack[sp].type) {
				case 'g':
					tr = word_gender(to->lang, tstack[sp].p,
								tstack[sp].arg);
					break;

				case 'c':
					tr = word_case(to->lang, tstack[sp].p,
								tstack[sp].arg);
					break;
				case 'q':
					tr = word_quantity(to->lang,
						tstack[sp].p, tstack[sp].arg);
					break;
				}

				strnzcpy(tstack[sp].p, 
					 sizeof(buf) - (tstack[sp].p - buf),
					 tr);
				point = strchr(tstack[sp].p, '\0');
			}

			sp--;
			s++;
			continue;

		case '{':
			if (*(s+1) == '}')
				s++;
			*point++ = *s++;
			continue;

		case '$':
			s++;

			switch (code = *s++) {
			default:  
				i = " <@@@> ";
				log_printf("act_raw: '%s': bad code $%c",
					   str, code);
				continue;
	
			case 't': 
			case 'T':
			case 'u':
			case 'U':
				i = (code == 'U') ? arg3 :
				    (code == 'T') ? arg2 : arg1;
				if (IS_SET(flags, ACT_TRANS))
					i = GETMSG(i, to->lang);
				if (IS_SET(flags, ACT_STRANS))
					i = translate(ch, to, i);
				break;
	
			case 'r':
				i = mlstr_mval(room1->name);
				break;

			case 'R':
				i = mlstr_mval(room3->name);
				break;

			case 'n':
				i = PERS(ch, to);
				if (sp < 0)
					i = fix_short(i);
				break;
	
			case 'N':
				i = PERS(vch, to);
				if (sp < 0)
					i = fix_short(i);
				break;

			case 'i':
				i = PERS(vch1, to);
				if (sp < 0)
					i = fix_short(i);
				break;

			case 'I':
				i = PERS(vch3, to);
				if (sp < 0)
					i = fix_short(i);
				break;

			case 'j':
				snprintf(tmp, sizeof(tmp), "%d", num1);
				i = tmp;
				break;

			case 'J':
				snprintf(tmp, sizeof(tmp), "%d", num3);
				i = tmp;
				break;

			case 'e':
				i = he_she[SEX(ch, to)];
				break;
	
			case 'E':
				i = he_she[SEX(vch, to)];
				break;
	
			case 'm':
				i = him_her[SEX(ch, to)];
				break;
	
			case 'M':
				i = him_her[SEX(vch, to)];
				break;
	
			case 's':
				i = his_her[SEX(ch, to)];
				break;
	
			case 'S':
				i = his_her[SEX(vch, to)];
				break;
	
			case 'p':
				i = can_see_obj(to, obj1) ?
					mlstr_cval(obj1->short_descr, to) :
					GETMSG("something", to->lang);
				if (sp < 0)
					i = fix_short(i);
				break;
	
			case 'P':
				i = can_see_obj(to, obj2) ?
					mlstr_cval(obj2->short_descr, to) :
					GETMSG("something", to->lang);
				if (sp < 0)
					i = fix_short(i);
				break;
	
			case 'd':
				if (IS_NULLSTR(arg2))
					i = GETMSG("door", to->lang);
				else {
					one_argument(arg2, tmp, sizeof(tmp));
					i = tmp;
				}
				break;

			case 'g':
			case 'c':
			case 'q':
				if (*(s+1) != '{') {
					log_printf("act_raw: '%s': "
						   "syntax error", str);
					continue;
				}

				if (++sp >= TSTACK_SZ) {
					log_printf("act_raw: '%s': "
						   "tstack overflow", str);
					continue;
				}

				tstack[sp].p = point;
				tstack[sp].type = code;
				subcode = *s++;
				s++;

				switch (code) {
				case 'c':
					tstack[sp].arg = subcode - '0';
					break;

				case 'g':
					switch (subcode) {
					case 'N':
						tstack[sp].arg = vch->sex;
						break;

					case 'n':
						tstack[sp].arg = ch->sex;
						break;

					case 'i':
						tstack[sp].arg = vch1->sex;
						break;

					case 'I':
						tstack[sp].arg = vch3->sex;
						break;

					case 't':
						tstack[sp].arg = to->sex;
						break;

					case 'p':
						tstack[sp].arg = obj1->pIndexData->gender;
						break;

					case 'P':
						tstack[sp].arg = obj2->pIndexData->gender;
						break;

					default:
						log_printf("act_raw: '%s': "
							   "bad subcode '%c'",
							   str, subcode);
						sp--;
						break;
					}
					break;

				case 'q':
					switch(subcode) {
					case 'j':
						tstack[sp].arg = num1;
						break;

					case 'J':
						tstack[sp].arg = num3;
						break;
					default:
						log_printf("act_raw: '%s': "
							   "bad subcode '%c'",
							   str, subcode);
						sp--;
						break;
					}
					break;
				}
				continue;
			}
	
			if (i) {
				while ((*point++ = *i++));
				point--;
			}
			break;
		}
	}
 
	if (!IS_SET(flags, ACT_NOLF))
		*point++	= '\n';
	*point		= '\0';

/* first non-control char is uppercased */
	point = (char*) cstrfirst(buf);
	*point = UPPER(*point);

	parse_colors(buf, tmp, sizeof(tmp), OUTPUT_FORMAT(to));

	if (!IS_NPC(to)) {
		if ((IS_SET(to->comm, COMM_AFK) || to->desc == NULL) &&
		     IS_SET(flags, ACT_TOBUF))
			buf_add(to->pcdata->buffer, tmp);
		else if (to->desc) {
			if (IS_SET(to->comm, COMM_QUIET_EDITOR)
			&&  to->desc->pString)
				buf_add(to->pcdata->buffer, tmp);
			else
				write_to_buffer(to->desc, tmp, 0);
		}
	}
	else {
		if (!IS_SET(flags, ACT_NOTRIG))
			mp_act_trigger(tmp, to, ch, arg1, arg2, TRIG_ACT);
		if (to->desc)
			write_to_buffer(to->desc, tmp, 0);
	}
}

