/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: colors.c,v 1.8 2000-01-26 09:43:10 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include "typedef.h"
#include "comm_colors.h"
#include "str.h"

static const char* color(char type, int format);

enum {
	COLOR_BLACK,		/* normal colors */
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_WHITE,

	COLOR_DARK_GREY,	/* light colors */
	COLOR_BRIGHT_RED,
	COLOR_BRIGHT_GREEN,
	COLOR_BRIGHT_YELLOW,
	COLOR_BRIGHT_BLUE,
	COLOR_BRIGHT_MAGENTA,
	COLOR_BRIGHT_CYAN,
	COLOR_BRIGHT_WHITE,

	COLOR_CLEAR,		/* special colors */
	COLOR_BEEP,
	COLOR_CR,
	COLOR_LF,
	COLOR_LT,
	COLOR_GT,
	COLOR_AMP,

	COLOR_MAX
};

typedef struct format_data FORMAT_DATA;
struct format_data {
	const char *	name;
	const char *	colors[COLOR_MAX];
};

static int reset_color = COLOR_CLEAR;
static int curr_color = COLOR_CLEAR;

FORMAT_DATA format_table[] =
{
	{ "dumb",
		{
			str_empty,	/* normal colors */
			str_empty,
			str_empty,
			str_empty,
			str_empty,
			str_empty,
			str_empty,
			str_empty,

			str_empty,	/* light colors */
			str_empty,
			str_empty,
			str_empty,
			str_empty,
			str_empty,
			str_empty,
			str_empty,

			str_empty,	/* special colors */
			str_empty,
			str_empty,
			"\n\r",
			"<",
			">",
			"&"
		}
	},

	{ "ansi",
		{
			"\033[0;30m",
			"\033[0;31m",
			"\033[0;32m",
			"\033[0;33m",
			"\033[0;34m",
			"\033[0;35m",
			"\033[0;36m",
			"\033[0;37m",

			"\033[1;30m",
			"\033[1;31m",
			"\033[1;32m",
			"\033[1;33m",
			"\033[1;34m",
			"\033[1;35m",
			"\033[1;36m",
			"\033[1;37m",

			"\033[0m",
			"\007",
			str_empty,
			"\n\r",
			"<",
			">",
			"&"
		}
	},

	{ "html",
		{
			"<FONT COLOR=#000000>",
			"<FONT COLOR=#800000>",
			"<FONT COLOR=#008000>",
			"<FONT COLOR=#808000>",
			"<FONT COLOR=#000080>",
			"<FONT COLOR=#800080>",
			"<FONT COLOR=#008080>",
			"<FONT COLOR=#C0C0C0>",

			"<FONT COLOR=#606060>",
			"<FONT COLOR=#FF0000>",
			"<FONT COLOR=#00FF00>",
			"<FONT COLOR=#FFFF00>",
			"<FONT COLOR=#0000FF>",
			"<FONT COLOR=#FF00FF>",
			"<FONT COLOR=#00FFFF>",
			"<FONT COLOR=#FFFFFF>",

			"<FONT COLOR=#C0C0C0>",
			str_empty,
			str_empty,
			"\n",
			"&lt;",
			"&gt;",
			"&amp;"
		}
	},

	{ NULL }
};

static const char special_symbols[] = "\r\n;<>";

/*
 * Parse color symbols. len MUST BE > 1
 */
void parse_colors(const char *i, char *o, size_t len, int format)
{
	char *p;

	reset_color = curr_color = COLOR_CLEAR;
	for (p = o; *i && p - o < len - 1; i++) {
		if (strchr(special_symbols, *i) != NULL
		||  (*i == '{' && *(i+1))) {
			strnzcpy(p, len - 1 - (p - o),
				 color(*i == '{' ? *++i : *i, format));
			p = strchr(p, '\0');
			continue;
		}
		*p++ = *i;
	}
	*p = '\0';
}

static const char* color(char type, int format)
{
	int color;

	switch (type) {
/* normal colors */
	case 'd':
		color = COLOR_BLACK;
		break;
	case 'r':
		color = COLOR_RED;
		break;
	case 'g':
		color = COLOR_GREEN;
		break;
	case 'y':
		color = COLOR_YELLOW;
		break;
	case 'b':
		color = COLOR_BLUE;
		break;
	case 'm':
		color = COLOR_MAGENTA;
		break;
	case 'c':
		color = COLOR_CYAN;
		break;
	case 'w':
		color = COLOR_WHITE;
		break;

/* light colors */
	case 'D':
		color = COLOR_DARK_GREY;
		break;
	case 'R':
		color = COLOR_BRIGHT_RED;
		break;
	case 'G':
		color = COLOR_BRIGHT_GREEN;
		break;
	case 'Y':
		color = COLOR_BRIGHT_YELLOW;
		break;
	case 'B':
		color = COLOR_BRIGHT_BLUE;
		break;
	case 'M':
		color = COLOR_BRIGHT_MAGENTA;
		break;
	case 'C':
		color = COLOR_BRIGHT_CYAN;
		break;
	case 'W':
		color = COLOR_BRIGHT_WHITE;
		break;

/* special colors */
	case 'x':
		color = COLOR_CLEAR;
		break;
	case '*':
		color = COLOR_BEEP;
		break;
	case '\r':
		return format_table[format].colors[COLOR_CR];
	case '\n':
		return format_table[format].colors[COLOR_LF];
	case '<':
		return format_table[format].colors[COLOR_LT];
	case '>':
		return format_table[format].colors[COLOR_GT];
	case '&':
		return format_table[format].colors[COLOR_AMP];
/* special cases */
	case 'z':
		return format_table[format].colors[curr_color = reset_color];
		break;
	case '{':
		return "{";
	default:
		return str_empty;
	}

	reset_color = curr_color;
	return format_table[format].colors[curr_color = color];
}

int format_lookup(const char *name)
{
	int i;

	for (i = 0; format_table[i].name; i++)
		if (!str_cmp(name, format_table[i].name))
			return i;

	return FORMAT_DUMB;
}

