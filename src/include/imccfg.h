/*
 * IMC2 version 0.10 - an inter-mud communications protocol
 * Copyright (C) 1996 & 1997 Oliver Jowett <oliver@randomly.org>
 *
 * IMC2 Gold versions 1.00 though 2.00 are developed by MudWorld.
 * Copyright (C) 1999 - 2002 Haslage Net Electronics (Anthony R. Haslage)
 *
 * IMC2 MUD-Net version 3.10 is developed by Alsherok and Crimson Oracles
 * Copyright (C) 2002 Roger Libiez ( Samson )
 * Additional code Copyright (C) 2002 Orion Elder
 * Registered with the United States Copyright Office
 * TX 5-555-584
 *
 * IMC2 Continuum version 4.00 is developed by Rogel
 * Copyright (C) 2003 by Rogel, WhoStyles Copyright (C) 2003 by Kris Craig
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: imccfg.h,v 1.1.2.3 2003-10-02 07:24:28 matrim Exp $
 */

#define SOCIALSTRUCT		// We have olc for socials
#define first_descriptor	descriptor_list
#define IMCMAX_LEVEL	MAX_LEVEL
#define IMCSTRALLOC	str_dup
#define IMCSTRFREE	free_string
#define IMCMAXPLAYERS	0
#define CH_IMCDATA(ch)	(PC(ch)->imcchardata)
#define CH_IMCLEVEL(ch)	((ch)->level)
#define CH_IMCNAME(ch)	((ch)->name)
#define CH_IMCTITLE(ch)	(PC(ch)->title)
#define CH_IMCRANK(ch)	(str_empty)
#define CH_IMCSEX(ch)	((ch)->sex)
#define FIRST_IMCBLACKLIST(ch)	(PC(ch)->imcchardata->imcfirst_blacklist)
#define LAST_IMCBLACKLIST(ch)	(PC(ch)->imcchardata->imclast_blacklist)
#define IMC_CSUBSCRIBED(ch)	(PC(ch)->imcchardata->imc_csubscribed)
#define IMC_RREPLY(ch)	(PC(ch)->imcchardata->rreply)
#define IMC_PFLAGS(ch)	(PC(ch)->imcchardata->imc_pflags)
#define IS_IMCVISIBLE(ch)	(!IS_NPC(ch) && !IS_SET(IMC_PFLAGS(ch), PSET_IMCPVISIBLE) && !ch->invis_level && !ch->incog_level && !IS_AFFECTED(ch, AFF_FADE | AFF_HIDE | AFF_CAMOUFLAGE | AFF_INVIS | AFF_BLEND | AFF_IMP_INVIS))
