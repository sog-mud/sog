#ifndef _RESOURCE_H_
#define _RESOURCE_H_

/*
 * $Id: resource.h,v 1.78 1998-06-21 22:39:56 efdi Exp $
 */

#define msg(msgid, ch) vmsg(msgid, ch, ch)
char* vmsg(int msgid, CHAR_DATA *ch, CHAR_DATA *victim);
char* exact_msg(int msgid, int i_lang, int sex);
void msgdb_load();

extern char** ilang_names;
extern nilang;

enum {
	NOTHING, 
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
	SLEEPING_AT,
	SLEEPING_ON,
	SLEEPING_IN,
	SLEEPING,
	RESTING_AT,
	RESTING_ON,
	RESTING_IN,
	RESTING,
	SITTING_AT,
	SITTING_ON,
	SITTING_IN,
	SITTING,
	STANDING_AT,
	STANDING_ON,
	STANDING_IN,
	STANDING,
	FIGHTING,
	IS_HERE,
	IS_IN_PERFECT_HEALTH,
	HAS_A_FEW_SCRATCHES,
	HAS_SOME_SMALL_BUT_DISGUSTING_CUTS,
	IS_COVERED_WITH_BLEEDING_WOUNDS,
	IS_WRITHING_IN_AGONY,
	IS_CONVULSING_ON_THE_GROUND,
	IS_NEARLY_DEAD,
	YOU_ARE,
	YOU_ARE_CARRYING,
	YOU_ARE_USING,
	IS_USING,
	YOU_PEEK_AT_THE_INVENTORY,
	COMM_INTERFACE_LANGUAGE_UNDEFINED,
	COMM_SHOW_LANGUAGE,
	COMM_LANGUAGE_USAGE_PRE,
	COMM_LANGUAGE_USAGE_POST,
	COND_EXCELLENT,
	COND_GOOD,
	COND_FINE,
	COND_AVERAGE,
	COND_POOR,
	COND_FRAGILE,
	WEATHER_DAY_BEGUN,
	WEATHER_SUN_IN_THE_EAST,
	WEATHER_SUN_IN_THE_WEST,
	WEATHER_NIGHT_BEGUN,
	WEATHER_GETTING_CLOUDY,
	WEATHER_IT_STARTS_TO_RAIN,
	WEATHER_THE_CLOUDS_DISAPPEAR,
	WEATHER_LIGHTNING_FLASHES,
	WEATHER_THE_RAIN_STOPPED,
	WEATHER_LIGHTNING_STOPPED,
	QUEST_YOU_HAVE_D_MINUTES,
	QUEST_MAY_THE_GODS_GO,
	COMM_ORDERS_YOU_TO,
	COMM_YOU_PUSH,
	COMM_PUSHES_YOU,
	COMM_PUSHES_N_TO,
	HERA_IS_FROM_HERE,
	S_S,
	ITS_S,
	TIME_DAWN,
	TIME_MORNING,
	TIME_MID_DAY,
	TIME_EVENING,
	TIME_NIGHT,
	ALAS_YOU_CANNOT_GO,
	THE_D_IS_CLOSED,
	WHAT_YOU_LEAVE,
	ROOM_IS_PRIVATE,
	MOUNT_REFUSES_GO_THAT_WAY,
	YOU_ARENT_ALLOWED_THERE,
	YOU_FEEL_TOO_BLOODY,
	YOUR_MOUNT_CANT_FLY,
	YOU_CANT_FLY,
	YOU_CANT_TAKE_MOUNT_THERE,
	YOU_NEED_A_BOAT,
	YOU_TOO_EXHAUSTED,
	YOU_CANT_BRING_N_CITY,
	YOU_ARENT_ALLOWED_CITY,
	YOU_FOLLOW_N,
	I_SEE_NO_EXIT_T_HERE,
	I_SEE_NO_T_HERE,
	I_SEE_NO_DOOR_T_HERE,
	YOU_CANT_DO_THAT,
	N_LOOKS_ALL_AROUND,
	LOOKING_AROUND_YOU_SEE,
	OPEN_WHAT,
	ITS_ALREADY_OPEN,
	ITS_LOCKED,
	YOU_OPEN_P,
	N_OPENS_P,
	THATS_NOT_A_CONTAINER,
	N_OPENS_THE_D,
	OK,
	THE_D_OPENS,
	CLOSE_WHAT,
	ITS_ALREADY_CLOSED,
	YOU_CLOSE_P,
	N_CLOSES_P,
	N_CLOSES_THE_D,
	THE_D_CLOSES,
	LOCK_WHAT,
	ITS_NOT_CLOSED,
	IT_CANT_BE_LOCKED,
	YOU_LACK_THE_KEY,
	ITS_ALREADY_LOCKED,
	YOU_LOCK_P,
	N_LOCKS_P,
	CLICK,
	N_LOCKS_THE_D,
	THE_D_CLICKS,
	UNLOCK_WHAT,
	IT_CANT_BE_UNLOCKED,
	ITS_ALREADY_UNLOCKED,
	YOU_UNLOCK_P,
	N_UNLOCKS_P,
	N_UNLOCKS_THE_D,
	PICK_WHAT,
	CANT_PICK_MOUNTED,
	N_IS_STANDING_TOO_CLOSE_TO_LOCK,
	YOU_FAILED,
	YOU_PICK_THE_LOCK_ON_P,
	N_PICKS_THE_LOCK_ON_P,
	IT_CANT_BE_PICKED,
	N_PICKS_THE_D,
	MAYBE_YOU_SHOULD_FINISH_FIGHTING_FIRST,
	YOU_DONT_SEE_THAT,
	YOU_CANT_FIND_PLACE_STAND,
	THERES_NO_ROOM_TO_STAND_ON,
	YOU_CANT_WAKE_UP,
	YOU_WAKE_AND_STAND_UP,
	N_WAKES_AND_STANDS_UP,
	YOU_WAKE_AND_STAND_AT,
	N_WAKES_AND_STANDS_AT,
	YOU_WAKE_AND_STAND_ON,
	N_WAKES_AND_STANDS_ON,
	YOU_WAKE_AND_STAND_IN,
	N_WAKES_AND_STANDS_IN,
	FEEL_BLOOD_HEATS,
	YOU_STAND_UP,
	N_STANDS_UP,
	YOU_STAND_AT,
	N_STANDS_AT,
	YOU_STAND_ON,
	N_STANDS_ON,
	YOU_STAND_IN,
	N_STANDS_IN,
	YOU_ARE_ALREADY_STANDING,
	YOU_ARE_ALREADY_FIGHTING,
	YOU_CANT_REST_MOUNTED,
	YOU_CANT_REST_RIDDEN,
	YOU_ARE_ALREADY_SLEEPING,
	YOU_CANT_REST_ON_THAT,
	THERES_NO_MORE_ROOM_ON,
	YOU_WAKE_AND_REST,
	N_WAKES_AND_RESTS,
	YOU_WAKE_UP_AND_REST_AT,
	N_WAKES_UP_AND_RESTS_AT,
	YOU_WAKE_UP_AND_REST_ON,
	N_WAKES_UP_AND_RESTS_ON,
	YOU_WAKE_UP_AND_REST_IN,
	N_WAKES_UP_AND_RESTS_IN,
	YOU_ARE_ALREADY_RESTING,
	YOU_REST,
	N_SITS_DOWN_AND_RESTS,
	YOU_SIT_DOWN_AT_AND_REST,
	N_SITS_DOWN_AT_AND_RESTS,
	YOU_SIT_DOWN_ON_AND_REST,
	N_SITS_DOWN_ON_AND_RESTS,
	YOU_SIT_DOWN_IN_AND_REST,
	N_SITS_DOWN_IN_AND_RESTS,
	N_RESTS,
	YOU_REST_AT,
	N_RESTS_AT,
	YOU_REST_ON,
	N_RESTS_ON,
	YOU_REST_IN,
	N_RESTS_IN,
	YOU_CANT_SIT_MOUNTED,
	YOU_CANT_SIT_RIDDEN,
	YOU_CANT_SIT_ON_THAT,
	YOU_WAKE_AND_SIT_UP,
	N_WAKES_AND_SITS_UP,
	YOU_WAKE_AND_SIT_AT,
	N_WAKES_AND_SITS_AT,
	YOU_WAKE_AND_SIT_ON,
	N_WAKES_AND_SITS_ON,
	YOU_WAKE_AND_SIT_IN,
	N_WAKES_AND_SITS_IN,
	YOU_STOP_RESTING,
	YOU_SIT_AT,
	N_SITS_AT,
	YOU_SIT_ON,
	N_SITS_ON,
	YOU_ARE_ALREADY_SITTING_DOWN,
	YOU_SIT_DOWN,
	N_SITS_DOWN_ON_THE_GROUND,
	YOU_SIT_DOWN_AT,
	N_SITS_DOWN_AT,
	YOU_SIT_DOWN_IN,
	N_SITS_DOWN_IN,
	YOU_CANT_SLEEP_MOUNTED,
	YOU_CANT_SLEEP_RIDDEN,
	YOU_GO_TO_SLEEP,
	N_GOES_TO_SLEEP,
	YOU_CANT_SLEEP_ON_THAT,
	THERES_NO_ROOM_ON_P_FOR_YOU,
	YOU_GO_TO_SLEEP_AT,
	N_GOES_TO_SLEEP_AT,
	YOU_GO_TO_SLEEP_ON,
	N_GOES_TO_SLEEP_ON,
	YOU_GO_TO_SLEEP_IN,
	N_GOES_TO_SLEEP_IN,
	YOU_ARE_ASLEEP_YOURSELF,
	THEY_ARENT_HERE,
	N_IS_ALREADY_AWAKE,
	YOU_CANT_WAKE_M,
	N_WAKES_YOU,
	YOU_CANT_SNEAK_MOUNTED,
	YOU_ATTEMPT_TO_MOVE_SILENTLY,
	YOU_CANT_HIDE_MOUNTED,
	YOU_CANT_HIDE_RIDDEN,
	YOU_CANNOT_HIDE_GLOWING,
	YOU_ATTEMPT_TO_HIDE,
	YOU_CANT_CAMOUFLAGE_MOUNTED,
	YOU_CANT_CAMOUFLAGE_RIDDEN,
	YOU_DONT_KNOW_CAMOUFLAGE,
	YOU_CANT_CAMOUFLAGE_GLOWING,
	THERES_NO_COVER_HERE,
	N_TRIES_TO_CAMOUFLAGE,
	YOU_ATTEMPT_TO_CAMOUFLAGE,
	YOU_FADE_INTO_EXIST,
	N_FADES_INTO_EXIST,
	YOU_TRAMPLE_AROUND_LOUDLY,
	ONLY_PLAYERS_RECALL,
	RECALL_FOR_BELOW_10,
	N_PRAYS_FOR_TRANSPORTATION,
	YOU_ARE_COMPLETELY_LOST,
	GODS_FORSAKEN_YOU,
	YOU_ARE_STILL_FIGHTING,
	RECALL_FROM_COMBAT,
	N_DISAPPEARS,
	N_APPEARS_IN_THE_ROOM,
	YOU_CANT_DO_THAT_HERE,
	YOU_HAVE_D_TRAINING_SESSIONS,
	YOU_CAN_TRAIN,
	NOT_ENOUGH_TRAININGS,
	YOUR_DURABILITY_INCREASES,
	N_DURABILITY_INCREASES,
	YOUR_POWER_INCREASES,
	N_POWER_INCREASES,
	YOUR_T_IS_MAX,
	YOUR_T_INCREASES,
	N_T_INCREASES,
	THERE_ARE_NO_TRAIN_TRACKS_HERE,
	N_CHECKS_TRACKS,
	TRACKS_LEAD_S,
	DONT_SEE_TRACKS,
	YOU_CANT_BE_MORE_VAMPIRE,
	YOU_SHOW_MORE_UGGLY,
	GO_AND_ASK_QUESTOR,
	GO_KILL_PLAYER,
	WAIT_NIGHT,
	FEEL_GREATER,
	CANNOT_RECOGNIZE,
	DONT_KNOW_BITE,
	MUST_TRANSFORM_VAMP,
	BITE_WHOM,
	THEY_MUST_BE_SLEEPING,
	HOW_CAN_YOU_SNEAK_YOU,
	CANT_BITE_FIGHTING_PERS,
	DOESNT_WORTH_UP,
	DOESNT_WORTH_TO_DO,
	HELP_TRIED_TO_BITE,
	BASH_WHATS_THAT,
	IS_GUSHING_BLOOD,
	POS_NAME_DEAD,
	POS_NAME_MORTALLY_WOUNDED,
	POS_NAME_INCAPACITATED,
	POS_NAME_STUNNED,
	POS_NAME_SLEEPING,
	POS_NAME_RESTING,
	POS_NAME_SITTING,
	POS_NAME_FIGHTING,
	POS_NAME_STANDING,
	LEAVES,
	LEAVES_RIDING_ON,
	LEAVES_T,
	LEAVES_T_RIDING_ON,
	ARRIVED,
	ARRIVED_RIDING,
	QUEST_THANK_YOU_BRAVE,
	SKILLS_HAVE_BECOME_BETTER,
	SKILLS_LEARN_FROM_MISTAKES,
	HERA_STEPS_INTO_RIDING_ON,
	HERA_STEPS_INTO,
	WHEN_YOU_ATTEMPT_YOU_BREAK_WEBS,
	N_BREAKS_THE_WEBS,
	YOU_ATTEMPT_WEBS_HOLD_YOU,
	N_STRUGGLES_VAINLY_AGAINST_WEBS,
	YOU_STEP_OUT_SHADOWS,
	N_STEPS_OUT_OF_SHADOWS,
	YOU_STEP_OUT_COVER,
	N_STEPS_OUT_COVER,
	YOU_FAILED_TO_PASS,
	N_TRIES_TO_PASS_FAILED,
	TOO_PUMPED_TO_PRAY,
	CANT_BASH_DOORS_MOUNTED,
	CANT_BASH_DOORS_RIDDEN,
	BASH_WHICH_DOOR,
	WAIT_FIGHT_FINISH,
	N_TOO_CLOSE_TO_DOOR,
	TRY_TO_OPEN,
	SHIELD_PROTECTS_EXIT,
	YOU_SLAM_TRY_BREAK,
	N_SLAMS_TRY_BREAK,
	TATTOO_GLOWS_BLUE,
	TATTOO_GLOWS_RED,
	N_BASHES_AND_BREAK,
	YOU_SUCCESSED_TO_OPEN_DOOR,
	YOU_FALL_ON_FACE,
	N_FALLS_ON_FACE,	
	HUH,
	CURRENT_BLINK,
	BLINK_ON,
	BLINK_OFF,
	IS_S_A_STATUS,	
	DONT_HAVE_POWER,
	N_THROWS_GLOBE,
	N_IS_GONE,
	N_APPEARS_FROM_NOWHERE,
	YOU_CAN_ALREADY_DETECT_SNEAK,
	YOU_CAN_DETECT_THE_SNEAK,
	CANT_FADE_MOUNTED,
	CANT_FADE_RIDDEN,
	YOU_ATTEMPT_TO_FADE,
	LACK_SKILL_DRAIN_TOUCH,
	LET_IT_BE,
	DONT_WANT_DRAIN_MASTER,
	EVEN_YOU_NOT_SO_STUPID,
	YOU_TOUCH_NS_NECK,
	N_TOUCHES_YOUR_NECK,
	N_TOUCHES_NS_NECK,
	YOU_ARE_ALREADY_FLYING,
	YOU_START_TO_FLY,
	FIND_POTION_OR_WINGS,
	YOU_SLOWLY_TOUCH_GROUND,
	YOU_ARE_ALREADY_ON_GROUND,
	TYPE_WITH_UP_OR_DOWN,
	MOB_PROGS_I_DO_NOT_GET_PAID_ENOUGH,
	YOUR_GAIN_IS,
	YOU_RAISE_A_LEVEL,
	COMM_TELLS_YOU,
	COMM_YOU_TELL,
	COMM_YOUR_MESSAGE_DIDNT_GET_THROUGH,
	QUEST_N_ASKS_FOR_QUEST,
	QUEST_WRONG_CLASS,
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
	QUEST_N_GIVES_SECRET,
	QUEST_N_GIVES_YOU_SECRET,
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
	QUEST_THIS_IS_THE_NTH_TIME,
	QUEST_WONT_GIVE_AGAIN,
	QUEST_COMMANDS,
	QUEST_TYPE_HELP_QUEST,
	QUEST_YOU_MAY_NOW_QUEST_AGAIN,
	QUEST_RUN_OUT_TIME,
	QUEST_BETTER_HURRY,
	QUEST_ITEMS_AVAIL_PURCHASE,
	YOU_COMPLETED_SOMEONES_QUEST,
	SOMEONE_COMPLETED_YOUR_QUEST,
	ALMOST_COMPLETE_QUEST,
	RETURN_TO_QUESTER,
	CONSIDER_WHOM,
	DONT_EVEN_THINK,
	CAN_KILL_NAKED,
	IS_NOT_MATCH_FOR_YOU,
	LOOKS_EASY_KILL,
	PERFECT_MATCH,
	FEEL_LUCKY_PUNK,
	LAUGHS_AT_YOU_MERCILESSLY,
	DEATH_WILL_THANK_YOU,
	GRINS_EVILLY_WITH,
	GREETS_YOU_WARMLY,
	HOPE_YOU_WILL_TURN,
	GRINS_EVILLY_AT,
	GRINS_EVILLY,
	SMILES_HAPPILY,
	LOOKS_AS_YOU,
	LOOKS_VERY_DISINTERESTED,
	SCAN_HERE,
	SCAN_DOOR_CLOSED,
	PUSH_WHOM_WHERE,
	CANT_PUSH_MOUNTED,
	CANT_PUSH_RIDDEN,
	TOO_DAZED_TO_PUSH,
	THATS_POINTLESS,	
	DIR_IS_CLOSED,
	DIR_IS_LOCKED,
	YOU_WEBBED_WANT_WHAT,
	N_TRIES_PUSH_WEBBED,
	PUSH_VICT_WEBBED,
	N_PUSHES_VICT_WEBBED,
	OOPS,
	N_TRIED_PUSH_YOU,
	N_TRIED_PUSH_N,
	KEEP_HANDS_OUT,
	SAVAGELY_DEVOURS_CORPSE,
	PICKS_SOME_TRASH,
	YOU_BITE_N,
	N_BITES_N,
	N_BITES_YOU,
	DO_I_KNOW_YOU,
	DONT_REMEMBER_YOU,
	OK_MY_DEAR,
	CALMING_DOWN,
	COMM_N_SAYS,
	COMM_YOU_SAY,
	QUEST_INFO_LOCATION,
	CANT_PRAY_NOW,
	PRAYS_UPPER_LORD,
	ARENT_FIGHTING,
	ESCAPE_WHAT_DIR,
	CANT_ESCAPE_MOUNTED,
	CANT_ESCAPE_RIDDEN,
	TRY_FLEE,
	SOMETHING_PREVENTS_ESCAPE,
	ESCAPE_FAILED,
	N_ESCAPED,
	YOU_ESCAPED_FROM_COMBAT,
	YOU_FLED_FROM_COMBAT,
	YOU_LOSE_D_EXPS,
	WRONG_DIRECTION,
	COULDNT_ESCAPE,
	CANT_LAY_HANDS,
	CANT_CONCENTRATE_ENOUGH,
	WARM_FEELING,
	ALREADY_HUMAN,
	RETURN_TO_SIZE,
	MOUNT_WHAT,
	DONT_KNOW_RIDE,
	CANT_RIDE_THAT,
	BEAST_TOO_POWERFUL,
	S_BELONGS_TO_S,
	MOUNT_MUST_STAND,
	ALREADY_RIDDEN,
	ALREADY_RIDING,
	FAIL_TO_MOUNT,
	YOU_HOP_ON_N,
	N_HOPS_ON_N,
	N_HOPS_ON_YOU,
	YOU_DISMOUNT_N,
	N_DISMOUNTS_N,
	N_DISMOUNTS_YOU,
	YOU_ARENT_MOUNTED,
	MOUNT_DRUNKEN,
	YOU_FALL_OFF_N,
	N_FALLS_OFF_N,
	N_FALLS_OFF_YOU,
	N_DOESNT_LIKE_YOU,
	N_DOESNT_LIKE_N,
	YOU_DONT_LIKE_N,
	N_SNARLS_YOU,
	N_SNARLS_N,
	YOU_SNARL_N,
	FIGHTING_YOU,
	REBOOT_IN,
	AUTOREBOOT_IN,
	P_STRIKES_YOU,
	YOU_P_STRIKES_N,
	N_P_STRIKES_N,
	TARGET,
	INVIS,
	HIDDEN,
	CHARMED,
	TRANSLUCENT,
	PINK_AURA,
	UNDEAD,
	RIDDEN,
	IMPROVED,
	RED_AURA,
	GOLDEN_AURA,
	WHITE_AURA,
	BLUE_AURA,
	FADE,
	WANTED,
	CAMF,
	IS_DEAD,
	IS_MORTALLY_WOUNDED,
	IS_INCAPACITATED,
	IS_LYING_HERE_STUNNED,
	N_LOOKS_AT_SELF,
	N_LOOKS_AT_YOU,
	N_LOOKS_AT_N,
	SEE_NOTHING_SPECIAL,
	IS_RIDING_S,
	IS_RIDDEN_BY_S,
	DARK,
	MAGICAL,
	GLOWING,
	HUMMING,
	GLOWING_RED_EYES,
	FEEL_MORE_LIVES,	
	CANT_SEE_THING,
	HERE_RIDING,
	THATS_NOT_CONTAINER,
	IT_IS_EMPTY,
	ITS_FILLED_S,
	LESS_THAN_HALF,
	ABOUT_HALF,
	MORE_THAN_HALF,
	IT_IS_CLOSED,
	P_HOLDS,
	CANT_SEE_BUT_STARS,
	CANT_SEE_SLEEPING,
	PITCH_BLACK,
	LOOK_IN_WHAT,
	NOTHING_SPECIAL_IT,
	ONLY_SEE_ONE_S,
	ONLY_SEE_D_THOSE,
	NOTHING_SPECIAL_THERE,
	THE_D_IS_OPEN,
	EXA_WHAT,
	NO_COINS_PILE,
	ONE_GOLD_COIN,
	D_GOLD_COINS,
	ONE_SILVER_COIN,
	D_SILVER_COINS,
	D_SILVER_AND_D_GOLD,
	EXITS,
	OBVIOUS_EXITS_IMM,
	OBVIOUS_EXITS,
	TOO_DARK_TO_TELL,
	ROOM_D,
	NONE,
	CAPNONE,
	HAVE_D_GOLD_D_SILVER,
	AND_D_EXP,
	HAVE_KILLED,
	NON_GOODS,
	NON_EVILS,
	NON_NEUTRALS,
	GOODS,
	EVILS,
	NEUTRALS,
	NO_HELP_ON_WORD,
	PLAYERS_FOUND,
	MUST_PROVIDE_NAME,
	NO_ONE_THAT_NAME,
	D_CHARS_ON,
	MOST_SO_FAR_TODAY,
	MOST_TODAY_WAS,
	SOMETHING,
	COMPARE_WHAT,
	DONT_HAVE_ITEM,
	ARENT_WEAR_COMPARABLE,
	COMPARE_P_TO_ITSELF,
	CANT_COMPARE_P_P,
	P_P_LOOKS_SAME,
	P_LOOKS_BETTER_P,
	P_LOOKS_WORSE_P,
	TOO_DARK_SEE,
	PLAYERS_NEAR_YOU,
	DIDNT_FIND_ANY,
	CANT_CHANGE_TITLE,
	CHANGE_TITLE_TO_WHAT,
	ILLEGAL_CHARACTER_TITLE,
	NO_LINES_TO_REMOVE,
	YOUR_DESC_IS,
	DESC_CLEARED,
	DESC_TOO_LONG,
	BR_NONE_BR,
	REPORT_I_HAVE,
	REPORT_I_HAVE_TO_CH,
	YOU_SCAN_S,
	N_SCANS_S,
	FROZEN,
	ASK_MASTER,
	TOO_STUNNED,
	YOU_ARE_DEAD,
	HURT_FAR_TOO_BAD,
	YOU_TOO_STUNNED,
	I_YOUR_DREAMS,
	TOO_RELAXED,
	BETTER_STANDUP,
	NO_WAY_FIGHT,
	NOT_AFFECTED_SPELLS,
	YOU_ARE_AFFECTED,
	AFF_SPELL,
	AFF_MODIFIES,
	AFF_PERMANENTLY,
	AFF_FOR_D_HOURS,
	AFF_BY,
	ANTI_SOCIAL,
	YOU_CANT_TAKE_THAT,
	YOU_ZAPPED_BY_P,
	N_ZAPPED_BY_P,
	CANT_CARRY_ITEMS,
	CANT_CARRY_WEIGHT,
	N_APPEARS_USING,
	YOU_GET_P_FROM_P,
	N_GETS_P_FROM_P,
	YOU_GET_P,
	N_GETS_P,
	GET_WHAT,
	I_SEE_NOTHING_HERE,
	DONT_SEE_ANYTHING_LIKE_IN_T,
	DONT_BE_SO_GREEDY,
	I_SEE_NOTHING_IN_T,
	PUT_WHAT_IN_WHAT,
	CANT_FOLD_INTO_SELF,
	CANT_LET_GO_OF_IT,
	THAT_WOULD_BE_BAD_IDEA,
	IT_WONT_FIT,
	N_PUTS_P_ON_P,
	YOU_PUT_P_ON_P,
	N_PUTS_P_IN_P,
	YOU_PUT_P_IN_P,
	DROP_WHAT,
	DONT_HAVE_MUCH_SILVER,
	DONT_HAVE_MUCH_GOLD,
	N_DROPS_SOME_COINS,
	N_DROPS_P,
	YOU_DROP_P,
	YOU_BEGIN_TO_SNEAK_AGAIN,
	YOU_STEP_BACK_SHADOWS,
	N_WANDERS_ON_HOME,
	N_GOES_OUT,
	N_FLICKERS_OUT,
	P_FLICKERS,
	N_DISAPPEARS_VOID,
	YOU_DISAPPEAR_VOID,
	WITCH_CURSE_MAKES_N,
	WITCH_CURSE_MAKES_YOU,
	N_WRITHES_PLAGUE,
	YOU_WRITHE_PLAGUE,
	YOU_FEEL_HOT_FEVERISH,
	N_SHIVERS_ILL,
	N_SHIVERS_SUFFERS,
	YOU_SHIVER_SUFFER,
	WEAR_WHAT,
	MUST_BE_LEVEL_TO_USE,
	N_TRIES_TO_USE,
	N_LIGHTS_P_HOLD,
	YOU_LIGHT_P_HOLD,
	N_WEARS_P_LEFT_FINGER,
	YOU_WEAR_P_LEFT_FINGER,
	N_WEARS_P_RIGHT_FINGER,
	YOU_WEAR_P_RIGHT_FINGER,
	ALREADY_WEAR_TWO_RINGS,
	N_WEARS_P_NECK,
	YOU_WEAR_P_NECK,
	ALREADY_WEAR_TWO_NECK,
	N_WEARS_P_TORSO,
	YOU_WEAR_P_TORSO,
	N_WEARS_P_HEAD,
	YOU_WEAR_P_HEAD,
	N_WEARS_P_LEGS,
	YOU_WEAR_P_LEGS,
	N_WEARS_P_FEET,
	YOU_WEAR_P_FEET,
	N_WEARS_P_HANDS,
	YOU_WEAR_P_HANDS,
	N_WEARS_P_ARMS,
	YOU_WEAR_P_ARMS,
	N_WEARS_P_WAIST,
	YOU_WEAR_P_WAIST,
	N_WEARS_P_LEFT_WRIST,
	YOU_WEAR_P_LEFT_WRIST,
	N_WEARS_P_RIGHT_WRIST,
	YOU_WEAR_P_RIGHT_WRIST,
	ALREADY_WEAR_TWO_WRIST,
	CANT_USE_SHIELD_SECOND_WEAPON,
	YOUR_HANDS_TIRED_WEAPON,
	N_WEARS_P_SHIELD,
	YOU_WEAR_P_SHIELD,
	TOO_HEAVY_WIELD,
	NEED_TWO_HANDS,
	N_WIELDS_P,
	YOU_WIELD_P,
	P_LIKE_PART_OF_YOU,
	QUITE_CONFIDENT_P,
	SKILLED_WITH_P,
	SKILL_P_ADEQUATE,
	P_FEELS_CLUMSY,
	ALMOST_DROP_P,
	DONT_KNOW_THE_END,
	CANT_HOLD_WHILE_2_WEAPONS,
	N_HOLDS_P_HAND,
	YOU_HOLD_P_HAND,
	N_FLOAT_P,
	YOU_FLOAT_P,
	N_USES_TATTOO,
	YOU_USE_TATTOO,
	CANT_WEAR_IT,
	REMOVE_WHAT,
	N_SACS_SELF,
	YOU_SAC_SELF,
	CANT_FIND_IT,
	GODS_WOUDLNT_LIKE_THAT,
	P_NOT_ACCEPTABLE_SAC,
	SAC_GET_ONE_SILVER,
	SAC_GET_D_SILVER,
	N_SACS_P,
	YOUR_SAC_REVEALS_P,
	NS_SAC_REVEALS_P,
	YOUR_SAC_REVEALS_P_P,
	NS_SAC_REVEALS_P_P,
	AS_YOU_SAC,
	AS_N_SACS,
	FEW_THINGS,
	BUNCH_OF_OBJECTS,
	MANY_THINGS,
	LOT_OF_OBJECTS,
	ON_IT,
	SCATTER_ON_DIRT,
	SCATTER_OVER_WATER,
	SCATTER_AROUND,
	QUAFF_WHAT,
	DONT_HAVE_POTION,
	CAN_QUAFF_ONLY_POTIONS,
	TOO_POWERFUL_LIQUID,
	N_QUAFFS_P,
	YOU_QUAFF_P,
	DONT_HAVE_SCROLL,
	CAN_RECITE_ONLY_SCROLLS,
	SCROLL_TOO_COMPLEX,
	N_RECITES_P,
	YOU_RECITE_P,
	MISPRONOUNCE_SYLLABLE,
};

#endif
