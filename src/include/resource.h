#ifndef _RESOURCE_H_
#define _RESOURCE_H_

/*
 * $Id: resource.h,v 1.56 1998-06-02 15:56:06 fjoe Exp $
 */

#define msg(msgid, ch) vmsg(msgid, ch, ch)
char* vmsg(int msgid, CHAR_DATA *ch, CHAR_DATA *victim);
void msgdb_load();

extern char** ilang_names;
extern nilang;

enum {
	INFO_NOTHING, 
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
	COMM_INTERFACE_LANGUAGE_UNDEFINED,
	COMM_SHOW_LANGUAGE,
	COMM_LANGUAGE_USAGE_PRE,
	COMM_LANGUAGE_USAGE_POST,
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
	MOVE_LOCK_WHAT,
	MOVE_ITS_NOT_CLOSED,
	MOVE_IT_CANT_BE_LOCKED,
	MOVE_YOU_LACK_THE_KEY,
	MOVE_ITS_ALREADY_LOCKED,
	MOVE_YOU_LOCK_P,
	MOVE_N_LOCKS_P,
	MOVE_CLICK,
	MOVE_N_LOCKS_THE_D,
	MOVE_THE_D_CLICKS,
	MOVE_UNLOCK_WHAT,
	MOVE_IT_CANT_BE_UNLOCKED,
	MOVE_ITS_ALREADY_UNLOCKED,
	MOVE_YOU_UNLOCK_P,
	MOVE_N_UNLOCKS_P,
	MOVE_N_UNLOCKS_THE_D,
	MOVE_PICK_WHAT,
	MOVE_CANT_PICK_MOUNTED,
	MOVE_N_IS_STANDING_TOO_CLOSE_TO_LOCK,
	MOVE_YOU_FAILED,
	MOVE_YOU_PICK_THE_LOCK_ON_P,
	MOVE_N_PICKS_THE_LOCK_ON_P,
	MOVE_IT_CANT_BE_PICKED,
	MOVE_N_PICKS_THE_D,
	MOVE_MAYBE_YOU_SHOULD_FINISH_FIGHTING_FIRST,
	MOVE_YOU_DONT_SEE_THAT,
	MOVE_YOU_CANT_FIND_PLACE_STAND,
	MOVE_THERES_NO_ROOM_TO_STAND_ON,
	MOVE_YOU_CANT_WAKE_UP,
	MOVE_YOU_WAKE_AND_STAND_UP,
	MOVE_N_WAKES_AND_STANDS_UP,
	MOVE_YOU_WAKE_AND_STAND_AT,
	MOVE_N_WAKES_AND_STANDS_AT,
	MOVE_YOU_WAKE_AND_STAND_ON,
	MOVE_N_WAKES_AND_STANDS_ON,
	MOVE_YOU_WAKE_AND_STAND_IN,
	MOVE_N_WAKES_AND_STANDS_IN,
	MOVE_FEEL_BLOOD_HEATS,
	MOVE_YOU_STAND_UP,
	MOVE_N_STANDS_UP,
	MOVE_YOU_STAND_AT,
	MOVE_N_STANDS_AT,
	MOVE_YOU_STAND_ON,
	MOVE_N_STANDS_ON,
	MOVE_YOU_STAND_IN,
	MOVE_N_STANDS_IN,
	MOVE_YOU_ARE_ALREADY_STANDING,
	MOVE_YOU_ARE_ALREADY_FIGHTING,
	MOVE_YOU_CANT_REST_MOUNTED,
	MOVE_YOU_CANT_REST_RIDDEN,
	MOVE_YOU_ARE_ALREADY_SLEEPING,
	MOVE_YOU_CANT_REST_ON_THAT,
	MOVE_THERES_NO_MORE_ROOM_ON,
	MOVE_YOU_WAKE_AND_REST,
	MOVE_N_WAKES_AND_RESTS,
	MOVE_YOU_WAKE_UP_AND_REST_AT,
	MOVE_N_WAKES_UP_AND_RESTS_AT,
	MOVE_YOU_WAKE_UP_AND_REST_ON,
	MOVE_N_WAKES_UP_AND_RESTS_ON,
	MOVE_YOU_WAKE_UP_AND_REST_IN,
	MOVE_N_WAKES_UP_AND_RESTS_IN,
	MOVE_YOU_ARE_ALREADY_RESTING,
	MOVE_YOU_REST,
	MOVE_N_SITS_DOWN_AND_RESTS,
	MOVE_YOU_SIT_DOWN_AT_AND_REST,
	MOVE_N_SITS_DOWN_AT_AND_RESTS,
	MOVE_YOU_SIT_DOWN_ON_AND_REST,
	MOVE_N_SITS_DOWN_ON_AND_RESTS,
	MOVE_YOU_SIT_DOWN_IN_AND_REST,
	MOVE_N_SITS_DOWN_IN_AND_RESTS,
	MOVE_N_RESTS,
	MOVE_YOU_REST_AT,
	MOVE_N_RESTS_AT,
	MOVE_YOU_REST_ON,
	MOVE_N_RESTS_ON,
	MOVE_YOU_REST_IN,
	MOVE_N_RESTS_IN,
	MOVE_YOU_CANT_SIT_MOUNTED,
	MOVE_YOU_CANT_SIT_RIDDEN,
	MOVE_YOU_CANT_SIT_ON_THAT,
	MOVE_YOU_WAKE_AND_SIT_UP,
	MOVE_N_WAKES_AND_SITS_UP,
	MOVE_YOU_WAKE_AND_SIT_AT,
	MOVE_N_WAKES_AND_SITS_AT,
	MOVE_YOU_WAKE_AND_SIT_ON,
	MOVE_N_WAKES_AND_SITS_ON,
	MOVE_YOU_WAKE_AND_SIT_IN,
	MOVE_N_WAKES_AND_SITS_IN,
	MOVE_YOU_STOP_RESTING,
	MOVE_YOU_SIT_AT,
	MOVE_N_SITS_AT,
	MOVE_YOU_SIT_ON,
	MOVE_N_SITS_ON,
	MOVE_YOU_ARE_ALREADY_SITTING_DOWN,
	MOVE_YOU_SIT_DOWN,
	MOVE_N_SITS_DOWN_ON_THE_GROUND,
	MOVE_YOU_SIT_DOWN_AT,
	MOVE_N_SITS_DOWN_AT,
	MOVE_YOU_SIT_DOWN_IN,
	MOVE_N_SITS_DOWN_IN,
	MOVE_YOU_CANT_SLEEP_MOUNTED,
	MOVE_YOU_CANT_SLEEP_RIDDEN,
	MOVE_YOU_GO_TO_SLEEP,
	MOVE_N_GOES_TO_SLEEP,
	MOVE_YOU_CANT_SLEEP_ON_THAT,
	MOVE_THERES_NO_ROOM_ON_P_FOR_YOU,
	MOVE_YOU_GO_TO_SLEEP_AT,
	MOVE_N_GOES_TO_SLEEP_AT,
	MOVE_YOU_GO_TO_SLEEP_ON,
	MOVE_N_GOES_TO_SLEEP_ON,
	MOVE_YOU_GO_TO_SLEEP_IN,
	MOVE_N_GOES_TO_SLEEP_IN,
	MOVE_YOU_ARE_ASLEEP_YOURSELF,
	MOVE_THEY_ARENT_HERE,
	MOVE_N_IS_ALREADY_AWAKE,
	MOVE_YOU_CANT_WAKE_M,
	MOVE_N_WAKES_YOU,
	MOVE_YOU_CANT_SNEAK_MOUNTED,
	MOVE_YOU_ATTEMPT_TO_MOVE_SILENTLY,
	MOVE_YOU_CANT_HIDE_MOUNTED,
	MOVE_YOU_CANT_HIDE_RIDDEN,
	MOVE_YOU_CANNOT_HIDE_GLOWING,
	MOVE_YOU_ATTEMPT_TO_HIDE,
	MOVE_YOU_CANT_CAMOUFLAGE_MOUNTED,
	MOVE_YOU_CANT_CAMOUFLAGE_RIDDEN,
	MOVE_YOU_DONT_KNOW_CAMOUFLAGE,
	MOVE_YOU_CANT_CAMOUFLAGE_GLOWING,
	MOVE_THERES_NO_COVER_HERE,
	MOVE_N_TRIES_TO_CAMOUFLAGE,
	MOVE_YOU_ATTEMPT_TO_CAMOUFLAGE,
	MOVE_YOU_FADE_INTO_EXIST,
	MOVE_N_FADES_INTO_EXIST,
	MOVE_YOU_TRAMPLE_AROUND_LOUDLY,
	MOVE_ONLY_PLAYERS_RECALL,
	MOVE_RECALL_FOR_BELOW_10,
	MOVE_N_PRAYS_FOR_TRANSPORTATION,
	MOVE_YOU_ARE_COMPLETELY_LOST,
	MOVE_GODS_FORSAKEN_YOU,
	MOVE_YOU_ARE_STILL_FIGHTING,
	MOVE_RECALL_FROM_COMBAT,
	MOVE_N_DISAPPEARS,
	MOVE_N_APPEARS_IN_THE_ROOM,
	MOVE_YOU_CANT_DO_THAT_HERE,
	MOVE_YOU_HAVE_D_TRAINING_SESSIONS,
	MOVE_YOU_CAN_TRAIN,
	MOVE_NOT_ENOUGH_TRAININGS,
	MOVE_YOUR_DURABILITY_INCREASES,
	MOVE_N_DURABILITY_INCREASES,
	MOVE_YOUR_POWER_INCREASES,
	MOVE_N_POWER_INCREASES,
	MOVE_YOUR_T_IS_MAX,
	MOVE_YOUR_T_INCREASES,
	MOVE_N_T_INCREASES,
	MOVE_THERE_ARE_NO_TRAIN_TRACKS_HERE,
	MOVE_N_CHECKS_TRACKS,
	MOVE_TRACKS_LEAD_S,
	MOVE_DONT_SEE_TRACKS,
	MOVE_YOU_CANT_BE_MORE_VAMPIRE,
	MOVE_YOU_SHOW_MORE_UGGLY,
	MOVE_GO_AND_ASK_QUESTOR,
	MOVE_GO_KILL_PLAYER,
	MOVE_WAIT_NIGHT,
	MOVE_FEEL_GREATER,
	MOVE_CANNOT_RECOGNIZE,
	MOVE_DONT_KNOW_BITE,
	MOVE_MUST_TRANSFORM_VAMP,
	MOVE_BITE_WHOM,
	MOVE_THEY_MUST_BE_SLEEPING,
	MOVE_HOW_CAN_YOU_SNEAK_YOU,
	MOVE_CANT_BITE_FIGHTING_PERS,
	MOVE_DOESNT_WORTH_UP,
	MOVE_DOESNT_WORTH_TO_DO,
	MOVE_HELP_TRIED_TO_BITE,
	MOVE_BASH_WHATS_THAT,
	INFO_IS_GUSHING_BLOOD,
	INFO_POS_NAME_DEAD,
	INFO_POS_NAME_MORTALLY_WOUNDED,
	INFO_POS_NAME_INCAPACITATED,
	INFO_POS_NAME_STUNNED,
	INFO_POS_NAME_SLEEPING,
	INFO_POS_NAME_RESTING,
	INFO_POS_NAME_SITTING,
	INFO_POS_NAME_FIGHTING,
	INFO_POS_NAME_STANDING,
	MOVE_LEAVES,
	MOVE_LEAVES_RIDING_ON,
	MOVE_LEAVES_T,
	MOVE_LEAVES_T_RIDING_ON,
	MOVE_ARRIVED,
	MOVE_ARRIVED_RIDING,
	QUEST_THANK_YOU_BRAVE,
	SKILLS_HAVE_BECOME_BETTER,
	SKILLS_LEARN_FROM_MISTAKES,
	HERA_STEPS_INTO_RIDING_ON,
	HERA_STEPS_INTO,
	MOVE_WHEN_YOU_ATTEMPT_YOU_BREAK_WEBS,
	MOVE_N_BREAKS_THE_WEBS,
	MOVE_YOU_ATTEMPT_WEBS_HOLD_YOU,
	MOVE_N_STRUGGLES_VAINLY_AGAINST_WEBS,
	MOVE_YOU_STEP_OUT_SHADOWS,
	MOVE_N_STEPS_OUT_OF_SHADOWS,
	MOVE_YOU_STEP_OUT_COVER,
	MOVE_N_STEPS_OUT_COVER,
	MOVE_YOU_FAILED_TO_PASS,
	MOVE_N_TRIES_TO_PASS_FAILED,
	MOVE_TOO_PUMPED_TO_PRAY,
	MOVE_CANT_BASH_DOORS_MOUNTED,
	MOVE_CANT_BASH_DOORS_RIDDEN,
	MOVE_BASH_WHICH_DOOR,
	MOVE_WAIT_FIGHT_FINISH,
	MOVE_N_TOO_CLOSE_TO_DOOR,
	MOVE_TRY_TO_OPEN,
	MOVE_SHIELD_PROTECTS_EXIT,
	MOVE_YOU_SLAM_TRY_BREAK,
	MOVE_N_SLAMS_TRY_BREAK,
	TATTOO_GLOWS_BLUE,
	TATTOO_GLOWS_RED,
	MOVE_N_BASHES_AND_BREAK,
	MOVE_YOU_SUCCESSED_TO_OPEN_DOOR,
	MOVE_YOU_FALL_ON_FACE,
	MOVE_N_FALLS_ON_FACE,	
	MOVE_HUH,
	MOVE_CURRENT_BLINK,
	MOVE_BLINK_ON,
	MOVE_BLINK_OFF,
	MOVE_IS_S_A_STATUS,	
	MOVE_DONT_HAVE_POWER,
	MOVE_N_THROWS_GLOBE,
	MOVE_N_IS_GONE,
	MOVE_N_APPEARS_FROM_NOWHERE,
	MOVE_YOU_CAN_ALREADY_DETECT_SNEAK,
	MOVE_YOU_CAN_DETECT_THE_SNEAK,
	MOVE_CANT_FADE_MOUNTED,
	MOVE_CANT_FADE_RIDDEN,
	MOVE_YOU_ATTEMPT_TO_FADE,
	MOVE_LACK_SKILL_DRAIN_TOUCH,
	MOVE_LET_IT_BE,
	MOVE_DONT_WANT_DRAIN_MASTER,
	MOVE_EVEN_YOU_NOT_SO_STUPID,
	MOVE_YOU_TOUCH_NS_NECK,
	MOVE_N_TOUCHES_YOUR_NECK,
	MOVE_N_TOUCHES_NS_NECK,
	MOVE_YOU_ARE_ALREADY_FLYING,
	MOVE_YOU_START_TO_FLY,
	MOVE_FIND_POTION_OR_WINGS,
	MOVE_YOU_SLOWLY_TOUCH_GROUND,
	MOVE_YOU_ARE_ALREADY_ON_GROUND,
	MOVE_TYPE_WITH_UP_OR_DOWN,
	MOB_PROGS_I_DO_NOT_GET_PAID_ENOUGH,
	UPDATE_YOUR_GAIN_IS,
	UPDATE_YOU_RAISE_A_LEVEL,
	COMM_TELLS_YOU,
	COMM_YOU_TELL,
	COMM_YOUR_MESSAGE_DIDNT_GET_THROUGH,
	QUEST_N_ASKS_FOR_QUEST,
	QUEST_YOU_ASK_FOR_QUEST,
	QUEST_YOU_ALREADY_ON_QUEST,
	QUEST_BRAVE_BUT_LET_SOMEONE_ELSE,
	QUEST_COME_BACK_LATER,
	QUEST_DONT_HAVE_QUESTS,
	QUEST_TRY_AGAIN_LATER,
	QUEST_RUNES_MOST_HEINOUS,
	QUEST_HAS_MURDERED,
	QUEST_THE_PENALTY_IS,
	QUEST_ENEMY_OF_MINE,
	QUEST_ELIMINATE_THREAT,
	QUEST_SEEK_S_OUT,
	QUEST_LOCATION_IS_IN_AREA,
	QUEST_VILE_PILFERERS,
	QUEST_MY_COURT_WIZARDESS,
	QUEST_IS_ALMOST_COMPLETE,
	QUEST_RECOVER_FABLED,
	QUEST_ARENT_ON_QUEST,
	QUEST_SLAY_DREADED,
	QUEST_YOU_HAVE_D_QP,
	QUEST_D_MIN_REMAINING,
	QUEST_LESS_MINUTE,
	QUEST_LEFT_FOR_QUEST,
	QUEST_WAIT_FIGHT_STOPS,
	QUEST_N_ASKS_LIST,
	QUEST_YOU_ASK_LIST,
	QUEST_TYPE_BUY,
	QUEST_NOT_ENOUGH_QP,
	QUEST_ITEM_BEYOND,
	QUEST_N_GIVES_PRACS,
	QUEST_N_GIVES_YOU_PRACS,
	QUEST_CANT_GAIN_VAMPIRE,
	QUEST_N_GIVES_SECRET,
	QUEST_N_GIVES_YOU_SECRET,
	QUEST_NOT_SAMURAI,
	QUEST_NO_DEATHS,
	QUEST_DONT_HAVE_KATANA,
	QUEST_AS_YOU_WIELD_IT,
	QUEST_NO_RELIGION,
	QUEST_ALREADY_TATTOOED,
	QUEST_N_TATTOOS_N,
	QUEST_N_TATTOOS_YOU,
	QUEST_N_GIVES_GOLD,
	QUEST_N_GIVES_YOU_GOLD,
	QUEST_NOT_HAVE_ITEM,
	QUEST_GIVES_P_TO_N,
	QUEST_GIVES_YOU_P,
	QUEST_INFORMS_COMPLETE,
	QUEST_YOU_INFORM_COMPLETE,
	QUEST_NEVER_QUEST,
	QUEST_GRATS_COMPLETE,
	QUEST_AS_A_REWARD,
	QUEST_GAIN_PRACS,
	QUEST_YOU_HAND_P,
	QUEST_N_HANDS_P,
	QUEST_HAVENT_COMPLETE,
	QUEST_DIDNT_COMPLETE_IN_TIME,
	QUEST_HAVE_TO_REQUEST,
	QUEST_TYPE_TROUBLE,
	QUEST_HAVENT_BOUGHT,
	QUEST_THIS_IS_THE_I_S,
	QUEST_ST,
	QUEST_ND,
	QUEST_RD,
	QUEST_WONT_GIVE_AGAIN,
	QUEST_COMMANDS,
	QUEST_TYPE_HELP_QUEST,
	QUEST_YOU_MAY_NOW_QUEST_AGAIN,
	QUEST_RUN_OUT_TIME,
	QUEST_BETTER_HURRY,
	QUEST_ITEMS_AVAIL_PURCHASE,
	FIGHT_YOU_COMPLETED_SOMEONES_QUEST,
	FIGHT_SOMEONE_COMPLETED_YOUR_QUEST,
	FIGHT_ALMOST_COMPLETE_QUEST,
	FIGHT_RETURN_TO_QUESTER,
	INFO_CONSIDER_WHOM,
	INFO_DONT_EVEN_THINK,
	INFO_CAN_KILL_NAKED,
	INFO_IS_NOT_MATCH_FOR_YOU,
	INFO_LOOKS_EASY_KILL,
	INFO_PERFECT_MATCH,
	INFO_FEEL_LUCKY_PUNK,
	INFO_LAUGHS_AT_YOU_MERCILESSLY,
	INFO_DEATH_WILL_THANK_YOU,
	INFO_GRINS_EVILLY_WITH,
	INFO_GREETS_YOU_WARMLY,
	INFO_HOPE_YOU_WILL_TURN,
	INFO_GRINS_EVILLY_AT,
	INFO_GRINS_EVILLY,
	INFO_SMILES_HAPPILY,
	INFO_LOOKS_AS_YOU,
	INFO_LOOKS_VERY_DISINTERESTED,
	MOVE_SCAN_HERE,
	MOVE_SCAN_DOOR_CLOSED,
	MOVE_PUSH_WHOM_WHERE,
	MOVE_CANT_PUSH_MOUNTED,
	MOVE_CANT_PUSH_RIDDEN,
	MOVE_TOO_DAZED_TO_PUSH,
	MOVE_THATS_POINTLESS,	
	MOVE_DIR_IS_CLOSED,
	MOVE_DIR_IS_LOCKED,
	MOVE_YOU_WEBBED_WANT_WHAT,
	MOVE_N_TRIES_PUSH_WEBBED,
	MOVE_PUSH_VICT_WEBBED,
	MOVE_N_PUSHES_VICT_WEBBED,
	MOVE_OOPS,
	MOVE_N_TRIED_PUSH_YOU,
	MOVE_N_TRIED_PUSH_N,
	MOVE_KEEP_HANDS_OUT,
	SPECIAL_SAVAGELY_DEVOURS_CORPSE,
	SPECIAL_PICKS_SOME_TRASH,
	SPECIAL_YOU_BITE_N,
	SPECIAL_N_BITES_N,
	SPECIAL_N_BITES_YOU,
	SPECIAL_DO_I_KNOW_YOU,
	SPECIAL_DONT_REMEMBER_YOU,
	SPECIAL_OK_MY_DEAR,
	UPDATE_CALMING_DOWN,
	COMM_N_SAYS,
	COMM_YOU_SAY,
	QUEST_INFO_LOCATION,
	MOVE_CANT_PRAY_NOW,
	MOVE_PRAYS_UPPER_LORD,
	MOVE_ARENT_FIGHTING,
	MOVE_ESCAPE_WHAT_DIR,
	MOVE_CANT_ESCAPE_MOUNTED,
	MOVE_CANT_ESCAPE_RIDDEN,
	MOVE_TRY_FLEE,
	MOVE_SOMETHING_PREVENTS_ESCAPE,
	MOVE_ESCAPE_FAILED,
	MOVE_N_ESCAPED,
	MOVE_ESCAPED_FROM_COMBAT,
	MOVE_WRONG_DIRECTION,
	MOVE_COULDNT_ESCAPE,
	MOVE_CANT_LAY_HANDS,
	MOVE_CANT_CONCENTRATE_ENOUGH,
	MOVE_WARM_FEELING,
	MOVE_ALREADY_HUMAN,
	MOVE_RETURN_TO_SIZE,
	MOVE_MOUNT_WHAT,
	MOVE_DONT_KNOW_RIDE,
	MOVE_CANT_RIDE_THAT,
	MOVE_BEAST_TOO_POWERFUL,
	MOVE_S_BELONGS_TO_S,
	MOVE_MOUNT_MUST_STAND,
	MOVE_ALREADY_RIDDEN,
	MOVE_ALREADY_RIDING,
	MOVE_FAIL_TO_MOUNT,
	MOVE_YOU_HOP_ON_N,
	MOVE_N_HOPS_ON_N,
	MOVE_N_HOPS_ON_YOU,
	MOVE_YOU_DISMOUNT_N,
	MOVE_N_DISMOUNTS_N,
	MOVE_N_DISMOUNTS_YOU,
	MOVE_YOU_ARENT_MOUNTED,
	MOVE_MOUNT_DRUNKEN,
	MOVE_YOU_FALL_OFF_N,
	MOVE_N_FALLS_OFF_N,
	MOVE_N_FALLS_OFF_YOU,
	MOVE_N_DOESNT_LIKE_YOU,
	MOVE_N_DOESNT_LIKE_N,
	MOVE_YOU_DONT_LIKE_N,
	MOVE_N_SNARLS_YOU,
	MOVE_N_SNARLS_N,
	MOVE_YOU_SNARL_N,
	INFO_FIGHTING_YOU,
};

#endif
