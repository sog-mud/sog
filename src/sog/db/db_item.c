/*-
 * Copyright (c) 2002 SoG Development Team
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
 * $Id: db_item.c,v 1.1.2.3 2003-09-11 10:39:51 tatyana Exp $
 */

#include <stdio.h>
#include "merc.h"
#include "db.h"
#include "bm.h"

DECLARE_DBLOAD_FUN(load_black_market);

DBFUN dbfun_bm[] =
{
	{ "ITEM",		load_black_market	},
	{ NULL }
};

DBDATA db_black_market = { dbfun_bm };

DBLOAD_FUN(load_black_market)
{
	bmitem_t *item;
	OBJ_DATA *obj = NULL;
	char *word = feof(fp) ? "End" : fread_word(fp);

	if (!str_cmp(word, "Vnum")) {
		OBJ_INDEX_DATA *pObjIndex;
		int vnum = fread_number(fp);

		if ((pObjIndex = get_obj_index(vnum)) != NULL) {
			obj = create_obj(pObjIndex, 0);
		} else {
			fread_to_end(fp);
			return;
		}
	}

	if (obj == NULL)
		return;

	item = bmitem_new();
	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			if (!str_cmp(word, "Affc")) {
				AFFECT_DATA *paf = aff_new();
				const char *skname = fread_word(fp);
				int sn = sn_lookup(skname);
				if (sn < 0)
					log("fread_obj: %s: %s: unknown skill", BLACK_MARKET_CONF, skname);

				paf->type = sn;
				paf->where = fread_number(fp);
				paf->level = fread_number(fp);
				paf->duration = fread_number(fp);
				paf->modifier = fread_number(fp);
				paf->location = fread_number(fp);
				paf->bitvector = fread_flags(fp);

				if (sn < 0)
					aff_free(paf);
				else
					SLIST_ADD(AFFECT_DATA,
						  obj->affected, paf);

				fMatch = TRUE;
				break;
			}
			break;

		case 'B':
			KEY("Bet", item->bet, fread_number(fp));
			SKEY("Buyer", item->buyer);
			break;
		case 'C':
			KEY("Cond", obj->condition, fread_number(fp));
			KEY("Cost", obj->cost, fread_number(fp));
			break;

		case 'D':
			MLSKEY("Description", obj->description);
			MLSKEY("Desc", obj->description);
			break;

		case 'E':
			KEY("ExtF", obj->extra_flags, fread_flags(fp));

			if (!str_cmp(word, "ExDe")) {
				ed_fread(fp, &obj->ed);
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "End")) {
				item->obj = obj;
				item->next = bmitem_list;
				bmitem_list = item;
				log("Black market: %s", item->obj->name);
				return;
			}
			break;

		case 'I':
			KEY("ItemTimer", item->timer, fread_number(fp));
			break;
		case 'L':
			KEY("Lev", obj->level, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", obj->name);
			break;

		case 'O':
			MLSKEY("Owner", obj->owner);
			break;

		case 'S':
			SKEY("Seller", item->seller);
			MLSKEY("ShD", obj->short_descr);
			if (!str_cmp(word, "Spell")) {
				int iValue = fread_number(fp);
				const char *skname = fread_word(fp);
				int sn = sn_lookup(skname);

				if (iValue < 0 || iValue > 3)
					log("fread_item %d: bad iValue", iValue);
				else if (sn < 0)
					log("fread_item: %s: unknown skill", skname);
				else
					obj->value[iValue] = sn;

				fMatch = TRUE;
				break;
			}
			break;

		case 'T':
			KEY("Time", obj->timer, fread_number(fp));
			break;

		case 'V':
			if (!str_cmp(word, "Val")) {
				obj->value[0] = fread_number(fp);
				obj->value[1] = fread_number(fp);
				obj->value[2] = fread_number(fp);
				obj->value[3] = fread_number(fp);
				obj->value[4] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;
		}

		if (!fMatch)
			db_error("load_item", "%s: Unknown keyword", word);
	}
}
