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
 * $Id: socials.h,v 1.9 2001-07-08 17:18:47 fjoe Exp $
 */

#ifndef _SOCIALS_H_
#define _SOCIALS_H_

/*
 * Structure for a social in the socials table.
 */
struct social_t
{
	const char *	name;
	flag_t		min_pos;

	mlstring	found_char;
	mlstring	found_vict;
	mlstring	found_notvict;

	mlstring	noarg_char;
	mlstring	noarg_room;

	mlstring	self_char;
	mlstring	self_room;

	mlstring	notfound_char;
};
typedef struct social_t social_t;

extern varr socials;

void	social_init	(social_t *soc);
void	social_destroy	(social_t *soc);

#define social_lookup(name)	((social_t *) vstr_lookup(&socials, (name)))
#define social_search(name)	((social_t *) vstr_search(&socials, (name)))

#endif
