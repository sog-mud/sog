/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: act.c,v 1.101 2003-09-29 23:11:46 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <merc.h>
#include <lang.h>
#include <memalloc.h>
#include <colors.h>

#include <sog.h>

#include "comm.h"

/*
 * char/mob short/long formatting
 */

static uint
GET_SEX(const mlstring *ml, uint to_lang)
{
	int gender = flag_value(gender_table, mlstr_val(ml, to_lang));
	return URANGE(0, gender, 4);
}

static int
PERS_SEX(CHAR_DATA *ch, CHAR_DATA *looker, uint to_lang)
{
	if (ch != looker
	&&  is_sn_affected(ch, "doppelganger")
	&&  ch->doppel != NULL
	&&  (IS_NPC(looker) || !IS_SET(PC(looker)->plr_flags, PLR_HOLYLIGHT)))
		ch = ch->doppel;

	return GET_SEX(&ch->gender, to_lang);
}

/*
 * smash '~'
 */
static const char *
smash_tilde(const char *s, int act_flags)
{
	char *p;
	static char buf[MAX_STRING_LENGTH];

	if (IS_SET(act_flags, ACT_NOFIXSH)
	||  !strchr(s, '~'))
		return s;

	for (p = buf; *s && p < buf + sizeof(buf) - 1; s++) {
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
const char *
format_short(mlstring *mlshort, const char *name, CHAR_DATA *to,
	     uint to_lang, int act_flags)
{
        const char *sshort;

        sshort = mlstr_val(mlshort, to_lang);

        if (!IS_SET(to->comm, COMM_NOENG)
	&&  sshort != mlstr_mval(mlshort)) {
		static char buf[MAX_STRING_LENGTH];
		char buf2[MAX_STRING_LENGTH];
		const char *format;

		/*
		 * enclose in tildes if we do want them
		 * (if we do not have tilde smasher enabled)
		 * and `sshort' does not already have them
		 */
		if (IS_SET(act_flags, ACT_NOFIXSH)
		&&  strchr(sshort, '~') == NULL)
			format = "~%s~ (%s)";			// notrans
		else
			format = "%s (%s)";			// notrans

		one_argument(name, buf2, sizeof(buf2));
		snprintf(buf, sizeof(buf), format, sshort, buf2);
		sshort = buf;
	}

	return smash_tilde(sshort, act_flags);
}

/*
 * format description (long descr for mobs, description for objs)
 *
 * eng name expected to be in form " (foo)" and is stripped
 * if ACT_NOENG is set
 */
const char *
format_long(mlstring *ml, CHAR_DATA *to)
{
	const char *s;
	const char *p, *q;
	static char buf[MAX_STRING_LENGTH];

	s = mlstr_val(ml, GET_LANG(to));
	if (IS_NULLSTR(s)
	||  !IS_SET(to->comm, COMM_NOENG)
	||  (p = strchr(s, '(')) == NULL
	||  (q = strchr(p+1, ')')) == NULL)
		return s;

	if (p != s && *(p-1) == ' ')
		p--;

	strlncpy(buf, s, sizeof(buf), p-s);
	strlcat(buf, q+1, sizeof(buf));
	return buf;
}

/*
 * PERS formatting stuff
 */
const char *
PERS(CHAR_DATA *ch, CHAR_DATA *to, uint to_lang, int act_flags)
{
	bool visible = (act_flags & ACT_NOCANSEE) != 0 || can_see(to, ch);

	if (is_sn_affected(ch, "doppelganger")
	&&  (IS_NPC(to) ||
	     !IS_SET(PC(to)->plr_flags, PLR_HOLYLIGHT)))
		ch = ch->doppel;

	if (!IS_NPC(to) && is_sn_affected(to, "hallucination"))
		ch = nth_char(ch, PC(to)->random_value);

	if (visible) {
		if (ch->shapeform) {
			if (IS_SET(act_flags, ACT_FORMSH)) {
				return format_short(
					&ch->shapeform->index->short_desc,
					ch->shapeform->index->name,
					to, to_lang, act_flags);
			}

			return smash_tilde(
			    mlstr_val(&ch->shapeform->index->short_desc, to_lang),
			    act_flags);
		}
		if (IS_NPC(ch)) {
			if (IS_SET(act_flags, ACT_FORMSH)) {
				return format_short(&ch->short_descr, ch->name,
						    to, to_lang, act_flags);
			}

			return smash_tilde(
			    mlstr_val(&ch->short_descr, to_lang),
			    act_flags);
		} else if (IS_AFFECTED(ch, AFF_TURNED) && !IS_IMMORTAL(to)) {
			return word_form(GETMSG(PC(ch)->form_name, to_lang),
					 GET_SEX(&ch->gender, to_lang), to_lang,
					 RULES_GENDER);
		}
		return ch->name;
	}

	if (IS_IMMORTAL(ch)) {
		return word_form(GETMSG("an immortal", to_lang),
				 GET_SEX(&ch->gender, to_lang), to_lang,
				 RULES_GENDER);
	}

	return GETMSG("someone", to_lang);
}

/* common and slang should have the same size */
char common[] =
	"aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"	// notrans
	"áÁâÂ÷×çÇäÄåÅ³£öÖúÚéÉêÊëËìÌíÍîÎïÏðÐòÒóÓôÔõÕæÆèÈãÃþÞûÛ"	// notrans
	"ýÝÿßùÙøØüÜàÀñÑ";					// notrans
char slang[] =
	"eEcCdDfFiIgGhHjJoOkKlLmMnNpPuUqQrRsStTvVyYwWxXzZaAbB"	// notrans
	"õÕâÂâÂïÏâÂáÁùÙâÂâÂõÕâÂâÂâÂâÂéÉáÁâÂâÂòÒâÂùÙâÂâÂâÂâÂâÂ"	// notrans
	"âÂÿßõÕøØáÁõÕùÙ";					// notrans

/* ch says, victim hears */
static char *
translate(const CHAR_DATA *ch, CHAR_DATA *victim, const char *i)
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
		if (IS_IMMORTAL(victim)) {
			snprintf(trans, sizeof(trans), "[%s] %s", // notrans
				 flag_string(slang_table, ch->slang), i);
		} else
			strlcpy(trans, i, sizeof(trans));
		return trans;
	}

	snprintf(trans, sizeof(trans), "[%s] ",		// notrans
		 flag_string(slang_table, ch->slang));
	o = strchr(trans, '\0');
	for (; *i && o < trans + sizeof(trans) - 1; i++, o++) {
		char *p = strchr(common, *i);
		*o = p ? slang[p-common] : *i;
	}
	*o = '\0';

	return trans;
}

const char *he_she [] = { "it",  "he",  "she", "they" };	// notrans
const char *him_her[] = { "it",  "him", "her", "them" };	// notrans
const char *his_her[] = { "its", "his", "her", "their" };	// notrans

struct tdata {
	char	type;
	int	arg;
	char *	p;
};

static const char *
act_format_text(const char *text, CHAR_DATA *ch, CHAR_DATA *to,
		uint to_lang, int act_flags)
{
	if (!IS_SET(act_flags, ACT_NOTRANS))
		text = GETMSG(text, to_lang);
	if (IS_SET(act_flags, ACT_STRANS))
		text = translate(ch, to, text);
	return smash_tilde(text, act_flags);
}

static const char *
act_format_mltext(const mlstring *mltext, CHAR_DATA *ch,
		  CHAR_DATA *to, uint to_lang, int act_flags)
{
	return act_format_text(mlstr_val(mltext, to_lang),
			       ch, to, to_lang, act_flags);
}

static const char *
act_format_obj(OBJ_DATA *obj, CHAR_DATA *to, uint to_lang,
	       int act_flags)
{
	if (!IS_NPC(to) && is_sn_affected(to, "hallucination"))
		obj = nth_obj(obj, PC(to)->random_value);

	if ((act_flags & ACT_NOCANSEE) == 0 && !can_see_obj(to, obj))
		return GETMSG("something", to_lang);

	if (IS_SET(act_flags, ACT_FORMSH)) {
		return format_short(&obj->short_descr, obj->pObjIndex->name,
				    to, to_lang, act_flags);
	}

	return smash_tilde(mlstr_val(&obj->short_descr, to_lang), act_flags);
}

static const gmlstr_t *
act_format_door(const gmlstr_t *gml)
{
	if (gml == NULL)
		return NULL;
	if (mlstr_null(&gml->ml))
		gml = glob_lookup("the door");
	return gml;
}

#define CHECK_STRING(p)							\
	if (p == NULL) {						\
		log(LOG_INFO, "act_buf: format '%s': NULL string arg",	\
		    format);						\
		i = NULL;						\
		break;							\
	}

#define CHECK_STRING2(p)						\
	if (p == NULL) {						\
		log(LOG_INFO, "act_buf: format '%s': NULL string arg",	\
		    format);						\
		sp--;							\
		break;							\
	}

#define CHECK_GMLSTR(gml)						\
	if (gml == NULL || !mlstr_valid(&(gml)->ml)) {			\
		log(LOG_INFO, "act_buf: format '%s': invalid mlstring arg",\
		    format);						\
		i = NULL;						\
		break;							\
	}

#define CHECK_GMLSTR2(gml)						\
	if (gml == NULL || !mlstr_valid(&(gml)->gender)) { 		\
		log(LOG_INFO, "act_buf: format '%s': invalid mlstring arg (2)",\
		    format);						\
		sp--;							\
		break;							\
	}

#define CHECK_TYPE(p, mem_type)						\
	if (!mem_is(p, mem_type)) {					\
		log(LOG_INFO, "act_buf: format '%s': expected type %d",	\
		    format, mem_type);					\
		i = NULL;						\
		break;							\
	}

#define CHECK_TYPE2(p, mem_type)					\
	if (!mem_is(p, mem_type)) {					\
		log(LOG_INFO, "act_buf: format '%s': expected type %d",	\
		    format, mem_type);					\
		sp--;							\
		break;							\
	}

#define ACT_FLAGS(flags)	((flags) | (inside_wform > 0 ? ACT_NOFIXSH : 0))

#define CHAR_ARG(vch)							\
	{								\
		if (vch == NULL) {					\
			i = GETMSG("somebody", opt->to_lang);		\
		} else {						\
			CHECK_TYPE(vch, MT_CHAR);			\
			i = PERS(vch, to, opt->to_lang,			\
				 ACT_FLAGS(opt->act_flags));		\
		}							\
	}

#define OBJ_ARG(obj)							\
	{								\
		if (obj == NULL) {					\
			i = GETMSG("Nothing", opt->to_lang);		\
		} else {						\
			CHECK_TYPE(obj, MT_OBJ);			\
			i = act_format_obj(obj,	to, opt->to_lang,	\
					   ACT_FLAGS(opt->act_flags));	\
		}							\
	}

#define TEXT_ARG(text, flags)						\
	{								\
		CHECK_STRING(text);					\
		i = act_format_text(text, ch, to, opt->to_lang,		\
				    ACT_FLAGS(flags));			\
	}

#define GMLTEXT_ARG(gml, flags)						\
	{								\
		CHECK_GMLSTR(gml);					\
		i = act_format_mltext(&(gml)->ml, ch, to, opt->to_lang,	\
				      ACT_FLAGS(flags));		\
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
 * $d - door_name(arg1)
 * $D
 * $e - he_she(ch)
 * $E - he_she(vch)
 * $f - $fnn{...} - misc formatting
 * $F - $Fnn{...} - ------//-------
 *		$fnn formats string with "%nns" format
 *		$Fnn formats string with "%nn.nns" format
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
 * $K - num(arg2)
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
 * $v - mltext(arg1)
 * $V - mltext(arg3)
 * $w - glob_lookup(arg1)
 * $W - glob_lookup(arg3)
 * $x
 * $X
 * $y
 * $Y
 * $z
 * $Z
 * ${ - "{{"
 * $} - "}"
 * $$ - "$"
 *
 */

#define TSTACK_SZ 4

#define VCH	(CAST(CHAR_DATA *, arg2))
#define VCH1	(CAST(CHAR_DATA *, arg1))
#define VCH3	(CAST(CHAR_DATA *, arg3))
#define NUM1	((int) arg1)
#define NUM2	((int) arg2)
#define NUM3	((int) arg3)
#define ROOM1	((const ROOM_INDEX_DATA *) arg1)
#define ROOM3	((const ROOM_INDEX_DATA *) arg3)
#define OBJ1	(CAST(OBJ_DATA *, arg1))
#define OBJ2	(CAST(OBJ_DATA *, arg2))
#define GML1	((const gmlstr_t *) arg1)
#define GML3	((const gmlstr_t *) arg3)

void
act_buf(const char *format, CHAR_DATA *ch, CHAR_DATA *to,
	const void *arg1, const void *arg2, const void *arg3,
	actopt_t *opt, char *buf, size_t buf_len)
{
	char		tmp	[MAX_STRING_LENGTH];
	char		tmp2	[MAX_STRING_LENGTH];
	const gmlstr_t	*gml;

	char *		point = buf;
	const char *	s;

	struct tdata	tstack[TSTACK_SZ];
	int		sp = -1;
	int		inside_wform = 0;

	int		actq;

	s = format = GETMSG(format, opt->to_lang);
	while (*s) {
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
				strlcpy(tstack[sp].p,
				    word_form(tstack[sp].p,
					tstack[sp].arg,
					opt->to_lang, rulecl),
				    buf_len - 3 - (tstack[sp].p - buf));
				point = strchr(tstack[sp].p, '\0');
				inside_wform--;
				break;
			}

			case 'f':
			case 'F':
				*point = '\0';
				snprintf(tmp2, sizeof(tmp2),
				    tstack[sp].type == 'f' ?
					"%%%ds" : "%%%d.%ds",	// notrans
				    tstack[sp].arg, abs(tstack[sp].arg));
				snprintf(tmp, sizeof(tmp), tmp2, tstack[sp].p);
				strlcpy(tstack[sp].p, tmp,
				    buf_len - 3 - (tstack[sp].p - buf));
				point = strchr(tstack[sp].p, '\0');
				break;

			case 'l':
				if (tstack[sp].arg == 'u')
					cstrtoupper(tstack[sp].p);
				break;
			}

			sp--;
			continue;

		case '$':
			s++;

			switch (code = *s++) {
			default:
				i = " <@@@> ";		// notrans
				log(LOG_INFO, "act_buf: '%s': bad code $%c",
					   format, code);
				continue;

			case '$':
				i = "$";		// notrans
				break;

			case '{':
				i = "{{";		// notrans
				break;

			case '}':
				i = "}";		// notrans
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

			case 'd':
				gml = act_format_door(GML1);
				GMLTEXT_ARG(gml,
					    opt->act_flags & ~ACT_STRANS);
				break;

			case 'v':
				GMLTEXT_ARG(GML1,
					    opt->act_flags & ~ACT_STRANS);
				break;

			case 'V':
				GMLTEXT_ARG(GML3,
					    opt->act_flags & ~ACT_STRANS);
				break;

			case 'w':
				gml = glob_lookup(arg1);
				GMLTEXT_ARG(gml, opt->act_flags & ~ACT_STRANS);
				break;

			case 'W':
				gml = glob_lookup(arg3);
				GMLTEXT_ARG(gml, opt->act_flags & ~ACT_STRANS);
				break;

/* room arguments */
			case 'r':
				CHECK_TYPE(ROOM1, MT_ROOM);
				i = mlstr_mval(&ROOM1->name);
				break;

			case 'R':
				CHECK_TYPE(ROOM3, MT_ROOM);
				i = mlstr_mval(&ROOM3->name);
				break;

/* char arguments */
			case 'n':
				CHAR_ARG(ch);
				break;

			case 'N':
				CHAR_ARG(VCH);
				break;

			case 'i':
				CHAR_ARG(VCH1);
				break;

			case 'I':
				CHAR_ARG(VCH3);
				break;

/* numeric arguments */
			case 'j':
				snprintf(tmp, sizeof(tmp), "%d", NUM1);
				i = tmp;
				break;

			case 'J':
				snprintf(tmp, sizeof(tmp), "%d", NUM3);
				i = tmp;
				break;

			case 'K':
				snprintf(tmp, sizeof(tmp), "%d", NUM2);
				i = tmp;
				break;

/* him/her arguments. obsolete. $gx{...} should be used instead */
			case 'e':
				CHECK_TYPE(ch, MT_CHAR);
				i = he_she[PERS_SEX(ch, to, opt->to_lang)];
				break;

			case 'E':
				CHECK_TYPE(VCH, MT_CHAR);
				i = he_she[PERS_SEX(VCH, to, opt->to_lang)];
				break;

			case 'm':
				CHECK_TYPE(ch, MT_CHAR);
				i = him_her[PERS_SEX(ch, to, opt->to_lang)];
				break;

			case 'M':
				CHECK_TYPE(VCH, MT_CHAR);
				i = him_her[PERS_SEX(VCH, to, opt->to_lang)];
				break;

			case 's':
				CHECK_TYPE(ch, MT_CHAR);
				i = his_her[PERS_SEX(ch, to, opt->to_lang)];
				break;

			case 'S':
				CHECK_TYPE(VCH, MT_CHAR);
				i = his_her[PERS_SEX(VCH, to, opt->to_lang)];
				break;

/* obj arguments */
			case 'p':
				OBJ_ARG(OBJ1);
				break;

			case 'P':
				OBJ_ARG(OBJ2);
				break;

/* $gx{...}, $cx{...}, $qx{...} arguments */
			case 'g':
			case 'c':
			case 'q':
			case 'f':
			case 'l':
			case 'F':
				if (++sp >= TSTACK_SZ) {
					log(LOG_INFO, "act_raw: '%s': tstack overflow", format);
					continue;
				}

				tstack[sp].p = point;
				tstack[sp].type = code;

				switch (code) {
				case 'f':
				case 'F': {
					char *pp;
					tstack[sp].arg = strtol(s, &pp, 10);
					s = pp;
					break;
				}

				case 'l':
					switch (subcode = *s++) {
					case 'u':
						tstack[sp].arg = subcode;
						break;
					default:
						log(LOG_INFO, "act_buf: '%s': bad subcode '%c' (pos %d)",
						    format, subcode,
						    s - format);
						sp--;
						continue;
					}
					break;

				case 'c':
					tstack[sp].arg = *s++ - '0';
					inside_wform++;
					break;

				case 'g':
					switch (subcode = *s++) {
					case 'N':
						CHECK_TYPE2(VCH, MT_CHAR);
						tstack[sp].arg =
							PERS_SEX(VCH, to, opt->to_lang);
						break;

					case 'n':
						CHECK_TYPE2(ch, MT_CHAR);
						tstack[sp].arg =
							PERS_SEX(ch, to, opt->to_lang);
						break;

					case 'i':
						CHECK_TYPE2(VCH1, MT_CHAR);
						tstack[sp].arg =
							PERS_SEX(VCH1, to, opt->to_lang);
						break;

					case 'I':
						CHECK_TYPE2(VCH3, MT_CHAR);
						tstack[sp].arg =
							PERS_SEX(VCH3, to, opt->to_lang);
						break;

					case 'o':
						tstack[sp].arg = opt->to_sex;
						break;

					case 'p':
						CHECK_TYPE2(OBJ1, MT_OBJ);
						tstack[sp].arg = GET_SEX(&OBJ1->pObjIndex->gender, opt->to_lang);
						break;

					case 'P':
						CHECK_TYPE2(OBJ2, MT_OBJ);
						tstack[sp].arg = GET_SEX(&OBJ2->pObjIndex->gender, opt->to_lang);
						break;

					case 'd':
						gml = act_format_door(GML1);
						CHECK_GMLSTR2(gml);
						tstack[sp].arg = GET_SEX(&gml->gender, opt->to_lang);
						break;

					case 'v':
						CHECK_GMLSTR2(GML1);
						tstack[sp].arg = GET_SEX(&GML1->gender, opt->to_lang);
						break;

					case 'V':
						CHECK_GMLSTR2(GML3);
						tstack[sp].arg = GET_SEX(&GML3->gender, opt->to_lang);
						break;

					case 'w':
						gml = glob_lookup(arg1);
						CHECK_GMLSTR2(gml);
						tstack[sp].arg = GET_SEX(&gml->gender, opt->to_lang);
						break;

					case 'W':
						gml = glob_lookup(arg3);
						CHECK_GMLSTR2(gml);
						tstack[sp].arg = GET_SEX(&gml->gender, opt->to_lang);
						break;

					case '0':
					case '1':
					case '2':
					case '3':
						tstack[sp].arg = subcode - '0';
						break;

					default:
						log(LOG_INFO, "act_buf: '%s': bad subcode '%c' (pos %d)",
						    format, subcode,
						    s - format);
						sp--;
						continue;
					}
					inside_wform++;
					break;

				case 'q':
					switch(subcode = *s++) {
					case 'j':
						tstack[sp].arg = NUM1;
						break;

					case 'K':
						tstack[sp].arg = NUM2;
						break;

					case 'J':
						tstack[sp].arg = NUM3;
						break;
					default:
						log(LOG_INFO, "act_buf: '%s': bad subcode '%c' (pos %d)",
						    format, subcode,
						    s - format);
						sp--;
						continue;
					}
					inside_wform++;
					break;
				}

				if (*s != '{') {
					log(LOG_INFO, "act_buf: '%s': syntax error (pos %d)",
					    format, s - format);
					sp--;
					continue;
				}

				s++;
				continue;
			}

			if (i != NULL) {
				while (point < buf + buf_len - 3 && *i)
					*point++ = *i++;
			}
			break;
		}
	}

	if (!IS_SET(opt->act_flags, ACT_NOLF))
		*point++ = '\n';
	*point = '\0';

/* first non-control char is uppercased */
	if (!IS_SET(opt->act_flags, ACT_NOUCASE))
		cstrtoupper(buf);

	if ((actq = ACTQ(opt->act_flags)) != 0) {
		msgq_t *msgq = NULL;
		clan_t *clan;

		if (actq == ACTQ_SAY && !IS_NPC(to))
			msgq = &PC(to)->msgq_say;
		else if (actq == ACTQ_TELL && !IS_NPC(to))
			msgq = &PC(to)->msgq_tell;
		else if (actq == ACTQ_GROUP && !IS_NPC(to))
			msgq = &PC(to)->msgq_group;
		else if (actq == ACTQ_CLAN &&
			 (clan = clan_lookup(ch->clan)) != NULL)
			msgq = &clan->msgq_clan;
		else if (actq == ACTQ_SOG && !IS_NPC(to))
			msgq = &PC(to)->msgq_sog;
		else if (actq == ACTQ_CHAN && !IS_NPC(to))
			msgq = &PC(to)->msgq_chan;
		else if (actq == ACTQ_IMMTALK)
			msgq = &msgq_immtalk;
		else {
			log(LOG_INFO,
			    "act_buf: %d: invalid actq (act_flags = 0x%08x)\n",
			    actq, opt->act_flags);
		}

		if (msgq != NULL)
			msgq_add(msgq, buf);
	}
}

static CHAR_DATA *
act_args(CHAR_DATA *ch, CHAR_DATA *vch, int act_flags)
{
	if (ACT_TO(act_flags) == TO_CHAR)
		return ch;

	if (ACT_TO(act_flags) == TO_VICT)
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

	if (ACT_TO(act_flags) == TO_CHAR && to != ch)
		return TRUE;
	if (ACT_TO(act_flags) == TO_VICT && (to != vch || to == ch))
		return TRUE;
	if (ACT_TO(act_flags) == TO_ROOM && to == ch)
		return TRUE;
	if (ACT_TO(act_flags) == TO_NOTVICT && (to == ch || to == vch))
		return TRUE;

	if (IS_NPC(to)
	&&  to->desc == NULL)
#if 0
	XXX MPC ACT
	&&  !HAS_TRIGGER(to, TRIG_ACT))
#endif
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
	if (IS_SET(act_flags, ACT_NODEAF)
	&&  !IS_IMMORTAL(to)
	&&  is_sn_affected(to, "deafen"))
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
	opt.to_sex = GET_SEX(&to->gender, opt.to_lang);
	opt.act_flags = act_flags;

	act_buf(format, ch, to, arg1, arg2, arg3,
		&opt, buf, sizeof(buf));
	parse_colors(buf, tmp, sizeof(tmp), OUTPUT_FORMAT(to));

	if (!IS_NPC(to)) {
		if ((IS_SET(to->comm, COMM_AFK) || to->desc == NULL)
		&&  IS_SET(act_flags, ACT_TOBUF))
			buf_append(PC(to)->buffer, tmp);
		else if (to->desc) {
			if (IS_SET(to->comm, COMM_QUIET_EDITOR)
			&&  to->desc->pString
			&&  !IS_SET(act_flags, ACT_SEDIT))
				buf_append(PC(to)->buffer, tmp);
			else
				write_to_buffer(to->desc, tmp, 0);
		}
	} else {
#if 0
		XXX MPC ACT
		if (!IS_SET(act_flags, ACT_NOTRIG))
			mp_act_trigger(tmp, to, ch, arg1, arg2, TRIG_ACT);
#endif
		if (to->desc)
			write_to_buffer(to->desc, tmp, 0);
	}
}

void
act_puts3(const char *format, CHAR_DATA *ch,
	  const void *arg1, const void *arg2, const void *arg3,
	  int act_flags, int min_pos)
{
	CHAR_DATA *to;
	CHAR_DATA *vch = CAST(CHAR_DATA *, arg2);

	if (IS_NULLSTR(format)
	||  (to = act_args(ch, vch, act_flags)) == NULL)
		return;

	if (ACT_TO(act_flags) == TO_CHAR
	||  ACT_TO(act_flags) == TO_VICT) {
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

void
act_mlputs3(mlstring *mlformat, CHAR_DATA *ch,
	    const void *arg1, const void *arg2, const void *arg3,
	    int act_flags, int min_pos)
{
	CHAR_DATA *to;
	CHAR_DATA *vch = CAST(CHAR_DATA *, arg2);

	if (mlstr_null(mlformat)
	||  (to = act_args(ch, vch, act_flags)) == NULL)
		return;

	if (IS_SET(act_flags, TO_CHAR | TO_VICT)) {
		if (!act_skip(ch, vch, to, act_flags, min_pos)) {
			act_raw(ch, to, arg1, arg2, arg3,
				mlstr_cval(mlformat, to), act_flags);
		}
		return;
	}

	for(; to; to = to->next_in_room) {
		if (act_skip(ch, vch, to, act_flags, min_pos))
			continue;

		act_raw(ch, to, arg1, arg2, arg3,
			mlstr_cval(mlformat, to), act_flags);
	}
}

void
act(const char *format, CHAR_DATA *ch,
    const void *arg1, const void *arg2, int act_flags)
{
	act_puts(format, ch, arg1, arg2, act_flags, POS_RESTING);
}

void
act_char(const char *format, CHAR_DATA *ch)
{
	act_puts(format, ch, NULL, NULL, TO_CHAR | ACT_NOUCASE, POS_DEAD);
}

void
act_puts(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	 int act_flags, int min_pos)
{
	act_puts3(format, ch, arg1, arg2, NULL, act_flags, min_pos);
}

void
act_mlputs(mlstring *mlformat, CHAR_DATA *ch,
	   const void *arg1, const void *arg2, int act_flags, int min_pos)
{
	act_mlputs3(mlformat, ch, arg1, arg2, NULL, act_flags, min_pos);
}

bool
buf_act3(BUFFER *buffer, int where, const char *format, CHAR_DATA *ch,
	 const void *arg1, const void *arg2, const void *arg3, int act_flags)
{
	actopt_t opt;
	char tmp[MAX_STRING_LENGTH];

	opt.to_lang = buf_lang(buffer);
	opt.act_flags = act_flags;

	act_buf(format, ch, ch, arg1, arg2, arg3, &opt, tmp, sizeof(tmp));
	return buf_copy(buffer, where, tmp);
}

bool
buf_act(BUFFER *buffer, int where, const char *format, CHAR_DATA *ch,
	const void *arg1, const void *arg2, int act_flags)
{
	return buf_act3(buffer, where, format, ch, arg1, arg2, NULL, act_flags);
}

const char *
act_speech(CHAR_DATA *ch, CHAR_DATA *vch, const char *text, const void *arg)
{
	static char buf[MAX_STRING_LENGTH];
	actopt_t opt;

	opt.to_lang = GET_LANG(vch);
	opt.act_flags = ACT_NOUCASE | ACT_NOLF;

	act_buf(text, ch, ch, NULL, arg, NULL, &opt, buf, sizeof(buf));
	return buf;
}

void
act_yell(CHAR_DATA *ch, const char *text, const void *arg, const char *format)
{
	DESCRIPTOR_DATA *d;

	if (format == NULL)
		format = "$n yells '{M$t{x'";

	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *vch = d->character;

		if (d->connected != CON_PLAYING
		||  vch == ch
		||  vch->in_room->area != ch->in_room->area
		||  (IS_SET(vch->in_room->room_flags, ROOM_SILENT) &&
		    !IS_IMMORTAL(vch)))
			continue;

		act_puts(format, ch, act_speech(ch, vch, text, arg), vch,
			 TO_VICT | ACT_SPEECH(ch), POS_DEAD);
	}
}

void
act_clan(CHAR_DATA *ch, const char *text, const void *arg)
{
	CHAR_DATA *vch;

	for (vch = char_list; vch; vch = vch->next) {
		if (vch == ch
		||  !IS_CLAN(vch->clan, ch->clan)
		||  IS_SET(vch->chan, CHAN_NOCLAN)
		||  (IS_SET(vch->in_room->room_flags, ROOM_SILENT) &&
		    !IS_IMMORTAL(vch)))
			continue;

		act_puts("[CLAN] $lu{$n}: {C$t{x", ch,
		    act_speech(ch, vch, text, arg), vch,
		    TO_VICT | ACT_TOBUF | (ACT_SPEECH(ch) & ~ACT_STRANS),
		    POS_DEAD);
	}
}

void
act_say(CHAR_DATA *ch, const char *text, const void *arg)
{
	CHAR_DATA *vch;

	act_puts("You say '{G$t{x'", ch,
		 act_speech(ch, ch, text, arg), NULL,
		 TO_CHAR | ACTQ_SAY | ACT_SPEECH(ch), POS_DEAD);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch == ch)
			continue;
		act_puts("$n says '{G$t{x'", ch,
		    act_speech(ch, vch, text, arg), vch,
		    TO_VICT | ACTQ_SAY | ACT_TOBUF | ACT_NOTWIT | ACT_SPEECH(ch),
		     POS_RESTING);
	}
}

void
act_around(ROOM_INDEX_DATA *room, const char *text, const void *arg)
{
	int door;

	for (door = 0; door < MAX_DIR; door++) {
		EXIT_DATA *pexit;

		if ((pexit = room->exit[door]) != NULL
		&&  pexit->to_room.r != NULL
		&&  pexit->to_room.r != room)
			act(text, pexit->to_room.r->people, NULL, arg, TO_ALL);
	}
}

void
wiznet(const char *msg, CHAR_DATA *ch, const void *arg,
       flag_t flag, flag_t flag_skip, int min_level)
{
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *vch = d->original ? d->original : d->character;

		if (d->connected != CON_PLAYING
		||  vch->level < LEVEL_IMMORTAL
		||  !IS_SET(PC(vch)->wiznet, WIZ_ON)
		||  (flag && !IS_SET(PC(vch)->wiznet, flag))
		||  (flag_skip && IS_SET(PC(vch)->wiznet, flag_skip))
		||  vch->level < min_level
		||  vch == ch)
			continue;

		if (IS_SET(PC(vch)->wiznet, WIZ_PREFIX)) {
			act_puts("--> ", d->character,		// notrans
				 NULL, NULL, TO_CHAR | ACT_NOLF,
				 POS_DEAD);
		}
		act_puts(msg, d->character, arg, ch,
			 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	}
}

void
yell(CHAR_DATA *victim, CHAR_DATA *ch, const char *text)
{
	if (IS_NPC(victim)
	||  IS_IMMORTAL(victim)
	||  victim->position <= POS_SLEEPING
	||  IS_EXTRACTED(victim)
	||  IS_SET(PC(victim)->plr_flags, PLR_GHOST))
		return;

	act_puts("You yell '{M$t{x'", victim,
		 act_speech(victim, victim, text, ch), NULL,
		 TO_CHAR | ACT_SPEECH(ch), POS_DEAD);
	act_yell(victim, text, ch, "$n yells in panic '{M$t{x'");
}

void
tell_char(CHAR_DATA *ch, CHAR_DATA *victim, const char *msg)
{
	if (ch == victim) {
		act_char("Talking to yourself, eh?", ch);
		return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOSPEAK)) {
		act("You can't speak in this form.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOTELL)) {
		act_char("Your message didn't get through.", ch);
		return;
	}

	if (victim == NULL
	|| (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		act_char("They aren't here.", ch);
		return;
	}

	if (!IS_IMMORTAL(ch)
	&&  !IS_IMMORTAL(victim)) {
		if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)) {
			act_char("You are in silent room, you can't tell.", ch);
			return;
		}

		if (IS_SET(victim->in_room->room_flags, ROOM_SILENT)) {
			act_puts("$E is in silent room.", ch, 0, victim,
				 TO_CHAR, POS_DEAD);
			return;
		}

		if (IS_SET(victim->comm, COMM_QUIET | COMM_DEAF)) {
			act_puts("$E is not receiving tells.", ch, 0, victim,
				 TO_CHAR, POS_DEAD);
			return;
		}
	}

	msg = garble(ch, msg);
	act_puts("You tell $N '{G$t{x'",
	    ch, msg, victim, TO_CHAR | ACTQ_TELL | ACT_SPEECH(ch), POS_DEAD);
	act_puts("$n tells you '{G$t{x'",
	    ch, msg, victim,
	    TO_VICT | ACTQ_TELL | ACT_TOBUF | ACT_NOTWIT | ACT_SPEECH(ch),
	    POS_SLEEPING);

	if (IS_NPC(ch))
		return;

	if (IS_NPC(victim)) {
		pull_mob_trigger(
		    TRIG_MOB_SPEECH, victim, ch, CAST(void *, msg));
		if (IS_EXTRACTED(ch) || IS_EXTRACTED(victim))
			return;

		pull_mob_trigger(
		    TRIG_MOB_TELL, victim, ch, CAST(void *, msg));
		if (IS_EXTRACTED(ch) || IS_EXTRACTED(victim))
			return;
	} else {
		if (!IS_IMMORTAL(victim)
		&&  !IS_IMMORTAL(ch)
		&&  is_name(ch->name, PC(victim)->twitlist))
			return;

		if (victim->desc == NULL)
			act_puts("$N seems to have misplaced $S link but "
				 "your tell will go through if $E returns.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		else if (IS_SET(victim->comm, COMM_AFK))
			act_puts("$E is AFK, but your tell will go through "
				 "when $E returns.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		PC(victim)->reply = ch;
		PC(ch)->retell = victim;
	}
}

const char *
garble(CHAR_DATA *ch, const char *i)
{
	static char not_garbled[] = "?!()[]{},.:;'\" ";		// notrans
	static char buf[MAX_STRING_LENGTH];
	char *o;

	if (!is_sn_affected(ch, "garble"))
		return i;

	for (o = buf; *i && o < buf + sizeof(buf) - 1; i++, o++) {
		if (strchr(not_garbled, *i))
			*o = *i;
		else
			*o = number_range(' ', 254);
	}
	*o = '\0';
	return buf;
}
