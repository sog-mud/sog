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
 * $Id: act.c,v 1.22 1999-05-22 13:55:59 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "comm_colors.h"
#include "mob_prog.h"
#include "db/lang.h"

/*
 * char/mob short/long formatting
 */

/*
 * smash '~'
 */
const char *fix_short(const char *s)
{
	char *p;
	static char buf[MAX_STRING_LENGTH];

	if (!strchr(s, '~'))
		return s;

	for (p = buf; *s && p-buf < sizeof(buf)-1; s++) {
		if (*s == '~')
			continue;
		*p++ = *s;
	}

	*p = '\0';
	return buf;
}

/*
 * format short description for mobs and objs
 *
 * name expected to be eng equiv. and is appended in form " (name)"
 * if ACT_NOENG is not set
 */
const char *format_short(mlstring *mlshort, const char *name, CHAR_DATA *to)
{
        static char buf[MAX_STRING_LENGTH];
        const char *sshort;

        sshort = fix_short(mlstr_cval(mlshort, to));
	strnzcpy(buf, sizeof(buf), sshort);

        if (!IS_SET(to->comm, COMM_NOENG)
	&&  sshort != mlstr_mval(mlshort)) {
		char buf2[MAX_STRING_LENGTH];
        	char buf3[MAX_STRING_LENGTH];

        	one_argument(name, buf3, sizeof(buf3));
		snprintf(buf2, sizeof(buf2), " (%s)", buf3);
		strnzcat(buf, sizeof(buf), buf2);
	}

        return buf;
}

/*
 * format description (long descr for mobs, description for objs)
 *
 * eng name expected to be in form " (foo)" and is stripped
 * if ACT_NOENG is set
 */
const char *format_descr(mlstring *ml, CHAR_DATA *to)
{
	const char *s;
	const char *p, *q;
	static char buf[MAX_STRING_LENGTH];

	s = mlstr_cval(ml, to);
	if (IS_NULLSTR(s)
	||  !IS_SET(to->comm, COMM_NOENG)
	||  (p = strchr(s, '(')) == NULL
	||  (q = strchr(p+1, ')')) == NULL)
		return s;

	if (p != s && *(p-1) == ' ')
		p--;

	strnzncpy(buf, sizeof(buf), s, p-s);
	strnzcat(buf, sizeof(buf), q+1);
	return buf;
}

/*
 * PERS formatting stuff
 */
const char *PERS2(CHAR_DATA *ch, CHAR_DATA *looker, int act_flags)
{
	if (is_affected(ch, gsn_doppelganger)
	&&  (IS_NPC(looker) || !IS_SET(looker->plr_flags, PLR_HOLYLIGHT)))
		ch = ch->doppel;

	if (can_see(looker, ch)) {
		if (IS_NPC(ch)) {
			const char *descr;

			if (IS_SET(act_flags, ACT_FORMSH)) {
				return format_short(ch->short_descr, ch->name,
						    looker);
			}

			descr = mlstr_cval(ch->short_descr, looker);
			if (IS_SET(act_flags, ACT_FIXSH))
				return fix_short(descr);

			return descr;
		}
		else if (is_affected(ch, gsn_vampire) && !IS_IMMORTAL(looker)) {
			return word_form(GETMSG("an ugly creature",
						looker->lang),
					 ch->sex, looker->lang, RULES_GENDER);
		}
		return ch->name;
	}

	if (IS_IMMORTAL(ch)) {
		return word_form(GETMSG("an immortal", looker->lang), ch->sex,
				 looker->lang, RULES_GENDER);
	}

	return "someone";
}

/* common and slang should have the same size */
char common[] =
	"aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"
	"�����������ų�����������������������������������������������������";
char slang[] =
	"eEcCdDfFiIgGhHjJoOkKlLmMnNpPuUqQrRsStTvVyYwWxXzZaAbB"
	"������������������������������������������������������������������";

/* ch says, victim hears */
static char *translate(CHAR_DATA *ch, CHAR_DATA *victim, const char *i)
{
	static char trans[MAX_STRING_LENGTH];
	char *o;
	race_t *r;

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
	return URANGE(0, ch->sex, 2);
}

static const char *
act_format_text(const char *text, CHAR_DATA *ch, CHAR_DATA *to, actopt_t *opt)
{
	if (IS_SET(opt->act_flags, ACT_TRANS))
		text = GETMSG(text, opt->to_lang);
	if (IS_SET(opt->act_flags, ACT_STRANS))
		text = translate(ch, to, text);
	return text;
}
	
static const char *
act_format_obj(OBJ_DATA *obj, CHAR_DATA *to, int sp, int act_flags)
{
	const char *descr;

	if (!can_see_obj(to, obj))
		return GETMSG("something", to->lang);

	if (sp < 0) {
		if (IS_SET(act_flags, ACT_FORMSH))
			return format_short(obj->short_descr, obj->name, to);

		return fix_short(mlstr_cval(obj->short_descr, to));
	}

	return descr = mlstr_cval(obj->short_descr, to);
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
 * b - like $t but text is parsed by act_buf (with the same args)
 * B - like $T ----- // ----
 * c - $cn{...} - case number ``n''
 * C
 * d - door name (arg2)
 * D
 * e - he_she(ch)
 * E - he_she(vch)
 * f
 * F
 * g - $gx{...} - gender form depending on sex of ``x'', where x is:
 *	n	- ch ($n)
 *	N	- vch ($N)
 *	i	- vch1 ($i)
 *	I	- vch3 ($I)
 *	o	- to
 *	p	- obj1 ($p)
 *	P	- obj2 ($P)
 *	t,u	- msg1 ($t, $u)
 *	T	- msg2 ($T)
 *	U	- msg3 ($U)
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
void act_buf(const char *format, CHAR_DATA *ch, CHAR_DATA *to,
	     const void *arg1, const void *arg2, const void *arg3,
	     actopt_t *opt, char *buf, size_t buf_len)
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
	char 		tmp	[MAX_STRING_LENGTH];

	char *		point = buf;
	const char *	s = GETMSG(format, opt->to_lang);

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
				int rulecl = 0;

				switch (tstack[sp].type) {
				case 'g':
					rulecl = RULES_GENDER;
					break;
				case 'c':
					rulecl = RULES_CASE;
					break;
				case 'q':
					rulecl = RULES_QTY;
					break;
				}

				*point = '\0';
				strnzcpy(tstack[sp].p, 
					 buf_len - (tstack[sp].p - buf),
					 word_form(tstack[sp].p, tstack[sp].arg,
						   opt->to_lang, rulecl));
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
					   format, code);
				continue;
/* text arguments */
			case 't': 
			case 'u':
				i = act_format_text(arg1, ch, to, opt);
				break;

			case 'T':
				i = act_format_text(arg2, ch, to, opt);
				break;

			case 'U':
				i = act_format_text(arg3, ch, to, opt);
				break;

			case 'b':
			case 'B':
				act_buf(code == 'b' ? arg1 : arg2, ch, to,
					arg1, arg2, arg3, opt,
					tmp, sizeof(tmp));
				i = tmp;
				break;
/* room arguments */
			case 'r':
				i = mlstr_mval(room1->name);
				break;

			case 'R':
				i = mlstr_mval(room3->name);
				break;

/* char arguments */
			case 'n':
				i = PERS2(ch, to,
				  (sp < 0) ? (opt->act_flags | ACT_FIXSH) : 0);
				break;

			case 'N':
				i = PERS2(vch, to,
				  (sp < 0) ? (opt->act_flags | ACT_FIXSH) : 0);
				break;

			case 'i':
				i = PERS2(vch1, to,
				  (sp < 0) ? (opt->act_flags | ACT_FIXSH) : 0);
				break;

			case 'I':
				i = PERS2(vch3, to,
				  (sp < 0) ? (opt->act_flags | ACT_FIXSH) : 0);
				break;

/* numeric arguments */
			case 'j':
				snprintf(tmp, sizeof(tmp), "%d", num1);
				i = tmp;
				break;

			case 'J':
				snprintf(tmp, sizeof(tmp), "%d", num3);
				i = tmp;
				break;

/* him/her arguments. obsolete. $gx{...} should be used instead */
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

/* obj arguments */
			case 'p':
				i = act_format_obj(obj1, to, sp,
						   opt->act_flags);
				break;

			case 'P':
				i = act_format_obj(obj2, to, sp,
						   opt->act_flags);
				break;

/* door arguments */
			case 'd':
				if (IS_NULLSTR(arg2))
					i = GETMSG("door", opt->to_lang);
				else {
					one_argument(arg2, tmp, sizeof(tmp));
					i = tmp;
				}
				break;

/* $gx{...}, $cx{...}, $qx{...} arguments */
			case 'g':
			case 'c':
			case 'q':
				if (*(s+1) != '{') {
					log_printf("act_raw: '%s': "
						   "syntax error", format);
					continue;
				}

				if (++sp >= TSTACK_SZ) {
					log_printf("act_raw: '%s': "
						   "tstack overflow", format);
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

					case 'o':
						tstack[sp].arg = opt->to_sex;
						break;

					case 'p':
						tstack[sp].arg = obj1->pIndexData->gender;
						break;

					case 'P':
						tstack[sp].arg = obj2->pIndexData->gender;
						break;

					case 't':
					case 'u':
						tstack[sp].arg =
							msg_gender(arg1);
						break;

					case 'T':
						tstack[sp].arg =
							msg_gender(arg2);
						break;

					case 'U':
						tstack[sp].arg =
							msg_gender(arg3);
						break;

					default:
						log_printf("act_raw: '%s': "
							   "bad subcode '%c'",
							   format, subcode);
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
							   format, subcode);
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
 
	if (!IS_SET(opt->act_flags, ACT_NOLF))
		*point++ = '\n';
	*point = '\0';

/* first non-control char is uppercased */
	if (!IS_SET(opt->act_flags, ACT_NOUCASE)) {
		point = (char*) cstrfirst(buf);
		*point = UPPER(*point);
	}
}

static CHAR_DATA*
act_args(CHAR_DATA *ch, CHAR_DATA *vch, int act_flags, const char *format)
{
	if (format == NULL)
		return NULL;

	if (IS_SET(act_flags, TO_CHAR))
		return ch;

	if (IS_SET(act_flags, TO_VICT))
		return vch;

	if (!ch || !ch->in_room)
		return NULL;

	return ch->in_room->people;
}
 
static bool
act_skip(CHAR_DATA *ch, CHAR_DATA *vch, CHAR_DATA *to,
	 int act_flags, int min_pos)
{
	if (to->position < min_pos)
		return TRUE;

	if (IS_SET(act_flags, TO_CHAR) && to != ch)
		return TRUE;
	if (IS_SET(act_flags, TO_VICT) && (to != vch || to == ch))
		return TRUE;
	if (IS_SET(act_flags, TO_ROOM) && to == ch)
		return TRUE;
	if (IS_SET(act_flags, TO_NOTVICT) && (to == ch || to == vch))
		return TRUE;

	if (IS_NPC(to)
	&&  to->desc == NULL
	&&  !HAS_TRIGGER(to, TRIG_ACT))
		return TRUE;
 
	if (IS_SET(act_flags, ACT_NOMORTAL) && !IS_NPC(to) && !IS_IMMORTAL(to))
		return TRUE;

/* twitlist handling */
	if (IS_SET(act_flags, ACT_NOTWIT)
	&&  !IS_NPC(to) && !IS_IMMORTAL(to)
	&&  !IS_NPC(ch) && !IS_IMMORTAL(ch)
	&&  is_name(ch->name, to->pcdata->twitlist))
		return TRUE;

/* check "deaf dumb blind" chars */
	if (IS_SET(act_flags, ACT_NODEAF) && is_affected(to, gsn_deafen))
		return TRUE;

/* skip verbose messages */
	if (IS_SET(act_flags, ACT_VERBOSE)
	&&  IS_SET(to->comm, COMM_NOVERBOSE))
		return TRUE;

	return FALSE;
}

static void
act_raw(CHAR_DATA *ch, CHAR_DATA *to,
	const void *arg1, const void *arg2, const void *arg3,
	const char *format, int act_flags)
{
	char buf[MAX_STRING_LENGTH];
	char tmp[MAX_STRING_LENGTH];
	actopt_t opt;

	opt.to_lang = to->lang;
	opt.to_sex = to->sex;
	opt.act_flags = act_flags;

	act_buf(format, ch, to, arg1, arg2, arg3,
		&opt, buf, sizeof(buf));
	parse_colors(buf, tmp, sizeof(tmp), OUTPUT_FORMAT(to));

	if (!IS_NPC(to)) {
		if ((IS_SET(to->comm, COMM_AFK) || to->desc == NULL) &&
		     IS_SET(act_flags, ACT_TOBUF))
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
		if (!IS_SET(act_flags, ACT_NOTRIG))
			mp_act_trigger(tmp, to, ch, arg1, arg2, TRIG_ACT);
		if (to->desc)
			write_to_buffer(to->desc, tmp, 0);
	}
}

void act_puts3(const char *format, CHAR_DATA *ch,
	       const void *arg1, const void *arg2, const void *arg3,
	       int act_flags, int min_pos)
{
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;

	if ((to = act_args(ch, vch, act_flags, format)) == NULL)
		return;

	if (IS_SET(act_flags, TO_CHAR | TO_VICT)) {
		if (!act_skip(ch, vch, to, act_flags, min_pos)) {
			act_raw(ch, to, arg1, arg2, arg3,
				format, act_flags);
		}
		return;
	}
		
	for(; to; to = to->next_in_room) {
		if (act_skip(ch, vch, to, act_flags, min_pos)) 
			continue;

		act_raw(ch, to, arg1, arg2, arg3,
			format, act_flags);
	}
}

