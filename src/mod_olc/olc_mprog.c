/*
 * $Id: olc_mprog.c,v 1.3 2001-08-27 16:53:17 fjoe Exp $
 */

#include <sys/types.h>
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

	{ "name",	mped_name,	NULL,		&strkey_mprogs	},
	{ "flags",	mped_flags,	NULL,		mprog_flags	},
	{ "code",	mped_code,	NULL,		NULL		},
	{ "compile",	mped_compile,	NULL,		NULL		},
	{ "dump",	mped_dump,	NULL,		NULL		},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

static DECLARE_FOREACH_CB_FUN(save_mprog_cb);
static void mprog_update_type(CHAR_DATA *ch, mprog_t *mp);

OLC_FUN(mped_create)
{
	mprog_t *mp;
	mprog_t mprog;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_MPROG) {
		act_char("MProgEd: Insufficient security for creatinig mprogs.", ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	/*
	 * olced_busy check is not needed since hash_insert
	 * adds new elements to the end of varr
	 */

	mprog_init(&mprog);
	mprog.name	= str_dup(arg);
	mp = hash_insert(&mprogs, mprog.name, &mprog);

	if (mp == NULL) {
		act_puts("MProgEd: $t: already exists.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	mp->text = str_dup(
	    "/*\n"
	    " * $Id: olc_mprog.c,v 1.3 2001-08-27 16:53:17 fjoe Exp $\n"
	    " */\n"
	    "\n");
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

	if ((mp = mprog_search(argument)) == NULL) {
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

	olc_printf(ch, "Saved mprogs:");
	hash_foreach(&mprogs, save_mprog_cb, ch, &found);
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
		if ((mp = mprog_search(argument)) == NULL) {
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
		buf_append(output, mp->text);
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(mped_list)
{
	BUFFER *buffer = buf_new(0);
	strkey_printall(&mprogs, buffer);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
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

	if (olced_str_text(ch, argument, cmd, &mp->text)) {
		mp->status = MP_S_DIRTY;
		return TRUE;
	}

	return FALSE;
}

OLC_FUN(mped_compile)
{
	mprog_t *mp;
	EDIT_MPROG(ch, mp);

	if (mprog_compile == NULL) {
		act_char("Module mod_mpc is not loaded.", ch);
		return FALSE;
	}

	if (mprog_compile(mp) < 0) {
		page_to_char(buf_string(mp->errbuf), ch);
		return FALSE;
	}

	act_char("Ok.", ch);
	return FALSE;
}

OLC_FUN(mped_dump)
{
	return FALSE;
}

bool
touch_mprog(mprog_t *mprog)
{
	SET_BIT(mprog->flags, MP_F_CHANGED);
	return TRUE;
}

/*--------------------------------------------------------------------
 * local functions
 */

static
FOREACH_CB_FUN(save_mprog_cb, p, ap)
{
	mprog_t *mp = (mprog_t *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	bool *pfound = va_arg(ap, bool *);

	FILE *fp;
	const char *filename;

	if (!IS_SET(mp->flags, MP_F_CHANGED))
		return NULL;

	filename = strkey_filename(mp->name, MPC_EXT);
	if ((fp = olc_fopen(MPC_PATH, filename, ch, -1)) == NULL)
		return NULL;

	REMOVE_BIT(mp->flags, MP_F_CHANGED);
	fwrite(mp->text, strlen(mp->text), 1, fp);
	fclose(fp);

	olc_printf(ch, "    %s (%s)", mp->name, filename);
	*pfound = TRUE;
	return NULL;
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
