/*
 * $Id: recycle.h,v 1.12 1998-09-17 15:51:22 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#ifndef _RECYCLE_H_
#define _RECYCLE_H_

/* externs */
extern int mobile_count;

/* note recycling */
#define ND NOTE_DATA
ND	*new_note  (void);
void	free_note  (NOTE_DATA *note);
#undef ND

/* ban data recycling */
#define BD BAN_DATA
BD	*new_ban  (void);
void	free_ban  (BAN_DATA *ban);
#undef BD

/* descriptor recycling */
#define DD DESCRIPTOR_DATA
DD	*new_descriptor  (void);
void	free_descriptor  (DESCRIPTOR_DATA *d);
#undef DD

/* extra descr recycling */
ED_DATA	*ed_new(void);
ED_DATA *ed_new2(const ED_DATA *ed, const char* name);
void	ed_fread(FILE *fp, ED_DATA **edp);
void	ed_free  (ED_DATA *ed);

/* affect recycling */
#define AD AFFECT_DATA
AD	*new_affect  (void);
void	free_affect  (AFFECT_DATA *af);
#undef AD

/* object recycling */
#define OD OBJ_DATA
OD	*new_obj  (void);
void	free_obj  (OBJ_DATA *obj);
#undef OD

/* character recyling */
CHAR_DATA	*new_char  (void);
void	free_char  (CHAR_DATA *ch);
PC_DATA	*new_pcdata  (void);
void	free_pcdata  (PC_DATA *pcdata);

/* mob id and memory procedures */
long 	get_pc_id  (void);
long	get_mob_id  (void);

HELP_DATA *	help_new	(void);
void		help_add	(AREA_DATA*, HELP_DATA*);
HELP_DATA *	help_lookup	(int num, const char *keyword);
void		help_show	(CHAR_DATA *ch, BUFFER *output,
				 const char *keyword);
void		help_free	(HELP_DATA*);

#endif

