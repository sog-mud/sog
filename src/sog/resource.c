/*
 * $Id: resource.c,v 1.9 1998-04-21 22:03:56 efdi Exp $
 */

#include <sys/time.h>
#include <stdio.h>
#include "merc.h"
#include "resource.h"
#include "english_strings.inc"
#include "russian_strings.inc"

char *BLANK_STRING = "{RBUG!!!{x";

int msg_fix_sex(int sex)
{
	return sex >= SEX_FEMALE ? SEX_FEMALE :
		sex <= SEX_NEUTRAL ? SEX_NEUTRAL : 
					SEX_MALE;	
}

char *msg(int resnum, CHAR_DATA *ch)
{
	if (resnum >= MSG_SEX_DEPENDENT)
		resnum += msg_fix_sex(ch->sex);
	if (resnum >= MSG_MAX_NUM)
		return BLANK_STRING;
	switch( ch->i_lang ) {
	case LANG_ENG:
		return strings_engl[ resnum ];
	case LANG_RUS:
		return strings_rus[ resnum ];
	default:
		return BLANK_STRING;
	}
}
