/*
 * $Id: resource.c,v 1.8 1998-04-18 07:11:56 fjoe Exp $
 */

#include "resource.h"
#include "english_strings.inc"
#include "russian_strings.inc"

char *BLANK_STRING = "{RBUG!!!{x";

char *msg(int resnum, int lang)
{
	switch( lang ) {
		case LANG_ENG:
			if (resnum >= sizeof(strings_engl) / sizeof(char*))
				return BLANK_STRING;
			else
				return strings_engl[ resnum ];
		case LANG_RUS:
			if (resnum >= sizeof(strings_rus) / sizeof(char*))
				return BLANK_STRING;
			else
				return strings_rus[ resnum ];
		default:
			return BLANK_STRING;
	}
}
