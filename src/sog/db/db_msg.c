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
 * $Id: db_msg.c,v 1.10 1999-12-16 12:24:55 fjoe Exp $
 */

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#include "typedef.h"
#include "const.h"
#include "varr.h"
#include "hash.h"
#include "mlstring.h"
#include "msg.h"
#include "db.h"
#include "str.h"
#include "mlstring.h"
#include "tables.h"

DECLARE_DBLOAD_FUN(load_msg);
DECLARE_DBINIT_FUN(init_msgdb);

DBFUN dbfun_msg[] =
{
	{ "MSG",	load_msg	},
	{ NULL }
};

DBDATA db_msg = { dbfun_msg, init_msgdb };

DBINIT_FUN(init_msgdb)
{
	if (!DBDATA_VALID(dbdata)) {
		int i;
		for (i = 0; i < MAX_MSG_HASH; i++) 
			varr_init(msg_hash_table+i, sizeof(msg_t), 4);
	}
}

DBLOAD_FUN(load_msg)
{
	msg_t m;

	mlstr_init(&m.ml, NULL);
	m.gender = 0;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (mlstr_null(&m.ml)) {
					db_error("load_msg",
						 "msg text not defined");
				}
				else
					msg_add(&m);
				return;
			}
			break;
		case 'G':
			KEY("Gender", m.gender,
			    fread_fword(gender_table, fp));
			break;
		case 'T':
			MLSKEY("Text", m.ml);
			break;
		}

		if (!fMatch) {
			db_error("load_msg", "%s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
