/*
 * $Id: olc_mprog.c,v 1.14 2003-05-08 10:39:44 fjoe Exp $
 */

#include <ctype.h>
#include <time.h>

#include "olc.h"

#include <mprog.h>

#define EDIT_MPROG(ch, mp)   (mp = (mprog_t *) ch->desc->pEdit)

/* Mobprog editor */
DECLARE_OLC_FUN(mped_create		);
DECLARE_OLC_FUN(mped_edit		);
DECLARE_OLC_FUN(mped_save		);
DECLARE_OLC_FUN(mped_touch		);
DECLARE_OLC_FUN(mped_show		);
DECLARE_OLC_FUN(mped_list		);

DECLARE_OLC_FUN(mped_name		);
DECLARE_OLC_FUN(mped_flags		);
DECLARE_OLC_FUN(mped_code		);
DECLARE_OLC_FUN(mped_compile		);
DECLARE_OLC_FUN(mped_dump		);
DECLARE_OLC_FUN(mped_where		);

static DECLARE_VALIDATE_FUN(validate_name);

olced_strkey_t strkey_mprogs = { &mprogs, MPC_PATH, MPC_EXT };

olc_cmd_t olc_cmds_mprog[] =
{
/*	{ command	function	}, */

	{ "create",	mped_create,	NULL,		NULL		},
	{ "edit",	mped_edit,	NULL,		NULL		},
	{ "",		mped_save,	NULL,		NULL		},
	{ "touch",	mped_touch,	NULL,		NULL		},
	{ "show",	mped_show,	NULL,		NULL		},
	{ "list",	mped_list,	NULL,		NULL		},

	{ "name",	mped_name,	validate_name,	&strkey_mprogs	},
	{ "flags",	mped_flags,	NULL,		mprog_flags	},
	{ "code",	mped_code,	NULL,		NULL		},
	{ "compile",	mped_compile,	NULL,		NULL		},
	{ "dump",	mped_dump,	NULL,		NULL		},
	{ "where",	mped_where,	NULL,		NULL		},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

static void mprog_update_type(CHAR_DATA *ch, mprog_t *mp);
static void mptrig_dump(BUFFER *buf, mprog_t *mp, varr *v, void *vo);

OLC_FUN(mped_create)
{
	mprog_t *mp;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_MPROG) {
		act_char("MProgEd: Insufficient security for creatinig mprogs.", ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if (arg[0] == '@') {
		act_puts("MProgEd: $t: invalid mprog name.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if ((mp = c_insert(&mprogs, arg)) == NULL) {
		act_puts("MProgEd: $t: already exists.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	mp->name = str_dup(arg);
	mp->text = str_printf(
	    "/*\n"
	    " * $%s$\n"
	    " */\n"
	    "\n", "Id");
	ch->desc->pEdit	= mp;
	OLCED(ch)	= olced_lookup(ED_MPROG);
	touch_mprog(mp);
	act_char("MProg created.", ch);

	mprog_update_type(ch, mp);
	return FALSE;
}

OLC_FUN(mped_edit)
{
	mprog_t *mp;

	if (PC(ch)->security < SECURITY_MPROG) {
		act_char("MProgEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if (argument[0] == '@'
	||  (mp = mprog_search(argument)) == NULL) {
		act_puts("MProgEd: $t: No such mprog.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= mp;
	OLCED(ch)	= olced_lookup(ED_MPROG);
	return FALSE;
}

OLC_FUN(mped_save)
{
	bool found = FALSE;
	mprog_t *mp;

	olc_printf(ch, "Saved mprogs:");

	C_FOREACH(mp, &mprogs) {
		FILE *fp;
		const char *filename;

		if (mp->name[0] == '@')
			continue;

		if (!IS_SET(mp->flags, MP_F_CHANGED))
			continue;

		filename = strkey_filename(mp->name, MPC_EXT);
		if ((fp = olc_fopen(MPC_PATH, filename, ch, -1)) == NULL)
			continue;

		REMOVE_BIT(mp->flags, MP_F_CHANGED);
		fwrite(mp->text, strlen(mp->text), 1, fp);
		fclose(fp);

		olc_printf(ch, "    %s (%s)", mp->name, filename);
		found = TRUE;
	}

	if (!found)
		olc_printf(ch, "    None.");
	return FALSE;
}

OLC_FUN(mped_touch)
{
	mprog_t *mp;
	EDIT_MPROG(ch, mp);
	touch_mprog(mp);
	return FALSE;
}

OLC_FUN(mped_show)
{
	BUFFER *output;
	mprog_t *mp;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_MPROG))
			EDIT_MPROG(ch, mp);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if (argument[0] == '@'
		||  (mp = mprog_search(argument)) == NULL) {
			act_puts("MProgEd: $t: No such mprog.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	output = buf_new(0);
	buf_printf(output, BUF_END, "Name:   [%s]\n", mp->name);
	buf_printf(output, BUF_END, "Type:   %s\n",
		   flag_string(mprog_types, mp->type));
	buf_printf(output, BUF_END, "Status: %s\n",
		   flag_string(mprog_states, mp->status));
	if (mp->flags) {
		buf_printf(output, BUF_END, "Flags:  [%s]\n",
			   flag_string(mprog_flags, mp->flags));
	}

	if (!IS_NULLSTR(mp->text)) {
		buf_append(output, "Code:\n");
		buf_append(output, strdump(mp->text, DL_COLOR));
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(mped_list)
{
	BUFFER *buf = buf_new(0);
	int col = 0;
	const char **p;

	C_FOREACH(p, &mprogs) {
		if ((*p)[0] == '@')
			continue;

		buf_printf(buf, BUF_END, "%-38.37s", *p);	// notrans

		if (++col % 2 == 0)
			buf_append(buf, "\n");
	}

	if (col % 2 != 0)
		buf_append(buf, "\n");

	page_to_char(buf_string(buf), ch);
	buf_free(buf);
	return FALSE;
}

OLC_FUN(mped_name)
{
	if (olced_strkey(ch, argument, cmd)) {
		mprog_t *mp;
		EDIT_MPROG(ch, mp);

		mprog_update_type(ch, mp);
		return TRUE;
	}

	return FALSE;
}

OLC_FUN(mped_flags)
{
	mprog_t *mp;
	EDIT_MPROG(ch, mp);

	olced_flag(ch, argument, cmd, &mp->flags);

	/* flags are not saved */
	return FALSE;
}

OLC_FUN(mped_code)
{
	mprog_t *mp;
	EDIT_MPROG(ch, mp);

	mp->status = MP_S_DIRTY;
	return olced_str_text(ch, argument, cmd, &mp->text);
}

OLC_FUN(mped_compile)
{
	mprog_t *mp;
	EDIT_MPROG(ch, mp);

	MPROG_COMPILE(ch, mp);
	return FALSE;
}

OLC_FUN(mped_dump)
{
	return FALSE;
}

OLC_FUN(mped_where)
{
	BUFFER *buf;
	int i;

	mprog_t *mp;
	EDIT_MPROG(ch, mp);

	buf = buf_new(-1);
	switch (mp->type) {
	case MP_T_MOB:
		for (i = 0; i < MAX_KEY_HASH; i++) {
			MOB_INDEX_DATA *pMob;

			for (pMob = mob_index_hash[i]; pMob != NULL; pMob = pMob->next)
				mptrig_dump(buf, mp, &pMob->mp_trigs, pMob);
		}
		break;
	case MP_T_OBJ:
		for (i = 0; i < MAX_KEY_HASH; i++) {
			OBJ_INDEX_DATA *pObj;

			for (pObj = obj_index_hash[i]; pObj != NULL; pObj = pObj->next)
				mptrig_dump(buf, mp, &pObj->mp_trigs, pObj);
		}
		break;
	case MP_T_ROOM:
		for (i = 0; i < MAX_KEY_HASH; i++) {
			ROOM_INDEX_DATA *pRoom;

			for (pRoom = room_index_hash[i]; pRoom != NULL; pRoom = pRoom->next)
				mptrig_dump(buf, mp, &pRoom->mp_trigs, pRoom);
		}
		break;

	default:
		buf_printf(buf, BUF_END,
		    "MprogEd: %s: invalid mprog type [%s] (%d).\n",
		    mp->name, flag_string(mprog_types, mp->type), mp->type);
		break;
	}

	if (IS_NULLSTR(buf_string(buf))) {
		buf_printf(buf, BUF_END, "MprogEd: %s: Not found anywhere.\n",
		    mp->name);
	}
	page_to_char(buf_string(buf), ch);
	buf_free(buf);
	return FALSE;
}

bool
touch_mprog(mprog_t *mprog)
{
	SET_BIT(mprog->flags, MP_F_CHANGED);
	return TRUE;
}

/*--------------------------------------------------------------------
 * static functions
 */

static
VALIDATE_FUN(validate_name)
{
	const char *name = (const char *) arg;

	if (name[0] == '@') {
		act_puts("MprogEd: $t: invalid mprog name.",
			 ch, name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	return TRUE;
}

static void
mprog_update_type(CHAR_DATA *ch, mprog_t *mp)
{
	char buf[MAX_INPUT_LENGTH];
	char *p;

	if ((p = strchr(mp->name, '_')) == NULL) {
		act_puts("MProgEd: $t: unable to determine mprog type (no underscores in name).",
			 ch, mp->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		mp->type = MP_T_NONE;
		return;
	}

	strnzncpy(buf, sizeof(buf), mp->name, (size_t) (p - mp->name));

	if ((mp->type = flag_svalue(mprog_types, buf)) < 0) {
		act_puts("MprogEd: $t: unknown type.",
			 ch, buf, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}

	act_puts("MProgEd: $t: type: $T.",
		 ch, mp->name, flag_string(mprog_types, mp->type),
		 TO_CHAR | ACT_NOTRANS, POS_DEAD);
}

static void
mptrig_dump(BUFFER *buf, mprog_t *mp, varr *v, void *vo)
{
	trig_t *t;
	bool found = FALSE;

	C_FOREACH(t, v) {
		if (!!strcmp(t->trig_prog, mp->name))
			continue;

		if (!found) {
			MOB_INDEX_DATA *pMob;
			OBJ_INDEX_DATA *pObj;
			ROOM_INDEX_DATA *pRoom;

			switch (mp->type) {
			case MP_T_MOB:
				pMob = (MOB_INDEX_DATA *) vo;
				buf_printf(buf, BUF_END,
				    "Mob  #%5d (%s)\n",
				    pMob->vnum, mlstr_mval(&pMob->short_descr));
				break;
			case MP_T_OBJ:
				pObj = (OBJ_INDEX_DATA *) vo;
				buf_printf(buf, BUF_END,
				    "Obj  #%5d (%s)\n",
				    pObj->vnum, mlstr_mval(&pObj->short_descr));
				break;
			case MP_T_ROOM:
				pRoom = (ROOM_INDEX_DATA *) vo;
				buf_printf(buf, BUF_END,
				    "Room #%5d (%s)\n",
				    pRoom->vnum, mlstr_mval(&pRoom->name));
				break;
			}
			found = TRUE;
		}

		buf_printf(buf, BUF_END,
		    "    Num: %2d  Arg: [%s]",
		    varr_index(v, t), t->trig_arg);
		if (t->trig_flags) {
			buf_printf(buf, BUF_END,
			    " Flags: [%s]",
			    flag_string(mptrig_flags, t->trig_flags));
		}
		buf_append(buf, "\n");
	}
}
