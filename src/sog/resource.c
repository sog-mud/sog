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
/* INFO_NOTHING */ 		"Ничего.\n\r",	

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
/* EQ_STUCK_IN */		"<stuck in>            ",

/* INFO_SLEEPING_AT */		" спит у %s.",
/* INFO_SLEEPING_ON */		" спит на %s.",
/* INFO_SLEEPING_IN */		" спит в %s.",
/* INFO_SLEEPING */		" спит здесь.",
/* INFO_RESTING_AT */		" отдыхает у %s.",
/* INFO_RESTING_ON */		" отдыхает на %s.",
/* INFO_RESTING_IN */		" отдыхает в %s.",
/* INFO_RESTING */		" отдыхает здесь.",
/* INFO_SITTING_AT */		" сидит у %s.",
/* INFO_SITTING_ON */		" сидит на %s.",
/* INFO_SITTING_IN */		" сидит в %s.",
/* INFO_SITTING */		" сидит здесь.",
/* INFO_STANDING_AT */		" стоит у %s.",
/* INFO_STANDING_ON */		" стоит на %s.",
/* INFO_STANDING_IN */		" стоит в %s.",
/* INFO_STANDING */		" стоит здесь.",
/* INFO_FIGHTING */		" здесь, сражается с ",
/* INFO_IS_HERE */		" здесь. ",

/* INFO_IS_IN_PERFECT_HEALTH */	"{Cв отличном состоянии{x.",
/* INFO_HAS_A_FEW_SCRATCHES */	"{bимеет несколько царапин{x.",
/* INFO_HAS_SOME_SMALL_BUT_DISGUSTING_CUTS */ "{Bимеет несколько маленьких ран и синяков{x.",
/* INFO_IS_COVERED_WITH_BLEEDING_WOUNDS */ "{Gимеет довольно много ран{x.",
/* INFO_IS_GUSHING_BLOOD */	"{Yистекает кровью{x.",
/* INFO_IS_WRITHING_IN_AGONY */	"{Mбьётся в агонии{x.",
/* INFO_IS_CONVULSING_ON_THE_GROUND */ "{Rв ужасном состоянии{x.",
/* INFO_IS_NEARLY_DEAD */	"{Rпочти труп{x.",

/* INFO_POS_NAME_DEAD_M */	"{RМЁРТВ{x",
/* INFO_POS_NAME_MORTALLY_WOUNDED_M */	"{Rсмертельно ранен{x",
/* INFO_POS_NAME_INCAPACITATED_M */	"{Rобессилен{x",
/* INFO_POS_NAME_STUNNED_M */	"{Rустал{x",
/* INFO_POS_NAME_SLEEPING_M */	"{Gспишь{x",
/* INFO_POS_NAME_RESTING_M */	"{Gотдыхаешь{x",
/* INFO_POS_NAME_SITTING_M */	"{Gсидишь{x",
/* INFO_POS_NAME_FIGHTING_M */	"{Rсражаешься{x",
/* INFO_POS_NAME_STANDING_M */	"{Gстоишь{x",
/* INFO_POS_NAME_DEAD_F */	"{RМЕРТВА{x",
/* INFO_POS_NAME_MORTALLY_WOUNDED_F */	"{Rсмертельно ранена{x",
/* INFO_POS_NAME_INCAPACITATED_F */	"{Rобессилена{x",
/* INFO_POS_NAME_STUNNED_F */	"{Rустала{x",
/* INFO_POS_NAME_SLEEPING_F */	"{Gспишь{x",
/* INFO_POS_NAME_RESTING_F */	"{Gотдыхаешь{x",
/* INFO_POS_NAME_SITTING_F */	"{Gсидишь{x",
/* INFO_POS_NAME_FIGHTING_F */	"{Rсражаешься{x",
/* INFO_POS_NAME_STANDING_F */	"{Gстоишь{x",

/* INFO_YOU_ARE */		"Ты",
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
