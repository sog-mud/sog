/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: objval.c,v 1.15 2001-09-12 19:43:19 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>
#include <rwfile.h>

/*-
 *
 * Three blind mice -- see how they run
 * They all ran after the farmer's wife
 * Who cut off their tails with a carving knife
 * Did you ever see such thing in a life
 * As three blind mice?
 *
 */

void
objval_init(flag_t item_type, vo_t *v)
{
	int i;

	switch (item_type) {
	default:
		for (i = 0; i < 5; i++)
			INT(v[i]) = 0;
		break;

	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
		INT(v[0]) = 0;
		INT(v[1]) = 0;
		v[2].s = str_empty;
		INT(v[3]) = 0;
		INT(v[4]) = 0;
		break;

	case ITEM_WEAPON:
	case ITEM_STAFF:
	case ITEM_WAND:
		INT(v[0]) = 0;
		INT(v[1]) = 0;
		INT(v[2]) = 0;
		v[3].s = str_empty;
		INT(v[4]) = 0;
		break;

	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
		INT(v[0]) = 0;
		for (i = 1; i < 5; i++)
			v[i].s = str_empty;
		break;

	case ITEM_BOOK:
		INT(v[0]) = 0;
		v[1].s = str_empty;
		INT(v[2]) = 0;
		INT(v[3]) = 0;
		v[4].s = str_empty;
		break;
	}
}

/*
 * copy obj values from src to dst
 * dst is assumed to be freed before the call
 */
void
objval_cpy(flag_t item_type, vo_t *dst, vo_t *src)
{
	int i;

	switch (item_type) {
	default:
		for (i = 0; i < 5; i++)
			dst[i] = src[i];
		break;

	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2].s = str_qdup(src[2].s);
		dst[3] = src[3];
		dst[4] = src[4];
		break;

	case ITEM_WEAPON:
	case ITEM_STAFF:
	case ITEM_WAND:
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3].s = str_qdup(src[3].s);
		dst[4] = src[4];
		break;

	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
		dst[0] = src[0];
		for (i = 1; i < 5; i++)
			dst[i].s = str_qdup(src[i].s);
		break;

	case ITEM_BOOK:
		dst[0] = src[0];
		dst[1].s = str_qdup(src[1].s);
		dst[2] = src[2];
		dst[3] = src[3];
		dst[4].s = str_qdup(src[4].s);
		break;
	}
}

void
objval_destroy(flag_t item_type, vo_t *v)
{
	int i;

	switch (item_type) {
	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
		free_string(v[2].s);
		break;

	case ITEM_WEAPON:
	case ITEM_STAFF:
	case ITEM_WAND:
		free_string(v[3].s);
		break;

	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
		for (i = 1; i < 5; i++)
			free_string(v[i].s);
		break;

	case ITEM_BOOK:
		free_string(v[1].s);
		free_string(v[4].s);
		break;
	}
}

void
fwrite_objval(flag_t item_type, vo_t *v, FILE *fp)
{
	/*
	 *  Using format_flags to write most values gives a strange
	 *  looking area file, consider making a case for each
	 *  item type later.
	 */

	switch (item_type) {
	default:
		fprintf(fp, "%s %s %s %s %s\n",
			FLAGS(v[0]),
			FLAGS(v[1]),
			FLAGS(v[2]),
			FLAGS(v[3]),
			FLAGS(v[4]));
		break;

	case ITEM_MONEY:
	case ITEM_ARMOR:
		fprintf(fp, "%d %d %d %d %d\n",
			INT(v[0]),
			INT(v[1]),
			INT(v[2]),
			INT(v[3]),
			INT(v[4]));
		break;

        case ITEM_DRINK_CON:
        case ITEM_FOUNTAIN:
		fprintf(fp, "%d %d '%s' %d %d\n",
			INT(v[0]),
			INT(v[1]),
			STR(v[2]),
			INT(v[3]),
			INT(v[4]));
		break;

        case ITEM_CONTAINER:
		fprintf(fp, "%d %s %d %d %d\n",
			INT(v[0]),
			FLAGS(v[1]),
			INT(v[2]),
			INT(v[3]),
			INT(v[4]));
		break;

        case ITEM_WEAPON:
		fprintf(fp, "%s %d %d '%s' %s\n",
			SFLAGS(weapon_class, v[0]),
			INT(v[1]),
			INT(v[2]),
			STR(v[3]),
			FLAGS(v[4]));
		break;

        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
		/* no negative numbers */
		fprintf(fp, "%d '%s' '%s' '%s' '%s'\n",
			UMAX(0, INT(v[0])),
			STR(v[1]),
			STR(v[2]),
			STR(v[3]),
			STR(v[4]));
		break;

        case ITEM_STAFF:
        case ITEM_WAND:
		fprintf(fp, "%d %d %d '%s' %d\n",
			INT(v[0]),
			INT(v[1]),
			INT(v[2]),
			STR(v[3]),
			INT(v[4]));
		break;

	case ITEM_PORTAL:
		fprintf(fp, "%s %s %s %d %d\n",
			FLAGS(v[0]),
			FLAGS(v[1]),
			FLAGS(v[2]),
			INT(v[3]),
			INT(v[4]));
		break;

	case ITEM_LIGHT:
	case ITEM_TATTOO:
	case ITEM_TREASURE:
		fprintf(fp, "%s %s %d %s %s\n",
			FLAGS(v[0]),
			FLAGS(v[1]),
			INT(v[2]),
			FLAGS(v[3]),
			FLAGS(v[4]));
		break;

	case ITEM_BOOK:
		fprintf(fp, "%s %s %d %s '%s'\n",
			SFLAGS(book_class, v[0]),
			STR(v[1]),
			INT(v[2]),
			SFLAGS(fail_effects, v[3]),
			STR(v[4]));
		break;

	case ITEM_FURNITURE:
		fprintf(fp, "%d %d %s %d %d\n",
			INT(v[0]),
			INT(v[1]),
			FLAGS(v[2]),
			INT(v[3]),
			INT(v[4]));
		break;
	}
}

void
fread_objval(flag_t item_type, vo_t *v, rfile_t *fp)
{
	int i;

	switch(item_type) {
	default:
		INT(v[0]) = fread_flags(fp);
		INT(v[1]) = fread_flags(fp);
		INT(v[2]) = fread_flags(fp);
		INT(v[3]) = fread_flags(fp);
		INT(v[4]) = fread_flags(fp);
		break;

	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
		INT(v[0]) = fread_number(fp);
		INT(v[1]) = fread_number(fp);
		STR_ASSIGN(v[2], c_fread_strkey(fp, &liquids, "fread_objval"));
		INT(v[3]) = fread_number(fp);
		INT(v[4]) = fread_number(fp);
		break;

	case ITEM_WEAPON:
		INT(v[0]) = fread_fword(weapon_class, fp);
		INT(v[1]) = fread_number(fp);
		INT(v[2]) = fread_number(fp);
		STR_ASSIGN(v[3], c_fread_strkey(fp, &damtypes, "fread_objval"));
		INT(v[4]) = fread_flags(fp);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		INT(v[0]) = fread_number(fp);
		INT(v[1]) = fread_number(fp);
		INT(v[2]) = fread_number(fp);
		STR_ASSIGN(v[3], c_fread_strkey(
		    fp, &skills, "fread_objval"));		// notrans
		INT(v[4]) = fread_number(fp);
		break;

	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
		INT(v[0]) = fread_number(fp);
		for (i = 1; i < 5; i++) {
			STR_ASSIGN(v[i], c_fread_strkey(
			    fp, &skills, "fread_objval"));
		}
		break;

	case ITEM_BOOK:
		INT(v[0]) = fread_fword(book_class, fp);
		STR_ASSIGN(v[1], c_fread_strkey(fp, &specs, "fread_objval"));
		INT(v[2]) = fread_number(fp);
		INT(v[3]) = fread_fword(fail_effects, fp);
		STR_ASSIGN(v[4], fread_sword(fp));
		break;
	}
}

void
objval_show(BUFFER *output, flag_t item_type, vo_t *v)
{
	switch(item_type) {
	default:	/* No values. */
		buf_append(output, "Obj has unknown item type.\n");
		break;

	case ITEM_TREASURE:
	case ITEM_CLOTHING:
	case ITEM_TRASH:
	case ITEM_KEY:
	case ITEM_BOAT:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_PROTECT:
	case ITEM_MAP:
	case ITEM_WARP_STONE :
	case ITEM_ROOM_KEY:
	case ITEM_GEM:
	case ITEM_JEWELRY:
	case ITEM_JUKEBOX:
	case ITEM_TATTOO:
		break;

	case ITEM_LIGHT:
		if (INT(v[2]) < 0) {
			buf_printf(output, BUF_END,
				   "[v2] Light:  [-1] (infinite)\n");
		} else {
			buf_printf(output, BUF_END,
				   "[v2] Light:  [%d]\n",
				   INT(v[2]));
		}
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		buf_printf(output, BUF_END,
			   "[v0] Level:          [%d]\n"
			   "[v1] Charges Total:  [%d]\n"
			   "[v2] Charges Left:   [%d]\n"
			   "[v3] Spell:          [%s]\n",
			   INT(v[0]),
			   INT(v[1]),
			   INT(v[2]),
			   STR1(v[3]));
		break;

	case ITEM_PORTAL:
		buf_printf(output, BUF_END,
			    "[v0] Charges:         [%d]\n"
			    "[v1] Exit Flags:      [%s]\n"
			    "[v2] Portal Flags:    [%s]\n"
			    "[v3] Goes to (vnum):  [%d]\n"
			    "[v4] Portal key:      [%d]\n",
			    INT(v[0]),
			    SFLAGS(exit_flags, v[1]),
			    SFLAGS(portal_flags , v[2]),
			    INT(v[3]),
			    INT(v[4]));
		break;

	case ITEM_FURNITURE:
		buf_printf(output, BUF_END,
			    "[v0] Max people:       [%d]\n"
			    "[v1] Max weight:       [%d]\n"
			    "[v2] Furniture Flags:  [%s]\n"
			    "[v3] Heal bonus:       [%d]\n"
			    "[v4] Mana bonus:       [%d]\n",
			    INT(v[0]),
			    INT(v[1]),
			    SFLAGS(furniture_flags, v[2]),
			    INT(v[3]),
			    INT(v[4]));
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		buf_printf(output, BUF_END,
			   "[v0] Level:  [%d]\n"
			   "[v1] Spell:  [%s]\n"
			   "[v2] Spell:  [%s]\n"
			   "[v3] Spell:  [%s]\n"
			   "[v4] Spell:  [%s]\n",
			   INT(v[0]),
			   STR1(v[1]),
			   STR1(v[2]),
			   STR1(v[3]),
			   STR1(v[4]));
		break;

/* ARMOR for ROM */

	case ITEM_ARMOR:
		buf_printf(output, BUF_END,
			   "[v0] AC pierce:  [%d]\n"
			   "[v1] AC bash:    [%d]\n"
			   "[v2] AC slash:   [%d]\n"
			   "[v3] AC exotic:  [%d]\n",
			   INT(v[0]),
			   INT(v[1]),
			   INT(v[2]),
			   INT(v[3]));
		break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?! */
	case ITEM_WEAPON:
		buf_printf(output, BUF_END,
			   "[v0] Weapon class:    [%s]\n"
			   "[v1] Number of dice:  [%d]\n"
			   "[v2] Type of dice:    [%d] (ave %d)\n"
			   "[v3] Type:            [%s]\n"
			   "[v4] Special type:    [%s]\n",
			   SFLAGS(weapon_class, v[0]),
			   INT(v[1]),
			   INT(v[2]), GET_AVE(v[1], v[2]),
			   STR(v[3]),
			   SFLAGS(weapon_type2,  v[4]));
		break;

	case ITEM_CONTAINER:
		buf_printf(output, BUF_END,
			   "[v0] Weight:       [%d kg]\n"
			   "[v1] Flags:        [%s]\n"
			   "[v2] Key:          [%d] (%s)\n"
			   "[v3] Capacity:     [%d kg]\n"
			   "[v4] Weight Mult:  [%d]\n",
			   INT(v[0]),
			   SFLAGS(cont_flags, v[1]),
		           INT(v[2]),
		           get_obj_index(INT(v[2])) ?
			   mlstr_mval(&get_obj_index(INT(v[2]))->short_descr) :
			   "none",
		           INT(v[3]),
		           INT(v[4]));
		break;

	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
		buf_printf(output, BUF_END,
			   "[v0] Liquid Total:  [%d]\n"
			   "[v1] Liquid Left:   [%d]\n"
			   "[v2] Liquid:        [%s]\n"
			   "[v3] Poisoned:      [%s]\n",
			   INT(v[0]),
			   INT(v[1]),
			   STR(v[2]),
			   INT(v[3]) ? "Yes" : "No");
		break;

	case ITEM_FOOD:
		buf_printf(output, BUF_END,
			   "[v0] Food hours:  [%d]\n"
			   "[v1] Full hours:  [%d]\n"
			   "[v3] Poisoned:    [%s]\n",
			   INT(v[0]),
			   INT(v[1]),
			   INT(v[3]) ? "yes" : "no");		// notrans
		break;

	case ITEM_MONEY:
		buf_printf(output, BUF_END,
			   "[v0] Silver:  [%d]\n"
			   "[v1] Gold:    [%d]\n",
			   INT(v[0]),
			   INT(v[1]));
		break;

	case ITEM_BOOK:
		buf_printf(output, BUF_END,
			"[v0] Book class:       [%s]\n"
			"[v1] Spec:             [%s]\n"
			"[v2] Base chance:      [%d%%]\n"
			"[v3] Fail effect:      [%s]\n"
			"[v4] Success message:  [%s]\n",
			SFLAGS(book_class, v[0]),
			STR(v[1]),
			INT(v[2]),
			SFLAGS(fail_effects, v[3]),
			STR(v[4]));
		break;
	}
}

/*
 * Return values:
 *	0 - pObj was changed successfully
 *	1 - pObj was not changed
 *	2 - pObj was not changed, do not show obj values
 */
int
objval_set(BUFFER *output, flag_t item_type, vo_t *v,
	   int value_num, const char *argument)
{
	int val;
	skill_t *sk;
	damtype_t *d;
	liquid_t *liq;
	spec_t *spc;

	switch (item_type) {
	default:
		return 1;

	case ITEM_LIGHT:
		switch (value_num) {
		default:
			return 1;
		case 2:
			buf_append(output, "HOURS OF LIGHT SET.\n\n");
			INT(v[2]) = atoi(argument);
			break;
		}
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_append(output, "SPELL LEVEL SET.\n\n");
			INT(v[0]) = atoi(argument);
			break;
		case 1:
			buf_append(output, "TOTAL NUMBER OF CHARGES SET.\n\n");
			INT(v[1]) = atoi(argument);
			break;
		case 2:
			buf_append(output, "CURRENT NUMBER OF CHARGES SET.\n\n");
			INT(v[2]) = atoi(argument);
			break;
		case 3:
			if (!str_cmp(argument, "none")) {
				STR_ASSIGN(v[3], str_empty);
				break;
			}
			if (!str_cmp(argument, "?")
			|| (sk = skill_lookup(argument)) == 0) {
				skills_dump(output, ST_SPELL);
				skills_dump(output, ST_PRAYER);
				return 2;
			}
			buf_append(output, "SPELL TYPE SET.\n");
			STR_ASSIGN(v[3],
				   str_qdup(gmlstr_mval(&sk->sk_name)));
			break;
		}
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		switch (value_num) {
		case 0:
			buf_append(output, "SPELL LEVEL SET.\n\n");
			INT(v[0]) = atoi(argument);
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			if (!str_cmp(argument, "none")) {
				STR_ASSIGN(v[value_num],
					       str_empty);
				break;
			}
			if (!str_cmp(argument, "?")
			||  (sk = skill_lookup(argument)) == 0) {
				skills_dump(output, ST_SPELL);
				skills_dump(output, ST_PRAYER);
				return 2;
			}
			buf_printf(output, BUF_END, "SPELL TYPE %d SET.\n\n", value_num);
			STR_ASSIGN(v[value_num],
				   str_qdup(gmlstr_mval(&sk->sk_name)));
			break;
		}
		break;

/* ARMOR for ROM: */

	case ITEM_ARMOR:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_append(output, "AC PIERCE SET.\n\n");
			INT(v[0]) = atoi(argument);
			break;
		case 1:
			buf_append(output, "AC BASH SET.\n\n");
			INT(v[1]) = atoi(argument);
			break;
		case 2:
			buf_append(output, "AC SLASH SET.\n\n");
			INT(v[2]) = atoi(argument);
			break;
		case 3:
			buf_append(output, "AC EXOTIC SET.\n\n");
			INT(v[3]) = atoi(argument);
			break;
		}
		break;

/* WEAPONS changed in ROM */

	case ITEM_WEAPON:
		switch (value_num) {
		case 0:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(weapon_class, argument)) < 0) {
				show_flags_buf(output, weapon_class);
				return 2;
			}
			buf_append(output, "WEAPON CLASS SET.\n\n");
			INT(v[0]) = val;
			break;
		case 1:
			buf_append(output, "NUMBER OF DICE SET.\n\n");
			INT(v[1]) = atoi(argument);
			break;
		case 2:
			buf_append(output, "TYPE OF DICE SET.\n\n");
			INT(v[2]) = atoi(argument);
			break;
		case 3:
			if (!str_cmp(argument, "?")
			||  (d = damtype_lookup(argument)) == NULL) {
				strkey_printall(&damtypes, output);
				return 2;
			}
			buf_append(output, "WEAPON TYPE SET.\n\n");
			STR_ASSIGN(v[3], str_qdup(d->dam_name));
			break;
		case 4:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(weapon_type2, argument)) < 0) {
				show_flags_buf(output, weapon_type2);
				return 2;
			}
			buf_append(output, "SPECIAL WEAPON TYPE TOGGLED.\n\n");
			TOGGLE_BIT(INT(v[4]), val);
			break;
		}
		break;

	case ITEM_PORTAL:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_append(output, "CHARGES SET.\n\n");
			INT(v[0]) = atoi(argument);
			break;
		case 1:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(exit_flags, argument)) < 0) {
				show_flags_buf(output, exit_flags);
				return 2;
			}
			buf_append(output, "EXIT FLAGS TOGGLED.\n\n");
			TOGGLE_BIT(INT(v[1]), val);
			break;
		case 2:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(portal_flags, argument)) < 0) {
				show_flags_buf(output, portal_flags);
				return 2;
			}
			buf_append(output, "PORTAL FLAGS TOGGLED.\n\n");
			TOGGLE_BIT(INT(v[2]), val);
			break;
		case 3:
			buf_append(output, "EXIT VNUM SET.\n\n");
			INT(v[3]) = atoi(argument);
			break;
		case 4:
			buf_append(output, "PORTAL KEY SET.\n\n");
			INT(v[4]) = atoi(argument);
		}
		break;

	case ITEM_FURNITURE:
		switch (value_num) {
		case 0:
			buf_append(output, "NUMBER OF PEOPLE SET.\n\n");
			INT(v[0]) = atoi(argument);
			break;
		case 1:
			buf_append(output, "MAX WEIGHT SET.\n\n");
			INT(v[1]) = atoi(argument);
			break;
		case 2:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(furniture_flags, argument)) < 0) {
				show_flags_buf(output, furniture_flags);
				return 2;
			}
		        buf_append(output, "FURNITURE FLAGS TOGGLED.\n\n");
			TOGGLE_BIT(INT(v[2]), val);
			break;
		case 3:
			buf_append(output, "HEAL BONUS SET.\n\n");
			INT(v[3]) = atoi(argument);
			break;
		case 4:
			buf_append(output, "MANA BONUS SET.\n\n");
			INT(v[4]) = atoi(argument);
			break;
		}
		break;

	case ITEM_CONTAINER:
		switch (value_num) {
		case 0:
			buf_append(output, "WEIGHT CAPACITY SET.\n\n");
			INT(v[0]) = atoi(argument);
			break;
		case 1:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(cont_flags, argument)) < 0) {
				show_flags_buf(output, cont_flags);
				return 2;
			}
			buf_append(output, "CONTAINER TYPE SET.\n\n");
			TOGGLE_BIT(INT(v[1]), val);
			break;
		case 2:
			if (atoi(argument) != 0) {
				if (!get_obj_index(atoi(argument))) {
					buf_append(output, "THERE IS NO SUCH ITEM.\n\n");
					return 1;
				}

				if (get_obj_index(atoi(argument))->item_type != ITEM_KEY) {
					buf_append(output, "THAT ITEM IS NOT A KEY.\n\n");
					return 1;
				}
			}
			buf_append(output, "CONTAINER KEY SET.\n\n");
			INT(v[2]) = atoi(argument);
			break;
		case 3:
			buf_append(output, "CONTAINER MAX WEIGHT SET.\n");
			INT(v[3]) = atoi(argument);
			break;
		case 4:
			buf_append(output, "WEIGHT MULTIPLIER SET.\n\n");
			INT(v[4]) = atoi(argument);
			break;
		}
		break;

	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_append(output, "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\n");
			INT(v[0]) = atoi(argument);
			break;
		case 1:
			buf_append(output, "CURRENT AMOUNT OF LIQUID HOURS SET.\n\n");
			INT(v[1]) = atoi(argument);
			break;
		case 2:
			if (!str_cmp(argument, "?")
			||  (liq = liquid_search(argument)) == NULL) {
				mlstrkey_printall(&liquids, output);
				return 2;
			}

			buf_append(output, "LIQUID TYPE SET.\n\n");
			STR_ASSIGN(v[2],
				   str_qdup(gmlstr_mval(&liq->lq_name)));
			break;
		case 3:
			buf_append(output, "POISON VALUE TOGGLED.\n\n");
			INT(v[3]) = !INT(v[3]);
			break;
		}
		break;

	case ITEM_FOOD:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_append(output, "HOURS OF FOOD SET.\n\n");
			INT(v[0]) = atoi(argument);
			break;
		case 1:
			buf_append(output, "HOURS OF FULL SET.\n\n");
			INT(v[1]) = atoi(argument);
			break;
		case 3:
			buf_append(output, "POISON VALUE TOGGLED.\n\n");
			INT(v[3]) = !INT(v[3]);
			break;
		}
		break;

	case ITEM_MONEY:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_append(output, "SILVER AMOUNT SET.\n\n");
			INT(v[0]) = atoi(argument);
			break;
		case 1:
			buf_append(output, "GOLD AMOUNT SET.\n\n");
			INT(v[1]) = atoi(argument);
			break;
		}
		break;

	case ITEM_BOOK:
		switch (value_num) {
		case 0:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(book_class, argument)) < 0) {
				show_flags_buf(output, book_class);
				return 2;
			}
			buf_append(output, "BOOK CLASS SET.\n\n");
			INT(v[0]) = val;
			break;
		case 1:
			if (!str_cmp(argument, "none")) {
				STR_ASSIGN(v[1], str_empty);
				break;
			}

			if (!str_cmp(argument, "?")
			||  (spc = spec_lookup(argument)) == 0) {
				buf_append(output, "No such spec.\n");
				return 2;
			}

			buf_append(output, "SPEC SET.\n\n");
			STR_ASSIGN(v[1], str_qdup(spc->spec_name));
			break;

		case 2:
			buf_append(output, "BASE CHANCE SET.\n\n");
			INT(v[2]) = atoi(argument);
			break;

		case 3:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(fail_effects, argument)) < 0) {
				show_flags_buf(output, fail_effects);
				return 2;
			}
			buf_append(output, "FAIL EFFECT SET.\n\n");
			INT(v[3]) = val;
			break;

		case 4:
			STR_ASSIGN(v[4], str_dup(argument));
			buf_append(output, "SUCCESS MESSAGE SET.\n\n");
			break;
		}
	}

	return 0;
}
