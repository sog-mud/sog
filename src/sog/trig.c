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
 * $Id: trig.c,v 1.41 2004-06-28 19:21:11 tatyana Exp $
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
static bool trig_fread_prog(trig_t *trig, int mp_type, const char *mp_name,
			    rfile_t *fp);

void
trig_init(trig_t *trig)
{
	trig->trig_type = TRIG_NONE;
	trig->trig_prog = str_empty;
	trig->trig_arg = str_empty;
	trig->trig_flags = 0;
	trig->trig_extra = NULL;
	trig->trig_paf = NULL;
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

static void
trig_cpy(trig_t *dst, trig_t *src)
{
	dst->trig_type = src->trig_type;
	dst->trig_prog = str_qdup(src->trig_prog);
	trig_set_arg(dst, str_qdup(src->trig_arg));
	dst->trig_paf = src->trig_paf;
}

void
trig_fread(trig_t *trig, int mp_type, const char *mp_name, rfile_t *fp)
{
	trig->trig_type = fread_fword(mptrig_types, fp);
	if (trig->trig_type < 0) {
		printlog(LOG_ERROR, "trig_fread: %s: unknown mptrig type",
		    rfile_tok(fp));
		fread_to_eol(fp);
		return;
	}

	trig_fread_prog(trig, mp_type, mp_name, fp);
}

bool
trig_fread_inline_prog(trig_t *trig, int mp_type, const char *mp_name,
		       rfile_t *fp)
{
	mprog_t *mp;

	if ((mp = (mprog_t *) c_insert(&mprogs, mp_name)) == NULL) {
		printlog(LOG_ERROR, "%s: %s: duplicate mprog",
		    __FUNCTION__, mp_name);
		return FALSE;
	}

	mp->name = str_dup(mp_name);
	mp->type = mp_type;
	mp->text = fread_string(fp);

	free_string(trig->trig_prog);
	trig->trig_prog = str_qdup(mp->name);
	return TRUE;
}

void
trig_fwrite(const char *pre, trig_t *trig, FILE *fp)
{
	const char *trig_prog = trig->trig_prog;

	if (trig_prog[0] == '@')
		trig_prog = "@";

	fprintf(fp, "%s %s %s %s~\n",
		pre, flag_string(mptrig_types, trig->trig_type),
		trig_prog, trig->trig_arg);

	if (trig_prog[0] == '@') {
		mprog_t *mp = mprog_lookup(trig->trig_prog);
		const char *text;

		if (mp == NULL) {
			printlog(LOG_ERROR, "%s: %s: mprog not found",
			    __FUNCTION__, trig->trig_prog);
			text = str_empty;
		} else
			text = mp->text;
		fwrite_string(fp, NULL, text);
	}
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

bool
trig_fread_list(varr *v, int mp_type, const char *mp_name, rfile_t *fp)
{
	trig_t *trig;

	int trig_type = fread_fword(mptrig_types, fp);
	if (trig_type < 0) {
		printlog(LOG_ERROR, "trig_fread: %s: unknown mptrig type",
		    rfile_tok(fp));
		fread_to_eol(fp);
		return FALSE;
	}

	trig = trig_new(v, trig_type);
	return trig_fread_prog(trig, mp_type, mp_name, fp);
}

void
trig_fwrite_list(const char *pre, varr *v, FILE *fp)
{
	trig_t *trig;

	C_FOREACH (trig_t *, trig, v)
		trig_fwrite(pre, trig, fp);
}

void
trig_dump_list(varr *v, BUFFER *buf)
{
	int cnt = 0;
	trig_t *trig;

	C_FOREACH (trig_t *, trig, v) {
		char buf2[MAX_INPUT_LENGTH];
		const char *trig_prog;

		if (cnt == 0) {
			buf_append(buf, "Num  Trigger     Program                    Arg [Flags]\n");
			buf_append(buf, "---- ----------- -------------------------- -----------------------------------\n");
		}

		if (trig->trig_prog[0] == '@') {
			const char *mp_status;
			mprog_t *mp;

			if ((mp = mprog_lookup(trig->trig_prog)) == NULL)
				mp_status = "not found";
			else {
				mp_status = flag_string(
				    mprog_states, mp->status);
			}

			snprintf(buf2, sizeof(buf2), "@inline: %s", mp_status);
			trig_prog = buf2;
		} else
			trig_prog = trig->trig_prog;

		buf_printf(buf, BUF_END, "[%2d] %-11s %-26s %s [%s]\n",
		    cnt, flag_string(mptrig_types, trig->trig_type),
		    trig_prog,
		    trig->trig_arg,
		    flag_string(mptrig_flags, trig->trig_flags));
		cnt++;
	}
}

trig_t *
trig_new(varr *v, int trig_type)
{
	int idx = 0;
	trig_t *t;

	C_FOREACH (trig_t *, t, v) {
		if (t->trig_type > trig_type) {
			idx = varr_index(v, t);
			break;
		}

		/*
		 * we hit the end
		 *
		 * c_size(v) > 1
		 */
		if (varr_index(v, t) == c_size(v) - 1) {
			idx = varr_index(v, t) + 1;
			break;
		}
	}

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
	if (!HAS_TEXT_ARG(trig->trig_type) || IS_NULLSTR(trig->trig_arg))
		return;

	if (trig_arg[0] == '#')
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
	printlog(LOG_ERROR, "invalid trigger arg '%s': %s",
	    trig->trig_arg, buf);
}

int
pull_mob_trigger(int trig_type,
		 CHAR_DATA *ch, CHAR_DATA *victim, void *arg)
{
	int rv = MPC_ERR_NOTFOUND, err;

	if (IS_NPC(ch)) {
		rv = err = pull_trigger_list(
		    trig_type, &ch->pMobIndex->mp_trigs, MP_T_MOB,
		    ch, victim, arg);
		if (err > 0)
			return err;
	}

	err = pull_trigger_list(
	    trig_type, &ch->mptrig_affected, MP_T_MOB, ch, victim, arg);
	if (err > 0)
		return err;

	return rv == 0 ? rv : err;
}

int
pull_obj_trigger(int trig_type,
		 OBJ_DATA *obj, CHAR_DATA *ch, void *arg)
{
	int rv, err;

	rv = err = pull_trigger_list(
	    trig_type, &obj->pObjIndex->mp_trigs, MP_T_OBJ, obj, ch, arg);
	if (err > 0)
		return err;

	err = pull_trigger_list(
	    trig_type, &obj->mptrig_affected, MP_T_OBJ, obj, ch, arg);
	if (err > 0)
		return err;

	return rv == 0 ? rv : err;
}

int
pull_room_trigger(int trig_type,
		  ROOM_INDEX_DATA *room, CHAR_DATA *ch, void *arg)
{
	int rv, err;

	rv = err = pull_trigger_list(
	    trig_type, &room->mp_trigs, MP_T_ROOM, room, ch, arg);
	if (err > 0)
		return err;

	err = pull_trigger_list(
	    trig_type, &room->mptrig_affected, MP_T_ROOM, room, ch, arg);
	if (err > 0)
		return err;

	return rv == 0 ? rv : err;
}

int
pull_spec_trigger(spec_t *spec, CHAR_DATA *ch,
		  const char *spn_rm, const char *spn_add)
{
	if (spec->mp_trig.trig_type == TRIG_NONE)
		return MPC_ERR_NOTFOUND;

	return pull_one_trigger(
	    &spec->mp_trig, MP_T_SPEC,
	    ch, CAST(void *, spn_rm), CAST(void *, spn_add));
}

bool
has_trigger(varr *v, int trig_type)
{
	return varr_bsearch(v, &trig_type, cmpint) != NULL;
}

const char *
genmpname_str(int mp_type, const char *str)
{
	static char buf[MAX_INPUT_LENGTH];

	snprintf(buf, sizeof(buf), "@%s$%s",
		 flag_string(mprog_types, mp_type), str);
	return buf;
}

const char *
genmpname_vnum(int mp_type, int vnum, int num)
{
	static char buf[MAX_INPUT_LENGTH];

	snprintf(buf, sizeof(buf), "@%s#%d#%d",
		 flag_string(mprog_types, mp_type), vnum, num);
	return buf;
}

const char *
format_mpname(mprog_t *mp)
{
	static char buf[MAX_STRING_LENGTH];
	const char *p, *q, *r;
	int vnum;
	AREA_DATA *a;

	if (mp->name[0] != '@')
		return mp->name;

	/* get type */
	p = mp->name + 1;
	if ((q = strchr(p, '#')) == NULL && (q = strchr(p, '$')) == NULL)
		return mp->name;
	strlncpy(buf, p, sizeof(buf), q - p);
	strlcat(buf, " ", sizeof(buf));
	if (*q == '$') {
		/* spec mprog */
		strlcat(buf, q + 1, sizeof(buf));
		return buf;
	}

	p = q;
	if ((q = strchr(p + 1, '#')) == NULL)
		return mp->name;
	r = strchr(buf, '\0');
	strlncat(buf, p, sizeof(buf), q - p);
	if ((vnum = atoi(r + 1)) == 0)
		return mp->name;
	if ((a = area_vnum_lookup(vnum)) != NULL) {
		strlcat(buf, " (", sizeof(buf));
		strlcat(buf, a->file_name, sizeof(buf));
		strlcat(buf, ")", sizeof(buf));
	}
	strlcat(buf, " trig ", sizeof(buf));
	strlcat(buf, q, sizeof(buf));
	return buf;
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
	void *arg5 = NULL;

	if (mprog_execute == NULL) {
#if 0
		printlog(LOG_ERROR, "%s: Module mod_mpc is not loaded",
		    __FUNCTION__);
#endif
		return MPC_ERR_UNLOADED;
	}

	if ((mp = mprog_lookup(trig->trig_prog)) == NULL) {
#if 0
		printlog(LOG_ERROR, "%s: %s: mprog not found",
		    __FUNCTION__, trig->trig_prog);
#endif
		return MPC_ERR_NOTFOUND;
	}

	if (mp->type != mp_type) {
#if 0
		printlog(LOG_ERROR,
		    "%s: %s: mprog is type `%s', type `%s' requested",
		    __FUNCTION__, trig->trig_prog,
		    flag_string(mprog_types, mp->type),
		    flag_string(mprog_types, mp_type));
#endif
		return MPC_ERR_TYPE_MISMATCH;
	}

	trig_arg = trig->trig_arg;
	if (mp->type == MP_T_MOB) {
		if (trig_arg[0] == '+')
			trig_arg++;
		else {
			CHAR_DATA *ch = (CHAR_DATA *) arg1;

			if (trig->trig_type != TRIG_MOB_FIGHT
			&&  trig->trig_type != TRIG_MOB_DEATH
			&&  trig->trig_type != TRIG_MOB_KILL
			&&  ch->position != ch->pMobIndex->default_pos)
				return MPC_ERR_COND_FAILED;
		}
	}

	/*
	 * NOTE: keep this in sync with trig_check_arg() in mod_olc/olc.c
	 */
	if (trig->trig_type == TRIG_MOB_BRIBE) {
		int silver_needed = atoi(trig_arg);
		int silver = (int) arg3;

		if (silver < silver_needed)
			return MPC_ERR_COND_FAILED;
		arg3 = NULL;
	} else if (HAS_TEXT_ARG(trig->trig_type)) {
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
		arg4 = arg3;
		arg3 = NULL;
	} else if (HAS_OBJ_ARG(trig->trig_type)) {
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
	} else if (HAS_CMD_ARG(trig->trig_type)) {
		const char *argument = arg3;
		char command[MAX_INPUT_LENGTH];

		argument = one_argument(argument, command, sizeof(command));
		if (!!str_prefix(command, trig_arg))
			return MPC_ERR_COND_FAILED;

		arg3 = NULL;
		arg4 = CAST(void *, argument);
	} else if (HAS_EXIT_ARG(trig->trig_type)) {
		if (!is_name(arg3, trig_arg))
			return MPC_ERR_COND_FAILED;
		arg4 = arg3;
		arg3 = NULL;
	} else {
		int chance = atoi(trig_arg);
		if (chance < number_percent())
			return MPC_ERR_COND_FAILED;
	}
	if (trig->trig_paf != NULL)
		arg5 = trig->trig_paf;

	return mprog_execute(mp, arg1, arg2, arg3, arg4, arg5);
}

static int
pull_trigger_list(int trig_type, varr *v, int mp_type,
		  void *arg1, void *arg2, void *arg3)
{
	trig_t *trig;
	int rv = MPC_ERR_NOTFOUND;
	bool seen_good = FALSE;
	varr vc;

	trig = varr_bsearch_lower(v, &trig_type, cmpint);
	if (trig == NULL)
		return MPC_ERR_NOTFOUND;

	/*
	 * Need to make trig list copy because affect triggers
	 * can be removed during mprog execution
	 */
	trig_init_list(&vc);
	VARR_EFOREACH(trig, trig, v) {
		trig_t *n;

		if (trig->trig_type != trig_type)
			break;

		n = varr_enew(&vc);
		trig_cpy(n, trig);
	}

	C_FOREACH (trig_t *, trig, &vc) {
		rv = pull_one_trigger(trig, mp_type, arg1, arg2, arg3);
		if (rv > 0)
			break;
		else if (rv == 0 && !seen_good)
			seen_good = TRUE;
	}
	trig_destroy_list(&vc);

	if (rv > 0)
		return rv;
	return seen_good ? 0 : rv;
}

static bool
trig_fread_prog(trig_t *trig, int mp_type, const char *mp_name, rfile_t *fp)
{
	trig->trig_prog = fread_sword(fp);
	trig_set_arg(trig, fread_string(fp));

	if (!str_cmp(trig->trig_prog, "@"))
		return trig_fread_inline_prog(trig, mp_type, mp_name, fp);
	else
		C_STRKEY_CHECK(__FUNCTION__, &mprogs, trig->trig_prog);

	return TRUE;
}
