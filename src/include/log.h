#ifndef _LOG_H_
#define _LOG_H_

/*
 * $Id: log.h,v 1.1 1998-06-18 05:19:13 fjoe Exp $
 */

void	bug		(const char *str, int param);
#define log(str) log_printf(str)
void	log_printf	(const char *str, ...);

#endif
