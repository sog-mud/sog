#include "resource.h"

char *BLANK_STRING = "{RBUG{x";

char *strings_engl[] =  {
/* English messages */
"",
/* INFO_NOTHING */ "Nothing.\n\r",

/* EQ_USED_AS_LIGHT */		"<used as light>     ",
/* EQ_WORN_ON_FINGER_1 */	"<worn on finger>    ",
/* EQ_WORN_ON_FINGER_2 */	"<worn on finger>    ",
/* EQ_WORN_AROUND_NECK_1 */	"<worn around neck>  ",
/* EQ_WORN_AROUND_NECK_2 */	"<worn around neck>  ",
/* EQ_WORN_ON_TORSO */		"<worn on torso>     ",
/* EQ_WORN_ON_HEAD */		"<worn on head>      ",
/* EQ_WORN_ON_LEGS */		"<worn on legs>      ",
/* EQ_WORN_ON_FEET */		"<worn on feet>      ",
/* EQ_WORN_ON_HANDS */		"<worn on hands>     ",
/* EQ_WORN_ON_ARMS */		"<worn on arms>      ",
/* EQ_WORN_AS_SHIELD */		"<worn as shield>    ",
/* EQ_WORN_ABOUT_BODY */	"<worn about body>   ",
/* EQ_WORN_ABOUT_WAIST */	"<worn about waist>  ",
/* EQ_WORN_ABOUT_WRIST_1 */	"<worn about wrist>  ",
/* EQ_WORN_ABOUT_WRIST_2 */	"<worn about wrist>  ",
/* EQ_WIELDED */		"<wielded>           ",
/* EQ_HELD */			"<held>              ",
/* EQ_FLOATING_NEARBY */	"<floating nearby>   ",
/* EQ_SCRATCHED_TATTOO */	"<scratched tattoo>  ",
/* EQ_DUAL_WIELDED */		"<dual wielded>      ",

/* INFO_SLEEPING_AT */		" is sleeping at %s.",
/* INFO_SLEEPING_ON */		" is sleeping on %s.",
/* INFO_SLEEPING_IN */		" is sleeping in %s.",
/* INFO_SLEEPING */		" is sleeping here.",
};
char *strings_rus[] = {
/* Russian messages (KOI-8) */
"",
/* INFO_NOTHING */ "������.\n\r",	

/* EQ_USED_AS_LIGHT */		"<��������>            ",
/* EQ_WORN_ON_FINGER1 */	"<������ �� �����>     ",
/* EQ_WORN_ON_FINGER2 */	"<������ �� �����>     ",
/* EQ_WORN_AROUND_NECK_1 */	"<������ �� ���>       ",
/* EQ_WORN_AROUND_NECK_2 */	"<������ �� ���>       ",
/* EQ_WORN_ON_TORSO */		"<������ �� ����>      ",
/* EQ_WORN_ON_HEAD */		"<������ �� ������>    ",
/* EQ_WORN_ON_LEGS */		"<������ �� ����>      ",
/* EQ_WORN_ON_FEET */		"<������ �� ������>    ",
/* EQ_WORN_ON_HANDS */		"<������ �� ����>      ",
/* EQ_WORN_ON_ARMS */		"<������� �� �����>    ",
/* EQ_WORN_AS_SHIELD */		"<���>                 ",
/* EQ_WORN_ABOUT_BODY */	"<�������� ������>     ",
/* EQ_WORN_ABOUT_WAIST */	"<�� �����>            ",
/* EQ_WORN_ABOUT_WRIST_1 */	"<����������� ��������>",
/* EQ_WORN_ABOUT_WRIST_2 */	"<����������� ��������>",
/* EQ_WIELDED */		"<����������>          ",
/* EQ_HELD */			"<������ � ����>       ",
/* EQ_FLOATING_NEARBY */	"<�������� ����������> ",
/* EQ_SCRATCHED_TATTOO */	"<����������>          ",
/* EQ_DUAL_WIELDED */		"<dual wielded>        ",

/* INFO_SLEEPING_AT */		" ���� � %s.",
/* INFO_SLEEPING_ON */		" ���� �� %s.",
/* INFO_SLEEPING_IN */		" ���� � %s.",
/* INFO_SLEEPING */		" ���� �����.",
};

char *msg(int resnum, int lang)
{
	if( resnum > MAX_STRING )
		return BLANK_STRING;

	switch( lang ) {
		case LANG_ENG:
			return strings_engl[ resnum ];
		case LANG_RUS:
			return strings_rus[ resnum ];
		default:
			return BLANK_STRING;
	}
}
