/*
 * $Id: resource.c,v 1.7 1998-04-17 18:10:24 efdi Exp $
 */

#include "resource.h"
#include "english_strings.inc"
#include "russian_strings.inc"

char *BLANK_STRING = "{RBUG!!!{x";

char *msg(int resnum, int lang)
{
	switch( lang ) {
		case LANG_ENG:
			if( resnum >= sizeof( strings_engl ) )
				return BLANK_STRING;
			else
				return strings_engl[ resnum ];
		case LANG_RUS:
			if( resnum >= sizeof( strings_rus ) )
				return BLANK_STRING;
			else
				return strings_rus[ resnum ];
		default:
			return BLANK_STRING;
	}
}
