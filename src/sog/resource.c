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
/* INFO_NOTHING */ "Ничего.\n\r",	

/* EQ_USED_AS_LIGHT */		"<освещает>            ",
/* EQ_WORN_ON_FINGER1 */	"<надето на палец>     ",
/* EQ_WORN_ON_FINGER2 */	"<надето на палец>     ",
/* EQ_WORN_AROUND_NECK_1 */	"<надето на шею>       ",
/* EQ_WORN_AROUND_NECK_2 */	"<надето на шею>       ",
/* EQ_WORN_ON_TORSO */		"<надето на тело>      ",
/* EQ_WORN_ON_HEAD */		"<надето на голову>    ",
/* EQ_WORN_ON_LEGS */		"<надето на ноги>      ",
/* EQ_WORN_ON_FEET */		"<надето на ступни>    ",
/* EQ_WORN_ON_HANDS */		"<надето на руки>      ",
/* EQ_WORN_ON_ARMS */		"<натдето на плечи>    ",
/* EQ_WORN_AS_SHIELD */		"<щит>                 ",
/* EQ_WORN_ABOUT_BODY */	"<накинуто сверху>     ",
/* EQ_WORN_ABOUT_WAIST */	"<на талии>            ",
/* EQ_WORN_ABOUT_WRIST_1 */	"<обхватывает запястья>",
/* EQ_WORN_ABOUT_WRIST_2 */	"<обхватывает запястья>",
/* EQ_WIELDED */		"<вооружение>          ",
/* EQ_HELD */			"<зажато в руке>       ",
/* EQ_FLOATING_NEARBY */	"<кружится поблизости> ",
/* EQ_SCRATCHED_TATTOO */	"<татуировка>          ",
/* EQ_DUAL_WIELDED */		"<dual wielded>        ",

/* INFO_SLEEPING_AT */		" спит у %s.",
/* INFO_SLEEPING_ON */		" спит на %s.",
/* INFO_SLEEPING_IN */		" спит в %s.",
/* INFO_SLEEPING */		" спит здесь.",
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
