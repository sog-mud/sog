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
 * $Id: rwfile.h,v 1.8 1999-12-20 08:31:20 fjoe Exp $
 */

#ifndef _RFILE_H_
#define _RFILE_H_

#ifdef USE_MMAP

struct rfile_t {
	const u_char *	p;
	off_t	len;
	off_t	pos;
	int	fd;

	const u_char *	tok;		/* token for fread_word */
	off_t		tok_len;	/* token length		*/
};

rfile_t *	rfile_open(const char *dir, const char *file);
void		rfile_close(rfile_t *fp);

#define		rfile_feof(fp)		((fp)->pos >= (fp)->len)

bool		IS_TOKEN(rfile_t *fp, const char *k);
const char *	rfile_tok(rfile_t *fp);
#define		rfile_tokfl(fp)		(UPPER((fp)->tok[0]))

#else /* USE_MMAP */

#define rfile_open(dir, file)	dfopen((dir), (file), "r")
#define rfile_close(fp)		fclose(fp)
#define rfile_feof(fp)		feof(fp)

extern char _token[MAX_STRING_LENGTH];

#define		IS_TOKEN(fp, k)		(!str_cmp(_token, (k)))
#define		rfile_tok(fp)		(_token)
#define		rfile_tokfl(fp)		(UPPER(_token[0]))

#endif /* USE_MMAP */

extern int	line_number;

int		xgetc		(rfile_t *fp);
void		xungetc		(rfile_t *fp);

void		fread_word	(rfile_t *fp);
void		fread_keyword	(rfile_t *fp);
const char *	fread_sword	(rfile_t *fp);

const char *	fread_string	(rfile_t *fp);
char		fread_letter	(rfile_t *fp);
int		fread_number	(rfile_t *fp);
flag_t 		fread_flags	(rfile_t *fp);
int64_t		fread_flags64	(rfile_t *fp);
void		fread_to_end	(rfile_t *fp);
void		fread_to_eol	(rfile_t *fp);
flag_t		fread_fword	(const flaginfo_t *table, rfile_t *fp); 
flag_t		fread_fstring	(const flaginfo_t *table, rfile_t *fp);

#define KEY(k, field, val)				\
		if (IS_TOKEN(fp, (k))) {		\
			(field) = (val);		\
			fMatch = TRUE;			\
			break;				\
		}

#define SKEY(k, field, val)				\
		if (IS_TOKEN(fp, (k))) {		\
			free_string(field);		\
			(field) = (val);		\
			fMatch = TRUE;			\
			break;				\
		}

#define MLSKEY(k, field)				\
		if (IS_TOKEN(fp, (k))) {		\
			mlstr_destroy(&(field));	\
			mlstr_fread(fp, &(field));	\
			fMatch = TRUE;			\
			break;				\
		}

#endif

