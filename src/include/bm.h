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
 * 3. Follow all guidelines in the diku license (contained in the file
 *    license.doc)
 * 4. Follow all guidelines in the Merc license (contained in the file
 *    license.txt)
 * 5. Follow all guidelines in the ROM license (contained in the file
 *    rom.license)
 * 6. Follow all guidelines in the Anatolia license (contained in the file
 *    anatolia.license)
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
 * $Id: bm.h,v 1.1.2.3 2002-10-24 07:59:25 tatyana Exp $
 */

#ifndef _BM_H_
#define _BM_H_

typedef struct bmitem_t bmitem_t;
struct bmitem_t
{
	OBJ_DATA	*obj;
	const char	*seller;
	const char	*buyer;
	int		bet;
	int		timer;
	bmitem_t	*next;
};

bmitem_t *bmitem_new(void);
void bmitem_free(bmitem_t *item);
void save_black_market(void);
void send_notice(CHAR_DATA *victim, bmitem_t *item, int type);
bool is_on_black_market(OBJ_DATA *obj);

extern bmitem_t *bmitem_list;

#define FINISH_PRICE(obj)		((obj->level + 9) * 100)
#define START_PRICE(obj)		((obj->level + 9) * 20)

#define TIME_NOBET		1500
#define TIME_HASBET		500
#define TIME_FINISH		50

#define NOTICE_BUYER		1
#define NOTICE_BET		2
#define NOTICE_SELLER		3

#endif /* _BM_H_ */
