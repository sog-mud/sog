#ifndef _RESOURCE_H_
#define _RESOURCE_H_

/*
 * $Id: resource.h,v 1.18 1998-04-26 21:29:05 efdi Exp $
 */

char* msg(int resnum, CHAR_DATA *ch);

#define LANG_ENG 0
#define LANG_RUS 1

enum {
	INFO_NOTHING = 1, 

	EQ_USED_AS_LIGHT,
	EQ_WORN_ON_FINGER_1,
	EQ_WORN_ON_FINGER_2,
	EQ_WORN_AROUND_NECK_1,
	EQ_WORN_AROUND_NECK_2,
	EQ_WORN_ON_TORSO,
	EQ_WORN_ON_HEAD,
	EQ_WORN_ON_LEGS,
	EQ_WORN_ON_FEET,
	EQ_WORN_ON_HANDS,
	EQ_WORN_ON_ARMS,
	EQ_WORN_AS_SHIELD,
	EQ_WORN_ABOUT_BODY,
	EQ_WORN_ABOUT_WAIST,
	EQ_WORN_AROUND_WRIST_1,
	EQ_WORN_AROUND_WRIST_2,
	EQ_WIELDED,
	EQ_HELD,
	EQ_FLOATING_NEARBY,
	EQ_SCRATCHED_TATTOO,
	EQ_DUAL_WIELDED,
	EQ_STUCK_IN,

	INFO_SLEEPING_AT,
	INFO_SLEEPING_ON,
	INFO_SLEEPING_IN,
	INFO_SLEEPING,
	INFO_RESTING_AT,
	INFO_RESTING_ON,
	INFO_RESTING_IN,
	INFO_RESTING,
	INFO_SITTING_AT,
	INFO_SITTING_ON,
	INFO_SITTING_IN,
	INFO_SITTING,
	INFO_STANDING_AT,
	INFO_STANDING_ON,
	INFO_STANDING_IN,
	INFO_STANDING,
	INFO_FIGHTING,
	INFO_IS_HERE,

	INFO_IS_IN_PERFECT_HEALTH,
	INFO_HAS_A_FEW_SCRATCHES,
	INFO_HAS_SOME_SMALL_BUT_DISGUSTING_CUTS,
	INFO_IS_COVERED_WITH_BLEEDING_WOUNDS,
	INFO_IS_WRITHING_IN_AGONY,
	INFO_IS_CONVULSING_ON_THE_GROUND,
	INFO_IS_NEARLY_DEAD,

	INFO_YOU_ARE,
	INFO_YOU_ARE_CARRYING,
	INFO_YOU_ARE_USING,
	INFO_IS_USING,
	INFO_YOU_PEEK_AT_THE_INVENTORY,
	
	COMM_SWITCH_TO_RUSSIAN,
	COMM_SWITCH_TO_ENGLISH,
	COMM_SHOW_LANGUAGE,
	COMM_LANGUAGE_USAGE,

	INFO_COND_EXCELLENT,
	INFO_COND_GOOD,
	INFO_COND_FINE,
	INFO_COND_AVERAGE,
	INFO_COND_POOR,
	INFO_COND_FRAGILE,

	UPDATE_WEATHER_DAY_BEGUN,
	UPDATE_WEATHER_SUN_IN_THE_EAST,
	UPDATE_WEATHER_SUN_IN_THE_WEST,
	UPDATE_WEATHER_NIGHT_BEGUN,
	UPDATE_WEATHER_GETTING_CLOUDY,
	UPDATE_WEATHER_IT_STARTS_TO_RAIN,
	UPDATE_WEATHER_THE_CLOUDS_DISAPPEAR,
	UPDATE_WEATHER_LIGHTNING_FLASHES,
	UPDATE_WEATHER_THE_RAIN_STOPPED,
	UPDATE_WEATHER_LIGHTNING_STOPPED,

	QUEST_QUESTOR_TELLS_YOU,
	QUEST_YOU_HAVE_D_MINUTES,
	QUEST_MAY_THE_GODS_GO,

	COMM_ORDERS_YOU_TO,
	COMM_YOU_PUSH,
	COMM_PUSHES_YOU,
	COMM_PUSHES_N_TO,
	
	HERA_IS_FROM_HERE,
	
	INFO_S_S,
	INFO_ITS_S,
	INFO_TIME_DAWN,
	INFO_TIME_MORNING,
	INFO_TIME_MID_DAY,
	INFO_TIME_EVENING,
	INFO_TIME_NIGHT,
	
	MOVE_ALAS_YOU_CANNOT_GO,
	MOVE_THE_D_IS_CLOSED,
	MOVE_WHAT_YOU_LEAVE,
	MOVE_ROOM_IS_PRIVATE,
	MOVE_MOUNT_MUST_STAND,
	MOVE_MOUNT_REFUSES_GO_THAT_WAY,
	MOVE_YOU_ARENT_ALLOWED_THERE,
	MOVE_YOU_FEEL_TOO_BLOODY,
	MOVE_YOUR_MOUNT_CANT_FLY,
	MOVE_YOU_CANT_FLY,
	MOVE_YOU_CANT_TAKE_MOUNT_THERE,
	MOVE_YOU_NEED_A_BOAT,
	MOVE_YOU_TOO_EXHAUSTED,
	MOVE_YOU_CANT_BRING_N_CITY,
	MOVE_YOU_ARENT_ALLOWED_CITY,
	MOVE_YOU_FOLLOW_N,
	MOVE_I_SEE_NO_EXIT_T_HERE,
	MOVE_I_SEE_NO_T_HERE,
	MOVE_I_SEE_NO_DOOR_T_HERE,
	MOVE_YOU_CANT_DO_THAT,
	MOVE_DISTANCE_HERE,
	MOVE_DISTANCE_NEARBY,
	MOVE_DISTANCE_NOT_FAR,
	MOVE_DISTANCE_OFF_IN,
	MOVE_N_LOOKS_ALL_AROUND,
	MOVE_LOOKING_AROUND_YOU_SEE,
	MOVE_OPEN_WHAT,
	MOVE_ITS_ALREADY_OPEN,
	MOVE_ITS_LOCKED,
	MOVE_YOU_OPEN_P,
	MOVE_N_OPENS_P,
	MOVE_THATS_NOT_A_CONTAINER,
	MOVE_N_OPENS_THE_D,
	MOVE_OK,
	MOVE_THE_D_OPENS,
	MOVE_CLOSE_WHAT,
	MOVE_ITS_ALREADY_CLOSED,
	MOVE_YOU_CLOSE_P,
	MOVE_N_CLOSES_P,
	MOVE_N_CLOSES_THE_D,
	MOVE_THE_D_CLOSES,

/*	Here start sex-dependent messages */
	MSG_SEX_DEPENDENT,
	INFO_IS_GUSHING_BLOOD = MSG_SEX_DEPENDENT,
	INFO_IS_GUSHING_BLOOD_M,
	INFO_IS_GUSHING_BLOOD_F,
	INFO_POS_NAME_DEAD,
	INFO_POS_NAME_DEAD_M,
	INFO_POS_NAME_DEAD_F,
	INFO_POS_NAME_MORTALLY_WOUNDED,
	INFO_POS_NAME_MORTALLY_WOUNDED_M,
	INFO_POS_NAME_MORTALLY_WOUNDED_F,
	INFO_POS_NAME_INCAPACITATED,
	INFO_POS_NAME_INCAPACITATED_M,
	INFO_POS_NAME_INCAPACITATED_F,
	INFO_POS_NAME_STUNNED,
	INFO_POS_NAME_STUNNED_M,
	INFO_POS_NAME_STUNNED_F,
	INFO_POS_NAME_SLEEPING,
	INFO_POS_NAME_SLEEPING_M,
	INFO_POS_NAME_SLEEPING_F,
	INFO_POS_NAME_RESTING,
	INFO_POS_NAME_RESTING_M,
	INFO_POS_NAME_RESTING_F,
	INFO_POS_NAME_SITTING,
	INFO_POS_NAME_SITTING_M,
	INFO_POS_NAME_SITTING_F,
	INFO_POS_NAME_FIGHTING,
	INFO_POS_NAME_FIGHTING_M,
	INFO_POS_NAME_FIGHTING_F,
	INFO_POS_NAME_STANDING,
	INFO_POS_NAME_STANDING_M,
	INFO_POS_NAME_STANDING_F,
	MOVE_LEAVES,
	MOVE_LEAVES_M,
	MOVE_LEAVES_F,
	MOVE_LEAVES_RIDING_ON,
	MOVE_LEAVES_RIDING_ON_M,
	MOVE_LEAVES_RIDING_ON_F,
	MOVE_LEAVES_T,
	MOVE_LEAVES_T_M,
	MOVE_LEAVES_T_F,
	MOVE_LEAVES_T_RIDING_ON,
	MOVE_LEAVES_T_RIDING_ON_M,
	MOVE_LEAVES_T_RIDING_ON_F,
	MOVE_ARRIVED,
	MOVE_ARRIVED_M,
	MOVE_ARRIVED_F,
	MOVE_ARRIVED_RIDING,
	MOVE_ARRIVED_RIDING_M,
	MOVE_ARRIVED_RIDING_F,
	QUEST_THANK_YOU_BRAVE,
	QUEST_THANK_YOU_BRAVE_M,
	QUEST_THANK_YOU_BRAVE_F,

	SKILLS_HAVE_BECOME_BETTER,
	SKILLS_HAVE_BECOME_BETTER_M,
	SKILLS_HAVE_BECOME_BETTER_F,
	SKILLS_LEARN_FROM_MISTAKES,
	SKILLS_LEARN_FROM_MISTAKES_M,
	SKILLS_LEARN_FROM_MISTAKES_F,

	HERA_STEPS_INTO_RIDING_ON,
	HERA_STEPS_INTO_RIDING_ON_M,
	HERA_STEPS_INTO_RIDING_ON_F,
	HERA_STEPS_INTO,
	HERA_STEPS_INTO_M,
	HERA_STEPS_INTO_F,

	MOVE_WHEN_YOU_ATTEMPT_YOU_BREAK_WEBS,
	MOVE_WHEN_YOU_ATTEMPT_YOU_BREAK_WEBS_M,
	MOVE_WHEN_YOU_ATTEMPT_YOU_BREAK_WEBS_F,
	MOVE_N_BREAKS_THE_WEBS,
	MOVE_N_BREAKS_THE_WEBS_M,
	MOVE_N_BREAKS_THE_WEBS_F,
	MOVE_YOU_ATTEMPT_WEBS_HOLD_YOU,
	MOVE_YOU_ATTEMPT_WEBS_HOLD_YOU_M,
	MOVE_YOU_ATTEMPT_WEBS_HOLD_YOU_F,
	MOVE_N_STRUGGLES_VAINLY_AGAINST_WEBS,
	MOVE_N_STRUGGLES_VAINLY_AGAINST_WEBS_M,
	MOVE_N_STRUGGLES_VAINLY_AGAINST_WEBS_F,
	MOVE_YOU_STEP_OUT_SHADOWS,
	MOVE_YOU_STEP_OUT_SHADOWS_M,
	MOVE_YOU_STEP_OUT_SHADOWS_F,
	MOVE_N_STEPS_OUT_OF_SHADOWS,
	MOVE_N_STEPS_OUT_OF_SHADOWS_M,
	MOVE_N_STEPS_OUT_OF_SHADOWS_F,
	MOVE_YOU_STEP_OUT_COVER,
	MOVE_YOU_STEP_OUT_COVER_M,
	MOVE_YOU_STEP_OUT_COVER_F,
	MOVE_N_STEPS_OUT_COVER,
	MOVE_N_STEPS_OUT_COVER_M,
	MOVE_N_STEPS_OUT_COVER_F,
	MOVE_YOU_FAILED_TO_PASS,
	MOVE_YOU_FAILED_TO_PASS_M,
	MOVE_YOU_FAILED_TO_PASS_F,
	MOVE_N_TRIES_TO_PASS_FAILED,
	MOVE_N_TRIES_TO_PASS_FAILED_M,
	MOVE_N_TRIES_TO_PASS_FAILED_F,
	
/*	End of all messages */
	MSG_MAX_NUM
};

#endif
