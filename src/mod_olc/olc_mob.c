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
 * $Id: olc_mob.c,v 1.97 2004-06-28 19:21:04 tatyana Exp $
 */

#include "olc.h"
#include <mprog.h>

#define EDIT_MOB(ch, mob)	(mob = (MOB_INDEX_DATA *) ch->desc->pEdit)

/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN(mobed_create		);
DECLARE_OLC_FUN(mobed_edit		);
DECLARE_OLC_FUN(mobed_touch		);
DECLARE_OLC_FUN(mobed_show		);
DECLARE_OLC_FUN(mobed_list		);

DECLARE_OLC_FUN(mobed_name		);
DECLARE_OLC_FUN(mobed_short		);
DECLARE_OLC_FUN(mobed_long		);
DECLARE_OLC_FUN(mobed_shop		);
DECLARE_OLC_FUN(mobed_desc		);
DECLARE_OLC_FUN(mobed_level		);
DECLARE_OLC_FUN(mobed_align		);

DECLARE_OLC_FUN(mobed_gender		);  /* ROM */
DECLARE_OLC_FUN(mobed_act		);  /* ROM */
DECLARE_OLC_FUN(mobed_mob		);  /* ROM */
DECLARE_OLC_FUN(mobed_affect		);  /* ROM */
DECLARE_OLC_FUN(mobed_invis		);
DECLARE_OLC_FUN(mobed_detect		);
DECLARE_OLC_FUN(mobed_ac		);  /* ROM */
DECLARE_OLC_FUN(mobed_form		);  /* ROM */
DECLARE_OLC_FUN(mobed_part		);  /* ROM */
DECLARE_OLC_FUN(mobed_material		);  /* ROM */
DECLARE_OLC_FUN(mobed_off		);  /* ROM */
DECLARE_OLC_FUN(mobed_size		);  /* ROM */
DECLARE_OLC_FUN(mobed_hitdice		);  /* ROM */
DECLARE_OLC_FUN(mobed_manadice		);  /* ROM */
DECLARE_OLC_FUN(mobed_damdice		);  /* ROM */
DECLARE_OLC_FUN(mobed_race		);  /* ROM */
DECLARE_OLC_FUN(mobed_startpos		);  /* ROM */
DECLARE_OLC_FUN(mobed_defaultpos	);  /* ROM */
DECLARE_OLC_FUN(mobed_gold		);  /* ROM */
DECLARE_OLC_FUN(mobed_hitroll		);  /* ROM */
DECLARE_OLC_FUN(mobed_damtype		);  /* ROM */
DECLARE_OLC_FUN(mobed_group		);  /* ROM */
DECLARE_OLC_FUN(mobed_trig		);  /* ROM */
DECLARE_OLC_FUN(mobed_prac		);
DECLARE_OLC_FUN(mobed_clan		);
DECLARE_OLC_FUN(mobed_clone		);
DECLARE_OLC_FUN(mobed_wizi		);
DECLARE_OLC_FUN(mobed_incog		);
DECLARE_OLC_FUN(mobed_fvnum		);
DECLARE_OLC_FUN(mobed_resist		);
DECLARE_OLC_FUN(mobed_addaffect		);
DECLARE_OLC_FUN(mobed_delaffect		);
DECLARE_OLC_FUN(mobed_del		);
DECLARE_OLC_FUN(mobed_where		);
DECLARE_OLC_FUN(mobed_xmult		);

DECLARE_VALIDATE_FUN(validate_fvnum	);

olc_cmd_t olc_cmds_mob[] =
{
/*	{ command	function	validator	args		}, */
	{ "create",	mobed_create,	NULL,		NULL		},
	{ "edit",	mobed_edit,	NULL,		NULL		},
	{ "",		NULL,		NULL,		NULL		},
	{ "touch",	mobed_touch,	NULL,		NULL		},
	{ "show",	mobed_show,	NULL,		NULL		},
	{ "list",	mobed_list,	NULL,		NULL		},

	{ "alignment",	mobed_align,	NULL,		NULL		},
	{ "desc",	mobed_desc,	NULL,		NULL		},
	{ "level",	mobed_level,	NULL,		NULL		},
	{ "long",	mobed_long,	NULL,		NULL		},
	{ "name",	mobed_name,	NULL,		NULL		},
	{ "shop",	mobed_shop,	NULL,		NULL		},
	{ "short",	mobed_short,	NULL,		NULL		},

	{ "gender",	mobed_gender,	NULL,		sex_table	},
	{ "act",	mobed_act,	NULL,		mob_act_flags	},
	{ "mob",	mobed_mob,	NULL,		mob_flags	},
	{ "affect",	mobed_affect,	NULL,		affect_flags	},
	{ "invis",	mobed_invis,	NULL,		id_flags	},
	{ "detect",	mobed_detect,	NULL,		id_flags	},
	{ "prac",	mobed_prac,	NULL,		skill_groups	},
	{ "armor",	mobed_ac,	NULL,		NULL		},
	{ "form",	mobed_form,	NULL,		form_flags	},
	{ "part",	mobed_part,	NULL,		part_flags	},
	{ "material",	mobed_material,	NULL,		NULL		},
	{ "off",	mobed_off,	NULL,		off_flags	},
	{ "size",	mobed_size,	NULL,		size_table	},
	{ "hitdice",	mobed_hitdice,	NULL,		NULL		},
	{ "manadice",	mobed_manadice,	NULL,		NULL		},
	{ "damdice",	mobed_damdice,	NULL,		NULL		},
	{ "race",	mobed_race,	NULL,		NULL		},
	{ "startpos",	mobed_startpos,	NULL,		position_table	},
	{ "defaultpos",	mobed_defaultpos, NULL,		position_table	},
	{ "wealth",	mobed_gold,	NULL,		NULL		},
	{ "hitroll",	mobed_hitroll,	NULL,		NULL		},
	{ "damtype",	mobed_damtype,	NULL,		NULL		},
	{ "group",	mobed_group,	NULL,		NULL		},
	{ "clan",	mobed_clan,	NULL,		&clans		},
	{ "trig",	mobed_trig,	NULL,		NULL		},
	{ "clone",	mobed_clone,	NULL,		NULL		},
	{ "wizi",	mobed_wizi,	NULL,		NULL		},
	{ "incog",	mobed_incog,	NULL,		NULL		},
	{ "fvnum",	mobed_fvnum,	validate_fvnum,	NULL		},
	{ "resist",	mobed_resist,	NULL,		NULL		},
	{ "addaffect",	mobed_addaffect, NULL,		NULL		},
	{ "delaffect",	mobed_delaffect, NULL,		NULL		},

	{ "where",	mobed_where,	NULL,		NULL		},
	{ "delete_mo",	olced_spell_out, NULL,		NULL		},
	{ "delete_mob", mobed_del,	NULL,		NULL		},
	{ "xmult",	mobed_xmult,	NULL,		NULL		},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

OLC_FUN(mobed_create)
{
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea;
	int  value;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	value = atoi(arg);
	if (!value)
		OLC_ERROR("'OLC CREATE'");

	pArea = area_vnum_lookup(value);
	if (!pArea) {
		act_char("MobEd: That vnum is not assigned an area.", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
		act_char("MobEd: Insufficient security.", ch);
		return FALSE;
	}

	if (get_mob_index(value)) {
		act_char("MobEd: Mobile vnum already exists.", ch);
		return FALSE;
	}

	pMob = c_insert(&mobiles, &value);
	pMob->vnum = value;
	if (value > top_vnum_mob)
		top_vnum_mob = value;

	ch->desc->pEdit		= (void*) pMob;
	OLCED(ch)		= olced_lookup(ED_MOB);
	TOUCH_AREA(pArea);
	act_char("MobEd: Mobile created.", ch);
	return FALSE;
}

OLC_FUN(mobed_edit)
{
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea;
	int value;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	value = atoi(arg);
	if ((pMob = get_mob_index(value)) == NULL) {
		act_char("MobEd: Vnum does not exist.", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(pMob->vnum);
	if (!IS_BUILDER(ch, pArea)) {
		act_char("MobEd: Insufficient security.", ch);
		return FALSE;
	}

	ch->desc->pEdit	= (void*) pMob;
	OLCED(ch)	= olced_lookup(ED_MOB);
	return FALSE;
}

OLC_FUN(mobed_touch)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	TOUCH_VNUM(pMob->vnum);
	return FALSE;
}

OLC_FUN(mobed_show)
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA	*pMob;
	AREA_DATA	*pArea;
	BUFFER *buf;
	int *gr;
	bool found;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_MOB))
			EDIT_MOB(ch, pMob);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		int value = atoi(arg);
		if ((pMob = get_mob_index(value)) == NULL) {
			act_char("MobEd: Vnum does not exist.", ch);
			return FALSE;
		}
	}

	buf = buf_new(0);

	pArea = area_vnum_lookup(pMob->vnum);
	buf_printf(buf, BUF_END,
		   "Name:        [%s]\n"
		   "Area:        [%5d] %s\n",
		   pMob->name, pArea->vnum, pArea->name);

	buf_printf(buf, BUF_END, "Vnum:        [%5d]    Race: [%s]\n",
		   pMob->vnum, pMob->race);
	if (pMob->fvnum)
		buf_printf(buf, BUF_END, "Female vnum: [%d]\n", pMob->fvnum);

	mlstr_dump(buf, "Gender:      ", &pMob->gender, DL_NONE);

	mlstr_dump(buf, "Short descr: ", &pMob->short_descr, DUMP_LEVEL(ch));
	buf_append(buf, "Long descr:\n");
	mlstr_dump(buf, str_empty, &pMob->long_descr, DUMP_LEVEL(ch));
	buf_append(buf, "Description:\n");
	mlstr_dump(buf, str_empty, &pMob->description, DUMP_LEVEL(ch));

	if (IN_TRANS_MODE(ch))
		goto bamfout;

	buf_printf(buf, BUF_END, "Act:         [%s]\n",
		flag_string(mob_act_flags, pMob->act));

	if (pMob->mob_flags) {
		buf_printf(buf, BUF_END, "Mob:         [%s]\n",
			flag_string(mob_flags, pMob->mob_flags));
	}
	if (!IS_NULLSTR(pMob->clan))
		buf_printf(buf, BUF_END, "Clan:        [%s]\n", pMob->clan);

	buf_printf(buf, BUF_END, "Level:       [%2d]    Align: [%4d]      Hitroll: [%2d] Dam Type:    [%s]\n",
		pMob->level,	pMob->alignment,
		pMob->hitroll,	pMob->damtype);

	if (pMob->group)
		buf_printf(buf, BUF_END, "Group:       [%5d]\n",
			   pMob->group);

	buf_printf(buf, BUF_END, "Hit dice:    [%2dd%-3d+%4d] ",
		   pMob->hit[DICE_NUMBER],
		   pMob->hit[DICE_TYPE],
		   pMob->hit[DICE_BONUS]);

	buf_printf(buf, BUF_END, "Damage dice: [%2dd%-3d+%4d] ",
			 pMob->damage[DICE_NUMBER],
			 pMob->damage[DICE_TYPE],
			 pMob->damage[DICE_BONUS]);

	buf_printf(buf, BUF_END, "Mana dice:   [%2dd%-3d+%4d]\n",
			 pMob->mana[DICE_NUMBER],
			 pMob->mana[DICE_TYPE],
			 pMob->mana[DICE_BONUS]);

	buf_printf(buf, BUF_END, "Affected by: [%s]\n",
		flag_string(affect_flags, pMob->affected_by));

	buf_printf(buf, BUF_END, "Invis:       [%s]\n",
		flag_string(id_flags, pMob->has_invis));

	buf_printf(buf, BUF_END, "Detect:      [%s]\n",
		flag_string(id_flags, pMob->has_detect));

	buf_printf(buf, BUF_END, "Armor:       [pierce: %d  bash: %d  slash: %d  exotic: %d]\n",
		pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
		pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC]);

	buf_printf(buf, BUF_END, "Form:        [%s]\n",
		flag_string(form_flags, pMob->form));

	buf_printf(buf, BUF_END, "Parts:       [%s]\n",
			flag_string(part_flags, pMob->parts));

	buf_printf(buf, BUF_END, "Off:         [%s]\n",
		flag_string(off_flags,  pMob->off_flags));

	buf_printf(buf, BUF_END, "Size:        [%s]\n",
		flag_string(size_table, pMob->size));

	buf_printf(buf, BUF_END, "Material:    [%s]\n",
		 pMob->material);

	buf_printf(buf, BUF_END, "Start pos:   [%s]\n",
	flag_string(position_table, pMob->start_pos));

	buf_printf(buf, BUF_END, "Default pos: [%s]\n",
		flag_string(position_table, pMob->default_pos));

	buf_printf(buf, BUF_END, "Wealth:      [%5d]\n", pMob->wealth);

	if (pMob->invis_level) {
		buf_printf(buf, BUF_END, "Invis level: [%d]\n",
			   pMob->invis_level);
	}

	if (pMob->incog_level) {
		buf_printf(buf, BUF_END, "Incog level: [%d]\n",
			   pMob->incog_level);
	}

	buf_printf(buf, BUF_END, "Expierence multiplier: [%d%%]\n",
	    pMob->xp_multiplier);

        found = FALSE;
	C_FOREACH (int *, gr, &pMob->practicer) {
		if (!found) {
			buf_printf(buf, BUF_END, "Practicer:   [");
			found = TRUE;
		} else
			buf_append(buf, " ");
		buf_append(buf, flag_string(skill_groups, *gr));
	}
	if (found)
		buf_append(buf, "]\n");

	dump_resists(buf, pMob->resists);
	aff_dump_list(pMob->affected, buf);

	if (pMob->pShop) {
		SHOP_DATA *pShop;
		int iTrade;

		pShop = pMob->pShop;

		buf_printf(buf, BUF_END, "Shop data for [%5d]:\n"
				"  Markup for purchaser: %d%%\n"
				"  Markdown for seller:  %d%%\n",
			pShop->keeper, pShop->profit_buy, pShop->profit_sell);
		buf_printf(buf, BUF_END, "  Hours: %d to %d.\n",
			pShop->open_hour, pShop->close_hour);

		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
			if (pShop->buy_type[iTrade] <= 0)
				continue;
			if (iTrade == 0) {
				buf_append(buf, "  Number Trades Type\n");
				buf_append(buf, "  ------ -----------\n");
			}
			buf_printf(buf, BUF_END, "  [%4d] %s\n", iTrade,
				flag_string(item_types, pShop->buy_type[iTrade]));
		}
	}

	trig_dump_list(&pMob->mp_trigs, buf);

bamfout:
	page_to_char(buf_string(buf), ch);
	buf_free(buf);
	return FALSE;
}

OLC_FUN(mobed_list)
{
	MOB_INDEX_DATA	*pMobIndex;
	AREA_DATA	*pArea;
	BUFFER		*buffer;
	char		arg  [MAX_INPUT_LENGTH];
	bool fAll, found;
	int vnum;
	int  col = 0;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC ALIST'");

	if ((pArea = get_edited_area(ch)) == NULL)
		pArea = ch->in_room->area;

	buffer = buf_new(0);
	fAll    = !str_cmp(arg, "all");
	found   = FALSE;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pMobIndex = get_mob_index(vnum)) != NULL) {
			if (fAll || is_name(arg, pMobIndex->name)) {
				found = TRUE;
				buf_printf(buffer, BUF_END, "[%5d] %-17.16s",
					   pMobIndex->vnum,
					   mlstr_mval(&pMobIndex->short_descr));
				if (++col % 3 == 0)
					buf_append(buffer, "\n");
			}
		}
	}

	if (!found) 
		act_char("MobEd: No mobiles in this area.", ch);
	else {
		if (col % 3 != 0)
			buf_append(buffer, "\n");

		page_to_char(buf_string(buffer), ch);
	}

	buf_free(buffer);
	return FALSE;
}

OLC_FUN(mobed_damtype)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_damtype(ch, argument, cmd, &pMob->damtype);
}

OLC_FUN(mobed_align)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->alignment);
}

OLC_FUN(mobed_level)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->level);
}

OLC_FUN(mobed_desc)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_mlstr_text(ch, argument, cmd, &pMob->description);
}

OLC_FUN(mobed_long)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_mlstr(ch, argument, cmd, &pMob->long_descr);
}

OLC_FUN(mobed_short)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_mlstr(ch, argument, cmd, &pMob->short_descr);
}

OLC_FUN(mobed_name)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_name(ch, argument, cmd, &pMob->name);
}

OLC_FUN(mobed_shop)
{
	MOB_INDEX_DATA *pMob;
	char command[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];

	argument = one_argument(argument, command, sizeof(command));
	argument = one_argument(argument, arg1, sizeof(arg1));

	EDIT_MOB(ch, pMob);

	if (command[0] == '\0') {
		act_char("Usage:  shop hours [#xopening] [#xclosing]", ch);
		act_char("         shop profit [#xbuying%] [#xselling%]", ch);
		act_char("         shop type [#x0-4] [item type]", ch);
		act_char("         shop assign", ch);
		act_char("         shop {{ remove | delete }", ch);
		return FALSE;
	}

	if (!str_cmp(command, "hours")) {
		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0' || !is_number(argument)) {
			act_char("Usage:  shop hours [#xopening] [#xclosing]", ch);
			return FALSE;
		}

		if (!pMob->pShop) {
			act_char("MobEd:  Debes crear un shop primero (shop assign).", ch);
			return FALSE;
		}

		pMob->pShop->open_hour = atoi(arg1);
		pMob->pShop->close_hour = atoi(argument);

		act_char("Shop hours set.", ch);
		return TRUE;
	}


	if (!str_cmp(command, "profit")) {
		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0' || !is_number(argument)) {
			act_char("Usage:  shop profit [#xbuying%] [#xselling%]", ch);
			return FALSE;
		}

		if (!pMob->pShop) {
			act_char("MobEd:  Debes crear un shop primero (shop assign).", ch);
			return FALSE;
		}

		pMob->pShop->profit_buy     = atoi(arg1);
		pMob->pShop->profit_sell    = atoi(argument);

		act_char("Shop profit set.", ch);
		return TRUE;
	}


	if (!str_cmp(command, "type")) {
		int value;
		int num;

		if (arg1[0] == '\0'
		||  !is_number(arg1)
		||  argument[0] == '\0') {
			act_char("Usage:  shop type [#x0-4] [item type]", ch);
			return FALSE;
		}

		if ((num = atoi(arg1)) >= MAX_TRADE) {
			act_puts("MobEd: May sell $j items max.",
				 ch, (const void *) MAX_TRADE, NULL,
				 TO_CHAR, POS_DEAD);
			return FALSE;
		}

		if (!pMob->pShop) {
			act_char("MobEd:  Debes crear un shop primero (shop assign).", ch);
			return FALSE;
		}

		if (!str_cmp(argument, "none")) {
			pMob->pShop->buy_type[num] = 0;
			act_puts("Shop type $j reset.",
				 ch, (const void *) num, NULL,
				 TO_CHAR, POS_DEAD);
			return TRUE;
		}

		if ((value = flag_value(item_types, argument)) < 0) {
			act_char("MobEd:  That type of item is not known.", ch);
			return FALSE;
		}

		pMob->pShop->buy_type[num] = value;

		act_char("Shop type set.", ch);
		return TRUE;
	}

	/* shop assign && shop delete by Phoenix */

	if (!str_prefix(command, "assign")) {
		if (pMob->pShop) {
			act_char("Mob already has a shop assigned to it.", ch);
			return FALSE;
		}

		pMob->pShop		= new_shop();
		if (!shop_first)
			shop_first	= pMob->pShop;
		if (shop_last)
			shop_last->next	= pMob->pShop;
		shop_last		= pMob->pShop;

		pMob->pShop->keeper	= pMob->vnum;

		act_char("New shop assigned to mobile.", ch);
		return TRUE;
	}

	if (!str_prefix(command, "remove")
	||  !str_prefix(command, "delete")) {
		SHOP_DATA *pShop;

		pShop		= pMob->pShop;
		pMob->pShop	= NULL;

		if (pShop == shop_first) {
			if (!pShop->next) {
				shop_first = NULL;
				shop_last = NULL;
			} else
				shop_first = pShop->next;
		} else {
			SHOP_DATA *ipShop;

			for (ipShop = shop_first; ipShop; ipShop = ipShop->next) {
				if (ipShop->next != pShop)
					continue;

				if (!pShop->next) {
					shop_last = ipShop;
					shop_last->next = NULL;
				} else
					ipShop->next = pShop->next;
			}
		}

		free_shop(pShop);

		act_char("Mobile is no longer a shopkeeper.", ch);
		return TRUE;
	}

	mobed_shop(ch, str_empty, cmd);
	return FALSE;
}

OLC_FUN(mobed_gender)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_gender(ch, argument, cmd, &pMob->gender);
}

OLC_FUN(mobed_act)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->act);
}

OLC_FUN(mobed_mob)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->mob_flags);
}

OLC_FUN(mobed_affect)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->affected_by);
}

OLC_FUN(mobed_invis)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->has_invis);
}

OLC_FUN(mobed_detect)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->has_detect);
}

OLC_FUN(mobed_prac)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	bool add;
	bool changed;

	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);

	argument = one_argument(argument, arg, sizeof(arg));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC MOB PRAC'");
		return FALSE;
	}

	if (!str_prefix(arg, "add"))
		add = TRUE;
	else if (!str_prefix(arg, "remove") || !str_prefix(arg, "delete"))
		add = FALSE;
	else if (!str_cmp(arg, "?")) {
		show_flags(ch, skill_groups);
		return FALSE;
	} else
		return cmd->olc_fun(ch, "", cmd);

	if (arg2[0] == '\0') {
		dofun("help", ch, "'OLC MOB PRAC'");
		return FALSE;
	}

	changed = FALSE;
	for (; arg2[0] != '\0';
	     argument = one_argument(argument, arg2, sizeof(arg2))) {
		int gr;

		gr = flag_value(skill_groups, arg2);
		if (gr < 0) {
			act("MobEd: $t: unknown skill group. Use 'prac ?' to see available skill groups",
			    ch, arg2, NULL, TO_CHAR | ACT_NOTRANS);
			continue;
		}

		if (add) {
			if (mob_add_practicer(pMob, gr)) {
				act("MobEd: $t: skill group added.",
				    ch, arg2, NULL, TO_CHAR | ACT_NOTRANS);
				changed = TRUE;
			}
		} else {
			if (mob_del_practicer(pMob, gr)) {
				act("MobEd: $t: skill group removed.",
				    ch, arg2, NULL, TO_CHAR | ACT_NOTRANS);
				changed = TRUE;
			}
		}
	}

	return changed;
}

OLC_FUN(mobed_ac)
{
	MOB_INDEX_DATA *pMob;
	char arg[MAX_INPUT_LENGTH];
	int pierce, bash, slash, exotic;

	do {  /* So that I can use break and send the syntax in one place */
		if (argument[0] == '\0')
			break;

		EDIT_MOB(ch, pMob);
		argument = one_argument(argument, arg, sizeof(arg));

		if (!is_number(arg))
			break;
		pierce = atoi(arg);
		argument = one_argument(argument, arg, sizeof(arg));

		if (arg[0] != '\0') {
			if (!is_number(arg))
				break;
			bash = atoi(arg);
			argument = one_argument(argument, arg, sizeof(arg));
		} else
			bash = pMob->ac[AC_BASH];

		if (arg[0] != '\0') {
			if (!is_number(arg))
				break;
			slash = atoi(arg);
			argument = one_argument(argument, arg, sizeof(arg));
		} else
			slash = pMob->ac[AC_SLASH];

		if (arg[0] != '\0') {
			if (!is_number(arg))
				break;
			exotic = atoi(arg);
		} else
			exotic = pMob->ac[AC_EXOTIC];

		pMob->ac[AC_PIERCE] = pierce;
		pMob->ac[AC_BASH]   = bash;
		pMob->ac[AC_SLASH]  = slash;
		pMob->ac[AC_EXOTIC] = exotic;

		act_char("Ac set.", ch);
		return TRUE;
	} while (FALSE);    /* Just do it once.. */

	act_char("Usage: ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]", ch);
	act_char("'help MOB_AC' gives a list of reasonable ac values.", ch);
	return FALSE;
}

OLC_FUN(mobed_form)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->form);
}

OLC_FUN(mobed_part)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->parts);
}

OLC_FUN(mobed_material)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_str(ch, argument, cmd, &pMob->material);
}

OLC_FUN(mobed_off)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->off_flags);
}

OLC_FUN(mobed_size)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->size);
}

OLC_FUN(mobed_hitdice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_dice(ch, argument, cmd, pMob->hit);
}

OLC_FUN(mobed_manadice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_dice(ch, argument, cmd, pMob->mana);
}

OLC_FUN(mobed_damdice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_dice(ch, argument, cmd, pMob->damage);
}

OLC_FUN(mobed_race)
{
	MOB_INDEX_DATA *pMob;
	race_t *r, *ro;

	if (argument[0]
	&&  (r = race_search(argument)) != NULL) {
		EDIT_MOB(ch, pMob);

		ro = race_lookup(pMob->race);
		free_string(pMob->race);
		pMob->race = str_qdup(r->name);

		if (ro == NULL) {
			pMob->act	  = r->act;
			pMob->affected_by = r->aff;
			pMob->has_invis	  = r->has_invis;
			pMob->has_detect  = r->has_detect;
			pMob->off_flags   = r->off;
			pMob->form        = r->form;
			pMob->parts       = r->parts;
		} else {
			pMob->act	  = (pMob->act & ~ro->act) | r->act;
			pMob->affected_by = (pMob->affected_by & ~ro->aff) |
							r->aff;
			pMob->has_invis	  = (pMob->has_invis & ~ro->has_invis) |
							r->has_invis;
			pMob->has_detect  = (pMob->has_detect & ~ro->has_detect) |
							r->has_detect;
			pMob->off_flags   = (pMob->off_flags & ~ro->off) |
							r->off;
			pMob->form        = (pMob->form & ~ro->form) |
							r->form;
			pMob->parts       = (pMob->parts & ~ro->parts) |
							r->parts;
		}

		act_char("Race set.", ch);
		return TRUE;
	}

	if (argument[0] == '?') {
		BUFFER *buf = buf_new(0);
		buf_append(buf, "Available races are:\n");
		c_strkey_dump(&races, buf);
		page_to_char(buf_string(buf), ch);
		buf_free(buf);
		return FALSE;
	}

	act_char("Usage: race [race]", ch);
	act_char("Type 'race ?' for a list of races.", ch);
	return FALSE;
}

OLC_FUN(mobed_startpos)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->start_pos);
}

OLC_FUN(mobed_defaultpos)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, cmd, &pMob->default_pos);
}

OLC_FUN(mobed_gold)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->wealth);
}

OLC_FUN(mobed_hitroll)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->hitroll);
}

OLC_FUN(mobed_group)
{
	MOB_INDEX_DATA *pMob;
	MOB_INDEX_DATA *pMTemp;
	char arg[MAX_STRING_LENGTH];
	int temp;
	BUFFER *buffer;
	bool found = FALSE;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0') {
		act_char("Usage: group [number]", ch);
		act_char("        group show [number]", ch);
		return FALSE;
	}

	if (is_number(argument)) {
		pMob->group = atoi(argument);
		act_char("Group set.", ch);
		return TRUE;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if (!strcmp(arg, "show") && is_number(argument)) {
		if (atoi(argument) == 0) {
			act_char("Are you crazy?", ch);
			return FALSE;
		}

		buffer = buf_new(0);

		for (temp = 0; temp < 65536; temp++) {
			pMTemp = get_mob_index(temp);
			if (pMTemp && (pMTemp->group == atoi(argument))) {
				found = TRUE;
				buf_printf(buffer, BUF_END, "[%5d] %s\n",
					   pMTemp->vnum, pMTemp->name);
			}
		}

		if (found)
			page_to_char(buf_string(buffer), ch);
		else
			act_char("No mobs in that group.", ch);

		buf_free(buffer);
		return FALSE;
	}

	return FALSE;
}

OLC_FUN(mobed_clan)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_foreign_strkey(ch, argument, cmd, &pMob->clan);
}

OLC_FUN(mobed_trig)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_trig(
	    ch, argument, cmd, &pMob->mp_trigs, MP_T_MOB, pMob->vnum, NULL);
}

OLC_FUN(mobed_clone)
{
	MOB_INDEX_DATA *pMob;
	MOB_INDEX_DATA *pFrom;
	char arg[MAX_INPUT_LENGTH];
	int i;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg))
		OLC_ERROR("'OLC CLONE'");

	i = atoi(arg);
	if ((pFrom = get_mob_index(i)) == NULL) {
		act_puts("MobEd: $j: Vnum does not exist.",
			 ch, (const void *) i, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	EDIT_MOB(ch, pMob);
	if (pMob == pFrom)
		return FALSE;

	free_string(pMob->name);
	pMob->name		= str_qdup(pFrom->name);
	free_string(pMob->material);
	pMob->material		= str_qdup(pFrom->material);
	free_string(pMob->damtype);
	pMob->damtype		= str_qdup(pFrom->damtype);
	mlstr_cpy(&pMob->short_descr, &pFrom->short_descr);
	mlstr_cpy(&pMob->long_descr, &pFrom->long_descr);
	mlstr_cpy(&pMob->description, &pFrom->description);

	pMob->group		= pFrom->group;
	pMob->act		= pFrom->act;
	pMob->affected_by	= pFrom->affected_by;
	pMob->has_invis		= pFrom->has_invis;
	pMob->has_detect	= pFrom->has_detect;
	pMob->alignment		= pFrom->alignment;
	pMob->level		= pFrom->level;
	pMob->hitroll		= pFrom->hitroll;
	pMob->off_flags		= pFrom->off_flags;
	pMob->start_pos		= pFrom->start_pos;
	pMob->default_pos	= pFrom->default_pos;
	mlstr_cpy(&pMob->gender, &pFrom->gender);
	pMob->wealth		= pFrom->wealth;
	pMob->form		= pFrom->form;
	pMob->parts		= pFrom->parts;
	pMob->size		= pFrom->size;
	pMob->practicer		= pFrom->practicer;
	free_string(pMob->clan);
	pMob->clan		= str_qdup(pFrom->clan);
	pMob->invis_level	= pFrom->invis_level;
	pMob->incog_level	= pFrom->incog_level;
	pMob->fvnum		= pFrom->fvnum;
	pMob->xp_multiplier	= pFrom->xp_multiplier;

	free_string(pMob->race);
	pMob->race		= str_qdup(pFrom->race);

	for (i = 0; i < 3; i++)
		pMob->hit[i]	= pFrom->hit[i];
	for (i = 0; i < 3; i++)
		pMob->mana[i]	= pFrom->mana[i];
	for (i = 0; i < 3; i++)
		pMob->damage[i]	= pFrom->damage[i];
	for (i = 0; i < 4; i++)
		pMob->ac[i]	= pFrom->ac[i];

	for (i = 0; i < MAX_RESIST; i++)
		pMob->resists[i] = pFrom->resists[i];

	aff_free_list(pMob->affected);
	pMob->affected = aff_dup_list(pFrom->affected, -1);

	return TRUE;
}

OLC_FUN(mobed_wizi)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->invis_level);
}

OLC_FUN(mobed_incog)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->incog_level);
}

OLC_FUN(mobed_fvnum)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->fvnum);
}

OLC_FUN(mobed_resist)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_resists(ch, argument, cmd, pMob->resists);
}

OLC_FUN(mobed_addaffect)
{
	MOB_INDEX_DATA *mob;
	EDIT_MOB(ch, mob);
	return olced_addaffect(ch, argument, cmd, mob->level, &mob->affected);
}

OLC_FUN(mobed_delaffect)
{
	MOB_INDEX_DATA *mob;
	EDIT_MOB(ch, mob);
	return olced_delaffect(ch, argument, cmd, &mob->affected);
}

OLC_FUN(mobed_del)
{
	MOB_INDEX_DATA *pMob;
	CHAR_DATA *mob, *mob_next;
	BUFFER *buf;

	EDIT_MOB(ch, pMob);

	if (olced_busy(ch, ED_MOB, pMob, NULL))
		return FALSE;

/* check that `pMob' is not in resets */
	buf = show_mob_resets(pMob->vnum);
	if (buf != NULL) {
		buf_prepend(buf, "MobEd: can't delete mob index: delete the following resets:\n");
		page_to_char(buf_string(buf), ch);
		buf_free(buf);
		return FALSE;
	}

/* delete all the instances of mob index */
	for (mob = npc_list; mob != NULL; mob = mob_next) {
		mob_next = mob->next;

		if (mob->pMobIndex == pMob)
			extract_char(mob, 0);
	}

	TOUCH_VNUM(pMob->vnum);

/* delete mob index itself */
	c_delete(&mobiles, pMob);
	act_char("MobEd: Mob index deleted.", ch);
	edit_done(ch->desc);
	return FALSE;
}

OLC_FUN(mobed_where)
{
	int vnum;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] != '\0') {
		if (!is_number(arg)) {
			act_char("Usage: where [<vnum>]", ch);
			return FALSE;
		}
		vnum = atoi(arg);
	} else {
		MOB_INDEX_DATA *mob;
		EDIT_MOB(ch, mob);
		vnum = mob->vnum;
	}

	show_resets(ch, vnum, "mob", show_mob_resets);
	return FALSE;
}

OLC_FUN(mobed_xmult)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->xp_multiplier);
}

/* Local functions */

VALIDATE_FUN(validate_fvnum)
{
	int fvnum = *(const int *) arg;

	if (!get_mob_index(fvnum)) {
		act_puts("MobEd: $j: no such vnum.",
			 ch, (const void *) fvnum, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	return TRUE;
}
