#ifndef _COMM_H_
#define _COMM_H_

/*
 * $Id: comm.h,v 1.4 1998-05-05 03:22:19 fjoe Exp $
 */

/* comm.c */
void	show_string	(struct descriptor_data *d, char *input);
void	close_socket	(DESCRIPTOR_DATA *dclose);
void	write_to_buffer	(DESCRIPTOR_DATA *d, const char *txt, int length);

#define send_to_char(txt, ch) char_puts(txt, ch)
void	char_puts(const char *txt, CHAR_DATA *ch);
void	char_printf(CHAR_DATA *ch, const char *format, ...);

void	page_to_char( const char *txt, CHAR_DATA *ch);

void	act(const char *format, CHAR_DATA *ch,
	    const void *arg1, const void *arg2, int type);
void	act_puts(const char *format, CHAR_DATA *ch, 
		 const void *arg1, const void *arg2, int type,
		 int min_pos);
void    act_printf(CHAR_DATA *ch, const void *arg1, const void *arg2,
		   int type, int min_pos, int msg_num, ...);

void    dump_to_scr(char *text);

#endif
