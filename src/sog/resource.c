/*
 * $Id: resource.c,v 1.4 1998-04-16 14:29:29 efdi Exp $
 */

#include "resource.h"

char *BLANK_STRING = "{RBUG{x";

char *strings_engl[] =  {
/* English messages */
"",
/* INFO_NOTHING */ 		"Nothing.\n\r",

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
/* EQ_STUCK_IN */		"<stuck in>          ",

/* INFO_SLEEPING_AT */		" is sleeping at %s.",
/* INFO_SLEEPING_ON */		" is sleeping on %s.",
/* INFO_SLEEPING_IN */		" is sleeping in %s.",
/* INFO_SLEEPING */		" is sleeping here.",
/* INFO_RESTING_AT */		" is resting at %s.",
/* INFO_RESTING_ON */		" is resting on %s.",
/* INFO_RESTING_IN */		" is resting in %s.",
/* INFO_RESTING */		" is resting here.",
/* INFO_SITTING_AT */		" is sitting at %s.",
/* INFO_SITTING_ON */		" is sitting on %s.",
/* INFO_SITTING_IN */		" is sitting in %s.",
/* INFO_SITTING */		" is sitting here.",
/* INFO_STANDING_AT */		" is standing at %s.",
/* INFO_STANDING_ON */		" is standing on %s.",
/* INFO_STANDING_IN */		" is standing in %s.",
/* INFO_STANDING */		" is standing here.",
/* INFO_FIGHTING */		" is here, fighting with ",
/* INFO_IS_HERE */		" is here. ",

/* INFO_IS_IN_PERFECT_HEALTH */	"{Cis in perfect health{x.",
/* INFO_HAS_A_FEW_SCRATCHES */	"{bhas a few scratches{x.",
/* INFO_HAS_SOME_SMALL_BUT_DISGUSTING_CUTS */ "{Bhas some small but disgusting cuts{x.",
/* INFO_IS_COVERED_WITH_BLEEDING_WOUNDS */ "{Gis covered with bleeding wounds{x.",
/* INFO_IS_GUSHING_BLOOD */	"{Yis gushing blood{x.",
/* INFO_IS_WRITHING_IN_AGONY */	"{Mis writhing in agony{x.",
/* INFO_IS_CONVULSING_ON_THE_GROUND */ "{Ris convulsing on the ground{x.",
/* INFO_IS_NEARLY_DEAD */	"{Ris nearly dead{x.",

/* INFO_POS_NAME_DEAD_M */	"{RDEAD{x",
/* INFO_POS_NAME_MORTALLY_WOUNDED_M */	"{Rmortally wounded{x",
/* INFO_POS_NAME_INCAPACITATED_M */	"{Rincapacitated{x",
/* INFO_POS_NAME_STUNNED_M */	"{Rstunned{x",
/* INFO_POS_NAME_SLEEPING_M */	"{Gsleeping{x",
/* INFO_POS_NAME_RESTING_M */	"{Gresting{x",
/* INFO_POS_NAME_SITTING_M */	"{Gsitting{x",
/* INFO_POS_NAME_FIGHTING_M */	"{Rfighting{x",
/* INFO_POS_NAME_STANDING_M */	"{Gstanding{x",
/* INFO_POS_NAME_DEAD_F */	"{RDEAD{x",
/* INFO_POS_NAME_MORTALLY_WOUNDED_F */	"{Rmortally wounded{x",
/* INFO_POS_NAME_INCAPACITATED_F */	"{Rincapacitated{x",
/* INFO_POS_NAME_STUNNED_F */	"{Rstunned{x",
/* INFO_POS_NAME_SLEEPING_F */	"{Gsleeping{x",
/* INFO_POS_NAME_RESTING_F */	"{Gresting{x",
/* INFO_POS_NAME_SITTING_F */	"{Gsitting{x",
/* INFO_POS_NAME_FIGHTING_F */	"{Rfighting{x",
/* INFO_POS_NAME_STANDING_F */	"{Gstanding{x",

/* INFO_YOU_ARE */		"You are",
};

char *strings_rus[] = {
/* Russian messages (KOI-8) */
"",
/* INFO_NOTHING */ 		"������.\n\r",	

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
/* EQ_STUCK_IN */		"<stuck in>            ",

/* INFO_SLEEPING_AT */		" ���� � %s.",
/* INFO_SLEEPING_ON */		" ���� �� %s.",
/* INFO_SLEEPING_IN */		" ���� � %s.",
/* INFO_SLEEPING */		" ���� �����.",
/* INFO_RESTING_AT */		" �������� � %s.",
/* INFO_RESTING_ON */		" �������� �� %s.",
/* INFO_RESTING_IN */		" �������� � %s.",
/* INFO_RESTING */		" �������� �����.",
/* INFO_SITTING_AT */		" ����� � %s.",
/* INFO_SITTING_ON */		" ����� �� %s.",
/* INFO_SITTING_IN */		" ����� � %s.",
/* INFO_SITTING */		" ����� �����.",
/* INFO_STANDING_AT */		" ����� � %s.",
/* INFO_STANDING_ON */		" ����� �� %s.",
/* INFO_STANDING_IN */		" ����� � %s.",
/* INFO_STANDING */		" ����� �����.",
/* INFO_FIGHTING */		" �����, ��������� � ",
/* INFO_IS_HERE */		" �����. ",

/* INFO_IS_IN_PERFECT_HEALTH */	"{C� �������� ���������{x.",
/* INFO_HAS_A_FEW_SCRATCHES */	"{b����� ��������� �������{x.",
/* INFO_HAS_SOME_SMALL_BUT_DISGUSTING_CUTS */ "{B����� ��������� ��������� ��� � �������{x.",
/* INFO_IS_COVERED_WITH_BLEEDING_WOUNDS */ "{G����� �������� ����� ���{x.",
/* INFO_IS_GUSHING_BLOOD */	"{Y�������� ������{x.",
/* INFO_IS_WRITHING_IN_AGONY */	"{M�أ��� � ������{x.",
/* INFO_IS_CONVULSING_ON_THE_GROUND */ "{R� ������� ���������{x.",
/* INFO_IS_NEARLY_DEAD */	"{R����� ����{x.",

/* INFO_POS_NAME_DEAD_M */	"{R����{x",
/* INFO_POS_NAME_MORTALLY_WOUNDED_M */	"{R���������� �����{x",
/* INFO_POS_NAME_INCAPACITATED_M */	"{R���������{x",
/* INFO_POS_NAME_STUNNED_M */	"{R�����{x",
/* INFO_POS_NAME_SLEEPING_M */	"{G�����{x",
/* INFO_POS_NAME_RESTING_M */	"{G���������{x",
/* INFO_POS_NAME_SITTING_M */	"{G������{x",
/* INFO_POS_NAME_FIGHTING_M */	"{R����������{x",
/* INFO_POS_NAME_STANDING_M */	"{G������{x",
/* INFO_POS_NAME_DEAD_F */	"{R������{x",
/* INFO_POS_NAME_MORTALLY_WOUNDED_F */	"{R���������� ������{x",
/* INFO_POS_NAME_INCAPACITATED_F */	"{R����������{x",
/* INFO_POS_NAME_STUNNED_F */	"{R������{x",
/* INFO_POS_NAME_SLEEPING_F */	"{G�����{x",
/* INFO_POS_NAME_RESTING_F */	"{G���������{x",
/* INFO_POS_NAME_SITTING_F */	"{G������{x",
/* INFO_POS_NAME_FIGHTING_F */	"{R����������{x",
/* INFO_POS_NAME_STANDING_F */	"{G������{x",

/* INFO_YOU_ARE */		"��",
};

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
