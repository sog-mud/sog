#ifndef _COMM_H_
#define _COMM_H_

/*
 * $Id: comm.h,v 1.5 1998-05-07 07:05:00 fjoe Exp $
 */

/* comm.c */
void	show_string	(struct descriptor_data *d, char *input);
void	close_socket	(DESCRIPTOR_DATA *dclose);
void	write_to_buffer	(DESCRIPTOR_DATA *d, const char *txt, int length);

#define send_to_char(txt, ch) char_puts(txt, ch)
void	char_puts(const char *txt, CHAR_DATA *ch);
#define char_nputs(msgid, ch) char_puts(msg(msgid, ch), ch)
void	char_printf(CHAR_DATA *ch, const char *format, ...);
void	char_nprintf(CHAR_DATA *ch, int msgid, ...);

void	page_to_char( const char *txt, CHAR_DATA *ch);

#define act(format, ch, arg1, arg2, type) \
		act_puts(format, ch, arg1, arg2, type, POS_RESTING)
void	act_puts(const char *format, CHAR_DATA *ch, 
		 const void *arg1, const void *arg2, int type,
		 int min_pos);
void    act_printf(CHAR_DATA *ch, const void *arg1, const void *arg2,
		    int type, int min_pos, int msg_num, ...);

void    dump_to_scr(char *text);

#endif
