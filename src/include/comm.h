/*
 * $Id: comm.h,v 1.24 1998-11-25 15:17:58 fjoe Exp $
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
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
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

#ifndef _COMM_H_
#define _COMM_H_

void	show_string	(struct descriptor_data *d, char *input);
void	close_descriptor(DESCRIPTOR_DATA *dclose);
void	write_to_buffer	(DESCRIPTOR_DATA *d, const char *txt, unsigned int length);

void	char_puts(const char *txt, CHAR_DATA *ch);
#define char_nputs(msgid, ch) char_puts(msg(msgid, ch), ch)
#define char_mlputs(m, ch) char_puts(mlstr_cval(m, ch), ch)
void	char_printf(CHAR_DATA *ch, const char *format, ...);

void	send_to_char(const char *txt, CHAR_DATA *ch);
void	page_to_char( const char *txt, CHAR_DATA *ch);

/* the following 5 act target flags are exclusive */
#define TO_ROOM		(A)
#define TO_NOTVICT	(B)
#define TO_VICT		(C)
#define TO_CHAR		(D)
#define TO_ALL		(E)

#define ACT_TOBUF	(F)
#define ACT_NOTRIG	(G)
#define ACT_NOTWIT	(H)
#define ACT_TRANS	(I)	/* do $t and $T translation		    */
#define ACT_NODEAF	(J)	/* skip is_affected(to, gsn_deafen) chars   */
#define ACT_STRANS	(K)	/* do $t and $T slang translation (from ch) */
#define ACT_NOMORTAL	(L)	/* skip mortals */
#define ACT_VERBOSE	(M)	/* skip if (!IS_SET(ch->comm, COMM_VERBOSE)) */

#define act(format, ch, arg1, arg2, type) \
		act_printf(ch, arg1, arg2, type, POS_RESTING, format)
#define	act_puts(format, ch, arg1, arg2, type, min_pos) \
		act_printf(ch, arg1, arg2, type, min_pos, format)
void    act_printf(CHAR_DATA *ch, const void *arg1, const void *arg2,
		   int type, int min_pos, const char* format, ...);
#define	act_nputs(msg_num, ch, arg1, arg2, type, min_pos) \
		act_nprintf(ch, arg1, arg2, type, min_pos, msg_num)
void    act_nprintf(CHAR_DATA *ch, const void *arg1, const void *arg2,
		    int type, int min_pos, int msg_num, ...);

void    dump_to_scr(char *text);

#endif
