#ifndef _COMM_H_
#define _COMM_H_

/*
 * $Id: comm.h,v 1.11 1998-09-01 18:29:16 fjoe Exp $
 */

void	show_string	(struct descriptor_data *d, char *input);
void	close_socket	(DESCRIPTOR_DATA *dclose);
void	write_to_buffer	(DESCRIPTOR_DATA *d, const char *txt, int length);

#define send_to_char(txt, ch) char_puts(txt, ch)
void	char_puts(const char *txt, CHAR_DATA *ch);
#define char_nputs(msgid, ch) char_puts(msg(msgid, ch), ch)
#define char_mlputs(m, ch) char_puts(mlstr_cval(m, ch), ch)
void	char_printf(CHAR_DATA *ch, const char *format, ...);
void	char_nprintf(CHAR_DATA *ch, int msgid, ...);

void	page_to_char( const char *txt, CHAR_DATA *ch);

/* the following 5 act target flags are exclusive */
#define TO_ROOM		(A)
#define TO_NOTVICT	(B)
#define TO_VICT		(C)
#define TO_CHAR		(D)
#define TO_ALL		(E)

#define TO_BUF		(F)
#define NO_TRIGGER	(G)

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
void	act_mlputs(CHAR_DATA *ch, const void *arg1, const void *arg2,
		   int type, int min_pos, mlstring*);

void    dump_to_scr(char *text);

#endif
