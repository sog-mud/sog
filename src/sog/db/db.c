/*
 * $Id: db.c,v 1.46 1998-07-25 15:02:38 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <limits.h>
#include <dirent.h>

#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "lookup.h"
#include "act_wiz.h"
#include "comm.h"
#include "magic.h"
#include "act_comm.h"
#include "rating.h"
#include "update.h"
#include "log.h"
#include "tables.h"
#include "buffer.h"
#include "mlstring.h"

#ifdef SUNOS
#include "compat.h"
#define d_namlen d_reclen
#endif

#ifdef SVR4
#define d_namlen d_reclen
#endif

void load_limited_objects();

extern	int	_filbuf		(FILE *);

#if !defined(OLD_RAND)
#ifdef BSD44
#	include <stdlib.h>
#	include <unistd.h>
#	include <time.h>
#else
	long random();
	void srandom(unsigned int);
	int getpid();
	time_t time(time_t *tloc);
#endif
#endif


/* externals for counting purposes */
extern	OBJ_DATA	*obj_free;
extern	CHAR_DATA	*char_free;
extern  DESCRIPTOR_DATA *descriptor_free;
extern	PC_DATA		*pcdata_free;
extern  AFFECT_DATA	*affect_free;

/*
 * Globals.
 */
HELP_DATA *		help_first;
HELP_DATA *		help_last;

HELP_AREA *		had_list;

SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;

NOTE_DATA *		note_free;

MPROG_CODE *		mprog_list;

CHAR_DATA *		char_list;
char *			help_greeting;
KILL_DATA		kill_table	[MAX_LEVEL];
NOTE_DATA *		note_list;
OBJ_DATA *		object_list;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

ROOM_INDEX_DATA	*	top_affected_room;
int			reboot_counter;

/* for limited objects */
int total_levels; 

/* gsns */
int  gsn_backstab;
int  gsn_dodge;
int  gsn_envenom;
int  gsn_hide;
int  gsn_peek;
int  gsn_pick_lock;
int  gsn_sneak;
int  gsn_steal;
int  gsn_disarm;
int  gsn_enhanced_damage;
int  gsn_kick;
int  gsn_parry;
int  gsn_rescue;
int  gsn_second_attack;
int  gsn_third_attack;
int  gsn_blindness;
int  gsn_charm_person;
int  gsn_curse;
int  gsn_remove_curse;
int  gsn_invis;
int  gsn_mass_invis;
int  gsn_poison;
int  gsn_plague;
int  gsn_sleep;
int  gsn_sanctuary;
int  gsn_fly;
int  gsn_death_protection;

int  gsn_fourth_attack;
int  gsn_dual_backstab;
int  gsn_cleave;
int  gsn_counter;
int  gsn_ambush;
int  gsn_camouflage;
int  gsn_circle;
int  gsn_nerve;
int  gsn_endure;
int  gsn_quiet_movement;
int  gsn_herbs;
int  gsn_tame;
int  gsn_butcher;
int  gsn_assassinate;
int  gsn_caltrops;
int  gsn_throw;
int  gsn_strangle;
int  gsn_blackjack;
int  gsn_bloodthirst;
int  gsn_spellbane;
int  gsn_resistance;
int  gsn_deathblow;
int  gsn_fade;
int  gsn_garble;
int  gsn_confuse;
int  gsn_track;
int  gsn_vanish;
int  gsn_chaos_blade;
int  gsn_terangreal;
int  gsn_dispel_evil;
int  gsn_dispel_good;
int  gsn_wrath;
int  gsn_stalker;
int  gsn_tesseract;
int  gsn_randomizer;
int  gsn_trophy;
int  gsn_truesight;
int  gsn_brew;
int  gsn_flamestrike;
int  gsn_shadowlife;
int  gsn_ruler_badge;
int  gsn_remove_badge;
int  gsn_dragon_strength;
int  gsn_dragon_breath;
int  gsn_warcry;
int  gsn_entangle;
int  gsn_dragonsword;
int  gsn_knight;
int  gsn_guard;
int  gsn_guard_call;
int  gsn_love_potion;
int  gsn_deafen;
int  gsn_protective_shield;
int  gsn_protection_heat_cold;
int  gsn_trance;
int  gsn_demon_summon;
int  gsn_nightwalker;
int  gsn_bear_call;
int  gsn_squire;
int  gsn_lightning_bolt;
int  gsn_disperse;
int  gsn_bless;
int  gsn_weaken;
int  gsn_haste;
int  gsn_cure_critical;
int  gsn_cure_serious;
int  gsn_burning_hands;
int  gsn_acid_blast;
int  gsn_ray_of_truth;
int  gsn_spell_craft;
int  gsn_giant_strength;
int  gsn_explode;
int  gsn_acid_breath;
int  gsn_fire_breath;
int  gsn_frost_breath;
int  gsn_gas_breath;
int  gsn_lightning_breath;
int  gsn_cure_light;
int  gsn_magic_missile;
int  gsn_demonfire;
int  gsn_faerie_fire;
int  gsn_shield;
int  gsn_chill_touch;
int  gsn_second_weapon;
int  gsn_target;
int  gsn_sand_storm;
int  gsn_scream;
int  gsn_enchant_sword;
int  gsn_tiger_power;
int  gsn_lion_call;
int  gsn_hara_kiri;
int  gsn_mouse;
int  gsn_enhanced_armor;
int  gsn_vampire;
int  gsn_vampiric_bite;
int  gsn_light_res;
int  gsn_blink;
int  gsn_hunt;
int  gsn_path_find;
int  gsn_critical;
int  gsn_detect_sneak;
int  gsn_mend;
int  gsn_shielding;
int  gsn_blind_fighting;
int  gsn_riding;
int  gsn_smithing;
int  gsn_swimming;
int  gsn_move_camf;
int  gsn_protection_heat;
int  gsn_protection_cold;
int  gsn_teleport;
int  gsn_witch_curse;
int  gsn_terangreal;
int  gsn_kassandra;
int  gsn_sebat;
int  gsn_matandra;
int  gsn_armor_use;
int  gsn_world_find;
int  gsn_vampiric_touch;
int  gsn_cure_poison;
int  gsn_fire_shield;
int  gsn_fear;
int  gsn_settraps;
int  gsn_mental_attack;
int  gsn_secondary_attack;
int  gsn_mortal_strike;
int  gsn_shield_cleave;
int  gsn_weapon_cleave;
int  gsn_slow;
int  gsn_imp_invis;
int  gsn_camp;
int  gsn_push;
int  gsn_tail;
int  gsn_power_stun;
int  gsn_clan_recall;
int  gsn_escape;
int  gsn_lay_hands;
int  gsn_grip;
int  gsn_concentrate;
int  gsn_master_sword;
int  gsn_master_hand;
int  gsn_fifth_attack;
int  gsn_area_attack;
int  gsn_reserved;
int  gsn_bandage;
int  gsn_web;
int  gsn_bow;
int  gsn_bash_door;
int  gsn_katana;
int  gsn_bluefire;
int  gsn_crush;
int  gsn_perception;
int  gsn_control_animal;
int  gsn_sanctify_lands;
int  gsn_deadly_venom;
int  gsn_cursed_lands;
int  gsn_lethargic_mist;
int  gsn_black_death;
int  gsn_mysterious_dream;
int  gsn_sense_life;
int  gsn_arrow;
int  gsn_lance;
int  gsn_evil_spirit;
int  gsn_make_arrow;
int  gsn_green_arrow;
int  gsn_red_arrow;
int  gsn_white_arrow;
int  gsn_blue_arrow;
int  gsn_make_bow;
int  gsn_blindness_dust;
int  gsn_poison_smoke;
int  gsn_mastering_spell;

/* new_gsns */

int  gsn_axe;
int  gsn_dagger;
int  gsn_flail;
int  gsn_mace;
int  gsn_polearm;
int  gsn_shield_block;
int  gsn_spear;
int  gsn_sword;
int  gsn_whip;
 
int  gsn_bash;
int  gsn_berserk;
int  gsn_dirt;
int  gsn_hand_to_hand;
int  gsn_trip;
 
int  gsn_fast_healing;
int  gsn_haggle;
int  gsn_lore;
int  gsn_meditation;
 
int  gsn_scrolls;
int  gsn_staves;
int  gsn_wands;
int  gsn_recall;
int  gsn_detect_hidden;
int  gsn_frenzy;


/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];
char *			string_hash		[MAX_KEY_HASH];

AREA_DATA *		area_first;
AREA_DATA *		area_last;
AREA_DATA *		current_area;
int			line_number;

char *			string_space;
char *			top_string;
char			str_empty	[1];

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int                     top_vnum_room;		/* OLC */
int                     top_vnum_mob;		/* OLC */
int                     top_vnum_obj;		/* OLC */
int			top_mprog_index;	/* OLC */
int 			mobile_count = 0;
int			newmobs = 0;
int			newobjs = 0;


/*
 * Memory management. 
 * Increase MAX_STRING if you have too.	
 * Tune the others only if you understand what you're doing.
 */
#define			MAX_STRING	3500000
#define			MAX_PERM_BLOCK	150000
#define			MAX_MEM_LIST	11

void *			rgFreeList	[MAX_MEM_LIST];
const int		rgSizeList	[MAX_MEM_LIST]	=
{
	16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64
};

int			nAllocString;
int			sAllocString;
int			nAllocPerm;
int			sAllocPerm;



/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];

/*
 * Local booting procedures.
*/
void    init_mm         (void);
void	load_area	(FILE *fp);
void    load_areadata   (FILE *fp);			   /* OLC */
void	load_helps	(FILE *fp, char *fname);
void    load_omprogs    (FILE *fp);
void	load_old_mob	(FILE *fp);
void 	load_mobiles	(FILE *fp);
void	load_old_obj	(FILE *fp);
void 	load_objects	(FILE *fp);
void	load_resets	(FILE *fp);
void	load_rooms	(FILE *fp);
void	load_shops	(FILE *fp);
void 	load_socials	(FILE *fp);
void	load_specials	(FILE *fp);
void	load_notes	(void);
void	load_bans	(void);
void    load_olimits    (FILE *fp);
void    load_practicer  (FILE *fp);
void    load_resetmsg   (FILE *fp);
void    load_aflag	(FILE *fp);
void	load_mobprogs	(FILE *fp);
 
void	fix_exits	(void);
void    fix_mobprogs	(void);

void	reset_area	(AREA_DATA * pArea);

int xgetc(FILE *fp);
void xungetc(int c, FILE *fp);

/*
 * Big mama top level function.
 */
void boot_db(void)
{
	long lhour, lday, lmonth;
	int sn;
	FILE *fpList;

	/*
	 * Init some data space stuff.
	 */
	if ((string_space = calloc(1, MAX_STRING)) == NULL) {
		log_printf("boot_db: can't alloc %d string space.", MAX_STRING);
		exit(1);
	}
	top_string	= string_space;
	fBootDb		= TRUE;

	/*
	 * Init random number generator.
	 */
	init_mm();

	/*
	 * Set time and weather.
	 */

	lhour	= (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SCD);
	time_info.hour	= lhour  % 24;
	lday		= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth		= lday   / 35;
	time_info.month	= lmonth % 17;
	time_info.year	= lmonth / 17;

	     if (time_info.hour <  5) weather_info.sunlight = SUN_DARK;
	else if (time_info.hour <  6) weather_info.sunlight = SUN_RISE;
	else if (time_info.hour < 19) weather_info.sunlight = SUN_LIGHT;
	else if (time_info.hour < 20) weather_info.sunlight = SUN_SET;
	else                          weather_info.sunlight = SUN_DARK;

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if (time_info.month >= 7 && time_info.month <=12)
		weather_info.mmhg += number_range(1, 50);
	else
		weather_info.mmhg += number_range(1, 80);

	     if (weather_info.mmhg <=  980) weather_info.sky = SKY_LIGHTNING;
	else if (weather_info.mmhg <= 1000) weather_info.sky = SKY_RAINING;
	else if (weather_info.mmhg <= 1020) weather_info.sky = SKY_CLOUDY;
	else                                weather_info.sky = SKY_CLOUDLESS;

	/* room_affect_data */
	top_affected_room = NULL;
		
	/* reboot counter */
	reboot_counter = 1440;	/* 12 hours */

	/*
	 * Assign gsn's for skills which have them.
	 */
	for (sn = 0; sn < MAX_SKILL; sn++)
		if (skill_table[sn].pgsn != NULL)
			*skill_table[sn].pgsn = sn;

	/*
	 * Read in all the area files.
	 */
	if ((fpList = fopen(AREA_LIST, "r")) == NULL) {
		perror(AREA_LIST);
		exit(1);
	}

	for (; ;) {
		strcpy(strArea, fread_word(fpList));
		if (strArea[0] == '$')
			break;

		if (strArea[0] == '-')
			fpArea = stdin;
		else {
			if ((fpArea = fopen(strArea, "r")) == NULL) {
				perror(strArea);
				exit(1);
			}
		}

		current_area = NULL;
		line_number = 1;

		for (; ;) {
			char *word;

			if (fread_letter(fpArea) != '#') {
				log("boot_db: # not found.");
				exit(1);
			}

			word = fread_word(fpArea);

			     if (word[0] == '$'           )                 break;
			else if (!str_cmp(word, "AREA"   )) load_area    (fpArea);
  /* OLC */		else if (!str_cmp(word, "AREADATA")) load_areadata(fpArea);
			else if (!str_cmp(word, "HELPS"  )) load_helps   (fpArea, strArea);
			else if (!str_cmp(word, "MOBOLD" )) load_old_mob (fpArea);
			else if (!str_cmp(word, "MOBILES")) load_mobiles (fpArea);
			else if (!str_cmp(word, "MOBPROGS")) load_mobprogs(fpArea);
			else if (!str_cmp(word, "OBJOLD" )) load_old_obj (fpArea);
		  	else if (!str_cmp(word, "OBJECTS")) load_objects (fpArea);
			else if (!str_cmp(word, "RESETS" )) load_resets  (fpArea);
			else if (!str_cmp(word, "ROOMS"  )) load_rooms   (fpArea);
			else if (!str_cmp(word, "SHOPS"  )) load_shops   (fpArea);
			else if (!str_cmp(word, "SOCIALS")) load_socials (fpArea);
			else if (!str_cmp(word, "OMPROGS")) load_omprogs (fpArea);
		        else if (!str_cmp(word, "OLIMITS")) load_olimits (fpArea);
			else if (!str_cmp(word, "SPECIALS")) load_specials(fpArea);
			else if (!str_cmp(word, "PRACTICERS")) load_practicer(fpArea);
			else if (!str_cmp(word, "RESETMESSAGE")) load_resetmsg(fpArea);
			else if (!str_cmp(word, "FLAG"))	 load_aflag(fpArea);
			else {
			    log("boot_db: bad section name.");
			    exit(1);
			}
		}

		if (fpArea != stdin)
			fclose(fpArea);
		fpArea = NULL;

		if (current_area != NULL) {
			REMOVE_BIT(current_area->area_flags, AREA_LOADING);
			current_area = NULL;
		}
	}
	fclose(fpList);

	/*
	 * Fix up exits.
	 * Declare db booting over.
	 * Reset all areas once.
	 * Load up the songs, notes and ban files.
	 */
	fix_exits();
	fix_mobprogs();
	load_limited_objects();
	log_printf("Total non-immortal levels > 5: %d", total_levels);

	fBootDb	= FALSE;
	convert_objects();           /* ROM OLC */
	area_update();
	load_notes();
	load_bans();
}



/*
 * Snarf an 'area' header line.
 */
void load_area(FILE *fp)
{
	AREA_DATA *pArea;

	pArea = alloc_perm(sizeof(*pArea));
	pArea->reset_first	= NULL;
	pArea->reset_last	= NULL;
	pArea->file_name	= fread_string(fp);

	pArea->area_flags	= AREA_LOADING;
	pArea->security		= 9;
	pArea->vnum		= top_area;
	pArea->builders		= str_dup("None");

	pArea->name		= fread_string(fp);
	fread_letter(fp);			/* '{' */
	pArea->low_range	= fread_number(fp);
	pArea->high_range	= fread_number(fp);
	fread_letter(fp);			/* '}' */
	pArea->credits		= str_dup(fread_word(fp));	
	free_string(fread_string(fp));
	pArea->min_vnum		= fread_number(fp);
	pArea->max_vnum		= fread_number(fp);
	pArea->age		= 15;
	pArea->nplayer		= 0;
	pArea->empty		= FALSE;
	pArea->count		= 0;
	pArea->resetmsg		= mlstr_new();
	pArea->area_flag	= 0;

	if (area_first == NULL)
		area_first = pArea;
	if (area_last != NULL) 
		area_last->next = pArea;

	area_last	= pArea;
	pArea->next	= NULL;
	current_area	= pArea;

	top_area++;
}


/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the load_areadata format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY(literal, field, value)			\
		if (!str_cmp(word, literal)) {		\
			field  = value;			\
			fMatch = TRUE;			\
			break;				\
		}

#define SKEY(string, field)				\
		if (!str_cmp(word, string)) {		\
			free_string(field);		\
			field = fread_string(fp);	\
			fMatch = TRUE;			\
			break;				\
		}

#define MLSKEY(string, field)				\
		if (!str_cmp(word, string)) {		\
			mlstr_fread(fp, field);		\
			fMatch = TRUE;			\
			break;				\
		}



/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
void load_areadata(FILE *fp)
{
	AREA_DATA *pArea;
	char      *word;
	bool      fMatch;

	pArea			= alloc_perm(sizeof(*pArea));
	pArea->age		= 15;
	pArea->nplayer		= 0;
	pArea->file_name	= str_dup(strArea);
	pArea->vnum		= top_area;
	pArea->name		= str_dup("New Area");
	pArea->builders		= str_dup("");
	pArea->security		= 9;                    /* 9 -- Hugin */
	pArea->min_vnum		= 0;
	pArea->max_vnum		= 0;
	pArea->area_flags	= 0;
	pArea->low_range	= 0;
	pArea->high_range	= 0;          
	pArea->area_flag	= 0;
	pArea->resetmsg		= mlstr_new();
/*  pArea->recall       = ROOM_VNUM_TEMPLE;        ROM OLC */
 
	for (; ;) {
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'B':
			SKEY("Builders", pArea->builders);
			break;
		case 'C':
			SKEY("Credits", pArea->credits);
			break;
		case 'E':
			if (!str_cmp(word, "End")) {
				fMatch = TRUE;
				if (area_first == NULL)
			        	area_first = pArea;
				if (area_last  != NULL)
			        	area_last->next = pArea;
				area_last	= pArea;
				pArea->next	= NULL;
				current_area	= pArea;
				top_area++;
				return;
			}
			break;
		case 'F':
			KEY("Flags", pArea->area_flag, fread_flags(fp));
			break;
		case 'L':
			if (!str_cmp(word, "LevelRange")) {
				pArea->low_range = fread_number(fp);
				pArea->high_range = fread_number(fp);
			}
			break;
		case 'N':
			SKEY("Name", pArea->name);
			break;
		case 'R':
			MLSKEY("ResetMessage", pArea->resetmsg);
			break;
		case 'S':
			KEY("Security", pArea->security, fread_number(fp));
			break;
		case 'V':
			if (!str_cmp(word, "VNUMs")) {
				pArea->min_vnum = fread_number(fp);
				pArea->max_vnum = fread_number(fp);
			}
			break;
		}
	}
}


/*
 * Sets vnum range for area using OLC protection features.
 */
void vnum_check(int vnum)
{
	if (area_last->min_vnum == 0 || area_last->max_vnum == 0) {
		log_printf("%s: min_vnum or max_vnum not assigned",
			   area_last->file_name);
#if 0
		area_last->min_vnum = area_last->max_vnum = vnum;
#endif
	}

	if (vnum != URANGE(area_last->min_vnum, vnum, area_last->max_vnum)) {
		log_printf("%s: %d not in area vnum range",
			   area_last->file_name, vnum);
#if 0
		if (vnum < area_last->min_vnum)
			area_last->min_vnum = vnum;
		else
			area_last->max_vnum = vnum;
#endif
	}
}
 

/*
 * Snarf a help section.
 */
void load_helps(FILE *fp, char *fname)
{
	HELP_DATA *pHelp;
	int level;
	char *keyword;

	for (; ;) {
		HELP_AREA * had;
	
		level		= fread_number(fp);
		keyword		= fread_string(fp);
	
		if (keyword[0] == '$')
			break;
	
		if (!had_list) {
			had			= new_had();
			had->filename		= str_dup(fname);
			had->area		= current_area;
			if (current_area)
				current_area->helps	= had;
			had_list		= had;
		}
		else
		if (str_cmp(fname, had_list->filename)) {
			had			= new_had();
			had->filename		= str_dup(fname);
			had->area		= current_area;
			if (current_area)
				current_area->helps	= had;
			had->next		= had_list;
			had_list		= had;
		}
		else
			had			= had_list;
	
		pHelp		= new_help();
		pHelp->level	= level;
		pHelp->keyword	= keyword;
		mlstr_fread(fp, pHelp->text);

		if (!str_cmp(pHelp->keyword, "greeting"))
			help_greeting = mlstr_mval(pHelp->text);

		if (help_first == NULL)
			help_first = pHelp;
		if (help_last  != NULL)
			help_last->next = pHelp;

		help_last		= pHelp;
		pHelp->next		= NULL;
	
		if (had->first == NULL)
			had->first	= pHelp;
		if (had->last == NULL)
			had->last	= pHelp;
	
		had->last->next_area	= pHelp;
		had->last		= pHelp;
		pHelp->next_area	= NULL;
	}
}


/*
 * Snarf a mob section.  old style 
 */
void load_old_mob(FILE *fp)
{
	MOB_INDEX_DATA *pMobIndex;
	/* for race updating */
	int race;
	char name[MAX_STRING_LENGTH];

	if (!area_last) {  /* OLC */
		log("load_mobiles: no #AREA seen yet.");
		exit(1);
	}

	for (; ;) {
		int vnum;
		char letter;
		int iHash;

		letter	= fread_letter(fp);
		if (letter != '#') {
			log("load_mobiles: # not found.");
			exit(1);
		}

		vnum	= fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if (get_mob_index(vnum) != NULL) {
			log_printf("load_mobiles: vnum %d duplicated.", vnum);
			exit(1);
		}
		fBootDb = TRUE;

		pMobIndex		  = alloc_perm(sizeof(*pMobIndex));
		pMobIndex->short_descr	  = mlstr_new();
		pMobIndex->long_descr	  = mlstr_new();
		pMobIndex->description	  = mlstr_new();

		pMobIndex->vnum		  = vnum;
		pMobIndex->area		  = area_last;
		pMobIndex->new_format	  = FALSE;
		pMobIndex->player_name	  = fread_string(fp);
		mlstr_fread(fp, pMobIndex->short_descr);
		mlstr_fread(fp, pMobIndex->long_descr);
		mlstr_fread(fp, pMobIndex->description);

		pMobIndex->act		  = fread_flags(fp) | ACT_NPC;
		pMobIndex->affected_by	  = fread_flags(fp);
		pMobIndex->practicer	  = 0;

		/* chronos corrected detection of ROM */
		if (IS_AFFECTED(pMobIndex,C))	/* detect evil */
			 SET_BIT(pMobIndex->detection,C);
		if (IS_AFFECTED(pMobIndex,D))	/* detect invis */
			 SET_BIT(pMobIndex->detection,D);
		if (IS_AFFECTED(pMobIndex,E))	/* detect magic */
			 SET_BIT(pMobIndex->detection,E);
		if (IS_AFFECTED(pMobIndex,F))	/* detect hidden */
			 SET_BIT(pMobIndex->detection,F);
		if (IS_AFFECTED(pMobIndex,G))	/* detect good */
			 SET_BIT(pMobIndex->detection,G);
		if (IS_AFFECTED(pMobIndex,Z))	/* dark vision */
			 SET_BIT(pMobIndex->detection,Z);
		if (IS_AFFECTED(pMobIndex,ee))	/* acute vision */
			 SET_BIT(pMobIndex->detection,ee);
		REMOVE_BIT(pMobIndex->affected_by,(C|D|E|F|G|Z|ee));

		pMobIndex->pShop	= NULL;
		pMobIndex->alignment	= fread_number(fp);
		letter			= fread_letter(fp);
		pMobIndex->level	= fread_number(fp);
		/*
		 * The unused stuff is for imps who want to use the old-style
		 * stats-in-files method.
		 */
				  fread_number(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		/* 'd'	*/	  fread_letter(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		/* '+'	*/	  fread_letter(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		/* 'd'	*/	  fread_letter(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		/* '+'	*/	  fread_letter(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		pMobIndex->wealth       = fread_number(fp)/20;	
		/* xp can't be used! */	  fread_number(fp);	/* Unused */
		pMobIndex->start_pos	= fread_number(fp);	/* Unused */
		pMobIndex->default_pos	= fread_number(fp);	/* Unused */

		if (pMobIndex->start_pos < POS_SLEEPING)
			pMobIndex->start_pos = POS_STANDING;
		if (pMobIndex->default_pos < POS_SLEEPING)
			pMobIndex->default_pos = POS_STANDING;

		/*
		 * Back to meaningful values.
		 */
		pMobIndex->sex			= fread_number(fp);

		/* compute the race BS */
		one_argument(pMobIndex->player_name,name);
 
		if (name[0] == '\0' || (race =  race_lookup(name)) == 0) {
			/* fill in with blanks */
			pMobIndex->race = race_lookup("human");
			pMobIndex->detection = race_table[pMobIndex->race].det;
			pMobIndex->affected_by = pMobIndex->affected_by |
				race_table[pMobIndex->race].aff;
			pMobIndex->off_flags = OFF_DODGE | OFF_DISARM |
				OFF_TRIP | ASSIST_VNUM;
			pMobIndex->imm_flags = 0;
			pMobIndex->res_flags = 0;
			pMobIndex->vuln_flags = 0;
			pMobIndex->form = FORM_EDIBLE | FORM_SENTIENT |
				FORM_BIPED | FORM_MAMMAL;
			pMobIndex->parts = PART_HEAD | PART_ARMS | PART_LEGS |
				PART_HEART | PART_BRAINS | PART_GUTS;
		}
		else {
			pMobIndex->race = race;
			pMobIndex->detection = race_table[race].det;
			pMobIndex->affected_by = 
				pMobIndex->affected_by | race_table[race].aff;
			pMobIndex->off_flags = OFF_DODGE | OFF_DISARM |
				OFF_TRIP | ASSIST_RACE | race_table[race].off;
			pMobIndex->imm_flags = race_table[race].imm;
			pMobIndex->res_flags = race_table[race].res;
			pMobIndex->vuln_flags = race_table[race].vuln;
			pMobIndex->form = race_table[race].form;
			pMobIndex->parts = race_table[race].parts;
		}

		if (letter != 'S') {
			log_printf("load_mobiles: vnum %d non-S.", vnum);
			exit(1);
		}

		convert_mobile(pMobIndex);		/* ROM OLC */

		iHash			= vnum % MAX_KEY_HASH;
		pMobIndex->next		= mob_index_hash[iHash];
		mob_index_hash[iHash]	= pMobIndex;
		top_mob_index++;
							/* OLC */
		top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;
		vnum_check(vnum);			/* OLC */
		kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
	}
}


/*
 * Snarf an obj section.  old style 
 */
void load_old_obj(FILE *fp)
{
	OBJ_INDEX_DATA *pObjIndex;

	if (!area_last)
		db_error("load_objects", "no #AREA seen yet.");

	for (; ;) {
		int i;
		int vnum;
		char letter;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#')
			db_error("load_objects", "# not found.");

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if (get_obj_index(vnum) != NULL)
			db_error("load_objects", "vnum %d duplicated.", vnum);
		fBootDb = TRUE;

		pObjIndex		= alloc_perm(sizeof(*pObjIndex));
		pObjIndex->short_descr	= mlstr_new();
		pObjIndex->description	= mlstr_new();
		pObjIndex->vnum		= vnum;
		pObjIndex->area         = area_last;	/* OLC */
		pObjIndex->new_format	= FALSE;
		pObjIndex->reset_num	= 0;

		pObjIndex->name		= fread_string(fp);
		mlstr_fread(fp, pObjIndex->short_descr);
		mlstr_fread(fp, pObjIndex->description);
		/* Action description */  fread_string(fp);
		pObjIndex->material	= str_dup("copper");

		pObjIndex->item_type	= fread_number(fp);
		pObjIndex->extra_flags	= fread_flags(fp);
		pObjIndex->wear_flags	= fread_flags(fp);
		pObjIndex->value[0]	= fread_number(fp);
		pObjIndex->value[1]	= fread_number(fp);
		pObjIndex->value[2]	= fread_number(fp);
		pObjIndex->value[3]	= fread_number(fp);
		pObjIndex->value[4]	= 0;
		pObjIndex->level	= 0;
		pObjIndex->condition 	= 100;
		pObjIndex->weight	= fread_number(fp);
		pObjIndex->cost		= fread_number(fp);	/* Unused */
		/* Cost per day */	  fread_number(fp);
		pObjIndex->limit	= -1;
		for (i = 0; i < OPROG_MAX; i++)
			pObjIndex->oprogs[i] = NULL;

		if (pObjIndex->item_type == ITEM_WEAPON)
			if (is_name("two",pObjIndex->name) 
			||  is_name("two-handed",pObjIndex->name) 
			||  is_name("claymore",pObjIndex->name))
				SET_BIT(pObjIndex->value[4], WEAPON_TWO_HANDS);

		for (; ;) {
			char letter;

			letter = fread_letter(fp);

			if (letter == 'A') {
				AFFECT_DATA *paf;

				paf		= alloc_perm(sizeof(*paf));
				paf->where	= TO_OBJECT;
				paf->type	= -1;
				paf->level	= 20; /* RT temp fix */
				paf->duration	= -1;
				paf->location	= fread_number(fp);
				paf->modifier	= fread_number(fp);
				paf->bitvector	= 0;
				SLIST_ADD(AFFECT_DATA,
					  pObjIndex->affected, paf);
				top_affect++;
			}

			else if (letter == 'E') 
				ed_fread(fp, &pObjIndex->ed);
			else {
				xungetc(letter, fp);
				break;
			}
		}

		/* fix armors */
		if (pObjIndex->item_type == ITEM_ARMOR) {
			pObjIndex->value[1] = pObjIndex->value[0];
			pObjIndex->value[2] = pObjIndex->value[1];
		}

		/*
		 * Translate spell "slot numbers" to internal "skill numbers."
		 */
		switch (pObjIndex->item_type) {
		case ITEM_PILL:
		case ITEM_POTION:
		case ITEM_SCROLL:
			pObjIndex->value[1] = slot_lookup(pObjIndex->value[1]);
			pObjIndex->value[2] = slot_lookup(pObjIndex->value[2]);
			pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
			pObjIndex->value[4] = slot_lookup(pObjIndex->value[4]);
			break;

		case ITEM_STAFF:
		case ITEM_WAND:
			pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
			break;
		}

		iHash			= vnum % MAX_KEY_HASH;
		pObjIndex->next		= obj_index_hash[iHash];
		obj_index_hash[iHash]	= pObjIndex;
		top_obj_index++;
								/* OLC */
		top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;
		vnum_check(vnum);				/* OLC */
	}
}


/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset(ROOM_INDEX_DATA *pR, RESET_DATA *pReset)
{
	RESET_DATA *pr;
 
	if (!pR)
		return;
 
	pr = pR->reset_last;
 
	if (!pr) {
		 pR->reset_first = pReset;
		 pR->reset_last  = pReset;
	}
	else {
		 pR->reset_last->next = pReset;
		 pR->reset_last       = pReset;
		 pR->reset_last->next = NULL;
	}

	top_reset++;
}
 

/*
 * Snarf a reset section.
 */
void load_resets(FILE *fp)
{
	RESET_DATA *pReset;
	int iLastRoom = 0;
	int iLastObj  = 0;

	if (area_last == NULL) {
		log("load_resets: no #AREA seen yet.");
		exit(1);
	}

	for (; ;) {
		ROOM_INDEX_DATA *pRoomIndex;
		EXIT_DATA *pexit;
		char letter;
		OBJ_INDEX_DATA *temp_index;

		if ((letter = fread_letter(fp)) == 'S')
			break;

		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}

		pReset		= alloc_perm(sizeof(*pReset));
		pReset->command	= letter;
		/* if_flag */	  fread_number(fp);
		pReset->arg1	= fread_number(fp);
		pReset->arg2	= fread_number(fp);
		pReset->arg3	= (letter == 'G' || letter == 'R')
				    ? 0 : fread_number(fp);
		pReset->arg4	= (letter == 'P' || letter == 'M')
				    ? fread_number(fp) : 0;
				  fread_to_eol(fp);

		/*
		 * Validate parameters.
		 * We're calling the index functions for the side effect.
		 */
		switch (letter) {
		default:
			log_printf("load_resets: bad command '%c'.", letter);
			exit(1);
			break;

		case 'M':
			get_mob_index  (pReset->arg1);
			if ((pRoomIndex = get_room_index(pReset->arg3))) {
				new_reset(pRoomIndex, pReset);
				iLastRoom = pReset->arg3;
			}
			break;

		case 'O':
			temp_index = get_obj_index  (pReset->arg1);
			temp_index->reset_num++;
			if ((pRoomIndex = get_room_index(pReset->arg3))) {
				new_reset(pRoomIndex, pReset);
				iLastObj = pReset->arg3;
			}
			break;

		case 'P':
			temp_index = get_obj_index  (pReset->arg1);
			temp_index->reset_num++;
			if ((pRoomIndex = get_room_index(iLastObj)))
				new_reset(pRoomIndex, pReset);
			break;

		case 'G':
		case 'E':
			temp_index = get_obj_index  (pReset->arg1);
			temp_index->reset_num++;
			if ((pRoomIndex = get_room_index(iLastRoom))) {
				new_reset(pRoomIndex, pReset);
				iLastObj = iLastRoom;
			}
			break;

		case 'D':
			pRoomIndex = get_room_index(pReset->arg1);

			if (pReset->arg2 < 0
			||  pReset->arg2 >= MAX_DIR
			||  pRoomIndex == NULL
			||  (pexit = pRoomIndex->exit[pReset->arg2]) == NULL
			||  !IS_SET( pexit->rs_flags, EX_ISDOOR)) {
				log_printf("load_resets: 'D': exit %d not door.",
					pReset->arg2);
				exit(1);
			}

			switch (pReset->arg3) {
			default:
				log_printf("load_resets: 'D': bad 'locks': %d.",
					pReset->arg3);
			case 0:
				break;
			case 1:
				SET_BIT(pexit->rs_flags, EX_CLOSED);
				SET_BIT(pexit->exit_info, EX_CLOSED);
				break;
			case 2:
				SET_BIT(pexit->rs_flags, EX_CLOSED | EX_LOCKED);
				SET_BIT(pexit->exit_info, EX_CLOSED | EX_LOCKED);
				break;
			}

			break;

		case 'R':
			pRoomIndex = get_room_index(pReset->arg1);

			if (pReset->arg2 < 0 || pReset->arg2 > MAX_DIR) {
				log_printf("load_resets: 'R': bad exit %d.",
					pReset->arg2);
				exit(1);
			}

			if (pRoomIndex != NULL)
				new_reset(pRoomIndex, pReset);

			break;
		}
	}
}


/*
 * Snarf a room section.
 */
void load_rooms(FILE *fp)
{
	ROOM_INDEX_DATA *pRoomIndex;

	if (area_last == NULL) {
		log("load_rooms: no #AREA seen yet.");
		exit(1);
	}

	for (; ;) {
		int vnum;
		char letter;
		int door;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#') {
			log("load_rooms: # not found.");
			exit(1);
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if (get_room_index(vnum) != NULL) {
			log_printf("load_rooms: vnum %d duplicated.", vnum);
			exit(1);
		}
		fBootDb = TRUE;

		pRoomIndex		= alloc_perm(sizeof(*pRoomIndex));
		pRoomIndex->name	= mlstr_new();
		pRoomIndex->description	= mlstr_new();

		pRoomIndex->owner	= str_dup("");
		pRoomIndex->people	= NULL;
		pRoomIndex->contents	= NULL;
		pRoomIndex->ed	= NULL;
		pRoomIndex->history     = NULL;
		pRoomIndex->area	= area_last;
		pRoomIndex->vnum	= vnum;
		mlstr_fread(fp, pRoomIndex->name);
		mlstr_fread(fp, pRoomIndex->description);
		/* Area number */	  fread_number(fp);
		pRoomIndex->room_flags	= fread_flags(fp);
 
		if (3000 <= vnum && vnum < 3400)
			SET_BIT(pRoomIndex->room_flags, ROOM_LAW);

		pRoomIndex->sector_type		= fread_number(fp);
		pRoomIndex->light		= 0;
		for (door = 0; door <= 5; door++)
			pRoomIndex->exit[door] = NULL;

		/* defaults */
		pRoomIndex->heal_rate = 100;
		pRoomIndex->mana_rate = 100;
		pRoomIndex->affected = NULL;
		pRoomIndex->affected_by = 0;
		pRoomIndex->aff_next = NULL;

		for (; ;) {
			letter = fread_letter(fp);

			if (letter == 'S')
				break;

			if (letter == 'H') /* healing room */
				pRoomIndex->heal_rate = fread_number(fp);
		
			else if (letter == 'M') /* mana room */
				pRoomIndex->mana_rate = fread_number(fp);

			else if (letter == 'D') {
				EXIT_DATA *pexit;
				int locks;
	
				door = fread_number(fp);
				if (door < 0 || door > 5) {
					log_printf("load_rooms: vnum %d has "
						   "bad door number.", vnum);
					exit(1);
				}
	
				pexit			= alloc_perm(sizeof(*pexit));
				pexit->description	= mlstr_new();

				mlstr_fread(fp, pexit->description);
				pexit->keyword		= fread_string(fp);
				pexit->exit_info	= 0;
				pexit->rs_flags		= 0;	/* OLC */
				locks			= fread_number(fp);
				pexit->key		= fread_number(fp);
				pexit->u1.vnum		= fread_number(fp);
				pexit->orig_door	= door;	/* OLC */
	
				switch (locks) {
				case 1:
					pexit->exit_info = EX_ISDOOR;
					pexit->rs_flags  = EX_ISDOOR;
					break;
				case 2:
					pexit->exit_info = EX_ISDOOR |
							   EX_PICKPROOF;
					pexit->rs_flags  = EX_ISDOOR |
							   EX_PICKPROOF;
					break;
				case 3:
					pexit->exit_info = EX_ISDOOR |
							   EX_NOPASS;
					pexit->rs_flags  = EX_ISDOOR |
							   EX_NOPASS;
					break;
				case 4:
					pexit->exit_info = EX_ISDOOR |
							   EX_NOPASS |
							   EX_PICKPROOF;
					pexit->rs_flags  = EX_ISDOOR |
							   EX_NOPASS |
							   EX_PICKPROOF;
					break;
				case 5:
					pexit->exit_info = EX_NOFLEE;
					pexit->rs_flags  = EX_NOFLEE;
					break;
				}
	
				pRoomIndex->exit[door] = pexit;
				top_exit++;
			}
			else if (letter == 'E') 
				ed_fread(fp, &pRoomIndex->ed);
			else if (letter == 'O') {
				if (pRoomIndex->owner[0] != '\0') {
					log("load_rooms: duplicate owner.");
					exit(1);
				}

				pRoomIndex->owner = fread_string(fp);
			}

			else {
				log_printf("load_rooms: vnum %d has flag "
					   "'%c' (not 'DES').",
					   vnum, letter);
				exit(1);
			}
		}

		iHash			= vnum % MAX_KEY_HASH;
		pRoomIndex->next	= room_index_hash[iHash];
		room_index_hash[iHash]	= pRoomIndex;
		top_room++;
								/* OLC */
		top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room;
		vnum_check(vnum);				/* OLC */
	}
}


/*
 * Load mobprogs section
 */
void load_mobprogs(FILE *fp)
{
    MPROG_CODE *pMprog;

    if (area_last == NULL)
    {
	log("load_mobprogs: no #AREA seen yet.");
	exit(1);
    }

    for (; ;)
    {
	int vnum;
	char letter;

	letter		  = fread_letter(fp);
	if (letter != '#')
	{
	    log("load_mobprogs: # not found.");
	    exit(1);
	}

	vnum		 = fread_number(fp);
	if (vnum == 0)
	    break;

	fBootDb = FALSE;
	if (get_mprog_index(vnum) != NULL)
	{
	    log_printf("load_mobprogs: vnum %d duplicated.", vnum);
	    exit(1);
	}
	fBootDb = TRUE;

	pMprog		= alloc_perm(sizeof(*pMprog));
	pMprog->vnum  	= vnum;
	pMprog->code  	= fread_string(fp);
	if (mprog_list == NULL)
	    mprog_list = pMprog;
	else
	{
	    pMprog->next = mprog_list;
	    mprog_list 	= pMprog;
	}
	top_mprog_index++;
    }
    return;
}


/*
 *  Translate mobprog vnums pointers to real code
 */
void fix_mobprogs(void)
{
    MOB_INDEX_DATA *pMobIndex;
    MPROG_LIST        *list;
    MPROG_CODE        *prog;
    int iHash;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
	for (pMobIndex   = mob_index_hash[iHash];
	      pMobIndex   != NULL;
	      pMobIndex   = pMobIndex->next)
	{
	    for(list = pMobIndex->mprogs; list != NULL; list = list->next)
	    {
		if ((prog = get_mprog_index(list->vnum)) != NULL)
		    list->code = prog->code;
		else
		{
		    log_printf("fix_mobprogs: code vnum %d not found.",
			       list->vnum);
		    exit(1);
		}
	    }
	}
    }
}
 

/*
 * Snarf a shop section.
 */
void load_shops(FILE *fp)
{
	SHOP_DATA *pShop;

	for (; ;)
	{
		MOB_INDEX_DATA *pMobIndex;
		int iTrade;

		pShop			= alloc_perm(sizeof(*pShop));
		pShop->keeper		= fread_number(fp);
		if (pShop->keeper == 0)
		    break;
		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
		    pShop->buy_type[iTrade]	= fread_number(fp);
		pShop->profit_buy	= fread_number(fp);
		pShop->profit_sell	= fread_number(fp);
		pShop->open_hour	= fread_number(fp);
		pShop->close_hour	= fread_number(fp);
					  fread_to_eol(fp);
		pMobIndex		= get_mob_index(pShop->keeper);
		pMobIndex->pShop	= pShop;

		if (shop_first == NULL)
		    shop_first = pShop;
		if (shop_last  != NULL)
		    shop_last->next = pShop;

		shop_last	= pShop;
		pShop->next	= NULL;
		top_shop++;
	}

	return;
}


/*
 * Snarf spec proc declarations.
 */
void load_specials(FILE *fp)
{
	for (; ;)
	{
		MOB_INDEX_DATA *pMobIndex;
		char letter;

		switch (letter = fread_letter(fp))
		{
		default:
		    log_printf("load_specials: letter '%c' not *MS.", letter);
		    exit(1);

		case 'S':
		    return;

		case '*':
		    break;

		case 'M':
		    pMobIndex		= get_mob_index	(fread_number (fp));
		    pMobIndex->spec_fun	= spec_lookup	(fread_word   (fp));
		    if (pMobIndex->spec_fun == 0) {
			log_printf("load_specials: 'M': vnum %d.",
				   pMobIndex->vnum);
			exit(1);
		    }
		    break;
		}

		fread_to_eol(fp);
	}
}


/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits(void)
{
	extern const int rev_dir [];
	ROOM_INDEX_DATA *pRoomIndex;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;
	int iHash;
	int door;

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for (pRoomIndex  = room_index_hash[iHash];
		      pRoomIndex != NULL;
		      pRoomIndex  = pRoomIndex->next)
		{
		    bool fexit;

		    fexit = FALSE;
		    for (door = 0; door <= 5; door++)
		    {
			if ((pexit = pRoomIndex->exit[door]) != NULL)
			{
			    if (pexit->u1.vnum <= 0 
			    || get_room_index(pexit->u1.vnum) == NULL)
				pexit->u1.to_room = NULL;
			    else
			    {
			   	fexit = TRUE; 
				pexit->u1.to_room = get_room_index(pexit->u1.vnum);
			    }
			}
		    }
		    if (!fexit)
			SET_BIT(pRoomIndex->room_flags,ROOM_NOMOB);
		}
	}

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for (pRoomIndex  = room_index_hash[iHash];
		      pRoomIndex != NULL;
		      pRoomIndex  = pRoomIndex->next)
		{
		    for (door = 0; door <= 5; door++)
		    {
			if ((pexit     = pRoomIndex->exit[door]     ) != NULL
			&&   (to_room   = pexit->u1.to_room          ) != NULL
			&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
			&&   pexit_rev->u1.to_room != pRoomIndex 
			&&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
			    log_printf("fix_exits: %d:%d -> %d:%d -> %d.",
				pRoomIndex->vnum, door,
				to_room->vnum,    rev_dir[door],
				(pexit_rev->u1.to_room == NULL)
				    ? 0 : pexit_rev->u1.to_room->vnum);
		    }
		}
	}

	return;
}


void print_resetmsg(AREA_DATA *pArea)
{
	DESCRIPTOR_DATA *d;
	bool is_empty = mlstr_null(pArea->resetmsg);
	
	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *ch;

		if (d->connected != CON_PLAYING)
			continue;

		ch = d->character;
		if (IS_NPC(ch) || !IS_AWAKE(ch) || ch->in_room->area != pArea)
			continue;

		if (is_empty)
			char_puts("You hear some squeaking sounds...\n\r", ch);
		else
			char_mlputs(pArea->resetmsg, ch);
	}
}


/*
 * Repopulate areas periodically.
 */
void area_update(void)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea != NULL; pArea = pArea->next) {
		if (++pArea->age < 3)
			continue;

		/*
		 * Check age and reset.
		 * Note: Mud School resets every 3 minutes (not 15).
		 */
		if ((!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
		||  pArea->age >= 31) {
			ROOM_INDEX_DATA *pRoomIndex;

			/*
			 * the rain devastates tracks on the ground
			 */
			if (weather_info.sky == SKY_RAINING)  {
				int i;
				DESCRIPTOR_DATA *d;
				CHAR_DATA *ch;
	 			for (d = descriptor_list; d!=NULL; d=d->next)  {
					if (d->connected != CON_PLAYING)
						continue;

					ch = (d->original != NULL) ?
						d->original : d->character;
					if (ch->in_room->area == pArea
					&&  get_skill(ch, gsn_track) > 50
					&& !IS_SET(ch->in_room->room_flags,
								ROOM_INDOORS))
					send_to_char("Rain devastates the tracks on the ground.\n\r", ch);
				}

				for (i = pArea->min_vnum; i < pArea->max_vnum;
									i++) {
					pRoomIndex = get_room_index(i);
					if (pRoomIndex == NULL
					||  IS_SET(pRoomIndex->room_flags,
								ROOM_INDOORS))
						continue;
					room_record("erased", pRoomIndex, -1);  
					if (number_percent() < 50)
						room_record("erased",
							    pRoomIndex, -1);
				}
			}

			reset_area(pArea);
			wiznet_printf(NULL, NULL, WIZ_RESETS, 0, 0,
		    		"%s has just been reset.", pArea->name);

			print_resetmsg(pArea);

			pArea->age = number_range(0, 3);
			pRoomIndex = get_room_index(200);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(210);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(220);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(230);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(ROOM_VNUM_SCHOOL);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			else if (pArea->nplayer == 0) 
				pArea->empty = TRUE;
		}
	}
}


#if 0
/*
 * Reset one area.
 */
void reset_area(AREA_DATA *pArea)
{
	RESET_DATA *pReset;
	CHAR_DATA *mob;
	bool last;
	int level;
	mob 	= NULL;
	last	= TRUE;
	level	= 0;
	for (pReset = pArea->reset_first; pReset != NULL; pReset = pReset->next)
	{
		ROOM_INDEX_DATA *pRoomIndex;
		MOB_INDEX_DATA *pMobIndex;
		OBJ_INDEX_DATA *pObjIndex;
		OBJ_INDEX_DATA *pObjToIndex;
		OBJ_INDEX_DATA *clan_item;
		EXIT_DATA *pexit;
		OBJ_DATA *obj;
		OBJ_DATA *obj_to;
		int count, limit, ci_vnum=0;

		switch (pReset->command)
		{
		default:
		    log_printf("Reset_area: bad command %c.", pReset->command);
		    break;

		case 'M':
		    if ((pMobIndex = get_mob_index(pReset->arg1)) == NULL)
		    {
			bug("Reset_area: 'M': bad vnum %d.", pReset->arg1);
			continue;
		    }

		    if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL)
		    {
			bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
			continue;
		    }

		    if (pMobIndex->count >= pReset->arg2)
		    {
			last = FALSE;
			break;
		    }

		    count = 0;
		    for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
			if (mob->pIndexData == pMobIndex)
			{
			    count++;
			    if (count >= pReset->arg4)
			    {
			    	last = FALSE;
			    	break;
			    }
			}

		    if (count >= pReset->arg4)
			break;

		    mob = create_mobile(pMobIndex);

		    /*
		     * Check for pet shop.
		     */
		    {
			ROOM_INDEX_DATA *pRoomIndexPrev;
			pRoomIndexPrev = get_room_index(pRoomIndex->vnum - 1);
			if (pRoomIndexPrev != NULL
			&&   IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
			    SET_BIT(mob->act, ACT_PET);
		    }

		    /* set area */
		    mob->zone = pRoomIndex->area;

		    char_to_room(mob, pRoomIndex);
		    level = URANGE(0, mob->level - 2, LEVEL_HERO - 1);
		    last  = TRUE;
		    break;

		case 'O':
		    if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL)
		    {
			bug("Reset_area: 'O': bad vnum %d.", pReset->arg1);
			continue;
		    }

		    if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL)
		    {
			bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
			continue;
		    }

		    if (pArea->nplayer > 0
		    ||   count_obj_list(pObjIndex, pRoomIndex->contents) > 0)
		    {
			last = FALSE;
			break;
		    }

		    switch(pObjIndex->vnum)
		    {
			case OBJ_VNUM_RULER_STAND:
			  ci_vnum = clan_table[CLAN_RULER].obj_vnum;
			  break;
			case OBJ_VNUM_INVADER_SKULL:
			  ci_vnum = clan_table[CLAN_INVADER].obj_vnum;
			  break;
			case OBJ_VNUM_SHALAFI_ALTAR:
			  ci_vnum = clan_table[CLAN_SHALAFI].obj_vnum;
			  break;
			case OBJ_VNUM_CHAOS_ALTAR:
			  ci_vnum = clan_table[CLAN_CHAOS].obj_vnum;
			  break;
			case OBJ_VNUM_KNIGHT_ALTAR:
			  ci_vnum = clan_table[CLAN_KNIGHT].obj_vnum;
			  break;
			case OBJ_VNUM_LIONS_ALTAR:
			  ci_vnum = clan_table[CLAN_LIONS].obj_vnum;
			  break;
			case OBJ_VNUM_BATTLE_THRONE:
			  ci_vnum = clan_table[CLAN_BATTLE].obj_vnum;
			  break;
			case OBJ_VNUM_HUNTER_ALTAR:
			  ci_vnum = clan_table[CLAN_HUNTER].obj_vnum;
			  break;
		    }

		    clan_item = get_obj_index(ci_vnum);
		    if (ci_vnum &&  clan_item->count > 0)
		     { 
			last = FALSE;
			break;
		     }

		    if ((pObjIndex->limit != -1) &&
		         (pObjIndex->count >= pObjIndex->limit))
		      {
		        last = FALSE;
		        break;
		      }

		    obj       = create_object(pObjIndex, UMIN(number_fuzzy(level),
							       LEVEL_HERO - 1));
		    obj->cost = 0;
		    obj_to_room(obj, pRoomIndex);
		    last = TRUE;
		    break;

		case 'P':
		    if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL)
		    {
			bug("Reset_area: 'P': bad vnum %d.", pReset->arg1);
			continue;
		    }

		    if ((pObjToIndex = get_obj_index(pReset->arg3)) == NULL)
		    {
			bug("Reset_area: 'P': bad vnum %d.", pReset->arg3);
			continue;
		    }

		    if (pReset->arg2 > 50) 		/* old format */
		        limit = 6;
		    else if (pReset->arg2 == -1) 	/* no limit */
		        limit = 999;
		    else
		        limit = pReset->arg2;

		    if (pArea->nplayer > 0
		    || (obj_to = get_obj_type(pObjToIndex)) == NULL
		    || (obj_to->in_room == NULL && !last)
		    || (pObjIndex->count >= limit && number_range(0,4) != 0)
		    || (count = count_obj_list(pObjIndex,obj_to->contains)) 
			> pReset->arg4)
		    {
			last = FALSE;
			break;
		    }

		    if ((pObjIndex->limit != -1              ) &&
		         (pObjIndex->count >= pObjIndex->limit))
		      {
		        last = FALSE;
		        dump_to_scr("Reseting area: [P] OBJ limit reached\n\r");
		        break;
		      }

		    while (count < pReset->arg4)
		    {
		        obj = create_object(pObjIndex, number_fuzzy(obj_to->level));
		    	obj_to_obj(obj, obj_to);
			count++;
			if (pObjIndex->count >= limit)
			    break;
		    }
		    /* fix object lock state! */
		    obj_to->value[1] = obj_to->pIndexData->value[1];
		    last = TRUE;
		    break;

		case 'G':
		case 'E':
		    if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL)
		    {
			bug("Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1);
			continue;
		    }

		    if (!last)
			break;

		    if (mob == NULL)
		    {
			bug("Reset_area: 'E' or 'G': null mob for vnum %d.",
			    pReset->arg1);
			last = FALSE;
			break;
		    }

		    if (mob->pIndexData->pShop != NULL)
		    {
			int olevel = 0,i,j;

			if (!pObjIndex->new_format)
			    switch (pObjIndex->item_type)
			{
			case ITEM_PILL:
			case ITEM_POTION:
			case ITEM_SCROLL:
			    olevel = MAX_LEVEL - 7;
			    for (i = 1; i < 5; i++)
			    {
				if (pObjIndex->value[i] > 0)
				{
			    	    for (j = 0; j < MAX_CLASS; j++)
				    {
					olevel = UMIN(olevel,
					         skill_table[pObjIndex->value[i]].
							     skill_level[j]);
				    }
				}
			    }
			   
			    olevel = UMAX(0,(olevel * 3 / 4) - 2);
			    break;
			case ITEM_WAND:		olevel = number_range(10, 20); break;
			case ITEM_STAFF:	olevel = number_range(15, 25); break;
			case ITEM_ARMOR:	olevel = number_range( 5, 15); break;
			case ITEM_WEAPON:	olevel = number_range( 5, 15); break;
			case ITEM_TREASURE:	olevel = number_range(10, 20); break;
			}

			obj = create_object(pObjIndex, olevel);
			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
		    }

		    else
		    {
		        if ((pObjIndex->limit == -1)  ||
		          (pObjIndex->count < pObjIndex->limit))
		          obj=create_object(pObjIndex,UMIN(number_fuzzy(level),
		                                           LEVEL_HERO - 1));
		        else break;

		    }

		    obj_to_char(obj, mob);
		    if (pReset->command == 'E')
			equip_char(mob, obj, pReset->arg3);
		    last = TRUE;
		    break;

		case 'D':
		    if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL)
		    {
			bug("Reset_area: 'D': bad vnum %d.", pReset->arg1);
			continue;
		    }

		    if ((pexit = pRoomIndex->exit[pReset->arg2]) == NULL)
			break;

		    switch (pReset->arg3)
		    {
		    case 0:
			REMOVE_BIT(pexit->exit_info, EX_CLOSED);
			REMOVE_BIT(pexit->exit_info, EX_LOCKED);
			break;

		    case 1:
			SET_BIT(   pexit->exit_info, EX_CLOSED);
			REMOVE_BIT(pexit->exit_info, EX_LOCKED);
			break;

		    case 2:
			SET_BIT(   pexit->exit_info, EX_CLOSED);
			SET_BIT(   pexit->exit_info, EX_LOCKED);
			break;
		    }

		    last = TRUE;
		    break;

		case 'R':
		    if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL)
		    {
			bug("Reset_area: 'R': bad vnum %d.", pReset->arg1);
			continue;
		    }

		    {
			int d0;
			int d1;

			for (d0 = 0; d0 < pReset->arg2 - 1; d0++)
			{
			    d1                   = number_range(d0, pReset->arg2-1);
			    pexit                = pRoomIndex->exit[d0];
			    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
			    pRoomIndex->exit[d1] = pexit;
			}
		    }
		    break;
		}
	}
}
#endif


/*
 * OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room( ROOM_INDEX_DATA *pRoom )
{
    RESET_DATA  *pReset;
    CHAR_DATA   *pMob;
    CHAR_DATA	*mob;
    OBJ_DATA    *pObj;
    CHAR_DATA   *LastMob = NULL;
    OBJ_DATA    *LastObj = NULL;
    int iExit;
    int level = 0;
    bool last;

    if ( !pRoom )
        return;

    pMob        = NULL;
    last        = FALSE;
    
    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
        EXIT_DATA *pExit;
        if ( ( pExit = pRoom->exit[iExit] )
	  /*  && !IS_SET( pExit->exit_info, EX_BASHED )   ROM OLC */ )  
        {
            pExit->exit_info = pExit->rs_flags;
            if ( ( pExit->u1.to_room != NULL )
              && ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) )
            {
                /* nail the other side */
                pExit->exit_info = pExit->rs_flags;
            }
        }
    }

    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
    {
        MOB_INDEX_DATA  *pMobIndex;
        OBJ_INDEX_DATA  *pObjIndex;
        OBJ_INDEX_DATA  *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;
	int count,limit=0;
	OBJ_INDEX_DATA *clan_item;
	int ci_vnum = 0;
        EXIT_DATA *pExit;
        int d0;
        int d1;

        switch ( pReset->command )
        {
        default:
                bug( "Reset_room: bad command %c.", pReset->command );
                break;

        case 'M':
            if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'M': bad vnum %d.", pReset->arg1 );
                continue;
            }

	    if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
	    {
		bug( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
		continue;
	    }
            if ( pMobIndex->count >= pReset->arg2 )
            {
                last = FALSE;
                break;
            }
/* */
	    count = 0;
	    for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
		if (mob->pIndexData == pMobIndex)
		{
		    count++;
		    if (count >= pReset->arg4)
		    {
		    	last = FALSE;
		    	break;
		    }
		}

	    if (count >= pReset->arg4)
		break;

/* */

            pMob = create_mobile( pMobIndex );

#if 0
	/* XXX was in ROM OLC /fjoe */

            /*
             * Some more hard coding.
             */
            if ( room_is_dark( pRoom ) )
                SET_BIT(pMob->affected_by, AFF_INFRARED);
#endif

            /*
             * Pet shop mobiles get ACT_PET set.
             */
            {
                ROOM_INDEX_DATA *pRoomIndexPrev;

                pRoomIndexPrev = get_room_index( pRoom->vnum - 1 );
                if ( pRoomIndexPrev
                    && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
                    SET_BIT( pMob->act, ACT_PET);
            }

		pMob->zone = pRoom->area;
            char_to_room( pMob, pRoom );

            LastMob = pMob;
            level  = URANGE( 0, pMob->level - 2, LEVEL_HERO - 1 ); /* -1 ROM */
            last = TRUE;
            break;

        case 'O':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                log_printf("reset_room: 'O' 1 : bad vnum %d", pReset->arg1);
                log_printf("reset_room: %c %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4 );
                continue;
            }

            if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                log_printf("reset_room: 'O' 2 : bad vnum %d.", pReset->arg3);
                log_printf("reset_room: %c %d %d %d", pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4 );
                continue;
            }

            if ( pRoom->area->nplayer > 0
              || count_obj_list( pObjIndex, pRoom->contents ) > 0 )
	    {
		last = FALSE;
		break;
	    }

		    switch(pObjIndex->vnum)
		    {
			case OBJ_VNUM_RULER_STAND:
			  ci_vnum = clan_table[CLAN_RULER].obj_vnum;
			  break;
			case OBJ_VNUM_INVADER_SKULL:
			  ci_vnum = clan_table[CLAN_INVADER].obj_vnum;
			  break;
			case OBJ_VNUM_SHALAFI_ALTAR:
			  ci_vnum = clan_table[CLAN_SHALAFI].obj_vnum;
			  break;
			case OBJ_VNUM_CHAOS_ALTAR:
			  ci_vnum = clan_table[CLAN_CHAOS].obj_vnum;
			  break;
			case OBJ_VNUM_KNIGHT_ALTAR:
			  ci_vnum = clan_table[CLAN_KNIGHT].obj_vnum;
			  break;
			case OBJ_VNUM_LIONS_ALTAR:
			  ci_vnum = clan_table[CLAN_LIONS].obj_vnum;
			  break;
			case OBJ_VNUM_BATTLE_THRONE:
			  ci_vnum = clan_table[CLAN_BATTLE].obj_vnum;
			  break;
			case OBJ_VNUM_HUNTER_ALTAR:
			  ci_vnum = clan_table[CLAN_HUNTER].obj_vnum;
			  break;
		    }

		    clan_item = get_obj_index(ci_vnum);
		    if (ci_vnum &&  clan_item->count > 0)
		     { 
			last = FALSE;
			break;
		     }

		    if ((pObjIndex->limit != -1) &&
		         (pObjIndex->count >= pObjIndex->limit))
		      {
		        last = FALSE;
		        break;
		      }

            pObj = create_object( pObjIndex,              /* UMIN - ROM OLC */
				  UMIN(number_fuzzy( level ), LEVEL_HERO -1) );
            pObj->cost = 0;
            obj_to_room( pObj, pRoom );
	    last = TRUE;
            break;

        case 'P':
		/* (P)ut command
		 * arg1 - vnum of obj to put
		 * arg2
		 * arg3 - vnum of obj to put into
		 * arg4
		 */

            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->arg3 );
                continue;
            }

            if (pReset->arg2 > 50) /* old format */
                limit = 6;
            else if (pReset->arg2 == -1) /* no limit */
                limit = 999;
            else
                limit = pReset->arg2;

            if ( pRoom->area->nplayer > 0
              || ( LastObj = get_obj_type( pObjToIndex ) ) == NULL
              || ( LastObj->in_room == NULL && !last)
              || ( pObjIndex->count >= limit /* && number_range(0,4) != 0 */ )
              || ( count = count_obj_list( pObjIndex, LastObj->contains ) ) > pReset->arg4  )
	    {
		last = FALSE;
		break;
	    }
				                /* lastObj->level  -  ROM */

		    if ((pObjIndex->limit != -1              ) &&
		         (pObjIndex->count >= pObjIndex->limit))
		      {
		        last = FALSE;
		        dump_to_scr("Reseting area: [P] OBJ limit reached\n\r");
		        break;
		      }

	    while (count < pReset->arg4)
	    {
            pObj = create_object( pObjIndex, number_fuzzy( LastObj->level ) );
            obj_to_obj( pObj, LastObj );
		count++;
		if (pObjIndex->count >= limit)
		    break;
	    }

	    /* fix object lock state! */
	    LastObj->value[1] = LastObj->pIndexData->value[1];
	    last = TRUE;
            break;

        case 'G':
        case 'E':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( !last )
                break;

            if ( !LastMob )
            {
                bug( "Reset_room: 'E' or 'G': null mob for vnum %d.",
                    pReset->arg1 );
                last = FALSE;
                break;
            }

            if ( LastMob->pIndexData->pShop )   /* Shop-keeper? */
            {
                int olevel=0,i,j;

		if (!pObjIndex->new_format)
                 switch ( pObjIndex->item_type )
                {
                case ITEM_PILL:
                case ITEM_POTION:
                case ITEM_SCROLL:
 		    olevel = MAX_LEVEL-7;
		    for (i = 1; i < 5; i++)
		    {
			if (pObjIndex->value[i] > 0)
			{
		    	    for (j = 0; j < MAX_CLASS; j++)
			    {
				olevel = UMIN(olevel,
				         skill_table[pObjIndex->value[i]].
						     skill_level[j]);
			    }
			}
		    }
		   
		    olevel = UMAX(0,(olevel * 3 / 4) - 2);
		    break;
		    
               case ITEM_WAND:         olevel = number_range( 10, 20 ); break;
                case ITEM_STAFF:        olevel = number_range( 15, 25 ); break;
                case ITEM_ARMOR:        olevel = number_range(  5, 15 ); break;
                /* ROM patch weapon, treasure */
		case ITEM_WEAPON:       olevel = number_range(  5, 15 ); break;
		case ITEM_TREASURE:     olevel = number_range( 10, 20 ); break;

#if 0 /* envy version */
                case ITEM_WEAPON:       if ( pReset->command == 'G' )
                                            olevel = number_range( 5, 15 );
                                        else
                                            olevel = number_fuzzy( level );
#endif /* envy version */

                  break;
                }

                pObj = create_object( pObjIndex, olevel );
		SET_BIT( pObj->extra_flags, ITEM_INVENTORY );  /* ROM OLC */

#if 0 /* envy version */
                if ( pReset->command == 'G' )
                    SET_BIT( pObj->extra_flags, ITEM_INVENTORY );
#endif /* envy version */

            }
#if 0
	    else   /* ROM OLC else version */
	    {
		int limit;
		if (pReset->arg2 > 50 )  /* old format */
		    limit = 6;
		else if ( pReset->arg2 == -1 || pReset->arg2 == 0 )  /* no limit */
		    limit = 999;
		else
		    limit = pReset->arg2;

		if ( pObjIndex->count < limit || number_range(0,4) == 0 )
		{
		    pObj = create_object( pObjIndex, 
			   UMIN( number_fuzzy( level ), LEVEL_HERO - 1 ) );
		    /* error message if it is too high */
		    if (pObj->level > LastMob->level + 3
		    ||  (pObj->item_type == ITEM_WEAPON 
		    &&   pReset->command == 'E' 
		    &&   pObj->level < LastMob->level -5 && pObj->level < 45))
			fprintf(stderr,
				"Err: obj %s (%d) -- %d, mob %s (%d) -- %d\n",
				mlstr_mval(pObj->short_descr),
				pObj->pIndexData->vnum,pObj->level,
				mlstr_mval(LastMob->short_descr),
				LastMob->pIndexData->vnum,LastMob->level);
		}
		else
		    break;
	    }
#endif									 
#if 0 /* envy else version */
            else
            {
                pObj = create_object( pObjIndex, number_fuzzy( level ) );
            }
#endif /* envy else version */

/* Anatolia else version */
		else {
		        if ((pObjIndex->limit == -1)  ||
		          (pObjIndex->count < pObjIndex->limit))
		          pObj=create_object(pObjIndex,UMIN(number_fuzzy(level),
		                                           LEVEL_HERO - 1));
		        else break;

		}

            obj_to_char( pObj, LastMob );
            if ( pReset->command == 'E' )
                equip_char( LastMob, pObj, pReset->arg3 );
            last = TRUE;
            break;

        case 'D':
		    if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL)
		    {
			bug("Reset_area: 'D': bad vnum %d.", pReset->arg1);
			continue;
		    }

		    if ((pExit = pRoomIndex->exit[pReset->arg2]) == NULL)
			break;

		    switch (pReset->arg3)
		    {
		    case 0:
			REMOVE_BIT(pExit->exit_info, EX_CLOSED);
			REMOVE_BIT(pExit->exit_info, EX_LOCKED);
			break;

		    case 1:
			SET_BIT(   pExit->exit_info, EX_CLOSED);
			REMOVE_BIT(pExit->exit_info, EX_LOCKED);
			break;

		    case 2:
			SET_BIT(   pExit->exit_info, EX_CLOSED);
			SET_BIT(   pExit->exit_info, EX_LOCKED);
			break;
		    }

		    last = TRUE;
            break;

        case 'R':
            if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'R': bad vnum %d.", pReset->arg1 );
                continue;
            }

            {

                for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
                {
                    d1                   = number_range( d0, pReset->arg2-1 );
                    pExit                = pRoomIndex->exit[d0];
                    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                    pRoomIndex->exit[d1] = pExit;
                }
            }
            break;
        }
    }
}

/*
 * OLC
 * Reset one area.
 */
void reset_area(AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoom;
	int vnum;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
        	if ((pRoom = get_room_index(vnum)))
			reset_room(pRoom);
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile(MOB_INDEX_DATA *pMobIndex)
{
	CHAR_DATA *mob;
	int i;
	AFFECT_DATA af;

	mobile_count++;

	if (pMobIndex == NULL)
	{
		bug("Create_mobile: NULL pMobIndex.", 0);
		exit(1);
	}

	mob = new_char();

	mob->pIndexData	= pMobIndex;

	mob->name	= str_dup(pMobIndex->player_name);    /* OLC */
	mlstr_cpy(mob->short_descr, pMobIndex->short_descr);    /* OLC */
	mlstr_cpy(mob->long_descr, pMobIndex->long_descr);     /* OLC */
	mlstr_cpy(mob->description, pMobIndex->description);    /* OLC */
	mob->id		= get_mob_id();
	mob->spec_fun	= pMobIndex->spec_fun;
	mob->class	= CLASS_CLERIC;

	if (pMobIndex->wealth) {
		long wealth;

		wealth = number_range(pMobIndex->wealth/2,
				      3 * pMobIndex->wealth/2);
		mob->gold = number_range(wealth/200,wealth/100);
		mob->silver = wealth - (mob->gold * 100);
	} 

	if (pMobIndex->new_format) {
		/* load in new style */
		/* read from prototype */
 		mob->group		= pMobIndex->group;
		mob->act 		= pMobIndex->act | ACT_NPC;
		mob->comm		= COMM_NOCHANNELS|COMM_NOSHOUT|COMM_NOTELL;
		mob->affected_by	= pMobIndex->affected_by;
		mob->detection		= pMobIndex->detection;
		mob->alignment		= pMobIndex->alignment;
		mob->level		= pMobIndex->level;
		mob->hitroll		= (mob->level / 2) + pMobIndex->hitroll;
		mob->damroll		= pMobIndex->damage[DICE_BONUS];
		mob->max_hit		= dice(pMobIndex->hit[DICE_NUMBER],
					       pMobIndex->hit[DICE_TYPE])
					  + pMobIndex->hit[DICE_BONUS];
		mob->hit		= mob->max_hit;
		mob->max_mana		= dice(pMobIndex->mana[DICE_NUMBER],
					       pMobIndex->mana[DICE_TYPE])
					  + pMobIndex->mana[DICE_BONUS];
		mob->mana		= mob->max_mana;
		mob->damage[DICE_NUMBER]= pMobIndex->damage[DICE_NUMBER];
		mob->damage[DICE_TYPE]	= pMobIndex->damage[DICE_TYPE];
		mob->dam_type		= pMobIndex->dam_type;
		
		if (mob->dam_type == 0)
		    switch(number_range(1,3))
		    {
		        case (1): mob->dam_type = 3;        break;  /* slash */
		        case (2): mob->dam_type = 7;        break;  /* pound */
		        case (3): mob->dam_type = 11;       break;  /* pierce */
		    }
		for (i = 0; i < 4; i++)
		    mob->armor[i]	= pMobIndex->ac[i]; 
		mob->off_flags		= pMobIndex->off_flags;
		mob->imm_flags		= pMobIndex->imm_flags;
		mob->res_flags		= pMobIndex->res_flags;
		mob->vuln_flags		= pMobIndex->vuln_flags;
		mob->start_pos		= pMobIndex->start_pos;
		mob->default_pos	= pMobIndex->default_pos;
		mob->sex		= pMobIndex->sex;
		if (mob->sex == SEX_EITHER) /* random sex */
		    mob->sex = number_range(1,2);
		mob->race		= pMobIndex->race;
		mob->form		= pMobIndex->form;
		mob->parts		= pMobIndex->parts;
		mob->size		= pMobIndex->size;
		mob->material		= str_dup(pMobIndex->material);

		/* computed on the spot */

		for (i = 0; i < MAX_STATS; i ++)
		    mob->perm_stat[i] = UMIN(25,11 + mob->level/4);
		    
		if (IS_SET(mob->act,ACT_WARRIOR))
		{
		    mob->perm_stat[STAT_STR] += 3;
		    mob->perm_stat[STAT_INT] -= 1;
		    mob->perm_stat[STAT_CON] += 2;
		}
		
		if (IS_SET(mob->act,ACT_THIEF))
		{
		    mob->perm_stat[STAT_DEX] += 3;
		    mob->perm_stat[STAT_INT] += 1;
		    mob->perm_stat[STAT_WIS] -= 1;
		}
		
		if (IS_SET(mob->act,ACT_CLERIC))
		{
		    mob->perm_stat[STAT_WIS] += 3;
		    mob->perm_stat[STAT_DEX] -= 1;
		    mob->perm_stat[STAT_STR] += 1;
		}
		
		if (IS_SET(mob->act,ACT_MAGE))
		{
		    mob->perm_stat[STAT_INT] += 3;
		    mob->perm_stat[STAT_STR] -= 1;
		    mob->perm_stat[STAT_DEX] += 1;
		}
		
		if (IS_SET(mob->off_flags,OFF_FAST))
		    mob->perm_stat[STAT_DEX] += 2;
		    
		mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
		mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

		/* let's get some spell action */
		if (IS_AFFECTED(mob,AFF_SANCTUARY))
		{
		    af.where	 = TO_AFFECTS;
		    af.type      = skill_lookup("sanctuary");
		    af.level     = mob->level;
		    af.duration  = -1;
		    af.location  = APPLY_NONE;
		    af.modifier  = 0;
		    af.bitvector = AFF_SANCTUARY;
		    affect_to_char(mob, &af);
		}

		if (IS_AFFECTED(mob,AFF_HASTE))
		{
		    af.where	 = TO_AFFECTS;
		    af.type      = skill_lookup("haste");
		    af.level     = mob->level;
	  	    af.duration  = -1;
		    af.location  = APPLY_DEX;
		    af.modifier  = 1 + (mob->level >= 18) + (mob->level >= 25) + 
				   (mob->level >= 32);
		    af.bitvector = AFF_HASTE;
		    affect_to_char(mob, &af);
		}

		if (IS_AFFECTED(mob,AFF_PROTECT_EVIL))
		{
		    af.where	 = TO_AFFECTS;
		    af.type	 = skill_lookup("protection evil");
		    af.level	 = mob->level;
		    af.duration	 = -1;
		    af.location	 = APPLY_SAVES;
		    af.modifier	 = -1;
		    af.bitvector = AFF_PROTECT_EVIL;
		    affect_to_char(mob,&af);
		}

		if (IS_AFFECTED(mob,AFF_PROTECT_GOOD))
		{
		    af.where	 = TO_AFFECTS;
		    af.type      = skill_lookup("protection good");
		    af.level     = mob->level;
		    af.duration  = -1;
		    af.location  = APPLY_SAVES;
		    af.modifier  = -1;
		    af.bitvector = AFF_PROTECT_GOOD;
		    affect_to_char(mob,&af);
		}  
	}
	else /* read in old format and convert */
	{
		mob->act		= pMobIndex->act;
		mob->affected_by	= pMobIndex->affected_by;
		mob->detection		= pMobIndex->detection;
		mob->alignment		= pMobIndex->alignment;
		mob->level		= pMobIndex->level;
		mob->hitroll		= UMAX(pMobIndex->hitroll,pMobIndex->level/4);
		mob->damroll		= pMobIndex->level /2 ;
		if (mob->level < 30)
		mob->max_hit		= mob->level * 20 + number_range(
						mob->level ,
						mob->level * 5);
		else if (mob->level < 60)
		mob->max_hit		= mob->level * 50 + number_range(
						mob->level * 10,
						mob->level * 50);
		else
		mob->max_hit		= mob->level * 100 + number_range(
						mob->level * 20,
						mob->level * 100);
		if (IS_SET(mob->act,ACT_MAGE | ACT_CLERIC))
			mob->max_hit *= 0.9;
		mob->hit		= mob->max_hit;
		mob->max_mana		= 100 + dice(mob->level,10);
		mob->mana		= mob->max_mana;
		switch(number_range(1,3))
		{
		    case (1): mob->dam_type = 3; 	break;  /* slash */
		    case (2): mob->dam_type = 7;	break;  /* pound */
		    case (3): mob->dam_type = 11;	break;  /* pierce */
		}
		for (i = 0; i < 3; i++)
		    mob->armor[i]	= interpolate(mob->level,100,-100);
		mob->armor[3]		= interpolate(mob->level,100,0);
		mob->race		= pMobIndex->race;
		mob->off_flags		= pMobIndex->off_flags;
		mob->imm_flags		= pMobIndex->imm_flags;
		mob->res_flags		= pMobIndex->res_flags;
		mob->vuln_flags		= pMobIndex->vuln_flags;
		mob->start_pos		= pMobIndex->start_pos;
		mob->default_pos	= pMobIndex->default_pos;
		mob->sex		= pMobIndex->sex;
		mob->form		= pMobIndex->form;
		mob->parts		= pMobIndex->parts;
		mob->size		= SIZE_MEDIUM;
		mob->material		= "";
/*
		for (i = 0; i < MAX_STATS; i ++)
		    mob->perm_stat[i] = 11 + mob->level/4;
 computed on the spot */

		for (i = 0; i < MAX_STATS; i ++)
		    mob->perm_stat[i] = UMIN(25,11 + mob->level/4);
		    
		if (IS_SET(mob->act,ACT_WARRIOR))
		{
		    mob->perm_stat[STAT_STR] += 3;
		    mob->perm_stat[STAT_INT] -= 1;
		    mob->perm_stat[STAT_CON] += 2;
		}
		
		if (IS_SET(mob->act,ACT_THIEF))
		{
		    mob->perm_stat[STAT_DEX] += 3;
		    mob->perm_stat[STAT_INT] += 1;
		    mob->perm_stat[STAT_WIS] -= 1;
		}
		
		if (IS_SET(mob->act,ACT_CLERIC))
		{
		    mob->perm_stat[STAT_WIS] += 3;
		    mob->perm_stat[STAT_DEX] -= 1;
		    mob->perm_stat[STAT_STR] += 1;
		}
		
		if (IS_SET(mob->act,ACT_MAGE))
		{
		    mob->perm_stat[STAT_INT] += 3;
		    mob->perm_stat[STAT_STR] -= 1;
		    mob->perm_stat[STAT_DEX] += 1;
		}
		
		if (IS_SET(mob->off_flags,OFF_FAST))
		    mob->perm_stat[STAT_DEX] += 2;
	}

	mob->position = mob->start_pos;


	/* link the mob to the world list */
	mob->next		= char_list;
	char_list		= mob;
	pMobIndex->count++;
	return mob;
}


/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone)
{
	int i;
	AFFECT_DATA *paf;

	if (parent == NULL || clone == NULL || !IS_NPC(parent))
		return;
	
	/* start fixing values */ 
	clone->name 	= str_dup(parent->name);
	clone->version	= parent->version;
	mlstr_cpy(clone->short_descr, parent->short_descr);
	mlstr_cpy(clone->long_descr, parent->long_descr);
	mlstr_cpy(clone->description, parent->description);
	clone->group	= parent->group;
	clone->sex		= parent->sex;
	clone->class	= parent->class;
	clone->race		= parent->race;
	clone->level	= parent->level;
	clone->trust	= 0;
	clone->timer	= parent->timer;
	clone->wait		= parent->wait;
	clone->hit		= parent->hit;
	clone->max_hit	= parent->max_hit;
	clone->mana		= parent->mana;
	clone->max_mana	= parent->max_mana;
	clone->move		= parent->move;
	clone->max_move	= parent->max_move;
	clone->gold		= parent->gold;
	clone->silver	= parent->silver;
	clone->exp		= parent->exp;
	clone->act		= parent->act;
	clone->comm		= parent->comm;
	clone->imm_flags	= parent->imm_flags;
	clone->res_flags	= parent->res_flags;
	clone->vuln_flags	= parent->vuln_flags;
	clone->invis_level	= parent->invis_level;
	clone->affected_by	= parent->affected_by;
	clone->detection	= parent->detection;
	clone->position	= parent->position;
	clone->practice	= parent->practice;
	clone->train	= parent->train;
	clone->saving_throw	= parent->saving_throw;
	clone->alignment	= parent->alignment;
	clone->hitroll	= parent->hitroll;
	clone->damroll	= parent->damroll;
	clone->wimpy	= parent->wimpy;
	clone->form		= parent->form;
	clone->parts	= parent->parts;
	clone->size		= parent->size;
	clone->material	= str_dup(parent->material);
	clone->extracted	= parent->extracted;
	clone->off_flags	= parent->off_flags;
	clone->dam_type	= parent->dam_type;
	clone->start_pos	= parent->start_pos;
	clone->default_pos	= parent->default_pos;
	clone->spec_fun	= parent->spec_fun;
	clone->status	= parent->status;
	clone->hunting	= NULL;

	for (i = 0; i < 4; i++)
		clone->armor[i]	= parent->armor[i];

	for (i = 0; i < MAX_STATS; i++)
	{
		clone->perm_stat[i]	= parent->perm_stat[i];
		clone->mod_stat[i]	= parent->mod_stat[i];
	}

	for (i = 0; i < 3; i++)
		clone->damage[i]	= parent->damage[i];

	/* now add the affects */
	for (paf = parent->affected; paf != NULL; paf = paf->next)
		affect_to_char(clone,paf);

}



/* 
 * Create an object with modifying the count 
 */
OBJ_DATA *create_object(OBJ_INDEX_DATA *pObjIndex, int level)
{
	return create_object_org(pObjIndex,level,TRUE);
}

/*
 * for player load/quit
 * Create an object and do not modify the count 
 */
OBJ_DATA *create_object_nocount(OBJ_INDEX_DATA *pObjIndex, int level)
{
	return create_object_org(pObjIndex,level,FALSE);
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object_org(OBJ_INDEX_DATA *pObjIndex, int level, bool Count)
{
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	int i;


	if (pObjIndex == NULL)
	{
		bug("Create_object: NULL pObjIndex.", 0);
		exit(1);
	}

	obj = new_obj();

	obj->pIndexData	= pObjIndex;

	for (i=1;i < MAX_CLAN;i++)
	  if (pObjIndex->vnum == clan_table[i].obj_vnum)  {
		if (count_obj_list(pObjIndex, object_list) > 0)
		  return(NULL);
		clan_table[i].obj_ptr = obj;
	  }
	if ((obj->pIndexData->limit != -1)  &&
		 (obj->pIndexData->count >= obj->pIndexData->limit))

	if (pObjIndex->new_format == 1)
	   dump_to_scr("");

	if (pObjIndex->new_format == 1)
 	obj->level = pObjIndex->level;
	else
		obj->level		= UMAX(0,level);
	obj->wear_loc	= -1;


	obj->name	= str_dup(pObjIndex->name);		/* OLC */
	mlstr_cpy(obj->short_descr, pObjIndex->short_descr);	/* OLC */
	mlstr_cpy(obj->description, pObjIndex->description);	/* OLC */
	obj->material	= str_dup(pObjIndex->material);
	obj->item_type	= pObjIndex->item_type;
	obj->extra_flags= pObjIndex->extra_flags;
	obj->wear_flags	= pObjIndex->wear_flags;
	obj->value[0]	= pObjIndex->value[0];
	obj->value[1]	= pObjIndex->value[1];
	obj->value[2]	= pObjIndex->value[2];
	obj->value[3]	= pObjIndex->value[3];
	obj->value[4]	= pObjIndex->value[4];
	obj->weight	= pObjIndex->weight;
	obj->from       = str_dup(""); /* used with body parts */
	obj->condition	= pObjIndex->condition;

	if (level == -1 || pObjIndex->new_format)
		obj->cost = pObjIndex->cost;
	else
		obj->cost = number_fuzzy(10)
				* number_fuzzy(level) * number_fuzzy(level);

	/*
	 * Mess with object properties.
	 */
	switch (obj->item_type)
	{
	default:
		bug("Read_object: vnum %d bad type.", pObjIndex->vnum);
		break;

	case ITEM_LIGHT:
		if (obj->value[2] == 999)
			obj->value[2] = -1;
		break;

	case ITEM_FURNITURE:
	case ITEM_TRASH:
	case ITEM_CONTAINER:
	case ITEM_DRINK_CON:
	case ITEM_KEY:
	case ITEM_FOOD:
	case ITEM_BOAT:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_FOUNTAIN:
	case ITEM_MAP:
	case ITEM_CLOTHING:
	case ITEM_PORTAL:
		if (!pObjIndex->new_format)
		    obj->cost /= 5;
		break;

	case ITEM_TREASURE:
	case ITEM_WARP_STONE:
	case ITEM_ROOM_KEY:
	case ITEM_GEM:
	case ITEM_JEWELRY:
	case ITEM_TATTOO:
		break;

	case ITEM_JUKEBOX:
		for (i = 0; i < 5; i++)
		   obj->value[i] = -1;
		break;

	case ITEM_SCROLL:
		if (level != -1 && !pObjIndex->new_format)
		    obj->value[0]	= number_fuzzy(obj->value[0]);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		if (level != -1 && !pObjIndex->new_format)
		{
		    obj->value[0]	= number_fuzzy(obj->value[0]);
		    obj->value[1]	= number_fuzzy(obj->value[1]);
		    obj->value[2]	= obj->value[1];
		}
		if (!pObjIndex->new_format)
		    obj->cost *= 2;
		break;

	case ITEM_WEAPON:
		if (level != -1 && !pObjIndex->new_format)
		{
		    obj->value[1] = number_fuzzy(number_fuzzy(1 * level / 4 + 2));
		    obj->value[2] = number_fuzzy(number_fuzzy(3 * level / 4 + 6));
		}
		break;

	case ITEM_ARMOR:
		if (level != -1 && !pObjIndex->new_format)
		{
		    obj->value[0]	= number_fuzzy(level / 5 + 3);
		    obj->value[1]	= number_fuzzy(level / 5 + 3);
		    obj->value[2]	= number_fuzzy(level / 5 + 3);
		}
		break;

	case ITEM_POTION:
	case ITEM_PILL:
		if (level != -1 && !pObjIndex->new_format)
		    obj->value[0] = number_fuzzy(number_fuzzy(obj->value[0]));
		break;

	case ITEM_MONEY:
		if (!pObjIndex->new_format)
		    obj->value[0]	= obj->cost;
		break;
	}
	
	for (paf = pObjIndex->affected; paf != NULL; paf = paf->next) 
		if (paf->location == APPLY_SPELL_AFFECT)
		    affect_to_obj(obj,paf);
	
	obj->next		= object_list;
	object_list		= obj;
	if (Count)
	  pObjIndex->count++;
	return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
	int i;
	AFFECT_DATA *paf;
	ED_DATA *ed,*ed_new;

	if (parent == NULL || clone == NULL)
		return;

	/* start fixing the object */
	clone->name 	= str_dup(parent->name);
	mlstr_cpy(clone->short_descr, parent->short_descr);
	mlstr_cpy(clone->description, parent->description);
	clone->item_type	= parent->item_type;
	clone->extra_flags	= parent->extra_flags;
	clone->wear_flags	= parent->wear_flags;
	clone->weight	= parent->weight;
	clone->cost		= parent->cost;
	clone->level	= parent->level;
	clone->condition	= parent->condition;
	clone->material	= str_dup(parent->material);
	clone->timer	= parent->timer;
	clone->from         = parent->from;
	clone->extracted    = parent->extracted;
	clone->pit          = parent->pit;
	clone->altar        = parent->altar;

	for (i = 0;  i < 5; i ++)
		clone->value[i]	= parent->value[i];

	/* affects */
	clone->enchanted	= parent->enchanted;
	
	for (paf = parent->affected; paf != NULL; paf = paf->next) 
		affect_to_obj(clone,paf);

	/* extended desc */
	for (ed = parent->ed; ed != NULL; ed = ed->next) {
		ed_new                  = ed_dup(ed);
		ed_new->next           	= clone->ed;
		clone->ed  	= ed_new;
	}

}


/*
 * Get an extra description from a list.
 */
ED_DATA *ed_lookup(const char *name, ED_DATA *ed)
{
	for (; ed != NULL; ed = ed->next) {
		if (is_name(name, ed->keyword))
			return ed;
	}
	return NULL;
}


/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index(int vnum)
{
	MOB_INDEX_DATA *pMobIndex;

	for (pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next)
	{
		if (pMobIndex->vnum == vnum)
		    return pMobIndex;
	}

	if (fBootDb)
	{
		bug("Get_mob_index: bad vnum %d.", vnum);
		exit(1);
	}

	return NULL;
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index(int vnum)
{
	OBJ_INDEX_DATA *pObjIndex;

	for (pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
		  pObjIndex != NULL;
		  pObjIndex  = pObjIndex->next)
	{
		if (pObjIndex->vnum == vnum)
		    return pObjIndex;
	}

	if (fBootDb)
	{
		bug("Get_obj_index: bad vnum %d.", vnum);
		exit(1);
	}

	return NULL;
}



/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index(int vnum)
{
	ROOM_INDEX_DATA *pRoomIndex;

	for (pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
		  pRoomIndex != NULL;
		  pRoomIndex  = pRoomIndex->next)
	{
		if (pRoomIndex->vnum == vnum)
		    return pRoomIndex;
	}

	if (fBootDb)
	{
		bug("Get_room_index: bad vnum %d.", vnum);
		exit(1);
	}

	return NULL;
}


int xgetc(FILE *fp)
{
	int c;

	c = getc(fp);
	if (c == '\n')
		line_number++;
	return c;
}

void xungetc(int c, FILE *fp)
{
	if (c == '\n')
		line_number--;
	ungetc(c, fp);
}

/*
 * Read a letter from a file.
 */
char fread_letter(FILE *fp)
{
	char c;

	do
	{
		c = xgetc(fp);
	}
	while (isspace(c));
	return c;
}



/*
 * Read a number from a file.
 */
int fread_number(FILE *fp)
{
	int number;
	bool sign;
	char c;

	do
	{
		c = xgetc(fp);
	}
	while (isspace(c));

	number = 0;

	sign   = FALSE;
	if (c == '+')
	{
		c = xgetc(fp);
	}
	else if (c == '-')
	{
		sign = TRUE;
		c = xgetc(fp);
	}

	if (!isdigit(c)) {
		db_error("fread_number", "bad format");
		exit(1);
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c      = xgetc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number(fp);
	else if (c != ' ')
		xungetc(c, fp);

	return number;
}

long fread_flags(FILE *fp)
{
	int number;
	char c;
	bool negative = FALSE;

	do
	{
		c = xgetc(fp);
	}
	while (isspace(c));

	if (c == '-')
	{
		negative = TRUE;
		c = xgetc(fp);
	}

	number = 0;

	if (!isdigit(c))
	{
		while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
		{
		    number += flag_convert(c);
		    c = xgetc(fp);
		}
	}

	while (isdigit(c))
	{
		number = number * 10 + c - '0';
		c = xgetc(fp);
	}

	if (c == '|')
		number += fread_flags(fp);

	else if  (c != ' ')
		xungetc(c,fp);

	if (negative)
		return -1 * number;

	return number;
}

long flag_convert(char letter)
{
	long bitsum = 0;
	char i;

	if ('A' <= letter && letter <= 'Z') 
	{
		bitsum = 1;
		for (i = letter; i > 'A'; i--)
		    bitsum *= 2;
	}
	else if ('a' <= letter && letter <= 'z')
	{
		bitsum = 67108864; /* 2^26 */
		for (i = letter; i > 'a'; i --)
		    bitsum *= 2;
	}

	return bitsum;
}




/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string(FILE *fp)
{
	char *plast;
	char c;

	plast = top_string + sizeof(char *);
	if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH])
	{
		bug("Fread_string: MAX_STRING %d exceeded.", MAX_STRING);
		exit(1);
	}

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
	{
		c = xgetc(fp);
	}
	while (isspace(c));

	if ((*plast++ = c) == '~')
		return &str_empty[0];

	for (;;)
	{
		/*
		 * Back off the char type lookup,
		 *   it was too dirty for portability.
		 *   -- Furey
		 */

		switch (*plast = xgetc(fp))
		{
		default:
		    plast++;
		    break;
 
		case EOF:
		    bug("Fread_string: EOF", 0);
		    return str_empty;
		    break;
 
		case '\n':
		    plast++;
		    *plast++ = '\r';
		    break;
 
		case '\r':
		    break;
 
		case '~':
		    plast++;
		    {
			union
			{
			    char *	pc;
			    char	rgc[sizeof(char *)];
			} u1;
			int ic;
			int iHash;
			char *pHash;
			char *pHashPrev;
			char *pString;

			plast[-1] = '\0';
			iHash     = UMIN(MAX_KEY_HASH - 1, plast - 1 - top_string);
			for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev)
			{
			    for (ic = 0; ic < sizeof(char *); ic++)
				u1.rgc[ic] = pHash[ic];
			    pHashPrev = u1.pc;
			    pHash    += sizeof(char *);

			    if (top_string[sizeof(char *)] == pHash[0]
			    &&   !strcmp(top_string+sizeof(char *)+1, pHash+1))
				return pHash;
			}

			if (fBootDb)
			{
			    pString		= top_string;
			    top_string		= plast;
			    u1.pc		= string_hash[iHash];
			    for (ic = 0; ic < sizeof(char *); ic++)
				pString[ic] = u1.rgc[ic];
			    string_hash[iHash]	= pString;

			    nAllocString += 1;
			    sAllocString += top_string - pString;
			    return pString + sizeof(char *);
			}
			else
			{
			    return str_dup(top_string + sizeof(char *));
			}
		    }
		}
	}
}

char *fread_string_eol(FILE *fp)
{
	static bool char_special[256-EOF];
	char *plast;
	char c;
 
	if (char_special[EOF-EOF] != TRUE)
	{
		char_special[EOF -  EOF] = TRUE;
		char_special['\n' - EOF] = TRUE;
		char_special['\r' - EOF] = TRUE;
	}
 
	plast = top_string + sizeof(char *);
	if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH])
	{
		bug("Fread_string: MAX_STRING %d exceeded.", MAX_STRING);
		exit(1);
	}
 
	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
	{
		c = xgetc(fp);
	}
	while (isspace(c));
 
	if ((*plast++ = c) == '\n')
		return &str_empty[0];
 
	for (;;)
	{
		if (!char_special[ (*plast++ = xgetc(fp)) - EOF ])
		    continue;
 
		switch (plast[-1])
		{
		default:
		    break;
 
		case EOF:
		    bug("Fread_string_eol  EOF", 0);
		    exit(1);
		    break;
 
		case '\n':  case '\r':
		    {
		        union
		        {
		            char *      pc;
		            char        rgc[sizeof(char *)];
		        } u1;
		        int ic;
		        int iHash;
		        char *pHash;
		        char *pHashPrev;
		        char *pString;
 
		        plast[-1] = '\0';
		        iHash     = UMIN(MAX_KEY_HASH - 1, plast - 1 - top_string);
		        for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev)
		        {
		            for (ic = 0; ic < sizeof(char *); ic++)
		                u1.rgc[ic] = pHash[ic];
		            pHashPrev = u1.pc;
		            pHash    += sizeof(char *);
 
		            if (top_string[sizeof(char *)] == pHash[0]
		            &&   !strcmp(top_string+sizeof(char *)+1, pHash+1))
		                return pHash;
		        }
 
		        if (fBootDb)
		        {
		            pString             = top_string;
		            top_string          = plast;
		            u1.pc               = string_hash[iHash];
		            for (ic = 0; ic < sizeof(char *); ic++)
		                pString[ic] = u1.rgc[ic];
		            string_hash[iHash]  = pString;
 
		            nAllocString += 1;
		            sAllocString += top_string - pString;
		            return pString + sizeof(char *);
		        }
		        else
		        {
		            return str_dup(top_string + sizeof(char *));
		        }
		    }
		}
	}
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE *fp)
{
	char c;

	do
		c = xgetc(fp);
	while (c != '\n' && c != '\r');

	do
		c = xgetc(fp);
	while (c == '\n' || c == '\r');

	xungetc(c, fp);
	return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE *fp)
{
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	do
		cEnd = xgetc(fp);
	while (isspace(cEnd));

	if (cEnd == '\'' || cEnd == '"')
		pword   = word;
	else {
		word[0] = cEnd;
		pword   = word+1;
		cEnd    = ' ';
	}

	for (; pword < word + MAX_INPUT_LENGTH; pword++)
	{
		*pword = xgetc(fp);
		if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd)
		{
		    if (cEnd == ' ')
			xungetc(*pword, fp);
		    *pword = '\0';
		    return word;
		}
	}

	bug("Fread_word: word too long.", 0);
	exit(1);
	return NULL;
}

/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem(int sMem)
{
	void *pMem;
	int *magic;
	int iList;

	sMem += sizeof(*magic);

	for (iList = 0; iList < MAX_MEM_LIST; iList++)
	{
		if (sMem <= rgSizeList[iList])
		    break;
	}

	if (iList == MAX_MEM_LIST)
	{
		bug("Alloc_mem: size %d too large.", sMem);
		exit(1);
	}

	if (rgFreeList[iList] == NULL)
	{
		pMem              = alloc_perm(rgSizeList[iList]);
	}
	else
	{
		pMem              = rgFreeList[iList];
		rgFreeList[iList] = * ((void **) rgFreeList[iList]);
	}

	magic = (int *) pMem;
	*magic = MAGIC_NUM;
	pMem += sizeof(*magic);

	return pMem;
}



/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem(void *pMem, int sMem)
{
	int iList;
	int *magic;

	pMem -= sizeof(*magic);
	magic = (int *) pMem;

	if (*magic != MAGIC_NUM)
	{
		bug("Attempt to recyle invalid memory of size %d.",sMem);
		bug((char*) pMem + sizeof(*magic),0);
		return;
	}

	*magic = 0;
	sMem += sizeof(*magic);

	for (iList = 0; iList < MAX_MEM_LIST; iList++)
	{
		if (sMem <= rgSizeList[iList])
		    break;
	}

	if (iList == MAX_MEM_LIST)
	{
		bug("Free_mem: size %d too large.", sMem);
		exit(1);
	}

	* ((void **) pMem) = rgFreeList[iList];
	rgFreeList[iList]  = pMem;

	return;
}


/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm(int sMem)
{
	static char *pMemPerm;
	static int iMemPerm;
	void *pMem;

	while (sMem % sizeof(long) != 0)
		sMem++;
	if (sMem > MAX_PERM_BLOCK)
	{
		bug("Alloc_perm: %d too large.", sMem);
		exit(1);
	}

	if (pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK)
	{
		iMemPerm = 0;
		if ((pMemPerm = calloc(1, MAX_PERM_BLOCK)) == NULL)
		{
		    perror("Alloc_perm");
		    exit(1);
		}
	}

	pMem        = pMemPerm + iMemPerm;
	iMemPerm   += sMem;
	nAllocPerm += 1;
	sAllocPerm += sMem;
	return pMem;
}



/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup(const char *str)
{
	char *str_new;

	if (str == NULL)
		return NULL;

	if (str[0] == '\0')
		return &str_empty[0];

	if (str >= string_space && str < top_string)
		return (char *) str;

	str_new = alloc_mem(strlen(str) + 1);
	strcpy(str_new, str);
	return str_new;
}


char *str_add(const char *str,...)
{
	va_list ap;
	size_t len;
	char *p;
	char *str_new;

	/* calculate length of sum */
	va_start(ap, str);
	len = strlen(str);
	while ((p = va_arg(ap, char*)) != NULL)
		len += strlen(p);
	va_end(ap);

	/* cat them */
	str_new = alloc_mem(len + 1);
	strcpy(str_new, str);
	va_start(ap, str);
	while ((p = va_arg(ap, char*)) != NULL)
		strcat(str_new, p);
	va_end(ap);

	return str_new;
}

/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string(char *pstr)
{
	if (pstr == NULL
	||  pstr == &str_empty[0]
	||  (pstr >= string_space && pstr < top_string))
		return;

	free_mem(pstr, strlen(pstr) + 1);
	return;
}


/*
 * str_printf -- like sprintf, but prints into string.
 *		 the format is string itself
 */
void str_printf(char** pstr,...)
{
	va_list ap;
	char buf[MAX_STRING_LENGTH];

	if (*pstr == NULL)
		return;

	va_start(ap, pstr);
	vsnprintf(buf, sizeof(buf), *pstr, ap);
	va_end(ap);

        free_string(*pstr);
        *pstr = str_dup(buf);
}

void do_areas(CHAR_DATA *ch, const char *argument)
{
	AREA_DATA *pArea1;
	AREA_DATA *pArea2;
	int iArea;
	int iAreaHalf;
	BUFFER *output;

	if (argument[0] != '\0') {
		send_to_char("No argument is used with this command.\n\r",ch);
		return;
	}

	iAreaHalf = (top_area + 1) / 2;
	pArea1    = area_first;
	pArea2    = area_first;
	for (iArea = 0; iArea < iAreaHalf; iArea++)
		pArea2 = pArea2->next;

	output = buf_new(0);
	buf_add(output, "Current areas of Muddy MUD: \n\r");
	for (iArea = 0; iArea < iAreaHalf; iArea++) {
		buf_printf(output,"{W{{{x%2d %3d{W} {B%-20s {C%8s{x ",
			pArea1->low_range,pArea1->high_range,
			pArea1->name,
			pArea1->credits);

		if (pArea2 != NULL) 
			buf_printf(output,"{W{{{x%2d %3d{W} {B%-20s {C%8s{x",
				pArea2->low_range,pArea2->high_range,
				pArea2->name,
				pArea2->credits);
		buf_add(output, "\n\r");

		pArea1 = pArea1->next;
		if (pArea2 != NULL)
			pArea2 = pArea2->next;
	}

	buf_add(output,"\n\r");	
	page_to_char(buf_string(output), ch);	
	buf_free(output);
}



void do_memory(CHAR_DATA *ch, const char *argument)
{
	char_printf(ch, "Affects %5d\n\r", top_affect  );
	char_printf(ch, "Areas   %5d\n\r", top_area    );
	char_printf(ch, "ExDes   %5d\n\r", top_ed      );
	char_printf(ch, "Exits   %5d\n\r", top_exit    );
	char_printf(ch, "Helps   %5d\n\r", top_help    );
	char_printf(ch, "Socials %5d\n\r", social_count);
	char_printf(ch, "Mobs    %5d(%d new format)\n\r",
				top_mob_index,newmobs); 
	char_printf(ch, "(in use)%5d\n\r", mobile_count);
	char_printf(ch, "Objs    %5d(%d new format)\n\r",
				top_obj_index,newobjs); 
	char_printf(ch, "Resets  %5d\n\r", top_reset   );
	char_printf(ch, "Rooms   %5d\n\r", top_room    );
	char_printf(ch, "Shops   %5d\n\r", top_shop    );
	char_printf(ch, "Strings %5d strings of %7d bytes (max %d).\n\r",
		nAllocString, sAllocString, MAX_STRING);
	char_printf(ch, "Perms   %5d blocks  of %7d bytes.\n\r",
		nAllocPerm, sAllocPerm);

	return;
}

void do_dump(CHAR_DATA *ch, const char *argument)
{
	int count,count2,num_pcs,aff_count;
	CHAR_DATA *fch;
	MOB_INDEX_DATA *pMobIndex;
	PC_DATA *pc;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	ROOM_INDEX_DATA *room;
	EXIT_DATA *exit;
	DESCRIPTOR_DATA *d;
	AFFECT_DATA *af;
	FILE *fp;
	int vnum,nMatch = 0;

	/* open file */
	fclose(fpReserve);
	fp = fopen("mem.dmp","w");

	/* report use of data structures */
	
	num_pcs = 0;
	aff_count = 0;

	/* mobile prototypes */
	fprintf(fp,"MobProt	%4d (%8d bytes)\n",
		top_mob_index, top_mob_index * (sizeof(*pMobIndex))); 

	/* mobs */
	count = 0;  count2 = 0;
	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		count++;
		if (fch->pcdata != NULL)
		    num_pcs++;
		for (af = fch->affected; af != NULL; af = af->next)
		    aff_count++;
	}
	for (fch = char_free; fch != NULL; fch = fch->next)
		count2++;

	fprintf(fp,"Mobs	%4d (%8d bytes), %2d free (%d bytes)\n",
		count, count * (sizeof(*fch)), count2, count2 * (sizeof(*fch)));

	/* pcdata */
	count = 0;
	for (pc = pcdata_free; pc != NULL; pc = pc->next)
		count++; 

	fprintf(fp,"Pcdata	%4d (%8d bytes), %2d free (%d bytes)\n",
		num_pcs, num_pcs * (sizeof(*pc)), count, count * (sizeof(*pc)));

	/* descriptors */
	count = 0; count2 = 0;
	for (d = descriptor_list; d != NULL; d = d->next)
		count++;
	for (d= descriptor_free; d != NULL; d = d->next)
		count2++;

	fprintf(fp, "Descs	%4d (%8d bytes), %2d free (%d bytes)\n",
		count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

	/* object prototypes */
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
		    for (af = pObjIndex->affected; af != NULL; af = af->next)
			aff_count++;
		    nMatch++;
		}

	fprintf(fp,"ObjProt	%4d (%8d bytes)\n",
		top_obj_index, top_obj_index * (sizeof(*pObjIndex)));


	/* objects */
	count = 0;  count2 = 0;
	for (obj = object_list; obj != NULL; obj = obj->next)
	{
		count++;
		for (af = obj->affected; af != NULL; af = af->next)
		    aff_count++;
	}
	for (obj = obj_free; obj != NULL; obj = obj->next)
		count2++;

	fprintf(fp,"Objs	%4d (%8d bytes), %2d free (%d bytes)\n",
		count, count * (sizeof(*obj)), count2, count2 * (sizeof(*obj)));

	/* affects */
	count = 0;
	for (af = affect_free; af != NULL; af = af->next)
		count++;

	fprintf(fp,"Affects	%4d (%8d bytes), %2d free (%d bytes)\n",
		aff_count, aff_count * (sizeof(*af)), count, count * (sizeof(*af)));

	/* rooms */
	fprintf(fp,"Rooms	%4d (%8d bytes)\n",
		top_room, top_room * (sizeof(*room)));

	 /* exits */
	fprintf(fp,"Exits	%4d (%8d bytes)\n",
		top_exit, top_exit * (sizeof(*exit)));

	fclose(fp);

	/* start printing out mobile data */
	fp = fopen("mob.dmp","w");

	fprintf(fp,"\nMobile Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_mob_index; vnum++)
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
		{
		    nMatch++;
		    fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
			pMobIndex->vnum,pMobIndex->count,
			pMobIndex->killed,mlstr_mval(pMobIndex->short_descr));
		}
	fclose(fp);

	/* start printing out object data */
	fp = fopen("obj.dmp","w");

	fprintf(fp,"\nObject Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
		    nMatch++;
		    fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
			pObjIndex->vnum,pObjIndex->count,
			pObjIndex->reset_num,
			mlstr_mval(pObjIndex->short_descr));
		}

	/* close file */
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
}



/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy(int number)
{
	switch (number_bits(2))
	{
	case 0:  number -= 1; break;
	case 3:  number += 1; break;
	}

	return UMAX(1, number);
}



/*
 * Generate a random number.
 */
int number_range(int from, int to)
{
	int power;
	int number;

	if (from == 0 && to == 0)
		return 0;

	if ((to = to - from + 1) <= 1)
		return from;

	for (power = 2; power < to; power <<= 1)
		;

	while ((number = number_mm() & (power -1)) >= to)
		;

	return from + number;
}



/*
 * Generate a percentile roll.
 */
int number_percent(void)
{
	int percent;

	while ((percent = number_mm() & (128-1)) > 99)
		;

	return 1 + percent;
}



/*
 * Generate a random door.
 */
int number_door(void)
{
	int door;

	while ((door = number_mm() & (8-1)) > 5)
		;

	return door;
}

int number_bits(int width)
{
	return number_mm() & ((1 << width) - 1);
}




/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
	back to the system srandom call.  If this doesn't work for you, 
	define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2+55];
#endif
 
void init_mm()
{
#if defined (OLD_RAND)
	int *piState;
	int iState;
 
	piState     = &rgiState[2];
 
	piState[-2] = 55 - 55;
	piState[-1] = 55 - 24;
 
	piState[0]  = ((int) current_time) & ((1 << 30) - 1);
	piState[1]  = 1;
	for (iState = 2; iState < 55; iState++)
	{
		piState[iState] = (piState[iState-1] + piState[iState-2])
		                & ((1 << 30) - 1);
	}
#else
	srandom(time(NULL)^getpid());
#endif
	return;
}
 
 
 
long number_mm(void)
{
#if defined (OLD_RAND)
	int *piState;
	int iState1;
	int iState2;
	int iRand;
 
	piState             = &rgiState[2];
	iState1             = piState[-2];
	iState2             = piState[-1];
	iRand               = (piState[iState1] + piState[iState2])
		                & ((1 << 30) - 1);
	piState[iState1]    = iRand;
	if (++iState1 == 55)
		iState1 = 0;
	if (++iState2 == 55)
		iState2 = 0;
	piState[-2]         = iState1;
	piState[-1]         = iState2;
	return iRand >> 6;
#else
	return random() >> 6;
#endif
}


/*
 * Roll some dice.
 */
int dice(int number, int size)
{
	int idice;
	int sum;

	switch (size)
	{
	case 0: return 0;
	case 1: return number;
	}

	for (idice = 0, sum = 0; idice < number; idice++)
		sum += number_range(1, size);

	return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate(int level, int value_00, int value_32)
{
	return value_00 + level * (value_32 - value_00) / 32;
}



/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde(char *str)
{
		for (; *str; str++) 
			if (*str == '~') *str = '-';
}

void smash_percent(char* str)
{
		for (; *str; str++)
			if (*str == '%')
				if (*(str+1) == '%')
					str++;
				else
					*str = '#';
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp(const char *astr, const char *bstr)
{
	if (astr == NULL)
	{
		bug("Str_cmp: null astr.", 0);
		return TRUE;
	}

	if (bstr == NULL)
	{
		bug("Str_cmp: null bstr.", 0);
		return TRUE;
	}

	for (; *astr || *bstr; astr++, bstr++)
	{
		if (LOWER(*astr) != LOWER(*bstr))
		    return TRUE;
	}

	return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const char *astr, const char *bstr)
{
	if (astr == NULL)
	{
		bug("Strn_cmp: null astr.", 0);
		return TRUE;
	}

	if (bstr == NULL)
	{
		bug("Strn_cmp: null bstr.", 0);
		return TRUE;
	}

	for (; *astr; astr++, bstr++)
	{
		if (LOWER(*astr) != LOWER(*bstr))
		    return TRUE;
	}

	return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const char *astr, const char *bstr)
{
	int sstr1;
	int sstr2;
	int ichar;
	char c0;

	if ((c0 = LOWER(astr[0])) == '\0')
		return FALSE;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
	{
		if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
		    return FALSE;
	}

	return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const char *astr, const char *bstr)
{
	int sstr1;
	int sstr2;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);
	if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
		return FALSE;
	else
		return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize(const char *str)
{
	static char strcap[MAX_STRING_LENGTH];
	int i;

	for (i = 0; str[i] != '\0'; i++)
		strcap[i] = LOWER(str[i]);
	strcap[i] = '\0';
	strcap[0] = UPPER(strcap[0]);
	return strcap;
}


/*
 * Append a string to a file.
 */
void append_file(CHAR_DATA *ch, const char *file, const char *str)
{
	FILE *fp;

	if (IS_NPC(ch) || str[0] == '\0')
		return;

	fclose(fpReserve);
	if ((fp = fopen(file, "a")) == NULL)
	{
		perror(file);
		send_to_char("Could not open the file!\n\r", ch);
	}
	else
	{
		fprintf(fp, "[%5d] %s: %s\n",
		    ch->in_room ? ch->in_room->vnum : 0, ch->name, str);
		fclose(fp);
	}

	fpReserve = fopen(NULL_FILE, "r");
	return;
}




/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain(void)
{
	return;
}


void load_olimits(FILE *fp)
{
	int vnum;
	int limit;
	char ch;
	OBJ_INDEX_DATA *pIndex;

	for (ch = fread_letter(fp); ch != 'S'; ch = fread_letter(fp))
	{
	  switch(ch)
		{
		case 'O':
		  vnum = fread_number(fp);
		  limit = fread_number(fp);
		  if ((pIndex = get_obj_index(vnum)) == NULL)
		    {
		      bug("Load_olimits: bad vnum %d",vnum);
		      exit(1);
		    }
		  else pIndex->limit = limit;
		  break;

		case '*':
		  fread_to_eol(fp);
		  break;
		default:
		  bug("Load_olimits: bad command '%c'",ch);
		  exit(1);
		}
	}
}


/*
 * Add the objects in players not logged on to object count 
 */
void load_limited_objects()
{
	struct dirent *dp;

	int i;
	DIR *dirp;
	FILE *pfile;
	char letter;
	char *word;
	char buf[PATH_MAX]; 
	bool fReadLevel;
	char buf2[160];
	int vnum;

	total_levels = 0;


	if ((dirp = opendir(PLAYER_DIR)) == NULL) {
		bug("Load_limited_objects: unable to open player directory.",
		    0);
		exit(1);
	}

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		char* pname;

		if (dp->d_namlen < 3)
			continue;

		snprintf(buf, sizeof(buf), "%s%s", PLAYER_DIR, dp->d_name);
		fReadLevel = FALSE;

		if ((pfile = fopen(buf, "r")) == NULL) {
			bug("Load_limited_objects: Can't open player file.", 0);
			continue;
		}

		pname = NULL;
		for (letter = fread_letter(pfile); letter != EOF;
						letter = fread_letter(pfile)) {
			if (letter == 'L') {
				if (!fReadLevel) {
				
				word = fread_word(pfile);
			      
				if (!str_cmp(word, "evl")
				||  !str_cmp(word,"ev")
				||  !str_cmp(word, "evel")) {
					i = fread_number(pfile);
					fReadLevel = TRUE;
					total_levels += UMAX(0,i - 5);
					snprintf(buf2, sizeof(buf2),
						 "[%s]'s file +: %d\n\r",
						 buf, UMAX(0, i-5));
					dump_to_scr(buf2);
					continue;
				}
				}
			}
			else if (letter == '#') {
				word = fread_word(pfile);

				if (!str_cmp(word, "O")
				||  !str_cmp(word, "OBJECT")) {
				  fread_word(pfile); 
				  fBootDb = FALSE;
				  vnum = fread_number(pfile);
				  if (get_obj_index(vnum) != NULL)
				  	get_obj_index(vnum)->count++;
				  fBootDb = TRUE;
				}
			} else if (letter == 'P') {
				if (!strcmp(fread_word(pfile), "C_Killed")) {
					if (pname == NULL) {
						bug("load_limited_objects: "
						    "PC_Killed before Name "
						    "in pfile", 0);
						exit(1);
					}

					rating_add(pname, fread_number(pfile));
				}	
			} 
			else if (letter == 'N') {
				if (strcmp(fread_word(pfile), "ame") == 0
				&&  pname == NULL)
					pname = fread_string(pfile);
			}
		}

		free_string(pname);
		fclose(pfile);
	}
	closedir(dirp);
}

/*
 * Snarf can prac declarations.
 */
void load_practicer(FILE *fp)
{
	for (; ;) {
		MOB_INDEX_DATA *pMobIndex;
		char letter;
		char *gname;
		int group;

		switch (letter = fread_letter(fp)) {
		default:
			log_printf("load_practicer: letter '%c' not *MS.",
				   letter);
			exit(1);

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			pMobIndex = get_mob_index(fread_number(fp));
			gname = fread_word(fp);
			if ((group = flag_lookup(gname, skill_groups)) == 0) {
				log_printf("load_practicer: 'M': vnum %d: "
					   "unknown group '%s'",
					   pMobIndex->vnum, gname);
				exit(1);
			}
			SET_BIT(pMobIndex->practicer, group);
			break;
		}

		fread_to_eol(fp);
	}
}


void load_resetmsg(FILE *fp)
{
	mlstr_fread(fp, current_area->resetmsg);
}

void load_aflag(FILE *fp)
{
	current_area->area_flag = fread_flags(fp);
}


char *format_flags(int flags)
{
	int count, pos = 0;
	static char buf[53];

	for (count = 0; count < 32;  count++)
	{
	    if (IS_SET(flags,1<<count))
	    {
	        if (count < 26)
	            buf[pos] = 'A' + count;
	        else
	            buf[pos] = 'a' + (count - 26);
	        pos++;
	    }
	}

	if (pos == 0)
	{
	    buf[pos] = '0';
	    pos++;
	}

	buf[pos] = '\0';

	return buf;
}


MPROG_CODE *get_mprog_index(int vnum)
{
	MPROG_CODE *prg;
	for (prg = mprog_list; prg; prg = prg->next) {
	    	if (prg->vnum == vnum)
        		return(prg);
	}
	return NULL;
}    
 
void db_error(const char* fn, const char* fmt,...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	log_printf("%s: line %d: %s: %s",
		   current_area == NULL ? "???" : current_area->file_name,
		   line_number, fn, buf);
	exit(1);
}

/*****************************************************************************
 Name:		fix_string
 Purpose:	Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string(const char *str)
{
    static char strfix[MAX_STRING_LENGTH * 2];
    int i;
    int o;

    if (str == NULL)
        return '\0';

    for (o = i = 0; str[i+o] != '\0'; i++)
    {
        if (str[i+o] == '\r' || str[i+o] == '~')
            o++;
        strfix[i] = str[i+o];
    }
    strfix[i] = '\0';
    return strfix;
}

