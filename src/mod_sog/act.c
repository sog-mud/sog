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
 * $Id: act.c,v 1.41 1999-10-06 09:56:13 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "comm_colors.h"
#include "mob_prog.h"
#include "lang.h"
#include "memalloc.h"

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
const char *format_short(mlstring *mlshort, const char *name, CHAR_DATA *to,
			 int act_flags)
{
        const char *sshort;

        sshort = mlstr_cval(mlshort, to);

        if (!IS_SET(to->comm, COMM_NOENG)
	&&  sshort != mlstr_mval(mlshort)) {
        	static char buf[MAX_STRING_LENGTH];
		char buf2[MAX_STRING_LENGTH];
		const char *format;

		if (IS_SET(act_flags, ACT_NOFIXSH)
		&&  strchr(sshort, '~') == NULL)
			format = "~%s~ (%s)";
		else
			format = "%s (%s)";

        	one_argument(name, buf2, sizeof(buf2));
		snprintf(buf, sizeof(buf), format, sshort, buf2);
		sshort = buf;
	}

	if (IS_SET(act_flags, ACT_NOFIXSH))
		return sshort;
	return fix_short(sshort);
}

/*
 * format description (long descr for mobs, description for objs)
 *
 * eng name expected to be in form " (foo)" and is stripped
 * if ACT_NOENG is set
 */
const char *format_long(mlstring *ml, CHAR_DATA *to)
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
	if (is_affected(ch, "doppelganger")
	&&  (IS_NPC(looker) ||
	     !IS_SET(PC(looker)->plr_flags, PLR_HOLYLIGHT)))
		ch = ch->doppel;

	if (can_see(looker, ch)) {
		if (IS_NPC(ch)) {
			const char *descr;

			if (IS_SET(act_flags, ACT_FORMSH)) {
				return format_short(&ch->short_descr, ch->name,
						    looker, act_flags);
			}

			descr = mlstr_cval(&ch->short_descr, looker);
			if (IS_SET(act_flags, ACT_NOFIXSH))
				return descr;
			return fix_short(descr);
		}
		else if (IS_AFFECTED(ch, AFF_TURNED) && !IS_IMMORTAL(looker)) {
			return word_form(GETMSG(PC(ch)->form_name,
						GET_LANG(looker)),
					 ch->sex, GET_LANG(looker),
					 RULES_GENDER);
		}
		return ch->name;
	}

	if (IS_IMMORTAL(ch)) {
		return word_form(GETMSG("an immortal", GET_LANG(looker)),
				 ch->sex, GET_LANG(looker), RULES_GENDER);
	}

	return GETMSG("someone", GET_LANG(looker));
}

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
	race_t *r;

	if (*i == '\0'
	||  ch == victim
	||  (ch == NULL) || (victim == NULL)
	||  IS_NPC(ch) || IS_NPC(victim)
	||  IS_IMMORTAL(ch) || IS_IMMORTAL(victim)
	||  ch->slang == SLANG_COMMON
	||  ((r = race_lookup(ORG_RACE(victim))) &&
	     r->race_pcdata &&
	     ch->slang == r->race_pcdata->slang)) {
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
	if (is_affected(ch, "doppelganger")
	&&  (IS_NPC(looker) || !IS_SET(PC(looker)->plr_flags, PLR_HOLYLIGHT)))
		ch = ch->doppel;
	return URANGE(0, ch->sex, 2);
}

static const char *
act_format_text(const char *text, CHAR_DATA *ch, CHAR_DATA *to,
		int to_lang, int act_flags)
{
	if (!IS_SET(act_flags, ACT_NOTRANS))
		text = GETMSG(text, to_lang);
	if (IS_SET(act_flags, ACT_STRANS))
		text = translate(ch, to, text);
	if (IS_SET(act_flags, ACT_NOFIXTEXT))
		return text;
	return fix_short(text);
}
	
static const char *
act_format_obj(OBJ_DATA *obj, CHAR_DATA *looker, int act_flags)
{
	const char *descr;

	if (!can_see_obj(looker, obj))
		return GETMSG("something", GET_LANG(looker));

	if (IS_SET(act_flags, ACT_FORMSH)) {
		return format_short(&obj->short_descr, obj->name,
				    looker, act_flags);
	}

	descr = mlstr_cval(&obj->short_descr, looker);
	if (IS_SET(act_flags, ACT_NOFIXSH))
		return descr;
	return fix_short(descr);
}

static const char *
door_name(const char *name)
{
	static char buf[MAX_STRING_LENGTH];

	if (IS_NULLSTR(name))
		return "door";

	one_argument(name, buf, sizeof(buf));
	return buf;
}

#define CHECK_STRING(p)						\
	if (p == NULL) {					\
		log("act_buf: format '%s', NULL string arg",	\
		    format);					\
		i = NULL;					\
		break;						\
	}

#define CHECK_STRING2(p)					\
	if (p == NULL) {					\
		log("act_buf: format '%s', NULL string arg",	\
		    format);					\
		sp--;						\
		break;						\
	}

#define CHECK_TYPE(p, mem_type)					\
	if (!mem_is(p, mem_type)) {				\
		log("act_buf: format '%s', expected type %d",	\
		    format, mem_type);				\
		i = NULL;					\
		break;						\
	}

#define CHECK_TYPE2(p, mem_type)				\
	if (!mem_is(p, mem_type)) {				\
		log("act_buf: format '%s', expected type %d",	\
		    format, mem_type);				\
		sp--;						\
		break;						\
	}

#define ACT_FLAGS(flags, sp)	((flags) | ((sp) < 0 ? 0 : ACT_NOFIXSH))

#define CHAR_ARG(vch)							\
	{								\
		if (vch == NULL) {					\
			i = GETMSG("Noone", opt->to_lang);		\
		} else {						\
			int flags;					\
			CHECK_TYPE(vch, MT_CHAR);			\
			flags = ACT_FLAGS(opt->act_flags, sp);		\
			i = PERS2(vch, to, flags);			\
		}							\
	}

#define OBJ_ARG(obj)							\
	{								\
		if (obj == NULL) {					\
			i = GETMSG("Nothing", opt->to_lang);		\
		} else {						\
			int flags;					\
			CHECK_TYPE(obj, MT_OBJ);			\
			flags = ACT_FLAGS(opt->act_flags, sp);		\
			i = act_format_obj(obj,	to, flags);		\
		}							\
	}

#define TEXT_ARG(text, flags)						\
	{								\
		CHECK_STRING(text);					\
		i = act_format_text(text, ch, to, opt->to_lang,		\
				    ACT_FLAGS(flags, sp));		\
	}

/*
 * vch is (CHAR_DATA*) arg2
 * vch1 is (CHAR_DATA*) arg1
 * obj1 is (OBJ_DATA*) arg1
 * obj2 is (OBJ_DATA*) arg2
 *
 * Known act_xxx format codes are:
 *
 * $a
 * $A
 * $b
 * $B
 * $c - $cn{...} - case number ``n''
 * $C
 * $d - door name (arg2)
 * $D
 * $e - he_she(ch)
 * $E - he_she(vch)
 * $f - $fnn{...} - misc formatting
 * $F - $Fnn{...} - ------//-------
 *		$fnn formats string with "%snn" format
 *		$Fnn formats string with "%snn.nn" format
 * $g - $gx{...} - gender form depending on sex of ``x'', where x is:
 *	d	- door name ($d)
 *	n	- ch ($n)
 *	N	- vch ($N)
 *	i	- vch1 ($i)
 *	I	- vch3 ($I)
 *	o	- to
 *	p	- obj1 ($p)
 *	P	- obj2 ($P)
 *	t	- msg1 ($t)
 *	T	- msg2 ($T)
 *	u	- msg1 ($u), without slang translation
 *	U	- msg3 ($U), without slang translation
 * $G
 * $h
 * $H
 * $i - name(vch1)
 * $I - name(vch3)
 * $j - num(arg1)
 * $J - num(arg3)
 * $k
 * $K
 * $l - $lx{...} - tolower/toupper convertions
 *	u	- uppercase first letter
 * $L
 * $m - him_her(ch)
 * $M - him_her(vch)
 * $n - name(ch)
 * $N - name(vch)
 * $o
 * $O
 * $p - name(obj1)
 * $P - name(obj2)
 * $q - $qx{...} - numeric form depending on ``x'' where x is:
 *	j - num(arg1)
 *	J - num(arg2)
 * $Q
 * $r - room name (arg1)
 * $R - room name (arg3)
 * $s - his_her(ch)
 * $S - his_her(vch)
 * $t - text(arg1)
 * $T - text(arg2)
 * $u - text(arg1)
 * $U - text(arg3)
 * $v
 * $V
 * $w
 * $W
 * $x
 * $X
 * $y
 * $Y
 * $z
 * $Z
 * ${ - "{{"
 * $$ - "$"
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
	char		tmp2	[MAX_STRING_LENGTH];

	char *		point = buf;
	const char *	s;

	struct tdata	tstack[TSTACK_SZ];
	int		sp = -1;

	s = format = GETMSG(format, opt->to_lang);

	while(*s) {
		char		code;
		char		subcode;
		const char *	i;

		switch (*s) {
		default:
			*point++ = *s++;
			break;

		case '}':
			s++;

			if (sp < 0)
				continue;

			if (sp >= TSTACK_SZ) {
				sp--;
				continue;
			}

			switch (tstack[sp].type) {
			case 'g':
			case 'c':
			case 'q': {
				int rulecl;

				if (tstack[sp].type == 'g')
					rulecl = RULES_GENDER;
				else if (tstack[sp].type == 'c')
					rulecl = RULES_CASE;
				else
					rulecl = RULES_QTY;

				*point = '\0';
				strnzcpy(tstack[sp].p, 
					 buf_len - 3 - (tstack[sp].p - buf),
					 word_form(tstack[sp].p, tstack[sp].arg,
						   opt->to_lang, rulecl));
				point = strchr(tstack[sp].p, '\0');
				break;
			}

			case 'f':
			case 'F':
				*point = '\0';
				snprintf(tmp2, sizeof(tmp2),
					 tstack[sp].type == 'f' ?
					 	"%%%ds" : "%%%d.%ds",
					 tstack[sp].arg, abs(tstack[sp].arg));
				snprintf(tmp, sizeof(tmp), tmp2, tstack[sp].p);
				strnzcpy(tstack[sp].p,
					 buf_len - 3 - (tstack[sp].p - buf),
					 tmp);
				point = strchr(tstack[sp].p, '\0');
				break;

			case 'l':
				if (tstack[sp].arg == 'u')
					*tstack[sp].p = UPPER(*tstack[sp].p);
				break;
			}

			sp--;
			continue;

		case '$':
			s++;

			switch (code = *s++) {
			default:  
				i = " <@@@> ";
				log("act_buf: '%s': bad code $%c",
					   format, code);
				continue;

			case '$':
				i = "$";
				break;
				
			case '{':
				i = "{{";
				break;

/* text arguments */
			case 't': 
				TEXT_ARG(arg1, opt->act_flags);
				break;

			case 'T':
				TEXT_ARG(arg2, opt->act_flags);
				break;

			case 'u':
				TEXT_ARG(arg1, opt->act_flags & ~ACT_STRANS);
				break;

			case 'U':
				TEXT_ARG(arg3, opt->act_flags & ~ACT_STRANS);
				break;

/* room arguments */
			case 'r':
				CHECK_TYPE(room1, MT_ROOM);
				i = mlstr_mval(&room1->name);
				break;

			case 'R':
				CHECK_TYPE(room3, MT_ROOM);
				i = mlstr_mval(&room3->name);
				break;

/* char arguments */
			case 'n':
				CHAR_ARG(ch);
				break;

			case 'N':
				CHAR_ARG(vch);
				break;

			case 'i':
				CHAR_ARG(vch1);
				break;

			case 'I':
				CHAR_ARG(vch3);
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
				CHECK_TYPE(ch, MT_CHAR);
				i = he_she[SEX(ch, to)];
				break;
	
			case 'E':
				CHECK_TYPE(vch, MT_CHAR);
				i = he_she[SEX(vch, to)];
				break;
	
			case 'm':
				CHECK_TYPE(ch, MT_CHAR);
				i = him_her[SEX(ch, to)];
				break;
	
			case 'M':
				CHECK_TYPE(vch, MT_CHAR);
				i = him_her[SEX(vch, to)];
				break;
	
			case 's':
				CHECK_TYPE(ch, MT_CHAR);
				i = his_her[SEX(ch, to)];
				break;
	
			case 'S':
				CHECK_TYPE(vch, MT_CHAR);
				i = his_her[SEX(vch, to)];
				break;

/* obj arguments */
			case 'p':
				OBJ_ARG(obj1);
				break;

			case 'P':
				OBJ_ARG(obj2);
				break;

/* door arguments */
			case 'd':
				CHECK_STRING(arg2);
				i = GETMSG(door_name(arg2), GET_LANG(to));
				break;

/* $gx{...}, $cx{...}, $qx{...} arguments */
			case 'g':
			case 'c':
			case 'q':
			case 'f':
			case 'l':
			case 'F':
				if (++sp >= TSTACK_SZ) {
					log("act_raw: '%s': "
					    "tstack overflow", format);
					continue;
				}

				tstack[sp].p = point;
				tstack[sp].type = code;

				switch (code) {
				case 'f':
				case 'F':
					tstack[sp].arg =
						strtol(s, (char**) &s, 10);
					break;

				case 'l':
					switch (subcode = *s++) {
					case 'u':
						tstack[sp].arg = subcode;
						break;
					default:
						log("act_buf: '%s': "
						    "bad subcode '%c' "
						    "(pos %d)",
						    format, subcode,
						    s - format);
						sp--;
						continue;
					}
					break;

				case 'c':
					tstack[sp].arg = *s++ - '0';
					break;

				case 'g':
					switch (subcode = *s++) {
					case 'd':
						CHECK_STRING(arg2);
						tstack[sp].arg =
						    msg_gender(door_name(arg2));
						break;

					case 'N':
						CHECK_TYPE2(vch, MT_CHAR);
						tstack[sp].arg = vch->sex;
						break;

					case 'n':
						CHECK_TYPE2(ch, MT_CHAR);
						tstack[sp].arg = ch->sex;
						break;

					case 'i':
						CHECK_TYPE2(vch1, MT_CHAR);
						tstack[sp].arg = vch1->sex;
						break;

					case 'I':
						CHECK_TYPE2(vch3, MT_CHAR);
						tstack[sp].arg = vch3->sex;
						break;

					case 'o':
						tstack[sp].arg = opt->to_sex;
						break;

					case 'p':
						CHECK_TYPE2(obj1, MT_OBJ);
						tstack[sp].arg = obj1->pObjIndex->gender;
						break;

					case 'P':
						CHECK_TYPE2(obj2, MT_OBJ);
						tstack[sp].arg = obj2->pObjIndex->gender;
						break;

					case 't':
					case 'u':
						CHECK_STRING(arg1);
						tstack[sp].arg =
							msg_gender(arg1);
						break;

					case 'T':
						CHECK_STRING(arg2);
						tstack[sp].arg =
							msg_gender(arg2);
						break;

					case 'U':
						CHECK_STRING(arg3);
						tstack[sp].arg =
							msg_gender(arg3);
						break;

					case '0':
					case '1':
					case '2':
					case '3':
						tstack[sp].arg = subcode - '0';
						break;

					default:
						log("act_buf: '%s': "
						    "bad subcode '%c' "
						    "(pos %d)",
						    format, subcode,
						    s - format);
						sp--;
						continue;
					}
					break;

				case 'q':
					switch(subcode = *s++) {
					case 'j':
						tstack[sp].arg = num1;
						break;

					case 'J':
						tstack[sp].arg = num3;
						break;
					default:
						log("act_buf: '%s': "
						    "bad subcode '%c' "
						    "(pos %d)",
						    format, subcode,
						    s - format);
						sp--;
						continue;
					}
					break;
				}

				if (*s != '{') {
					log("act_buf: '%s': "
					    "syntax error (pos %d)",
					    format, s - format);
					sp--;
					continue;
				}

				s++;
				continue;
			}
	
			if (i != NULL) {
				while (point - buf < buf_len - 3 && *i)
					*point++ = *i++;
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
	&&  is_name(ch->name, PC(to)->twitlist))
		return TRUE;

/* check "deaf dumb blind" chars */
	if (IS_SET(act_flags, ACT_NODEAF) && is_affected(to, "deafen"))
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

	opt.to_lang = GET_LANG(to);
	opt.to_sex = to->sex;
	opt.act_flags = act_flags;

	act_buf(format, ch, to, arg1, arg2, arg3,
		&opt, buf, sizeof(buf));
	parse_colors(buf, tmp, sizeof(tmp), OUTPUT_FORMAT(to));

	if (!IS_NPC(to)) {
		if ((IS_SET(to->comm, COMM_AFK) || to->desc == NULL) &&
		     IS_SET(act_flags, ACT_TOBUF))
			buf_add(PC(to)->buffer, tmp);
		else if (to->desc) {
			if (IS_SET(to->comm, COMM_QUIET_EDITOR)
			&&  to->desc->pString)
				buf_add(PC(to)->buffer, tmp);
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

const char *
act_speech(CHAR_DATA *ch, CHAR_DATA *vch, const char *text, const void *arg)
{
	static char buf[MAX_STRING_LENGTH];

	if (arg) {
		actopt_t opt;

		opt.to_lang = GET_LANG(vch);
		opt.act_flags = ACT_NOUCASE | ACT_NOLF;

		act_buf(text, ch, ch, arg, NULL, NULL, &opt, buf, sizeof(buf));
		text = buf;
	}

	return text;
}

void act_yell(CHAR_DATA *ch, const char *text, const void *arg,
	      const char *format)
{
	DESCRIPTOR_DATA *d;

	if (format == NULL)
		format = "$n yells '{M$t{x'";

	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *vch;

		if (d->connected != CON_PLAYING
		||  (vch = d->character) == NULL
		||  vch == ch
		||  vch->in_room == NULL
		||  vch->in_room->area != ch->in_room->area)
			continue;

		act_puts(format, ch, act_speech(ch, vch, text, arg), vch,
	    		 TO_VICT | ACT_SPEECH(ch), POS_DEAD);
	}
}

void act_clan(CHAR_DATA *ch, const char *text, const void *arg)
{
	CHAR_DATA *vch;

	for (vch = char_list; vch; vch = vch->next) {
		if (vch == ch
		||  vch->clan != ch->clan
		||  IS_SET(vch->comm, COMM_NOCLAN))
			continue;

		act_puts("[CLAN] $lu{$n}: {C$t{x", ch,
			 act_speech(ch, vch, text, arg), vch,
			 TO_VICT | ACT_TOBUF | (ACT_SPEECH(ch) & ~ACT_STRANS),
			 POS_DEAD);
	}
}

void act_say(CHAR_DATA *ch, const char *text, const void *arg)
{
	CHAR_DATA *vch;

	act_puts("You say '{G$t{x'", ch,
		 act_speech(ch, ch, text, arg), NULL,
		 TO_CHAR | ACT_SPEECH(ch), POS_DEAD);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch == ch)
			continue;
		act_puts("$n says '{G$t{x'", ch,
			 act_speech(ch, vch, text, arg), vch,
		 	 TO_VICT | ACT_TOBUF | ACT_NOTWIT | ACT_SPEECH(ch),
			 POS_RESTING);
	}
}

