#ifndef _RESOURCE_H_
#define _RESOURCE_H_

/*
 * $Id: resource.h,v 1.88 1998-08-14 05:45:16 fjoe Exp $
 */

#define msg(msgid, ch) vmsg(msgid, ch, ch)
char* vmsg(int msgid, CHAR_DATA *ch, CHAR_DATA *victim);
char* exact_msg(int msgid, int lang, int sex);
void msgdb_load();
int lang_lookup(const char*);
extern char** lang_table;
extern nlang;

enum {
	MSG_NOTHING, 
	MSG_EQ_USED_AS_LIGHT,
	MSG_EQ_WORN_ON_FINGER_1,
	MSG_EQ_WORN_ON_FINGER_2,
	MSG_EQ_WORN_AROUND_NECK_1,
	MSG_EQ_WORN_AROUND_NECK_2,
	MSG_EQ_WORN_ON_TORSO,
	MSG_EQ_WORN_ON_HEAD,
	MSG_EQ_WORN_ON_LEGS,
	MSG_EQ_WORN_ON_FEET,
	MSG_EQ_WORN_ON_HANDS,
	MSG_EQ_WORN_ON_ARMS,
	MSG_EQ_WORN_AS_SHIELD,
	MSG_EQ_WORN_ABOUT_BODY,
	MSG_EQ_WORN_ABOUT_WAIST,
	MSG_EQ_WORN_AROUND_WRIST_1,
	MSG_EQ_WORN_AROUND_WRIST_2,
	MSG_EQ_WIELDED,
	MSG_EQ_HELD,
	MSG_EQ_FLOATING_NEARBY,
	MSG_EQ_SCRATCHED_TATTOO,
	MSG_EQ_DUAL_WIELDED,
	MSG_EQ_STUCK_IN,
	MSG_SLEEPING_AT,
	MSG_SLEEPING_ON,
	MSG_SLEEPING_IN,
	MSG_SLEEPING,
	MSG_RESTING_AT,
	MSG_RESTING_ON,
	MSG_RESTING_IN,
	MSG_RESTING,
	MSG_SITTING_AT,
	MSG_SITTING_ON,
	MSG_SITTING_IN,
	MSG_SITTING,
	MSG_STANDING_AT,
	MSG_STANDING_ON,
	MSG_STANDING_IN,
	MSG_STANDING,
	MSG_FIGHTING,
	MSG_IS_HERE,
	MSG_IS_IN_PERFECT_HEALTH,
	MSG_HAS_A_FEW_SCRATCHES,
	MSG_HAS_SOME_SMALL_BUT_DISGUSTING_CUTS,
	MSG_IS_COVERED_WITH_BLEEDING_WOUNDS,
	MSG_IS_WRITHING_IN_AGONY,
	MSG_IS_CONVULSING_ON_THE_GROUND,
	MSG_IS_NEARLY_DEAD,
	MSG_YOU_ARE,
	MSG_YOU_ARE_CARRYING,
	MSG_YOU_ARE_USING,
	MSG_IS_USING,
	MSG_YOU_PEEK_AT_THE_INVENTORY,
	MSG_INTERFACE_LANGUAGE_IS,
	MSG_LANG_USAGE_PRE,
	MSG_LANG_USAGE_POST,
	MSG_COND_EXCELLENT,
	MSG_COND_GOOD,
	MSG_COND_FINE,
	MSG_COND_AVERAGE,
	MSG_COND_POOR,
	MSG_COND_FRAGILE,
	MSG_WEATHER_DAY_BEGUN,
	MSG_WEATHER_SUN_IN_THE_EAST,
	MSG_WEATHER_SUN_IN_THE_WEST,
	MSG_WEATHER_NIGHT_BEGUN,
	MSG_WEATHER_GETTING_CLOUDY,
	MSG_WEATHER_IT_STARTS_TO_RAIN,
	MSG_WEATHER_THE_CLOUDS_DISAPPEAR,
	MSG_WEATHER_LIGHTNING_FLASHES,
	MSG_WEATHER_THE_RAIN_STOPPED,
	MSG_WEATHER_LIGHTNING_STOPPED,
	MSG_YOU_HAVE_D_MINUTES,
	MSG_MAY_THE_GODS_GO,
	MSG_ORDERS_YOU_TO,
	MSG_YOU_PUSH,
	MSG_PUSHES_YOU,
	MSG_PUSHES_N_TO,
	MSG_HERA_IS_FROM_HERE,
	MSG_ITS_S,
	MSG_TIME_DAWN,
	MSG_TIME_MORNING,
	MSG_TIME_MID_DAY,
	MSG_TIME_EVENING,
	MSG_TIME_NIGHT,
	MSG_ALAS_YOU_CANNOT_GO,
	MSG_THE_D_IS_CLOSED,
	MSG_WHAT_YOU_LEAVE,
	MSG_ROOM_IS_PRIVATE,
	MSG_MOUNT_REFUSES_GO_THAT_WAY,
	MSG_YOU_ARENT_ALLOWED_THERE,
	MSG_YOU_FEEL_TOO_BLOODY,
	MSG_YOUR_MOUNT_CANT_FLY,
	MSG_YOU_CANT_FLY,
	MSG_YOU_CANT_TAKE_MOUNT_THERE,
	MSG_YOU_NEED_A_BOAT,
	MSG_YOU_TOO_EXHAUSTED,
	MSG_YOU_CANT_BRING_N_CITY,
	MSG_YOU_ARENT_ALLOWED_CITY,
	MSG_YOU_FOLLOW_N,
	MSG_I_SEE_NO_EXIT_T_HERE,
	MSG_I_SEE_NO_T_HERE,
	MSG_I_SEE_NO_DOOR_T_HERE,
	MSG_YOU_CANT_DO_THAT,
	MSG_N_LOOKS_ALL_AROUND,
	MSG_LOOKING_AROUND_YOU_SEE,
	MSG_OPEN_WHAT,
	MSG_ITS_ALREADY_OPEN,
	MSG_ITS_LOCKED,
	MSG_YOU_OPEN_P,
	MSG_N_OPENS_P,
	MSG_THATS_NOT_A_CONTAINER,
	MSG_N_OPENS_THE_D,
	MSG_OK,
	MSG_THE_D_OPENS,
	MSG_CLOSE_WHAT,
	MSG_ITS_ALREADY_CLOSED,
	MSG_YOU_CLOSE_P,
	MSG_N_CLOSES_P,
	MSG_N_CLOSES_THE_D,
	MSG_THE_D_CLOSES,
	MSG_LOCK_WHAT,
	MSG_ITS_NOT_CLOSED,
	MSG_IT_CANT_BE_LOCKED,
	MSG_YOU_LACK_THE_KEY,
	MSG_ITS_ALREADY_LOCKED,
	MSG_YOU_LOCK_P,
	MSG_N_LOCKS_P,
	MSG_CLICK,
	MSG_N_LOCKS_THE_D,
	MSG_THE_D_CLICKS,
	MSG_UNLOCK_WHAT,
	MSG_IT_CANT_BE_UNLOCKED,
	MSG_ITS_ALREADY_UNLOCKED,
	MSG_YOU_UNLOCK_P,
	MSG_N_UNLOCKS_P,
	MSG_N_UNLOCKS_THE_D,
	MSG_PICK_WHAT,
	MSG_CANT_PICK_MOUNTED,
	MSG_N_IS_STANDING_TOO_CLOSE_TO_LOCK,
	MSG_YOU_FAILED,
	MSG_YOU_PICK_THE_LOCK_ON_P,
	MSG_N_PICKS_THE_LOCK_ON_P,
	MSG_IT_CANT_BE_PICKED,
	MSG_N_PICKS_THE_D,
	MSG_MAYBE_YOU_SHOULD_FINISH_FIGHTING_FIRST,
	MSG_YOU_DONT_SEE_THAT,
	MSG_YOU_CANT_FIND_PLACE_STAND,
	MSG_THERES_NO_ROOM_TO_STAND_ON,
	MSG_YOU_CANT_WAKE_UP,
	MSG_YOU_WAKE_AND_STAND_UP,
	MSG_N_WAKES_AND_STANDS_UP,
	MSG_YOU_WAKE_AND_STAND_AT,
	MSG_N_WAKES_AND_STANDS_AT,
	MSG_YOU_WAKE_AND_STAND_ON,
	MSG_N_WAKES_AND_STANDS_ON,
	MSG_YOU_WAKE_AND_STAND_IN,
	MSG_N_WAKES_AND_STANDS_IN,
	MSG_FEEL_BLOOD_HEATS,
	MSG_YOU_STAND_UP,
	MSG_N_STANDS_UP,
	MSG_YOU_STAND_AT,
	MSG_N_STANDS_AT,
	MSG_YOU_STAND_ON,
	MSG_N_STANDS_ON,
	MSG_YOU_STAND_IN,
	MSG_N_STANDS_IN,
	MSG_YOU_ARE_ALREADY_STANDING,
	MSG_YOU_ARE_ALREADY_FIGHTING,
	MSG_YOU_CANT_REST_MOUNTED,
	MSG_YOU_CANT_REST_RIDDEN,
	MSG_YOU_ARE_ALREADY_SLEEPING,
	MSG_YOU_CANT_REST_ON_THAT,
	MSG_THERES_NO_MORE_ROOM_ON,
	MSG_YOU_WAKE_AND_REST,
	MSG_N_WAKES_AND_RESTS,
	MSG_YOU_WAKE_UP_AND_REST_AT,
	MSG_N_WAKES_UP_AND_RESTS_AT,
	MSG_YOU_WAKE_UP_AND_REST_ON,
	MSG_N_WAKES_UP_AND_RESTS_ON,
	MSG_YOU_WAKE_UP_AND_REST_IN,
	MSG_N_WAKES_UP_AND_RESTS_IN,
	MSG_YOU_ARE_ALREADY_RESTING,
	MSG_YOU_REST,
	MSG_N_SITS_DOWN_AND_RESTS,
	MSG_YOU_SIT_DOWN_AT_AND_REST,
	MSG_N_SITS_DOWN_AT_AND_RESTS,
	MSG_YOU_SIT_DOWN_ON_AND_REST,
	MSG_N_SITS_DOWN_ON_AND_RESTS,
	MSG_YOU_SIT_DOWN_IN_AND_REST,
	MSG_N_SITS_DOWN_IN_AND_RESTS,
	MSG_N_RESTS,
	MSG_YOU_REST_AT,
	MSG_N_RESTS_AT,
	MSG_YOU_REST_ON,
	MSG_N_RESTS_ON,
	MSG_YOU_REST_IN,
	MSG_N_RESTS_IN,
	MSG_YOU_CANT_SIT_MOUNTED,
	MSG_YOU_CANT_SIT_RIDDEN,
	MSG_YOU_CANT_SIT_ON_THAT,
	MSG_YOU_WAKE_AND_SIT_UP,
	MSG_N_WAKES_AND_SITS_UP,
	MSG_YOU_WAKE_AND_SIT_AT,
	MSG_N_WAKES_AND_SITS_AT,
	MSG_YOU_WAKE_AND_SIT_ON,
	MSG_N_WAKES_AND_SITS_ON,
	MSG_YOU_WAKE_AND_SIT_IN,
	MSG_N_WAKES_AND_SITS_IN,
	MSG_YOU_STOP_RESTING,
	MSG_YOU_SIT_AT,
	MSG_N_SITS_AT,
	MSG_YOU_SIT_ON,
	MSG_N_SITS_ON,
	MSG_YOU_ARE_ALREADY_SITTING_DOWN,
	MSG_YOU_SIT_DOWN,
	MSG_N_SITS_DOWN_ON_THE_GROUND,
	MSG_YOU_SIT_DOWN_AT,
	MSG_N_SITS_DOWN_AT,
	MSG_YOU_SIT_DOWN_IN,
	MSG_N_SITS_DOWN_IN,
	MSG_YOU_CANT_SLEEP_MOUNTED,
	MSG_YOU_CANT_SLEEP_RIDDEN,
	MSG_YOU_GO_TO_SLEEP,
	MSG_N_GOES_TO_SLEEP,
	MSG_YOU_CANT_SLEEP_ON_THAT,
	MSG_THERES_NO_ROOM_ON_P_FOR_YOU,
	MSG_YOU_GO_TO_SLEEP_AT,
	MSG_N_GOES_TO_SLEEP_AT,
	MSG_YOU_GO_TO_SLEEP_ON,
	MSG_N_GOES_TO_SLEEP_ON,
	MSG_YOU_GO_TO_SLEEP_IN,
	MSG_N_GOES_TO_SLEEP_IN,
	MSG_YOU_ARE_ASLEEP_YOURSELF,
	MSG_THEY_ARENT_HERE,
	MSG_N_IS_ALREADY_AWAKE,
	MSG_YOU_CANT_WAKE_M,
	MSG_N_WAKES_YOU,
	MSG_YOU_CANT_SNEAK_MOUNTED,
	MSG_YOU_ATTEMPT_TO_MOVE_SILENTLY,
	MSG_YOU_CANT_HIDE_MOUNTED,
	MSG_YOU_CANT_HIDE_RIDDEN,
	MSG_YOU_CANNOT_HIDE_GLOWING,
	MSG_YOU_ATTEMPT_TO_HIDE,
	MSG_YOU_CANT_CAMOUFLAGE_MOUNTED,
	MSG_YOU_CANT_CAMOUFLAGE_RIDDEN,
	MSG_YOU_DONT_KNOW_CAMOUFLAGE,
	MSG_YOU_CANT_CAMOUFLAGE_GLOWING,
	MSG_THERES_NO_COVER_HERE,
	MSG_N_TRIES_TO_CAMOUFLAGE,
	MSG_YOU_ATTEMPT_TO_CAMOUFLAGE,
	MSG_YOU_FADE_INTO_EXIST,
	MSG_N_FADES_INTO_EXIST,
	MSG_YOU_TRAMPLE_AROUND_LOUDLY,
	MSG_ONLY_PLAYERS_RECALL,
	MSG_RECALL_FOR_BELOW_10,
	MSG_N_PRAYS_FOR_TRANSPORTATION,
	MSG_YOU_ARE_COMPLETELY_LOST,
	MSG_GODS_FORSAKEN_YOU,
	MSG_YOU_ARE_STILL_FIGHTING,
	MSG_RECALL_FROM_COMBAT,
	MSG_N_DISAPPEARS,
	MSG_N_APPEARS_IN_THE_ROOM,
	MSG_YOU_CANT_DO_THAT_HERE,
	MSG_YOU_HAVE_D_TRAINING_SESSIONS,
	MSG_YOU_CAN_TRAIN,
	MSG_NOT_ENOUGH_TRAININGS,
	MSG_YOUR_DURABILITY_INCREASES,
	MSG_N_DURABILITY_INCREASES,
	MSG_YOUR_POWER_INCREASES,
	MSG_N_POWER_INCREASES,
	MSG_YOUR_T_IS_MAX,
	MSG_YOUR_T_INCREASES,
	MSG_N_T_INCREASES,
	MSG_THERE_ARE_NO_TRAIN_TRACKS_HERE,
	MSG_N_CHECKS_TRACKS,
	MSG_TRACKS_LEAD_S,
	MSG_DONT_SEE_TRACKS,
	MSG_YOU_CANT_BE_MORE_VAMPIRE,
	MSG_YOU_SHOW_MORE_UGGLY,
	MSG_GO_AND_ASK_QUESTOR,
	MSG_GO_KILL_PLAYER,
	MSG_WAIT_NIGHT,
	MSG_FEEL_GREATER,
	MSG_CANNOT_RECOGNIZE,
	MSG_DONT_KNOW_BITE,
	MSG_MUST_TRANSFORM_VAMP,
	MSG_BITE_WHOM,
	MSG_THEY_MUST_BE_SLEEPING,
	MSG_HOW_CAN_YOU_SNEAK_YOU,
	MSG_CANT_BITE_FIGHTING_PERS,
	MSG_DOESNT_WORTH_UP,
	MSG_DOESNT_WORTH_TO_DO,
	MSG_HELP_TRIED_TO_BITE,
	MSG_BASH_WHATS_THAT,
	MSG_IS_GUSHING_BLOOD,
	MSG_POS_NAME_DEAD,
	MSG_POS_NAME_MORTALLY_WOUNDED,
	MSG_POS_NAME_INCAPACITATED,
	MSG_POS_NAME_STUNNED,
	MSG_POS_NAME_SLEEPING,
	MSG_POS_NAME_RESTING,
	MSG_POS_NAME_SITTING,
	MSG_POS_NAME_FIGHTING,
	MSG_POS_NAME_STANDING,
	MSG_LEAVES,
	MSG_LEAVES_RIDING_ON,
	MSG_LEAVES_T,
	MSG_LEAVES_T_RIDING_ON,
	MSG_ARRIVED,
	MSG_ARRIVED_RIDING,
	MSG_THANK_YOU_BRAVE,
	MSG_SKILLS_HAVE_BECOME_BETTER,
	MSG_SKILLS_LEARN_FROM_MISTAKES,
	MSG_HERA_STEPS_INTO_RIDING_ON,
	MSG_HERA_STEPS_INTO,
	MSG_WHEN_YOU_ATTEMPT_YOU_BREAK_WEBS,
	MSG_N_BREAKS_THE_WEBS,
	MSG_YOU_ATTEMPT_WEBS_HOLD_YOU,
	MSG_N_STRUGGLES_VAINLY_AGAINST_WEBS,
	MSG_YOU_STEP_OUT_SHADOWS,
	MSG_N_STEPS_OUT_OF_SHADOWS,
	MSG_YOU_STEP_OUT_COVER,
	MSG_N_STEPS_OUT_COVER,
	MSG_YOU_FAILED_TO_PASS,
	MSG_N_TRIES_TO_PASS_FAILED,
	MSG_TOO_PUMPED_TO_PRAY,
	MSG_CANT_BASH_DOORS_MOUNTED,
	MSG_CANT_BASH_DOORS_RIDDEN,
	MSG_BASH_WHICH_DOOR,
	MSG_WAIT_FIGHT_FINISH,
	MSG_N_TOO_CLOSE_TO_DOOR,
	MSG_TRY_TO_OPEN,
	MSG_SHIELD_PROTECTS_EXIT,
	MSG_YOU_SLAM_TRY_BREAK,
	MSG_N_SLAMS_TRY_BREAK,
	MSG_TATTOO_GLOWS_BLUE,
	MSG_TATTOO_GLOWS_RED,
	MSG_N_BASHES_AND_BREAK,
	MSG_YOU_SUCCESSED_TO_OPEN_DOOR,
	MSG_YOU_FALL_ON_FACE,
	MSG_N_FALLS_ON_FACE,	
	MSG_HUH,
	MSG_CURRENT_BLINK,
	MSG_BLINK_ON,
	MSG_BLINK_OFF,
	MSG_IS_S_A_STATUS,
	MSG_DONT_HAVE_POWER,
	MSG_N_THROWS_GLOBE,
	MSG_N_IS_GONE,
	MSG_N_APPEARS_FROM_NOWHERE,
	MSG_YOU_CAN_ALREADY_DETECT_SNEAK,
	MSG_YOU_CAN_DETECT_THE_SNEAK,
	MSG_CANT_FADE_MOUNTED,
	MSG_CANT_FADE_RIDDEN,
	MSG_YOU_ATTEMPT_TO_FADE,
	MSG_LACK_SKILL_DRAIN_TOUCH,
	MSG_LET_IT_BE,
	MSG_DONT_WANT_DRAIN_MASTER,
	MSG_EVEN_YOU_NOT_SO_STUPID,
	MSG_YOU_TOUCH_NS_NECK,
	MSG_N_TOUCHES_YOUR_NECK,
	MSG_N_TOUCHES_NS_NECK,
	MSG_YOU_ARE_ALREADY_FLYING,
	MSG_YOU_START_TO_FLY,
	MSG_FIND_POTION_OR_WINGS,
	MSG_YOU_SLOWLY_TOUCH_GROUND,
	MSG_YOU_ARE_ALREADY_ON_GROUND,
	MSG_TYPE_WITH_UP_OR_DOWN,
	MSG_MOB_PROGS_I_DO_NOT_GET_PAID_ENOUGH,
	MSG_YOUR_GAIN_IS,
	MSG_YOU_RAISE_A_LEVEL,
	MSG_TELLS_YOU,
	MSG_YOU_TELL,
	MSG_YOUR_MESSAGE_DIDNT_GET_THROUGH,
	MSG_N_ASKS_FOR_QUEST,
	MSG_WRONG_CLASS,
	MSG_YOU_ASK_FOR_QUEST,
	MSG_YOU_ALREADY_ON_QUEST,
	MSG_BRAVE_BUT_LET_SOMEONE_ELSE,
	MSG_COME_BACK_LATER,
	MSG_DONT_HAVE_QUESTS,
	MSG_TRY_AGAIN_LATER,
	MSG_RUNES_MOST_HEINOUS,
	MSG_HAS_MURDERED,
	MSG_THE_PENALTY_IS,
	MSG_ENEMY_OF_MINE,
	MSG_ELIMINATE_THREAT,
	MSG_SEEK_S_OUT,
	MSG_LOCATION_IS_IN_AREA,
	MSG_VILE_PILFERERS,
	MSG_MY_COURT_WIZARDESS,
	MSG_IS_ALMOST_COMPLETE,
	MSG_RECOVER_FABLED,
	MSG_ARENT_ON_QUEST,
	MSG_SLAY_DREADED,
	MSG_YOU_HAVE_D_QP,
	MSG_D_MIN_REMAINING,
	MSG_LESS_MINUTE,
	MSG_LEFT_FOR_QUEST,
	MSG_WAIT_FIGHT_STOPS,
	MSG_N_ASKS_LIST,
	MSG_YOU_ASK_LIST,
	MSG_TYPE_BUY,
	MSG_NOT_ENOUGH_QP,
	MSG_ITEM_BEYOND,
	MSG_N_GIVES_PRACS,
	MSG_N_GIVES_YOU_PRACS,
	MSG_N_GIVES_SECRET,
	MSG_N_GIVES_YOU_SECRET,
	MSG_NO_DEATHS,
	MSG_DONT_HAVE_KATANA,
	MSG_AS_YOU_WIELD_IT,
	MSG_NO_RELIGION,
	MSG_ALREADY_TATTOOED,
	MSG_N_TATTOOS_N,
	MSG_N_TATTOOS_YOU,
	MSG_N_GIVES_GOLD,
	MSG_N_GIVES_YOU_GOLD,
	MSG_NOT_HAVE_ITEM,
	MSG_GIVES_P_TO_N,
	MSG_GIVES_YOU_P,
	MSG_INFORMS_COMPLETE,
	MSG_YOU_INFORM_COMPLETE,
	MSG_NEVER_QUEST,
	MSG_GRATS_COMPLETE,
	MSG_AS_A_REWARD,
	MSG_GAIN_PRACS,
	MSG_YOU_HAND_P,
	MSG_N_HANDS_P,
	MSG_HAVENT_COMPLETE,
	MSG_DIDNT_COMPLETE_IN_TIME,
	MSG_HAVE_TO_REQUEST,
	MSG_TYPE_TROUBLE,
	MSG_HAVENT_BOUGHT,
	MSG_THIS_IS_THE_NTH_TIME,
	MSG_WONT_GIVE_AGAIN,
	MSG_COMMANDS,
	MSG_TYPE_HELP_QUEST,
	MSG_YOU_MAY_NOW_QUEST_AGAIN,
	MSG_RUN_OUT_TIME,
	MSG_BETTER_HURRY,
	MSG_ITEMS_AVAIL_PURCHASE,
	MSG_YOU_COMPLETED_SOMEONES_QUEST,
	MSG_SOMEONE_COMPLETED_YOUR_QUEST,
	MSG_ALMOST_COMPLETE_QUEST,
	MSG_RETURN_TO_QUESTER,
	MSG_CONSIDER_WHOM,
	MSG_DONT_EVEN_THINK,
	MSG_CAN_KILL_NAKED,
	MSG_IS_NOT_MATCH_FOR_YOU,
	MSG_LOOKS_EASY_KILL,
	MSG_PERFECT_MATCH,
	MSG_FEEL_LUCKY_PUNK,
	MSG_LAUGHS_AT_YOU_MERCILESSLY,
	MSG_DEATH_WILL_THANK_YOU,
	MSG_GRINS_EVILLY_WITH,
	MSG_GREETS_YOU_WARMLY,
	MSG_HOPE_YOU_WILL_TURN,
	MSG_GRINS_EVILLY_AT,
	MSG_GRINS_EVILLY,
	MSG_SMILES_HAPPILY,
	MSG_LOOKS_AS_YOU,
	MSG_LOOKS_VERY_DISINTERESTED,
	MSG_SCAN_HERE,
	MSG_SCAN_DOOR_CLOSED,
	MSG_PUSH_WHOM_WHERE,
	MSG_CANT_PUSH_MOUNTED,
	MSG_CANT_PUSH_RIDDEN,
	MSG_TOO_DAZED_TO_PUSH,
	MSG_THATS_POINTLESS,	
	MSG_DIR_IS_CLOSED,
	MSG_DIR_IS_LOCKED,
	MSG_YOU_WEBBED_WANT_WHAT,
	MSG_N_TRIES_PUSH_WEBBED,
	MSG_PUSH_VICT_WEBBED,
	MSG_N_PUSHES_VICT_WEBBED,
	MSG_OOPS,
	MSG_N_TRIED_PUSH_YOU,
	MSG_N_TRIED_PUSH_N,
	MSG_KEEP_HANDS_OUT,
	MSG_SAVAGELY_DEVOURS_CORPSE,
	MSG_PICKS_SOME_TRASH,
	MSG_YOU_BITE_N,
	MSG_N_BITES_N,
	MSG_N_BITES_YOU,
	MSG_DO_I_KNOW_YOU,
	MSG_DONT_REMEMBER_YOU,
	MSG_OK_MY_DEAR,
	MSG_YOU_SETTLE_DOWN,
	MSG_N_SAYS,
	MSG_YOU_SAY,
	MSG_INFO_LOCATION,
	MSG_CANT_PRAY_NOW,
	MSG_PRAYS_UPPER_LORD,
	MSG_ARENT_FIGHTING,
	MSG_ESCAPE_WHAT_DIR,
	MSG_CANT_ESCAPE_MOUNTED,
	MSG_CANT_ESCAPE_RIDDEN,
	MSG_TRY_FLEE,
	MSG_SOMETHING_PREVENTS_ESCAPE,
	MSG_ESCAPE_FAILED,
	MSG_N_ESCAPED,
	MSG_YOU_ESCAPED_FROM_COMBAT,
	MSG_YOU_FLED_FROM_COMBAT,
	MSG_YOU_LOSE_D_EXPS,
	MSG_WRONG_DIRECTION,
	MSG_COULDNT_ESCAPE,
	MSG_CANT_LAY_HANDS,
	MSG_CANT_CONCENTRATE_ENOUGH,
	MSG_WARM_FEELING,
	MSG_ALREADY_HUMAN,
	MSG_RETURN_TO_SIZE,
	MSG_MOUNT_WHAT,
	MSG_DONT_KNOW_RIDE,
	MSG_CANT_RIDE_THAT,
	MSG_BEAST_TOO_POWERFUL,
	MSG_S_BELONGS_TO_S,
	MSG_MOUNT_MUST_STAND,
	MSG_ALREADY_RIDDEN,
	MSG_ALREADY_RIDING,
	MSG_FAIL_TO_MOUNT,
	MSG_YOU_HOP_ON_N,
	MSG_N_HOPS_ON_N,
	MSG_N_HOPS_ON_YOU,
	MSG_YOU_DISMOUNT_N,
	MSG_N_DISMOUNTS_N,
	MSG_N_DISMOUNTS_YOU,
	MSG_YOU_ARENT_MOUNTED,
	MSG_MOUNT_DRUNKEN,
	MSG_YOU_FALL_OFF_N,
	MSG_N_FALLS_OFF_N,
	MSG_N_FALLS_OFF_YOU,
	MSG_N_DOESNT_LIKE_YOU,
	MSG_N_DOESNT_LIKE_N,
	MSG_YOU_DONT_LIKE_N,
	MSG_N_SNARLS_YOU,
	MSG_N_SNARLS_N,
	MSG_YOU_SNARL_N,
	MSG_FIGHTING_YOU,
	MSG_REBOOT_IN,
	MSG_AUTOREBOOT_IN,
	MSG_P_STRIKES_YOU,
	MSG_YOU_P_STRIKES_N,
	MSG_N_P_STRIKES_N,
	MSG_TARGET,
	MSG_INVIS,
	MSG_HIDDEN,
	MSG_CHARMED,
	MSG_TRANSLUCENT,
	MSG_PINK_AURA,
	MSG_UNDEAD,
	MSG_RIDDEN,
	MSG_IMPROVED,
	MSG_RED_AURA,
	MSG_GOLDEN_AURA,
	MSG_WHITE_AURA,
	MSG_BLUE_AURA,
	MSG_FADE,
	MSG_WANTED,
	MSG_CAMF,
	MSG_IS_DEAD,
	MSG_IS_MORTALLY_WOUNDED,
	MSG_IS_INCAPACITATED,
	MSG_IS_LYING_HERE_STUNNED,
	MSG_N_LOOKS_AT_SELF,
	MSG_N_LOOKS_AT_YOU,
	MSG_N_LOOKS_AT_N,
	MSG_SEE_NOTHING_SPECIAL,
	MSG_IS_RIDING_S,
	MSG_IS_RIDDEN_BY_S,
	MSG_DARK,
	MSG_MAGICAL,
	MSG_GLOWING,
	MSG_HUMMING,
	MSG_GLOWING_RED_EYES,
	MSG_FEEL_MORE_LIVES,	
	MSG_CANT_SEE_THING,
	MSG_HERE_RIDING,
	MSG_THATS_NOT_CONTAINER,
	MSG_IT_IS_EMPTY,
	MSG_ITS_FILLED_S,
	MSG_LESS_THAN_HALF,
	MSG_ABOUT_HALF,
	MSG_MORE_THAN_HALF,
	MSG_IT_IS_CLOSED,
	MSG_P_HOLDS,
	MSG_CANT_SEE_BUT_STARS,
	MSG_CANT_SEE_SLEEPING,
	MSG_PITCH_BLACK,
	MSG_LOOK_IN_WHAT,
	MSG_NOTHING_SPECIAL_IT,
	MSG_ONLY_SEE_ONE_S,
	MSG_ONLY_SEE_D_THOSE,
	MSG_NOTHING_SPECIAL_THERE,
	MSG_THE_D_IS_OPEN,
	MSG_EXA_WHAT,
	MSG_NO_COINS_PILE,
	MSG_ONE_GOLD_COIN,
	MSG_D_GOLD_COINS,
	MSG_ONE_SILVER_COIN,
	MSG_D_SILVER_COINS,
	MSG_D_SILVER_AND_D_GOLD,
	MSG_EXITS,
	MSG_MSG_OBVIOUS_EXITS_IMM,
	MSG_OBVIOUS_EXITS,
	MSG_TOO_DARK_TO_TELL,
	MSG_ROOM_D,
	MSG_NONE,
	MSG_NONE_DOT,
	MSG_HAVE_D_GOLD_D_SILVER,
	MSG_AND_D_EXP,
	MSG_HAVE_KILLED,
	MSG_NON_GOODS,
	MSG_NON_EVILS,
	MSG_NON_NEUTRALS,
	MSG_GOODS,
	MSG_EVILS,
	MSG_NEUTRALS,
	MSG_NO_HELP_ON_WORD,
	MSG_PLAYERS_FOUND,
	MSG_MUST_PROVIDE_NAME,
	MSG_NO_ONE_THAT_NAME,
	MSG_D_CHARS_ON,
	MSG_MOST_SO_FAR_TODAY,
	MSG_MOST_TODAY_WAS,
	MSG_SOMETHING,
	MSG_COMPARE_WHAT,
	MSG_DONT_HAVE_ITEM,
	MSG_ARENT_WEAR_COMPARABLE,
	MSG_COMPARE_P_TO_ITSELF,
	MSG_CANT_COMPARE_P_P,
	MSG_P_P_LOOKS_SAME,
	MSG_P_LOOKS_BETTER_P,
	MSG_P_LOOKS_WORSE_P,
	MSG_TOO_DARK_SEE,
	MSG_PLAYERS_NEAR_YOU,
	MSG_DIDNT_FIND_ANY,
	MSG_CANT_CHANGE_TITLE,
	MSG_CHANGE_TITLE_TO_WHAT,
	MSG_ILLEGAL_CHARACTER_TITLE,
	MSG_NO_LINES_TO_REMOVE,
	MSG_YOUR_DESC_IS,
	MSG_REPORT_I_HAVE,
	MSG_REPORT_I_HAVE_TO_CH,
	MSG_YOU_SCAN_S,
	MSG_N_SCANS_S,
	MSG_FROZEN,
	MSG_ASK_MASTER,
	MSG_TOO_STUNNED,
	MSG_YOU_ARE_DEAD,
	MSG_HURT_FAR_TOO_BAD,
	MSG_YOU_TOO_STUNNED,
	MSG_IN_YOUR_DREAMS,
	MSG_TOO_RELAXED,
	MSG_BETTER_STANDUP,
	MSG_NO_WAY_FIGHT,
	MSG_NOT_AFFECTED_SPELLS,
	MSG_YOU_ARE_AFFECTED,
	MSG_AFF_SPELL,
	MSG_AFF_MODIFIES,
	MSG_AFF_PERMANENTLY,
	MSG_AFF_FOR_D_HOURS,
	MSG_AFF_BY,
	MSG_ANTI_SOCIAL,
	MSG_YOU_CANT_TAKE_THAT,
	MSG_YOU_ZAPPED_BY_P,
	MSG_N_ZAPPED_BY_P,
	MSG_CANT_CARRY_ITEMS,
	MSG_CANT_CARRY_WEIGHT,
	MSG_N_APPEARS_USING,
	MSG_YOU_GET_P_FROM_P,
	MSG_N_GETS_P_FROM_P,
	MSG_YOU_GET_P,
	MSG_N_GETS_P,
	MSG_GET_WHAT,
	MSG_I_SEE_NOTHING_HERE,
	MSG_DONT_SEE_ANYTHING_LIKE_IN_T,
	MSG_DONT_BE_SO_GREEDY,
	MSG_I_SEE_NOTHING_IN_T,
	MSG_PUT_WHAT_IN_WHAT,
	MSG_CANT_FOLD_INTO_SELF,
	MSG_CANT_LET_GO_OF_IT,
	MSG_THAT_WOULD_BE_BAD_IDEA,
	MSG_IT_WONT_FIT,
	MSG_N_PUTS_P_ON_P,
	MSG_YOU_PUT_P_ON_P,
	MSG_N_PUTS_P_IN_P,
	MSG_YOU_PUT_P_IN_P,
	MSG_DROP_WHAT,
	MSG_DONT_HAVE_MUCH_SILVER,
	MSG_DONT_HAVE_MUCH_GOLD,
	MSG_N_DROPS_SOME_COINS,
	MSG_N_DROPS_P,
	MSG_YOU_DROP_P,
	MSG_YOU_BEGIN_TO_SNEAK_AGAIN,
	MSG_YOU_STEP_BACK_SHADOWS,
	MSG_N_WANDERS_ON_HOME,
	MSG_N_GOES_OUT,
	MSG_N_FLICKERS_OUT,
	MSG_P_FLICKERS,
	MSG_N_DISAPPEARS_VOID,
	MSG_YOU_DISAPPEAR_VOID,
	MSG_WITCH_CURSE_MAKES_N,
	MSG_WITCH_CURSE_MAKES_YOU,
	MSG_N_WRITHES_PLAGUE,
	MSG_YOU_WRITHE_PLAGUE,
	MSG_YOU_FEEL_HOT_FEVERISH,
	MSG_N_SHIVERS_ILL,
	MSG_N_SHIVERS_SUFFERS,
	MSG_YOU_SHIVER_SUFFER,
	MSG_WEAR_WHAT,
	MSG_MUST_BE_LEVEL_TO_USE,
	MSG_N_TRIES_TO_USE,
	MSG_N_LIGHTS_P_HOLD,
	MSG_YOU_LIGHT_P_HOLD,
	MSG_N_WEARS_P_LEFT_FINGER,
	MSG_YOU_WEAR_P_LEFT_FINGER,
	MSG_N_WEARS_P_RIGHT_FINGER,
	MSG_YOU_WEAR_P_RIGHT_FINGER,
	MSG_ALREADY_WEAR_TWO_RINGS,
	MSG_N_WEARS_P_NECK,
	MSG_YOU_WEAR_P_NECK,
	MSG_ALREADY_WEAR_TWO_NECK,
	MSG_N_WEARS_P_TORSO,
	MSG_YOU_WEAR_P_TORSO,
	MSG_N_WEARS_P_HEAD,
	MSG_YOU_WEAR_P_HEAD,
	MSG_N_WEARS_P_LEGS,
	MSG_YOU_WEAR_P_LEGS,
	MSG_N_WEARS_P_FEET,
	MSG_YOU_WEAR_P_FEET,
	MSG_N_WEARS_P_HANDS,
	MSG_YOU_WEAR_P_HANDS,
	MSG_N_WEARS_P_ARMS,
	MSG_YOU_WEAR_P_ARMS,
	MSG_N_WEARS_P_WAIST,
	MSG_YOU_WEAR_P_WAIST,
	MSG_N_WEARS_P_LEFT_WRIST,
	MSG_YOU_WEAR_P_LEFT_WRIST,
	MSG_N_WEARS_P_RIGHT_WRIST,
	MSG_YOU_WEAR_P_RIGHT_WRIST,
	MSG_ALREADY_WEAR_TWO_WRIST,
	MSG_CANT_USE_SHIELD_SECOND_WEAPON,
	MSG_YOUR_HANDS_TIRED_WEAPON,
	MSG_N_WEARS_P_SHIELD,
	MSG_YOU_WEAR_P_SHIELD,
	MSG_TOO_HEAVY_WIELD,
	MSG_NEED_TWO_HANDS,
	MSG_N_WIELDS_P,
	MSG_YOU_WIELD_P,
	MSG_P_LIKE_PART_OF_YOU,
	MSG_QUITE_CONFIDENT_P,
	MSG_SKILLED_WITH_P,
	MSG_SKILL_P_ADEQUATE,
	MSG_P_FEELS_CLUMSY,
	MSG_ALMOST_DROP_P,
	MSG_DONT_KNOW_THE_END,
	MSG_CANT_HOLD_WHILE_2_WEAPONS,
	MSG_N_HOLDS_P_HAND,
	MSG_YOU_HOLD_P_HAND,
	MSG_N_FLOAT_P,
	MSG_YOU_FLOAT_P,
	MSG_N_USES_TATTOO,
	MSG_YOU_USE_TATTOO,
	MSG_CANT_WEAR_IT,
	MSG_REMOVE_WHAT,
	MSG_N_SACS_SELF,
	MSG_YOU_SAC_SELF,
	MSG_CANT_FIND_IT,
	MSG_GODS_WOUDLNT_LIKE_THAT,
	MSG_P_NOT_ACCEPTABLE_SAC,
	MSG_SAC_GET_ONE_SILVER,
	MSG_SAC_GET_D_SILVER,
	MSG_N_SACS_P,
	MSG_YOUR_SAC_REVEALS_P,
	MSG_P_IS_REVEALED_BY_N_SAC,
	MSG_YOUR_SAC_REVEALS_P_P,
	MSG_P_IS_REVEALED_BY_N_SAC_P,
	MSG_AS_YOU_SAC,
	MSG_AS_N_SACS,
	MSG_FEW_THINGS,
	MSG_BUNCH_OF_OBJECTS,
	MSG_MANY_THINGS,
	MSG_LOT_OF_OBJECTS,
	MSG_ON_IT,
	MSG_SCATTER_ON_DIRT,
	MSG_SCATTER_OVER_WATER,
	MSG_SCATTER_AROUND,
	MSG_QUAFF_WHAT,
	MSG_DONT_HAVE_POTION,
	MSG_CAN_QUAFF_ONLY_POTIONS,
	MSG_TOO_POWERFUL_LIQUID,
	MSG_N_QUAFFS_P,
	MSG_YOU_QUAFF_P,
	MSG_DONT_HAVE_SCROLL,
	MSG_CAN_RECITE_ONLY_SCROLLS,
	MSG_SCROLL_TOO_COMPLEX,
	MSG_N_RECITES_P,
	MSG_YOU_RECITE_P,
	MSG_MISPRONOUNCE_SYLLABLE,
	MSG_NS_P_STRIKES_N,
	MSG_P_STRIKES_N,
	MSG_CANT_REMOVE_IT,
	MSG_YOU_MUST_SCRATCH_TO_REMOVE,
	MSG_YOU_REMOVE_P_IN_PAIN,
	MSG_N_REMOVES_P_IN_PAIN,
	MSG_N_STOPS_USING_P,
	MSG_YOU_STOP_USING_P,
	MSG_EAT_WHAT,
	MSG_NOT_EDIBLE,
	MSG_TOO_FULL_TO_EAT,
	MSG_N_EATS_P,
	MSG_YOU_EAT_P,
	MSG_NO_LONGER_HUNGRY,
	MSG_YOU_ARE_FULL,
	MSG_N_CHOKES_GAGS,
	MSG_YOU_CHOKE_GAG,
	MSG_DRINK_WHAT,
	MSG_FAIL_TO_REACH_MOUTH,
	MSG_CANT_DRINK_FROM_THAT,
	MSG_TOO_FULL_TO_DRINK,
	MSG_N_DRINKS_T_FROM_P,
	MSG_YOU_DRINK_T_FROM_P,
	MSG_YOU_FEEL_DRUNK,
	MSG_YOUR_THIRST_QUENCHED,
	MSG_UNWORTHY_CANT_HOLD,
	MSG_N_UTTERS_THE_WORDS,
	MSG_CONFEDERACY_PRAYS,
	MSG_CONFEDERACY_VANISHES,
	MSG_REACH_TRUE_SOURCE_STOP,
	MSG_CANT_GET_RIGHT_INTONATIONS,
	MSG_CAST_WHAT_WHERE,
	MSG_DONT_KNOW_ANY_SPELLS_NAME,
	MSG_MUST_TRANSFORM_VAMPIRE,
	MSG_THATS_NOT_A_SPELL,
	MSG_YOUR_SPELL_FIZZLES_FAILS,
	MSG_N_SPELL_FIZZLES_FAILS,
	MSG_YOUR_SPELLBANE_DEFLECTS,
	MSG_N_SPELLBANE_DEFLECTS,
	MSG_CAST_SPELL_ON_WHOM,
	MSG_CANT_CAST_SPELL_ON_N_FAR,
};

#endif
