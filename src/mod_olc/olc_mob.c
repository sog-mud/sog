#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "olc.h"
#include "comm.h"
#include "string_edit.h"
#include "db.h"
#include "tables.h"
#include "buffer.h"
#include "mlstring.h"
#include "lookup.h"

#define MEDIT(fun)		bool fun(CHAR_DATA *ch, const char *argument)
#define EDIT_MOB(Ch, Mob)	(Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit)

/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN(medit_show		);
DECLARE_OLC_FUN(medit_create		);
DECLARE_OLC_FUN(medit_name		);
DECLARE_OLC_FUN(medit_short		);
DECLARE_OLC_FUN(medit_long		);
DECLARE_OLC_FUN(medit_shop		);
DECLARE_OLC_FUN(medit_desc		);
DECLARE_OLC_FUN(medit_level		);
DECLARE_OLC_FUN(medit_align		);
DECLARE_OLC_FUN(medit_spec		);

DECLARE_OLC_FUN(medit_sex		);  /* ROM */
DECLARE_OLC_FUN(medit_act		);  /* ROM */
DECLARE_OLC_FUN(medit_affect		);  /* ROM */
DECLARE_OLC_FUN(medit_ac		);  /* ROM */
DECLARE_OLC_FUN(medit_form		);  /* ROM */
DECLARE_OLC_FUN(medit_part		);  /* ROM */
DECLARE_OLC_FUN(medit_imm		);  /* ROM */
DECLARE_OLC_FUN(medit_res		);  /* ROM */
DECLARE_OLC_FUN(medit_vuln		);  /* ROM */
DECLARE_OLC_FUN(medit_material		);  /* ROM */
DECLARE_OLC_FUN(medit_off		);  /* ROM */
DECLARE_OLC_FUN(medit_size		);  /* ROM */
DECLARE_OLC_FUN(medit_hitdice		);  /* ROM */
DECLARE_OLC_FUN(medit_manadice		);  /* ROM */
DECLARE_OLC_FUN(medit_damdice		);  /* ROM */
DECLARE_OLC_FUN(medit_race		);  /* ROM */
DECLARE_OLC_FUN(medit_startpos		);  /* ROM */
DECLARE_OLC_FUN(medit_defaultpos	);  /* ROM */
DECLARE_OLC_FUN(medit_gold		);  /* ROM */
DECLARE_OLC_FUN(medit_hitroll		);  /* ROM */
DECLARE_OLC_FUN(medit_damtype		);  /* ROM */
DECLARE_OLC_FUN(medit_group		);  /* ROM */
DECLARE_OLC_FUN(medit_trigadd		);  /* ROM */
DECLARE_OLC_FUN(medit_trigdel		);  /* ROM */
DECLARE_OLC_FUN(medit_detect		); 
DECLARE_OLC_FUN(medit_prac		); 

DECLARE_VALIDATE_FUN(validate_act	);

OLC_CMD_DATA medit_table[] =
{
/*  { command		function	validator	params		}, */
    { "alignment",	medit_align					},
    { "create",		medit_create					},
    { "desc",		medit_desc					},
    { "level",		medit_level					},
    { "long",		medit_long					},
    { "name",		medit_name					},
    { "shop",		medit_shop					},
    { "short",		medit_short					},
    { "show",		medit_show					},
    { "spec",		medit_spec					},

    { "sex",		medit_sex,		NULL,	sex_table	},
    { "act",		medit_act,	validate_act,	act_flags	},
    { "affect",		medit_affect,		NULL,	affect_flags	},
    { "detect",		medit_detect,		NULL,	detect_flags	},
    { "prac",		medit_prac,		NULL,	skill_groups	},
    { "armor",		medit_ac					},
    { "form",		medit_form,		NULL,	form_flags	},
    { "part",		medit_part,		NULL,	part_flags	},
    { "imm",		medit_imm,		NULL,	imm_flags	},
    { "res",		medit_res,		NULL,	res_flags	},
    { "vuln",		medit_vuln,		NULL,	vuln_flags	},
    { "material",	medit_material					},
    { "off",		medit_off,		NULL,	off_flags	},
    { "size",		medit_size,		NULL,	size_table	},
    { "hitdice",	medit_hitdice					},
    { "manadice",	medit_manadice					},
    { "damdice",	medit_damdice					},
    { "race",		medit_race					},
    { "startpos",	medit_startpos,		NULL,	position_table	},
    { "defaultpos",	medit_defaultpos,	NULL,	position_table	},
    { "wealth",		medit_gold					},
    { "hitroll",	medit_hitroll					},
    { "damtype",	medit_damtype					},
    { "group",		medit_group					},
    { "trigadd",	medit_trigadd					},
    { "trigdel",	medit_trigdel					},

    { "?",		show_help					},
    { "commands",	show_commands					},
    { "version",	show_version					},

    { NULL }
};

/* Entry point for editing mob_index_data. */
void do_medit(CHAR_DATA *ch, const char *argument)
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);

    if (IS_NPC(ch))
    	return;

    if (is_number(arg1))
    {
	value = atoi(arg1);
	if (!(pMob = get_mob_index(value)))
	{
	    send_to_char("MEdit:  That vnum does not exist.\n\r", ch);
	    return;
	}

	pArea = get_vnum_area(pMob->vnum);
	if (!IS_BUILDER(ch, pArea))
	{
		send_to_char("Insuficiente seguridad para modificar mobs.\n\r" , ch);
	        return;
	}

	ch->desc->pEdit = (void *)pMob;
	ch->desc->editor = ED_MOBILE;
	return;
    }
    else
    {
	if (!str_cmp(arg1, "create"))
	{
	    value = atoi(argument);
	    if (arg1[0] == '\0' || value == 0)
	    {
		send_to_char("Syntax:  edit mobile create [vnum]\n\r", ch);
		return;
	    }

	    pArea = get_vnum_area(value);

	    if (!pArea)
	    {
		send_to_char("OEdit:  That vnum is not assigned an area.\n\r", ch);
		return;
	    }

	    if (!IS_BUILDER(ch, pArea))
	    {
		send_to_char("Insuficiente seguridad para modificar mobs.\n\r" , ch);
	        return;
	    }

	    if (medit_create(ch, argument))
	    {
		SET_BIT(pArea->flags, AREA_CHANGED);
		ch->desc->editor = ED_MOBILE;
	    }
	    return;
	}
    }

    send_to_char("MEdit:  There is no default mobile to edit.\n\r", ch);
    return;
}

/* Mobile Interpreter, called by do_medit. */
void medit(CHAR_DATA *ch, const char *argument)
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int  cmd;

    strcpy(arg, argument);
    smash_tilde(arg);
    argument = one_argument(arg, command);

    EDIT_MOB(ch, pMob);
    pArea = get_vnum_area(pMob->vnum);

    if (!IS_BUILDER(ch, pArea))
    {
	send_to_char("MEdit: Insufficient security to modify area.\n\r", ch);
	edit_done(ch);
	return;
    }

    if (!str_cmp(command, "done"))
    {
	edit_done(ch);
	return;
    }

    if (command[0] == '\0')
    {
        medit_show(ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; medit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, medit_table[cmd].name))
	{
	    if ((*medit_table[cmd].olc_fun) (ch, argument))
	    {
		SET_BIT(pArea->flags, AREA_CHANGED);
		return;
	    }
	    else
		return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret(ch, arg);
    return;
}

/*
 * Mobile Editor Functions.
 */
MEDIT(medit_show)
{
	MOB_INDEX_DATA	*pMob;
	AREA_DATA	*pArea;
	MPTRIG *mptrig;
	BUFFER *buf;

	EDIT_MOB(ch, pMob);

	buf = buf_new(0);

	pArea = get_vnum_area(pMob->vnum);
	buf_printf(buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
		pMob->name, pArea->vnum, pArea->name);

	buf_printf(buf, "Act:         [%s]\n\r",
		flag_string(act_flags, pMob->act));

	buf_printf(buf, "Vnum:        [%5d] Sex:   [%s]   Race: [%s]\n\r",
		pMob->vnum,
		flag_string(sex_table, pMob->sex),
		race_table[pMob->race].name);

	buf_printf(buf, "Level:       [%2d]    Align: [%4d]      Hitroll: [%2d] Dam Type:    [%s]\n\r",
		pMob->level,	pMob->alignment,
		pMob->hitroll,	attack_table[pMob->dam_type].name);

	if (pMob->group)
		buf_printf(buf, "Group:       [%5d]\n\r", pMob->group);

	buf_printf(buf, "Hit dice:    [%2dd%-3d+%4d] ",
			 pMob->hit[DICE_NUMBER],
			 pMob->hit[DICE_TYPE],
			 pMob->hit[DICE_BONUS]);

	buf_printf(buf, "Damage dice: [%2dd%-3d+%4d] ",
			 pMob->damage[DICE_NUMBER],
			 pMob->damage[DICE_TYPE],
			 pMob->damage[DICE_BONUS]);

	buf_printf(buf, "Mana dice:   [%2dd%-3d+%4d]\n\r",
			 pMob->mana[DICE_NUMBER],
			 pMob->mana[DICE_TYPE],
			 pMob->mana[DICE_BONUS]);

/* ROM values end */

	buf_printf(buf, "Affected by: [%s]\n\r",
		flag_string(affect_flags, pMob->affected_by));
	buf_printf(buf, "Can detect:  [%s]\n\r",
		flag_string(detect_flags, pMob->detection));

/* ROM values: */

	buf_printf(buf, "Armor:       [pierce: %d  bash: %d  slash: %d  magic: %d]\n\r",
		pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
		pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC]);

	buf_printf(buf, "Form:        [%s]\n\r",
		flag_string(form_flags, pMob->form));

	buf_printf(buf, "Parts:       [%s]\n\r",
		flag_string(part_flags, pMob->parts));

	buf_printf(buf, "Imm:         [%s]\n\r",
		flag_string(imm_flags, pMob->imm_flags));

	buf_printf(buf, "Res:         [%s]\n\r",
		flag_string(res_flags, pMob->res_flags));

	buf_printf(buf, "Vuln:        [%s]\n\r",
		flag_string(vuln_flags, pMob->vuln_flags));

	buf_printf(buf, "Off:         [%s]\n\r",
		flag_string(off_flags,  pMob->off_flags));

	buf_printf(buf, "Size:        [%s]\n\r",
		flag_string(size_table, pMob->size));

	buf_printf(buf, "Material:    [%s]\n\r",
		 pMob->material);

	buf_printf(buf, "Start pos.   [%s]\n\r",
		flag_string(position_table, pMob->start_pos));

	buf_printf(buf, "Default pos  [%s]\n\r",
		flag_string(position_table, pMob->default_pos));

	buf_printf(buf, "Wealth:      [%5d]\n\r", pMob->wealth);

/* ROM values end */

	if (pMob->spec_fun)
		buf_printf(buf, "Spec fun:    [%s]\n\r",  spec_name(pMob->spec_fun));
	if (pMob->practicer)
		buf_printf(buf, "Practicer:   [%s]\n\r",
			flag_string(skill_groups, pMob->practicer));

	mlstr_dump(buf, "Short descr: ", pMob->short_descr);
	mlstr_dump(buf, "Long descr: ", pMob->long_descr);
	mlstr_dump(buf, "Description: ", pMob->description);

	if (pMob->pShop) {
		SHOP_DATA *pShop;
		int iTrade;

		pShop = pMob->pShop;

		buf_printf(buf, "Shop data for [%5d]:\n\r"
				"  Markup for purchaser: %d%%\n\r"
				"  Markdown for seller:  %d%%\n\r",
			pShop->keeper, pShop->profit_buy, pShop->profit_sell);
		buf_printf(buf, "  Hours: %d to %d.\n\r",
			pShop->open_hour, pShop->close_hour);

		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
			if (pShop->buy_type[iTrade] != 0) {
			if (iTrade == 0) {
				buf_add(buf, "  Number Trades Type\n\r");
				buf_add(buf, "  ------ -----------\n\r");
			}
			buf_printf(buf, "  [%4d] %s\n\r", iTrade,
				flag_string(type_flags, pShop->buy_type[iTrade]));
			}
		}
	}

	if (pMob->mptrig_list) {
		int cnt = 0;

		buf_printf(buf, "\n\rMOBPrograms for [%5d]:\n\r", pMob->vnum);

		for (mptrig = pMob->mptrig_list; mptrig; mptrig = mptrig->next) {
			if (cnt ==0) {
				buf_add(buf, " Number Vnum Trigger Phrase [Flags]\n\r");
				buf_add(buf, " ------ ---- ------- ----------------------------------------------------------\n\r");
			}

			buf_printf(buf, "[%5d] %4d %7s %s [%s]\n\r", cnt,
			mptrig->vnum, flag_string(mptrig_types, mptrig->type),
			mptrig->phrase,
			flag_string(mptrig_flags, mptrig->flags));
			cnt++;
		}
	}

	char_puts(buf_string(buf), ch);
	buf_free(buf);

	return FALSE;
}



MEDIT(medit_create)
{
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea;
	int  value;
	int  iHash;

	value = atoi(argument);
	if (argument[0] == '\0' || value == 0)
	{
		send_to_char("Syntax:  medit create [vnum]\n\r", ch);
		return FALSE;
	}

	pArea = get_vnum_area(value);

	if (!pArea)
	{
		send_to_char("MEdit:  That vnum is not assigned an area.\n\r", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea))
	{
		send_to_char("MEdit:  Vnum in an area you cannot build in.\n\r", ch);
		return FALSE;
	}

	if (get_mob_index(value))
	{
		send_to_char("MEdit:  Mobile vnum already exists.\n\r", ch);
		return FALSE;
	}

	pMob			= new_mob_index();
	pMob->vnum		= value;
		 
	if (value > top_vnum_mob)
		top_vnum_mob = value;        

	pMob->act		= ACT_NPC;
	iHash			= value % MAX_KEY_HASH;
	pMob->next		= mob_index_hash[iHash];
	mob_index_hash[iHash]	= pMob;
	ch->desc->pEdit		= (void *)pMob;

	send_to_char("Mobile Created.\n\r", ch);
	return TRUE;
}



MEDIT(medit_spec)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  spec [special function]\n\r", ch);
		return FALSE;
	}


	if (!str_cmp(argument, "none"))
	{
		 pMob->spec_fun = NULL;

		 send_to_char("Spec removed.\n\r", ch);
		 return TRUE;
	}

	if (spec_lookup(argument))
	{
		pMob->spec_fun = spec_lookup(argument);
		send_to_char("Spec set.\n\r", ch);
		return TRUE;
	}

	send_to_char("MEdit: No such special function.\n\r", ch);
	return FALSE;
}

MEDIT(medit_damtype)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  damtype [damage message]\n\r", ch);
		send_to_char("Para ver una lista de tipos de mensajes, pon '? weapon'.\n\r", ch);
		return FALSE;
	}

	pMob->dam_type = attack_lookup(argument);
	send_to_char("Damage type set.\n\r", ch);
	return TRUE;
}


MEDIT(medit_align)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, medit_align, &pMob->alignment);
}

MEDIT(medit_level)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, medit_level, &pMob->level);
}

MEDIT(medit_desc)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_mlstr_text(ch, argument, medit_desc, &pMob->description);
}

MEDIT(medit_long)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_mlstrnl(ch, argument, medit_long, &pMob->long_descr);
}

MEDIT(medit_short)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_mlstr(ch, argument, medit_short, &pMob->short_descr);
}

MEDIT(medit_name)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_str(ch, argument, medit_name, &pMob->name);
}

MEDIT(medit_shop)
{
	MOB_INDEX_DATA *pMob;
	char command[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];

	argument = one_argument(argument, command);
	argument = one_argument(argument, arg1);

	EDIT_MOB(ch, pMob);

	if (command[0] == '\0')
	{
		send_to_char("Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch);
		send_to_char("         shop profit [#xbuying%] [#xselling%]\n\r", ch);
		send_to_char("         shop type [#x0-4] [item type]\n\r", ch);
		send_to_char("         shop assign\n\r", ch);
		send_to_char("         shop remove\n\r", ch);
		return FALSE;
	}


	if (!str_cmp(command, "hours"))
	{
		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0' || !is_number(argument))
		{
			send_to_char("Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch);
			return FALSE;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Debes crear un shop primero (shop assign).\n\r", ch);
			return FALSE;
		}

		pMob->pShop->open_hour = atoi(arg1);
		pMob->pShop->close_hour = atoi(argument);

		send_to_char("Shop hours set.\n\r", ch);
		return TRUE;
	}


	if (!str_cmp(command, "profit"))
	{
		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0' || !is_number(argument))
		{
			send_to_char("Syntax:  shop profit [#xbuying%] [#xselling%]\n\r", ch);
			return FALSE;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Debes crear un shop primero (shop assign).\n\r", ch);
			return FALSE;
		}

		pMob->pShop->profit_buy     = atoi(arg1);
		pMob->pShop->profit_sell    = atoi(argument);

		send_to_char("Shop profit set.\n\r", ch);
		return TRUE;
	}


	if (!str_cmp(command, "type"))
	{
		int value;

		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0')
		{
			send_to_char("Syntax:  shop type [#x0-4] [item type]\n\r", ch);
			return FALSE;
		}

		if (atoi(arg1) >= MAX_TRADE)
		{
			char_printf(ch, "MEdit:  May sell %d items max.\n\r", MAX_TRADE);
			return FALSE;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Debes crear un shop primero (shop assign).\n\r", ch);
			return FALSE;
		}

		if ((value = flag_value(type_flags, argument)) == 0)
		{
			send_to_char("MEdit:  That type of item is not known.\n\r", ch);
			return FALSE;
		}

		pMob->pShop->buy_type[atoi(arg1)] = value;

		send_to_char("Shop type set.\n\r", ch);
		return TRUE;
	}

	/* shop assign && shop delete by Phoenix */

	if (!str_prefix(command, "assign"))
	{
		if (pMob->pShop)
		{
		 	send_to_char("Mob already has a shop assigned to it.\n\r", ch);
		 	return FALSE;
		}

		pMob->pShop		= new_shop();
		if (!shop_first)
		 	shop_first	= pMob->pShop;
		if (shop_last)
			shop_last->next	= pMob->pShop;
		shop_last		= pMob->pShop;

		pMob->pShop->keeper	= pMob->vnum;

		send_to_char("New shop assigned to mobile.\n\r", ch);
		return TRUE;
	}

	if (!str_prefix(command, "remove"))
	{
		SHOP_DATA *pShop;

		pShop		= pMob->pShop;
		pMob->pShop	= NULL;

		if (pShop == shop_first)
		{
			if (!pShop->next)
			{
			shop_first = NULL;
			shop_last = NULL;
			}
			else
			shop_first = pShop->next;
		}
		else
		{
			SHOP_DATA *ipShop;

			for (ipShop = shop_first; ipShop; ipShop = ipShop->next)
			{
			if (ipShop->next == pShop)
			{
				if (!pShop->next)
				{
					shop_last = ipShop;
					shop_last->next = NULL;
				}
				else
					ipShop->next = pShop->next;
			}
			}
		}

		free_shop(pShop);

		send_to_char("Mobile is no longer a shopkeeper.\n\r", ch);
		return TRUE;
	}

	medit_shop(ch, "");
	return FALSE;
}

MEDIT(medit_sex)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_sex, &pMob->sex);
}

MEDIT(medit_act)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_act, &pMob->act);
}

MEDIT(medit_affect)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_affect, &pMob->affected_by);
}

MEDIT(medit_detect) 
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_detect, &pMob->detection);
}

MEDIT(medit_prac) 
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_prac, &pMob->practicer);
}

MEDIT(medit_ac)
{
	MOB_INDEX_DATA *pMob;
	char arg[MAX_INPUT_LENGTH];
	int pierce, bash, slash, exotic;

	do   /* So that I can use break and send the syntax in one place */
	{
		if (argument[0] == '\0')  break;

		EDIT_MOB(ch, pMob);
		argument = one_argument(argument, arg);

		if (!is_number(arg))  break;
		pierce = atoi(arg);
		argument = one_argument(argument, arg);

		if (arg[0] != '\0')
		{
			if (!is_number(arg))  break;
			bash = atoi(arg);
			argument = one_argument(argument, arg);
		}
		else
			bash = pMob->ac[AC_BASH];

		if (arg[0] != '\0')
		{
			if (!is_number(arg))  break;
			slash = atoi(arg);
			argument = one_argument(argument, arg);
		}
		else
			slash = pMob->ac[AC_SLASH];

		if (arg[0] != '\0')
		{
			if (!is_number(arg))  break;
			exotic = atoi(arg);
		}
		else
			exotic = pMob->ac[AC_EXOTIC];

		pMob->ac[AC_PIERCE] = pierce;
		pMob->ac[AC_BASH]   = bash;
		pMob->ac[AC_SLASH]  = slash;
		pMob->ac[AC_EXOTIC] = exotic;
		
		send_to_char("Ac set.\n\r", ch);
		return TRUE;
	} while (FALSE);    /* Just do it once.. */

	send_to_char("Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
			  "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch);
	return FALSE;
}

MEDIT(medit_form)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_form, &pMob->form);
}

MEDIT(medit_part)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_part, &pMob->parts);
}

MEDIT(medit_imm)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_imm, &pMob->imm_flags);
}

MEDIT(medit_res)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_res, &pMob->res_flags);
}

MEDIT(medit_vuln)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_vuln, &pMob->vuln_flags);
}

MEDIT(medit_material)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_str(ch, argument, medit_material, &pMob->material);
}

MEDIT(medit_off)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_off, &pMob->off_flags);
}

MEDIT(medit_size)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_size, &pMob->size);
}

MEDIT(medit_hitdice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_dice(ch, argument, medit_hitdice, pMob->hit);
}

MEDIT(medit_manadice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_dice(ch, argument, medit_manadice, pMob->mana);
}

MEDIT(medit_damdice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_dice(ch, argument, medit_damdice, pMob->damage);
}

MEDIT(medit_race)
{
	MOB_INDEX_DATA *pMob;
	int race;

	if (argument[0] != '\0'
	&& (race = race_lookup(argument)) != 0) {
		EDIT_MOB(ch, pMob);

		pMob->race = race;
		pMob->act	  |= race_table[race].act;
		pMob->detection	  |= race_table[race].det;
		pMob->affected_by |= race_table[race].aff;
		pMob->off_flags   |= race_table[race].off;
		pMob->imm_flags   |= race_table[race].imm;
		pMob->res_flags   |= race_table[race].res;
		pMob->vuln_flags  |= race_table[race].vuln;
		pMob->form        |= race_table[race].form;
		pMob->parts       |= race_table[race].parts;

		send_to_char("Race set.\n\r", ch);
		return TRUE;
	}

	if (argument[0] == '?') {
		send_to_char("Available races are:", ch);

		for (race = 0; race_table[race].name != NULL; race++) {
			if ((race % 3) == 0)
			send_to_char("\n\r", ch);
			char_printf(ch, " %-15s", race_table[race].name);
		}

		send_to_char("\n\r", ch);
		return FALSE;
	}

	char_puts("Syntax:  race [race]\n\r"
		  "Type 'race ?' for a list of races.\n\r", ch);
	return FALSE;
}

MEDIT(medit_startpos)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_startpos, &pMob->start_pos);
}

MEDIT(medit_defaultpos)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag(ch, argument, medit_defaultpos, &pMob->default_pos);
}

MEDIT(medit_gold)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, medit_gold, &pMob->wealth);
}

MEDIT(medit_hitroll)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, medit_hitroll, &pMob->hitroll);
}

MEDIT(medit_group)
{
	MOB_INDEX_DATA *pMob;
	MOB_INDEX_DATA *pMTemp;
	char arg[MAX_STRING_LENGTH];
	int temp;
	BUFFER *buffer;
	bool found = FALSE;
	
	EDIT_MOB(ch, pMob);
	
	if (argument[0] == '\0') {
		send_to_char("Syntax: group [number]\n\r", ch);
		send_to_char("        group show [number]\n\r", ch);
		return FALSE;
	}
	
	if (is_number(argument))
	{
		pMob->group = atoi(argument);
		send_to_char("Group set.\n\r", ch);
		return TRUE;
	}
	
	argument = one_argument(argument, arg);
	
	if (!strcmp(arg, "show") && is_number(argument)) {
		if (atoi(argument) == 0) {
			send_to_char("Are you crazy?\n\r", ch);
			return FALSE;
		}

		buffer = buf_new(0);

		for (temp = 0; temp < 65536; temp++) {
			pMTemp = get_mob_index(temp);
			if (pMTemp && (pMTemp->group == atoi(argument))) {
				found = TRUE;
				buf_printf(buffer, "[%5d] %s\n\r",
					   pMTemp->vnum, pMTemp->name);
			}
		}

		if (found)
			page_to_char(buf_string(buffer), ch);
		else
			send_to_char("No mobs in that group.\n\r", ch);

		buf_free(buffer);
		return FALSE;
	}
	
	return FALSE;
}

MEDIT (medit_trigadd)
{
	int value;
	MOB_INDEX_DATA *pMob;
	MPTRIG *mptrig;
	MPCODE *mpcode;
	char trigger[MAX_STRING_LENGTH];
	char phrase[MAX_STRING_LENGTH];
	char num[MAX_STRING_LENGTH];

	EDIT_MOB(ch, pMob);
	argument=one_argument(argument, num);
	argument=one_argument(argument, trigger);
	argument=one_argument(argument, phrase);

	if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0') {
		 char_puts("Syntax: trigadd [vnum] [trigger] [phrase]\n\r",ch);
		 return FALSE;
	}

	if ((value = flag_value(mptrig_types, trigger)) < 0) {
		send_to_char("Valid triggers are:\n\r",ch);
		show_flag_cmds(ch, mptrig_types);
		return FALSE;
	}

	if ((mpcode = mpcode_lookup(atoi(num))) == NULL) {
		 send_to_char("No such MOBProgram.\n\r",ch);
		 return FALSE;
	}

	mptrig = mptrig_new(value, phrase, atoi(num));
	mptrig_add(pMob, mptrig);
	send_to_char("Trigger.\n\r",ch);
	return TRUE;
}

MEDIT (medit_trigdel)
{
	MOB_INDEX_DATA *pMob;
	MPTRIG *mptrig;
	MPTRIG *mptrig_next;
	char mprog[MAX_STRING_LENGTH];
	int value;
	int cnt = 0;

	EDIT_MOB(ch, pMob);

	one_argument(argument, mprog);
	if (!is_number(mprog) || mprog[0] == '\0') {
		send_to_char("Syntax:  trigdel [#mprog]\n\r",ch);
		return FALSE;
	}

	value = atoi (mprog);

	if (value < 0) {
		 char_puts("Only non-negative mprog-numbers allowed.\n\r",ch);
		 return FALSE;
	}

	if (!(mptrig = pMob->mptrig_list)) {
		 send_to_char("MEdit:  Nonexistent trigger.\n\r",ch);
		 return FALSE;
	}

	if (value == 0) {
		REMOVE_BIT(pMob->mptrig_types, pMob->mptrig_list->type);
		mptrig = pMob->mptrig_list;
		pMob->mptrig_list = mptrig->next;
		mptrig_free(mptrig);
	}
	else {
		while ((mptrig_next = mptrig->next) && (++cnt < value))
			mptrig = mptrig_next;

		if (mptrig_next) {
			REMOVE_BIT(pMob->mptrig_types, mptrig_next->type);
		        mptrig->next = mptrig_next->next;
			mptrig_free(mptrig_next);
		}
		else {
		        send_to_char("No such trigger.\n\r",ch);
		        return FALSE;
		}
	}

	send_to_char("Trigger removed.\n\r", ch);
	return TRUE;
}

VALIDATOR(validate_act)
{
	REMOVE_BIT(*(int*) arg, ACT_NPC);
	return TRUE;
}
