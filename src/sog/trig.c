/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: trig.c,v 1.20 2001-09-15 19:23:39 fjoe Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <merc.h>
#include <mprog.h>
#include <rwfile.h>

static int pull_one_trigger(trig_t *trig, int mp_type,
			    void *arg1, void *arg2, void *arg3);
static int pull_trigger_list(int trig_type, varr *v, int mp_type,
			     void *arg1, void *arg2, void *arg3);

void
trig_init(trig_t *trig)
{
	trig->trig_type = TRIG_NONE;
	trig->trig_prog = str_empty;
	trig->trig_arg = str_empty;
	trig->trig_flags = 0;
	trig->trig_extra = NULL;
}

void
trig_destroy(trig_t *trig)
{
	free_string(trig->trig_prog);
	free_string(trig->trig_arg);

	if (IS_SET(trig->trig_flags, TRIG_F_REGEXP)) {
		regfree(trig->trig_extra);
		free(trig->trig_extra);
	}
}

void
trig_fread(trig_t *trig, rfile_t *fp)
{
	trig->trig_type = fread_fword(mptrig_types, fp);
	if (trig->trig_type < 0) {
		log(LOG_ERROR, "trig_fread: %s: unknown mptrig type",
		    rfile_tok(fp));
		fread_to_eol(fp);
		return;
	}

	trig->trig_prog = fread_strkey(fp, &mprogs);
	trig_set_arg(trig, fread_string(fp));
}

void
trig_fwrite(const char *pre, trig_t *trig, FILE *fp)
{
	fprintf(fp, "%s %s %s %s~\n",
		pre, flag_string(mptrig_types, trig->trig_type),
		trig->trig_prog, trig->trig_arg);
}

static varr_info_t c_info_trigs = {
	&varr_ops,

	(e_init_t) trig_init,
	(e_destroy_t) trig_destroy,

	sizeof(trig_t), 2
};

void
trig_init_list(varr *v)
{
	c_init(v, &c_info_trigs);
}

void
trig_destroy_list(varr *v)
{
	c_destroy(v);
}

void
trig_fread_list(varr *v, rfile_t *fp)
{
	trig_t *trig;

	int trig_type = fread_fword(mptrig_types, fp);
	if (trig_type < 0) {
		log(LOG_ERROR, "trig_fread: %s: unknown mptrig type",
		    rfile_tok(fp));
		fread_to_eol(fp);
		return;
	}

	trig = trig_new(v, trig_type);
	trig->trig_prog = fread_strkey(fp, &mprogs);
	trig_set_arg(trig, fread_string(fp));
}

static
FOREACH_CB_FUN(trig_fwrite_list_cb, p, ap)
{
	trig_t *trig = (trig_t *) p;

	const char *pre = va_arg(ap, const char *);
	FILE *fp = va_arg(ap, FILE *);

	trig_fwrite(pre, trig, fp);
	return NULL;
}

void
trig_fwrite_list(const char *pre, varr *v, FILE *fp)
{
	c_foreach(v, trig_fwrite_list_cb, pre, fp);
}

static
FOREACH_CB_FUN(trig_dump_list_cb, p, ap)
{
	trig_t *trig = (trig_t *) p;

	int *pcnt = va_arg(ap, int *);
	BUFFER *buf = va_arg(ap, BUFFER *);

	if (*pcnt == 0) {
		buf_append(buf, "Num  Trigger     Program                    Arg [Flags]\n");
		buf_append(buf, "---- ----------- -------------------------- -----------------------------------\n");
	}

	buf_printf(buf, BUF_END, "[%2d] %-11s %-26s %s [%s]\n",
		   *pcnt, flag_string(mptrig_types, trig->trig_type),
		   trig->trig_prog, trig->trig_arg,
		   flag_string(mptrig_flags, trig->trig_flags));
	(*pcnt)++;

	return NULL;
}

void
trig_dump_list(varr *v, BUFFER *buf)
{
	int cnt = 0;

	c_foreach(v, trig_dump_list_cb, &cnt, buf);
}

static
FOREACH_CB_FUN(find_greater_cb, p, ap)
{
	trig_t *t = (trig_t *) p;

	varr *v = va_arg(ap, varr *);
	int trig_type = va_arg(ap, int);

	if (t->trig_type > trig_type)
		return t;

	/*
	 * we hit the end
	 *
	 * c_size(v) > 1
	 */
	if (varr_index(v, t) == c_size(v) - 1)
		return t + 1;

	return NULL;
}

trig_t *
trig_new(varr *v, int trig_type)
{
	int idx = 0;
	trig_t *t;

	if ((t = c_foreach(v, find_greater_cb, v, trig_type)) != NULL)
		idx = varr_index(v, t);

	t = varr_insert(v, idx);
	t->trig_type = trig_type;
	return t;
}

void
trig_set_arg(trig_t *trig, const char *arg)
{
	const char *p;
	int errcode;
	int cflags;
	char buf[MAX_INPUT_LENGTH];
	const char *trig_arg;

	trig_arg = trig->trig_arg = arg;

	/*
	 * skip non-text arg triggers and empty args
	 */
	if (!HAS_TEXT_ARG(trig) || IS_NULLSTR(trig->trig_arg))
		return;

	if (trig_arg[0] == '+')
		trig_arg++;

	/*
	 * check if trigger arg is case-dependent
	 */
	for (p = trig_arg; *p; p++) {
		if (ISUPPER(*p)) {
			SET_BIT(trig->trig_flags, TRIG_F_CASEDEP);
			break;
		}
	}

	/*
	 * check if trigger arg is regexp
	 */
	if (trig_arg[0] != '*')
		return;

	SET_BIT(trig->trig_flags, TRIG_F_REGEXP);

	cflags = REG_EXTENDED | REG_NOSUB;
	if (!IS_SET(trig->trig_flags, TRIG_F_CASEDEP))
		cflags |= REG_ICASE;

	trig->trig_extra = malloc(sizeof(regex_t));
	errcode = regcomp(trig->trig_extra, trig_arg+1, cflags);
	if (!errcode)
		return;

	regerror(errcode, trig->trig_extra, buf, sizeof(buf));
	log(LOG_ERROR, "invalid trigger arg '%s': %s",
	    trig->trig_arg, buf);
}

int
pull_mob_trigger(int trig_type,
		 CHAR_DATA *ch, CHAR_DATA *victim, void *arg)
{
	if (!IS_NPC(ch))
		return MPC_ERR_NOTFOUND;

	return pull_trigger_list(
	    trig_type, &ch->pMobIndex->mp_trigs, MP_T_MOB, ch, victim, arg);
}

int
pull_obj_trigger(int trig_type,
		 OBJ_DATA *obj, CHAR_DATA *ch, void *arg)
{
	return pull_trigger_list(
	    trig_type, &obj->pObjIndex->mp_trigs, MP_T_OBJ, obj, ch, arg);
}

int
pull_spec_trigger(spec_t *spec, CHAR_DATA *ch,
		  const char *spn_rm, const char *spn_add)
{
	if (spec->mp_trig.trig_type == TRIG_NONE)
		return MPC_ERR_NOTFOUND;

	return pull_one_trigger(
	    &spec->mp_trig, MP_T_SPEC,
	    ch, (void *) (uintptr_t) spn_rm, (void *) (uintptr_t) spn_add);
}

/*--------------------------------------------------------------------
 * local functions
 */

static int
pull_one_trigger(trig_t *trig, int mp_type,
		 void *arg1, void *arg2, void *arg3)
{
	mprog_t *mp;
	const char *trig_arg;
	void *arg4 = NULL;

	if (mprog_execute == NULL)
		return MPC_ERR_UNLOADED;

	if ((mp = mprog_lookup(trig->trig_prog)) == NULL)
		return MPC_ERR_NOTFOUND;

	if (mp->type != mp_type)
		return MPC_ERR_TYPE_MISMATCH;

	trig_arg = trig->trig_arg;
	if (mp->type == MP_T_MOB
	&&  trig_arg[0] == '+') {
		CHAR_DATA *ch = (CHAR_DATA *) arg1;

		if (trig->trig_type != TRIG_MOB_FIGHT
		&&  trig->trig_type != TRIG_MOB_DEATH
		&&  trig->trig_type != TRIG_MOB_KILL
		&&  ch->position != ch->pMobIndex->default_pos)
			return MPC_ERR_COND_FAILED;

		trig_arg++;
	}

	if (trig->trig_type == TRIG_MOB_BRIBE) {
		int silver_needed = atoi(trig_arg);
		int silver = (int) arg3;

		if (silver < silver_needed)
			return MPC_ERR_COND_FAILED;
	} else if (HAS_TEXT_ARG(trig)) {
		char *arg_lwr = strlwr(arg3);
		bool match = FALSE;

		if (IS_SET(trig->trig_flags, TRIG_F_REGEXP))
			match = !regexec(trig->trig_extra, arg3, 0, NULL, 0);
		else {
			if (!IS_SET(trig->trig_flags, TRIG_F_CASEDEP))
				arg3 = arg_lwr;
			match = strstr(arg3, trig_arg) != NULL;
		}

		if (!match)
			return MPC_ERR_COND_FAILED;
	} else if (HAS_OBJ_ARG(trig)) {
		OBJ_DATA *obj = (OBJ_DATA *) arg3;
		bool match = FALSE;

		if (is_number(trig_arg))
			match = obj->pObjIndex->vnum == atoi(trig_arg);
		else {
			const char *p = obj->pObjIndex->name;
			char buf[MAX_STRING_LENGTH];

			/*
			 * one of obj names should match trig->arg
			 */
			while (*p) {
				p = one_argument(p, buf, sizeof(buf));
				if ((match = is_name(buf, trig_arg)))
					break;
			}
		}

		if (!match)
			return MPC_ERR_COND_FAILED;
	} else if (HAS_CMD_ARG(trig)) {
		const char *argument = arg3;
		char command[MAX_INPUT_LENGTH];

		argument = one_argument(argument, command, sizeof(command));
		if (!!str_prefix(command, trig_arg))
			return MPC_ERR_COND_FAILED;

		arg3 = NULL;
		arg4 = (void *) (uintptr_t) argument;
	} else if (HAS_EXIT_ARG(trig)) {
		if (!is_name(arg3, trig_arg))
			return MPC_ERR_COND_FAILED;
	} else {
		int chance;

		if (trig->trig_type == TRIG_MOB_GREET) {
			CHAR_DATA *ch = (CHAR_DATA *) arg1;

			if (trig->trig_type == TRIG_MOB_EXIT
			&&  ch->position != ch->pMobIndex->default_pos)
				return MPC_ERR_COND_FAILED;
		}

		chance = atoi(trig_arg);
		if (chance < number_percent())
			return MPC_ERR_COND_FAILED;
	}

	if (trig->trig_type != TRIG_MOB_GIVE
	&&  trig->trig_type != TRIG_SPEC)
		arg3 = NULL;

	return mprog_execute(mp, arg1, arg2, arg3, arg4);
}

static int
pull_trigger_list(int trig_type, varr *v, int mp_type,
		  void *arg1, void *arg2, void *arg3)
{
	trig_t *trig;
	int rv = MPC_ERR_NOTFOUND;
	size_t i;
	bool seen_good = FALSE;

	trig = varr_bsearch_lower(v, &trig_type, cmpint);
	if (trig == NULL)
		return MPC_ERR_NOTFOUND;

	for (i = varr_index(v, trig); i < c_size(v); i++) {
		trig = VARR_GET(v, i);

		if (trig->trig_type != trig_type)
			break;

		rv = pull_one_trigger(trig, mp_type, arg1, arg2, arg3);
		if (rv > 0)
			return rv;
		else if (rv == 0 && !seen_good)
			seen_good = TRUE;
	}

	if (seen_good)
		return 0;

	return rv;
}
