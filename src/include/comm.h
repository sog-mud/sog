#ifndef _COMM_H_
#define _COMM_H_

/*
 * $Id: comm.h,v 1.6 1998-05-27 08:47:22 fjoe Exp $
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
		act_printf(ch, arg1, arg2, type, POS_RESTING, format)
#define	act_puts(format, ch, arg1, arg2, type, min_pos) \
		act_printf(ch, arg1, arg2, type, min_pos, format)
void    act_printf(CHAR_DATA *ch, const void *arg1, const void *arg2,
		   int type, int min_pos, const char* format, ...);
void    act_nprintf(CHAR_DATA *ch, const void *arg1, const void *arg2,
		    int type, int min_pos, int msg_num, ...);

void    dump_to_scr(char *text);

#endif
