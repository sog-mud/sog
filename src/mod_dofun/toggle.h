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
 * $Id: toggle.h,v 1.1.2.1 2002-11-23 18:54:05 fjoe Exp $
 */

#ifndef _TOGGLE_H_
#define _TOGGLE_H_

/*-----------------------------------------------------------------------------
 * toggle bit stuff
 */

struct toggle_t {
	const char *name;	/* flag name				*/
	const char *desc;	/* toggle description			*/
	flag_t *f;		/* flag table				*/
	flag64_t bit;		/* flag bit				*/
	const char *msg_on;	/* msg to print when flag toggled on	*/
	const char *msg_off;	/* ---//--- off				*/
};
typedef struct toggle_t toggle_t;

void toggle(CHAR_DATA *ch, const char *argument, toggle_t *tbl);
void print_toggles(CHAR_DATA *ch, toggle_t *t);

#endif /* _TOGGLE_H_ */
