/*
 * $Id: recycle.c,v 1.164 2004-02-11 22:56:17 fjoe Exp $
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

#include <sys/time.h>
#include <assert.h>
#include <dlfcn.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <merc.h>
#include <rwfile.h>
#include <module.h>
#include <module_decl.h>
#include <mprog.h>

flag_t		mud_options;

time_t		boot_time;
time_t		current_time;	/* time of this pulse */

TIME_INFO_DATA	time_info;
WEATHER_DATA	weather_info;
AUCTION_DATA	auction;
rating_t	rating_table[RATING_TABLE_SIZE];

ROOM_INDEX_DATA	*x_room_list = NULL;
CHAR_DATA	*top_affected_char = NULL;
OBJ_DATA	*top_affected_obj = NULL;

int		reboot_counter = 1440;
int		rebooter = 0;

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
const char	*target_name;

int		changed_flags;		/* changed object flags for OLC */

int		top_player;
int		top_vnum_mob;
int		top_vnum_obj;
int		top_vnum_room;

ban_t *		ban_list;

bool		wizlock;		/* Game is wizlocked		*/
bool		newlock;		/* Game is newlocked		*/
bool		merc_down;		/* Shutdown			*/

const char DEFAULT_PROMPT[] = "%hhp %mm %vmv Opp:%o {c%e{x# ";	// notrans

RUNGAME_FUN *run_game;
RUNGAME_FUN *run_game_bottom;

bool (*olc_interpret)(DESCRIPTOR_DATA *d, const char *argument);

fd_set	in_set;
fd_set	out_set;
fd_set	exc_set;

/*
 * Log-all switch.
 */
bool		fLogAll	= FALSE;

/*
 * paths
 */
const char TMP_PATH		[] = "tmp";		// notrans
const char PLAYER_PATH		[] = "player";		// notrans
const char GODS_PATH		[] = "gods";		// notrans
const char NOTES_PATH		[] = "notes";		// notrans
const char ETC_PATH		[] = "etc";		// notrans
const char AREA_PATH		[] = "area";		// notrans
const char LANG_PATH		[] = "lang";		// notrans
const char MODULES_PATH		[] = "modules";		// notrans
const char MPC_PATH		[] = "mpc";		// notrans

const char CLASSES_PATH		[] = "classes";		// notrans
const char CLANS_PATH		[] = "clans";		// notrans
const char RACES_PATH		[] = "races";		// notrans
const char SPEC_PATH		[] = "specs";		// notrans

const char RACE_EXT		[] = ".race";		// notrans
const char CLASS_EXT		[] = ".class";		// notrans
const char CLAN_EXT		[] = ".clan";		// notrans
const char SPEC_EXT		[] = ".spec";		// notrans
const char MPC_EXT		[] = ".mpc";		// notrans

#if defined (WIN32)
const char PLISTS_PATH		[] = "clans\\plists";	// notrans
const char NULL_FILE		[] = "NUL";		// notrans
#else
const char PLISTS_PATH		[] = "clans/plists";	// notrans
const char NULL_FILE		[] = "/dev/null";	// notrans
#endif

const char TMP_FILE		[] = "romtmp";		// notrans

const char MODULES_CONF		[] = "modules.conf";	// notrans
const char HOMETOWNS_CONF	[] = "hometowns.conf";	// notrans
const char SKILLS_CONF		[] = "skills.conf";	// notrans
const char SOCIALS_CONF		[] = "socials.conf";	// notrans
const char SYSTEM_CONF		[] = "system.conf";	// notrans
const char CMD_CONF		[] = "cmd.conf";	// notrans
const char DAMTYPES_CONF	[] = "damtypes.conf";	// notrans
const char MATERIALS_CONF	[] = "materials.conf";	// notrans
const char LIQUIDS_CONF		[] = "liquids.conf";	// notrans
const char UHANDLERS_CONF	[] = "uhandlers.conf";	// notrans
const char FORMS_CONF		[] = "forms.conf";	// notrans
const char EFFECTS_CONF		[] = "effects.conf";	// notrans

const char GLOB_GMLSTR_FILE	[] = "glob_gmlstr";	// notrans
const char MSGDB_FILE		[] = "msgdb";		// notrans
const char HINTS_FILE		[] = "hints";		// notrans

const char AREA_LIST		[] = "area.lst";	// notrans
const char LANG_LIST		[] = "lang.lst";	// notrans

const char NOTE_FILE		[] = "notes.not";	// notrans
const char IDEA_FILE		[] = "ideas.not";	// notrans
const char PENALTY_FILE		[] = "penal.not";	// notrans
const char NEWS_FILE		[] = "news.not";	// notrans
const char CHANGES_FILE		[] = "chang.not";	// notrans
const char SHUTDOWN_FILE	[] = "shutdown";	// notrans
const char EQCHECK_FILE		[] = "eqcheck";		// notrans
const char EQCHECK_SAVE_ALL_FILE[] = "eqcheck_save_all";// notrans

const char BAN_FILE		[] = "ban.txt";		// notrans
const char MAXON_FILE		[] = "maxon.txt";	// notrans
const char AREASTAT_FILE	[] = "areastat.txt";	// notrans
const char IMMLOG_FILE		[] = "immortals.log";	// notrans

const char *dir_name[] =
{
	"north", "east", "south", "west", "up", "down"
};

const char *from_dir_name[] =
{
	"the north", "the east", "the south", "the west", "the up", "the down"
};

const int rev_dir[] =
{
	2, 3, 0, 1, 5, 4
};

const int show_order[] = {
	WEAR_LIGHT,
	WEAR_FINGER_L,
	WEAR_FINGER_R,
	WEAR_NECK,
	WEAR_FACE,
	WEAR_BODY,
	WEAR_HEAD,
	WEAR_LEGS,
	WEAR_FEET,
	WEAR_HANDS,
	WEAR_ARMS,
	WEAR_SHIELD,
	WEAR_ABOUT,
	WEAR_WAIST,
	WEAR_WRIST_L,
	WEAR_WRIST_R,
	WEAR_WIELD,
	WEAR_SECOND_WIELD,
	WEAR_HOLD,
	WEAR_FLOAT,
	WEAR_TATTOO,
	WEAR_CLANMARK,
	-1
};

/*--------------------------------------------------------------------
 * ED_DATA
 */

int ed_count;

ED_DATA *
ed_new(void)
{
	ED_DATA *ed;
	ed = calloc(1, sizeof(*ed));
	ed_count++;
	return ed;
}

ED_DATA *
ed_new2(const ED_DATA *ed, const char* name)
{
	ED_DATA *ed2 = ed_new();
	ed2->keyword = str_qdup(ed->keyword);
	mlstr_printf(&ed2->description, &ed->description, name);
	return ed2;
}

void
ed_free(ED_DATA *ed)
{
	ED_DATA *ed_next;

	for (; ed; ed = ed_next) {
		ed_next = ed->next;

		free_string(ed->keyword);
		mlstr_destroy(&ed->description);
		free(ed);
		ed_count--;
	}
}

ED_DATA *
ed_dup(const ED_DATA *ed)
{
	ED_DATA *ned = NULL;
	ED_DATA **ped = &ned;

	for (; ed; ed = ed->next) {
		*ped = ed_new();
		(*ped)->keyword = str_qdup(ed->keyword);
		mlstr_cpy(&(*ped)->description, &ed->description);
		ped = &(*ped)->next;
	}

	return ned;
}

/*
 * Get an extra description from a list.
 */
ED_DATA *
ed_lookup(const char *name, ED_DATA *ed)
{
	int num;
	char arg[MAX_INPUT_LENGTH];

	num = number_argument(name, arg, sizeof(arg));
	for (; ed != NULL; ed = ed->next) {
		if (arg[0] && !is_name(arg, ed->keyword))
			continue;
		if (!--num)
			return ed;
	}
	return NULL;
}

void
ed_fread(rfile_t *fp, ED_DATA **edp)
{
	ED_DATA *ed	= ed_new();
	ed->keyword	= fread_string(fp);
	mlstr_fread(fp, &ed->description);
	SLIST_ADD(ED_DATA, *edp, ed);
}

void
ed_fwrite(FILE *fp, ED_DATA *ed)
{
	fprintf(fp, "E\n%s~\n", fix_string(ed->keyword));
	mlstr_fwrite(fp, NULL, &ed->description);
}

/*--------------------------------------------------------------------
 * var_t
 */

avltree_info_t c_info_vars =
{
	&avltree_ops,

	(e_init_t) var_init,
	(e_destroy_t) var_destroy,

	MT_PVOID, sizeof(var_t), ke_cmp_str
};

void
var_init(var_t *var)
{
	var->name = str_empty;
	var->type_tag = MT_INT;
	var->value.i = 0;
	var->var_flags = 0;
}

void
var_destroy(var_t *var)
{
	switch (var->type_tag) {
	case MT_STR:
		free_string(var->value.s);
		break;
	}
}

var_t *
var_get(avltree_t *vars, const char *name, int type_tag, int var_flags)
{
	var_t *var;

	/*
	 * check type
	 */
	if (type_tag != MT_INT
	&&  type_tag != MT_STR)
		return NULL;

	if ((var = (var_t *) c_lookup(vars, name)) == NULL) {
		var = c_insert(vars, name);
		assert(var != NULL);

		var->name = str_dup(name);
		var->type_tag = type_tag;
		return var;
	} else if (var->type_tag != type_tag)
		return NULL;

	var->var_flags = var_flags;
	return var;
}

void
fread_var(avltree_t *vars, rfile_t *fp)
{
	int type_tag = fread_fword(type_tags, fp);
	const char *name = fread_sword(fp);
	var_t *var;

	var = var_get(vars, name, type_tag, VAR_PERSISTENT);
	free_string(name);

	switch (type_tag) {
	case MT_INT:
		if (var == NULL) {
			fread_to_eol(fp);
			return;
		}
		var->value.i = fread_number(fp);
		break;

	case MT_STR:
		if (var == NULL) {
			free_string(fread_string(fp));
			return;
		}
		var->value.s = fread_string(fp);
		break;

	default:
		assert(0);
		break;
	}
}

void
fwrite_vars(avltree_t *vars, FILE *fp)
{
	var_t *var;

	C_FOREACH(var, vars) {
		if (!IS_SET(var->var_flags, VAR_PERSISTENT))
			continue;

		fprintf(fp, "Var %s '%s' ",
			flag_string(type_tags, var->type_tag), var->name);

		switch (var->type_tag) {
		case MT_INT:
			fprintf(fp, "%d\n", var->value.i);
			break;

		case MT_STR:
			fwrite_string(fp, NULL, var->value.s);
			break;

		default:
			assert(0);
			break;
		}
	}
}

/*--------------------------------------------------------------------
 * OBJ_DATA
 */

OBJ_DATA *object_list;
static OBJ_DATA *free_obj_list;

int obj_count;
int obj_free_count;

OBJ_DATA *
new_obj(void)
{
	OBJ_DATA *obj;

	if (free_obj_list) {
		obj = free_obj_list;
		free_obj_list = free_obj_list->next;
		obj_free_count--;
		mem_validate(obj);
	} else {
		obj = mem_alloc(MT_OBJ, sizeof(*obj));
		obj_count++;
	}

	memset(obj, 0, sizeof(*obj));
	obj->label = str_empty;
	c_init(&obj->vars, &c_info_vars);
	trig_init_list(&obj->mptrig_affected);
	return obj;
}

void
free_obj(OBJ_DATA *obj)
{
	if (!obj)
		return;

	if (!mem_is(obj, MT_OBJ)) {
		printlog(LOG_BUG, "free_obj: obj is not MT_OBJ");
		return;
	}
	mem_invalidate(obj);

	aff_free_list(obj->affected);
	obj->affected = NULL;

	ed_free(obj->ed);
	obj->ed = NULL;

	free_string(obj->label);
	obj->label = str_empty;

	mlstr_destroy(&obj->description);
	mlstr_destroy(&obj->short_descr);
	mlstr_destroy(&obj->owner);

	free_string(obj->material);
	obj->material = NULL;

	objval_destroy(obj->item_type, obj->value);

	c_destroy(&obj->vars);

	obj->next = free_obj_list;
	free_obj_list = obj;

	obj_free_count++;
}

/*--------------------------------------------------------------------
 * CHAR_DATA
 */

CHAR_DATA *char_list;
CHAR_DATA *char_list_lastpc;

static CHAR_DATA *free_npc_list;
int npc_free_count;

static CHAR_DATA *free_pc_list;
int pc_free_count;

int npc_count;
int pc_count;

static void
saff_init(saff_t *sa)
{
	sa->sn = str_empty;
	sa->type = str_empty;
	sa->mod = 0;
	sa->bit = 0;
}

static void
saff_destroy(saff_t *sa)
{
	free_string(sa->sn);
	free_string(sa->type);
}

static varr_info_t c_info_sk_affected =
{
	&varr_ops,

	(e_init_t) saff_init,
	(e_destroy_t) saff_destroy,

	sizeof(saff_t), 1
};

static void
pc_skill_init(pc_skill_t *pc_sk)
{
	pc_sk->sn = str_empty;
	pc_sk->percent = 0;
}

static varr_info_t c_info_learned =
{
	&varr_ops,

	(e_init_t) pc_skill_init,
	strkey_destroy,

	sizeof(pc_skill_t), 8
};

static varr_info_t c_info_pcspecs =
{
	&varr_ops,

	strkey_init,
	strkey_destroy,

	sizeof(const char *), 2
};

static int month_stat_mods[17][MAX_STAT] = {
	{  0,  0,  0,  0, -1,  1 },	// Winter
	{  1, -1, -1,  0,  0, -1 },	// the Winter Wolf
	{  2, -1, -1, -1, +2,  0 },	// the Frost Giant
	{  2,  0,  2,  0,  0,  0 },	// the Old Forces
	{  3,  0,  0,  0,  0,  0 },	// the Grand Struggle
	{  0,  0,  0,  0,  2,  1 },	// the Spring
	{  0,  0,  3,  0,  1,  1 },	// Nature
	{ -1, -1, -1, -1, -1,  0 },	// Futility
	{  2,  0,  1, -1,  2,  1 },	// the Dragon
	{  1,  1,  1,  1,  1,  1 },	// the Sun
	{  1,  0,  0,  0,  0,  0 },	// the Heat
	{  3,  0, -1,  0,  0,  0 },	// the Battle
	{  0,  0,  0,  2,  0,  0 },	// the Dark Shades
	{  0,  1,  0,  2,  0,  0 },	// the Shadows
	{  0,  1,  1,  2,  0,  0 },	// the Long Shadows
	{  0,  0,  3,  0,  0,  0 },	// the Ancient Darkness
	{  3, -2,  0,  0,  0, -3 }	// the Great Evil
};

static int day_stat_mods[7][MAX_STAT] = {
	{  0,  0,  1,  0,  0,  1 },	// the Moon
	{  2, -2, -1, -1,  2, -1 },	// the Bull
	{ -1, +2,  0,  0,  0, +1 },     // Deception
	{  3,  0,  0,  0,  0,  0 },	// Thunder
	{  0,  0,  0,  1,  0,  1 },	// Freedom
	{  1,  0,  2,  0,  1,  2 },	// the Great Gods
	{  1,  1,  1,  1,  1,  2 },     // the Sun
};

CHAR_DATA *
char_new(MOB_INDEX_DATA *pMobIndex)
{
	CHAR_DATA *ch;
	int i;
	int day = (time_info.day + 1) % 7;

	CHAR_DATA **free_list;
	int *free_count;
	int *count;
	size_t size;

	if (pMobIndex) {
		size = sizeof(*ch) + sizeof(NPC_DATA);
		count = &npc_count;
		free_count = &npc_free_count;
		free_list = &free_npc_list;
	} else {
		size = sizeof(*ch) + sizeof(PC_DATA);
		count = &pc_count;
		free_count = &pc_free_count;
		free_list = &free_pc_list;
	}

	if (*free_list) {
		ch = *free_list;
		*free_list = (*free_list)->next;
		(*free_count)--;
		mem_validate(ch);
	} else {
		ch = mem_alloc(MT_CHAR, size);
		(*count)++;
	}

	memset(ch, 0, size);

	ch->last_death_time	= -1;

	SET_HIT(ch, 20);
	SET_MANA(ch, 100);
	SET_MOVE(ch, 100);
	ch->position		= POS_STANDING;
	ch->damtype		= str_empty;
	ch->hitroll		= 0;
	ch->damroll		= 0;
	ch->saving_throw	= 0;
	ch->add_level		= 0;
	ch->race		= str_empty;

	for (i = 0; i < 4; i++)
		ch->armor[i] = 100;
	for (i = 0; i < MAX_STAT; i++) {
		ch->perm_stat[i] = 14 + month_stat_mods[time_info.month][i] +
		    day_stat_mods[day][i];
		ch->mod_stat[i] = 0;
	}

	for (i = 0; i < MAX_RESIST; i++)
		ch->res_mod[i] = 0;

	ch->luck		= 50;
	ch->luck_mod		= 0;

	c_init(&ch->sk_affected, &c_info_sk_affected);

	c_init(&ch->vars, &c_info_vars);
	trig_init_list(&ch->mptrig_affected);

	if (pMobIndex) {
		ch->pMobIndex = pMobIndex;
		ch->chan = CHAN_NOSHOUT | CHAN_NOMUSIC;
	} else {
		PC_DATA *pc = PC(ch);

		pc->add_age = 0;
		pc->logon = current_time;
		pc->version = PFILE_VERSION;
		pc->buffer = buf_new(0);

		c_init(&pc->learned, &c_info_learned);
		c_init(&pc->specs, &c_info_pcspecs);

		pc->pwd = str_empty;
		pc->bamfin = str_empty;
		pc->bamfout = str_empty;
		pc->title = str_empty;
		pc->twitlist = str_empty;
		pc->granted = str_empty;
		pc->form_name = str_empty;
		pc->wanted_by = str_empty;
		pc->dvdata = dvdata_new();
		pc->condition[COND_FULL] = 48;
		pc->condition[COND_THIRST] = 48;
		pc->condition[COND_HUNGER] = 48;
		pc->condition[COND_BLOODLUST] = 48;
		pc->condition[COND_DESIRE] = 48;
		pc->race = str_dup("human");
		pc->clan_status = CLAN_COMMONER;
		pc->plr_flags = PLR_NOSUMMON;
		pc->www_show_flags = WSHOW_RACE | WSHOW_SEX | WSHOW_TITLE;
		pc->bank_s = 0;
		pc->bank_g = 0;
		pc->hints_level = HINT_ALL;
		pc->last_offence_time = -1;
		ch->comm = COMM_COMBINE | COMM_PROMPT;
		pc->ll_time = current_time;
		pc->ll_host = str_empty;
		pc->ll_ip = str_empty;

		msgq_init(&pc->msgq_say, MSGQ_LEN_PERS);
		msgq_init(&pc->msgq_tell, MSGQ_LEN_PERS);
		msgq_init(&pc->msgq_group, MSGQ_LEN_PERS);
		msgq_init(&pc->msgq_sog, MSGQ_LEN_CHAN);
		msgq_init(&pc->msgq_chan, MSGQ_LEN_CHAN);
	}
	RESET_FIGHT_TIME(ch);
	return ch;
}

void
char_free(CHAR_DATA *ch)
{
	CHAR_DATA **free_list;
	int *free_count;

	if (!mem_is(ch, MT_CHAR)) {
		printlog(LOG_BUG, "free_char: ch is not MT_CHAR");
		return;
	}
	mem_invalidate(ch);

	if (IS_NPC(ch)) {
		NPC_DATA *npc = NPC(ch);

		free_count = &npc_free_count;
		free_list = &free_npc_list;

		free_string(npc->in_mind);
		npc->in_mind = NULL;
	} else {
		PC_DATA *pc = PC(ch);

		free_count = &pc_free_count;
		free_list = &free_pc_list;

		/* free pc stuff */
		c_destroy(&pc->specs);
		c_destroy(&pc->learned);

		free_string(pc->race);
		free_string(pc->petition);
		free_string(pc->pwd);
		free_string(pc->bamfin);
		free_string(pc->bamfout);
		free_string(pc->title);
		free_string(pc->twitlist);
		free_string(pc->enemy_list);
		free_string(pc->form_name);
		free_string(pc->granted);
		free_string(pc->wanted_by);
		buf_free(pc->buffer);
		dvdata_free(pc->dvdata);
		free_string(pc->ll_host);
		free_string(pc->ll_ip);

		msgq_destroy(&pc->msgq_say);
		msgq_destroy(&pc->msgq_tell);
		msgq_destroy(&pc->msgq_group);
		msgq_destroy(&pc->msgq_sog);
		msgq_destroy(&pc->msgq_chan);
	}

	if (ch->carrying != NULL) {
		printlog(LOG_BUG, "char_free: ch->carrying != NULL");
		ch->carrying = NULL;
	}

	aff_free_list(ch->affected);
	ch->affected = NULL;

	free_string(ch->name);
	ch->name = NULL;

	mlstr_destroy(&ch->gender);
	mlstr_destroy(&ch->short_descr);
	mlstr_destroy(&ch->long_descr);
	mlstr_destroy(&ch->description);

	free_string(ch->race);
	ch->race = str_empty;

	free_string(ch->class);
	ch->class = str_empty;

	free_string(ch->clan);
	ch->clan = str_empty;

	free_string(ch->material);
	ch->material = NULL;

	free_string(ch->damtype);
	ch->damtype = NULL;

	free(ch->shapeform);
	ch->shapeform = NULL;

	c_destroy(&ch->vars);

	ch->next = *free_list;
	*free_list = ch;

	(*free_count)++;
}

/*--------------------------------------------------------------------
 * RESET_DATA
 */

int reset_count;

RESET_DATA *
reset_new(void)
{
	RESET_DATA *pReset;

	pReset = calloc(1, sizeof(*pReset));
	pReset->command = 'X';

	reset_count++;
	return pReset;
}

void
reset_free(RESET_DATA *pReset)
{
	if (!pReset)
		return;
	reset_count--;
	free(pReset);
}

void
reset_add(ROOM_INDEX_DATA *room, RESET_DATA *reset, RESET_DATA *after)
{
	RESET_DATA *r;

	if (after == NULL) {
		/*
		 * add to the end
		 */

		reset->next = NULL;
		if (room->reset_first == NULL)
			room->reset_first = reset;
		if (room->reset_last == NULL)
			room->reset_last = reset;
		else
			room->reset_last = room->reset_last->next = reset;
		return;
	}

	for (r = room->reset_first; r != NULL; r = r->next) {
		if (r == after)
			break;
	}

	if (r == NULL) {
		printlog(LOG_BUG, "reset_add: `after' reset not found");
		return;
	}

	reset->next = r->next;
	r->next = reset;
	if (reset->next == NULL)
		room->reset_last = reset;
}

void
reset_del(ROOM_INDEX_DATA *room, RESET_DATA *reset)
{
	RESET_DATA *r;
	RESET_DATA *prev = NULL;

	for (r = room->reset_first; r != NULL; r = r->next) {
		if (r == reset)
			break;
		prev = r;
	}

	if (r == NULL)
		return;

	if (prev == NULL)
		room->reset_first = r->next;
	else
		prev->next = r->next;
	if (r->next == NULL)
		room->reset_last = prev;
}

RESET_DATA *
reset_lookup(ROOM_INDEX_DATA *room, int rnum)
{
	RESET_DATA *r;

	for (r = room->reset_first; r != NULL; r = r->next) {
		if (!--rnum)
			break;
	}

	return r;
}

/*--------------------------------------------------------------------
 * AREA_DATA
 */

int area_count;
AREA_DATA *area_first;
AREA_DATA *area_last;

AREA_DATA *
new_area(void)
{
	AREA_DATA *pArea;

	pArea = mem_alloc(MT_AREA, sizeof(*pArea));
	memset(pArea, 0, sizeof(*pArea));
	pArea->vnum		= area_count;
	pArea->file_name	= str_printf("area%d.are", pArea->vnum);
	pArea->builders		= str_empty;
	pArea->name		= str_dup("New area");		// notrans
	pArea->empty		= TRUE;              /* ROM patch */
	pArea->security		= 1;

	area_count++;
	return pArea;
}

void
free_area(AREA_DATA *pArea)
{
	free_string(pArea->name);
	free_string(pArea->file_name);
	free_string(pArea->builders);
	free_string(pArea->credits);
	area_count--;
	mem_free(pArea);
}

AREA_DATA *
area_lookup(int vnum)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next)
		if (pArea->vnum == vnum)
			return pArea;

	return 0;
}

AREA_DATA *
area_vnum_lookup(int vnum)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next) {
		 if (vnum >= pArea->min_vnum
		 &&  vnum <= pArea->max_vnum)
		     return pArea;
	}

	return 0;
}

/*--------------------------------------------------------------------
 * EXIT_DATA
 */

int exit_count;

EXIT_DATA *
new_exit(void)
{
	EXIT_DATA *pExit;

        pExit = calloc(1, sizeof(*pExit));
	pExit->keyword = str_empty;
	gmlstr_init(&pExit->short_descr);
	pExit->size = SIZE_GARGANTUAN;

        exit_count++;
	return pExit;
}

void
free_exit(EXIT_DATA *pExit)
{
	if (!pExit)
		return;

	free_string(pExit->keyword);
	gmlstr_destroy(&pExit->short_descr);

	exit_count--;
	free(pExit);
}

/*--------------------------------------------------------------------
 * ROOM_INDEX_DATA
 */

static void
room_index_init(ROOM_INDEX_DATA *pRoom)
{
	memset(pRoom, 0, sizeof(*pRoom));
	pRoom->heal_rate = 100;
	pRoom->mana_rate = 100;
	trig_init_list(&pRoom->mp_trigs);
	c_init(&pRoom->vars, &c_info_vars);
	trig_init_list(&pRoom->mptrig_affected);
}

static void
room_index_destroy(ROOM_INDEX_DATA *pRoom)
{
	int door;
	RESET_DATA *pReset;

	mlstr_destroy(&pRoom->name);
	mlstr_destroy(&pRoom->description);

	for (door = 0; door < MAX_DIR; door++) {
		if (pRoom->exit[door])
			free_exit(pRoom->exit[door]);
	}

	ed_free(pRoom->ed);

	for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
		reset_free(pReset);

	/*
	 * x_room_del checks triggers so it should be called
	 * before destroying them
	 */
	x_room_del(pRoom);
	trig_destroy_list(&pRoom->mp_trigs);

	c_destroy(&pRoom->vars);
}

avltree_info_t c_info_rooms =
{
	&avltree_ops,

	(e_init_t) room_index_init,
	(e_destroy_t) room_index_destroy,

	MT_ROOM, sizeof(ROOM_INDEX_DATA), ke_cmp_int
};

avltree_t rooms;

ROOM_INDEX_DATA *
get_room_index(int vnum)
{
	return c_lookup(&rooms, &vnum);
}

void
x_room_add(ROOM_INDEX_DATA *room)
{
	ROOM_INDEX_DATA *r;

	if (room->affected == NULL
	&&  !ROOM_HAS_TRIGGER(room, TRIG_ROOM_RANDOM))
		return;

	for (r = x_room_list; r != NULL; r = r->x_next) {
		if (r == room) {
			/* already there */
			return;
		}
	}

	room->x_next = x_room_list;
	x_room_list = room;
}

void
x_room_del(ROOM_INDEX_DATA *room)
{
	ROOM_INDEX_DATA *r;
	ROOM_INDEX_DATA *r_prev = NULL;

	if (room->affected != NULL || ROOM_HAS_TRIGGER(room, TRIG_ROOM_RANDOM))
		return;

	for (r = x_room_list, r_prev = NULL; r != NULL; r = r->x_next) {
		if (r == room)
			break;
		r_prev = r;
	}

	if (r != NULL) {
		if (r_prev == NULL)
			x_room_list = room->x_next;
		else
			r_prev->x_next = room->x_next;
	}
	room->x_next = NULL;
}

/*--------------------------------------------------------------------
 * SHOP_DATA
 */

int shop_count;
SHOP_DATA *shop_first;
SHOP_DATA *shop_last;

SHOP_DATA *
new_shop(void)
{
	SHOP_DATA *pShop;

        pShop = calloc(1, sizeof(*pShop));
	pShop->profit_buy   =   100;
	pShop->profit_sell  =   100;
	pShop->close_hour   =   23;

        shop_count++;
	return pShop;
}

void
free_shop(SHOP_DATA *pShop)
{
	if (!pShop)
		return;
	shop_count--;
	free(pShop);
}

/*--------------------------------------------------------------------
 * OBJ_INDEX_DATA
 */

static void
obj_index_init(OBJ_INDEX_DATA *pObj)
{
	memset(pObj, 0, sizeof(*pObj));
	pObj->name		= str_dup(str_empty);
	pObj->item_type		= ITEM_TRASH;
	objval_init(pObj->item_type, pObj->value);
	pObj->material		= str_dup("unknown");
	pObj->condition		= 100;
	pObj->limit		= -1;
	mlstr_init2(&pObj->gender, flag_string(gender_table, SEX_NEUTRAL));
	trig_init_list(&pObj->mp_trigs);
}

static void
obj_index_destroy(OBJ_INDEX_DATA *pObj)
{
	free_string(pObj->name);
	free_string(pObj->material);
	mlstr_destroy(&pObj->gender);
	mlstr_destroy(&pObj->short_descr);
	mlstr_destroy(&pObj->description);
	aff_free_list(pObj->affected);
	ed_free(pObj->ed);
	objval_destroy(pObj->item_type, pObj->value);
	trig_destroy_list(&pObj->mp_trigs);
}

avltree_info_t c_info_objects =
{
	&avltree_ops,

	(e_init_t) obj_index_init,
	(e_destroy_t) obj_index_destroy,

	MT_OBJ_INDEX, sizeof(OBJ_INDEX_DATA), ke_cmp_int
};

avltree_t objects;

OBJ_INDEX_DATA *
get_obj_index(int vnum)
{
	return c_lookup(&objects, &vnum);
}

/*--------------------------------------------------------------------
 * MOB_INDEX_DATA
 */

static varr_info_t c_info_practicer =
{
	&varr_ops, NULL, NULL,
	sizeof(int), 1
};

static void
mob_index_init(MOB_INDEX_DATA *pMob)
{
	int i;

	memset(pMob, 0, sizeof(*pMob));
	pMob->pShop		= NULL;
	pMob->name		= str_dup(str_empty);
	pMob->race		= str_dup("human");
	pMob->material		= str_dup("unknown");
	pMob->size		= SIZE_MEDIUM;
	pMob->start_pos		= POS_STANDING;
	pMob->default_pos	= POS_STANDING;
	pMob->xp_multiplier	= 100;
	pMob->damtype		= str_empty;
	mlstr_init2(&pMob->gender, flag_string(gender_table, SEX_NEUTRAL));
	for (i = 0; i < MAX_RESIST; i++)
		pMob->resists[i] = RES_UNDEF;
	c_init(&pMob->practicer, &c_info_practicer);
	trig_init_list(&pMob->mp_trigs);
}

static void
mob_index_destroy(MOB_INDEX_DATA *pMob)
{
	free_string(pMob->name);
	free_string(pMob->material);
	free_string(pMob->damtype);
	free_string(pMob->clan);
	free_string(pMob->race);
	mlstr_destroy(&pMob->gender);
	mlstr_destroy(&pMob->short_descr);
	mlstr_destroy(&pMob->long_descr);
	mlstr_destroy(&pMob->description);
	free_shop(pMob->pShop);
	c_destroy(&pMob->practicer);
	aff_free_list(pMob->affected);
	trig_destroy_list(&pMob->mp_trigs);
}

avltree_info_t c_info_mobiles =
{
	&avltree_ops,

	(e_init_t) mob_index_init,
	(e_destroy_t) mob_index_destroy,

	MT_MOB_INDEX, sizeof(MOB_INDEX_DATA), ke_cmp_int
};

avltree_t mobiles;

MOB_INDEX_DATA *
get_mob_index(int vnum)
{
	return c_lookup(&mobiles, &vnum);
}

bool
mob_add_practicer(MOB_INDEX_DATA *pMob, int group)
{
	int *gr;

	if ((gr = varr_bsearch(&pMob->practicer, &group, cmpint)) != NULL)
		return FALSE;

	gr = varr_enew(&pMob->practicer);
	*gr = group;
	varr_qsort(&pMob->practicer, cmpint);
	return TRUE;
}

bool
mob_del_practicer(MOB_INDEX_DATA *pMob, int group)
{
	int *gr;

	if ((gr = varr_bsearch(&pMob->practicer, &group, cmpint)) == NULL)
		return FALSE;

	varr_edelete(&pMob->practicer, gr);
	varr_qsort(&pMob->practicer, cmpint);
	return TRUE;
}

/*--------------------------------------------------------------------
 * effect_t
 */

avltree_t effects;

static void
effect_init(effect_t *eff)
{
	eff->name = str_empty;
	eff->fun_name = str_empty;
	eff->fun = NULL;
}

static void
effect_destroy(effect_t *eff)
{
	free_string(eff->name);
	free_string(eff->fun_name);
}

avltree_info_t c_info_effects =
{
	&avltree_ops,

	(e_init_t) effect_init,
	(e_destroy_t) effect_destroy,

	MT_PVOID, sizeof(effect_t), ke_cmp_str
};

/*--------------------------------------------------------------------
 * uhandler_t
 */

avltree_t uhandlers;

static void
uhandler_init(uhandler_t *hdlr)
{
	hdlr->name = str_empty;
	hdlr->fun_name = str_empty;
	hdlr->notify = str_empty;
	hdlr->ticks = 0;
	hdlr->iter_cl = NULL;
	hdlr->mod = MOD_UPDATE;
	hdlr->cnt = 0;
	hdlr->fun = NULL;
}

static void
uhandler_destroy(uhandler_t *hdlr)
{
	free_string(hdlr->name);
	free_string(hdlr->fun_name);
	free_string(hdlr->notify);
}

avltree_info_t c_info_uhandlers = {
	&avltree_ops,

	(e_init_t) uhandler_init,
	(e_destroy_t) uhandler_destroy,

	MT_PVOID, sizeof(uhandler_t), ke_cmp_str
};

void
uhandler_mod_load(module_t *m)
{
	uhandler_t *hdlr;

	C_FOREACH(hdlr, &uhandlers) {
		if (m->mod_id != hdlr->mod)
			continue;

		hdlr->fun = dlsym(m->dlh, hdlr->fun_name);
		if (hdlr->fun == NULL)
			printlog(LOG_ERROR, "%s: %s", __FUNCTION__, dlerror());
	}
}

void
uhandler_mod_unload(module_t *m)
{
	uhandler_t *hdlr;

	C_FOREACH(hdlr, &uhandlers) {
		if (m->mod_id == hdlr->mod)
			hdlr->fun = NULL;
	}
}

/*--------------------------------------------------------------------
 * skill_t
 */

avltree_t skills;

static void
evf_init(evf_t *evf)
{
	evf->event = -1;
	evf->fun_name = str_empty;
	evf->fun = NULL;
}

static void
evf_destroy(evf_t *evf)
{
	free_string(evf->fun_name);
}

static varr_info_t c_info_evf =
{
	&varr_ops,

	(e_init_t) evf_init,
	(e_destroy_t) evf_destroy,

	sizeof(evf_t), 1
};

static void
skill_init(skill_t *sk)
{
	gmlstr_init(&sk->sk_name);
	sk->fun_name = str_empty;
	sk->fun = NULL;
	sk->target = 0;
	sk->min_pos = 0;
	sk->min_mana = 0;
	sk->rank = 0;
	sk->beats = 0;
	gmlstr_init(&sk->noun_damage);
	sk->dam_class = DAM_NONE;
	sk->effect = str_empty;
	mlstr_init2(&sk->msg_off, str_empty);
	mlstr_init2(&sk->msg_obj, str_empty);
	sk->skill_flags = 0;
	sk->restrict_race = str_empty;
	sk->group = 0;
	sk->skill_type = ST_SKILL;
	c_init(&sk->events, &c_info_evf);
}

static void
skill_destroy(skill_t *sk)
{
	gmlstr_destroy(&sk->sk_name);
	free_string(sk->fun_name);
	gmlstr_destroy(&sk->noun_damage);
	free_string(sk->effect);
	mlstr_destroy(&sk->msg_off);
	mlstr_destroy(&sk->msg_obj);
	free_string(sk->restrict_race);
	c_destroy(&sk->events);
}

avltree_info_t c_info_skills =
{
	&avltree_ops,

	(e_init_t) skill_init,
	(e_destroy_t) skill_destroy,

	MT_SKILL, sizeof(skill_t), ke_cmp_mlstr
};

skill_t *
skill_search(const char *sn, int skill_type)
{
	skill_t *sk;

	if (IS_NULLSTR(sn))
		return NULL;

	/*
	 * try exact match first
	 */
	if ((sk = c_lookup(&skills, sn)) != NULL
	&&  IS_SET(sk->skill_type, skill_type))
		return sk;

	/*
	 * search by prefix
	 */
	C_FOREACH(sk, &skills) {
		if (!str_prefix(sn, gmlstr_mval(&sk->sk_name))
		&&  IS_SET(sk->skill_type, skill_type))
			return sk;
	}

	return NULL;
}

void
skills_dump(BUFFER *output, int skill_type)
{
	skill_t *sk;
	int col = 0;

	C_FOREACH(sk, &skills) {
		const char *sn = gmlstr_mval(&sk->sk_name);

		if (!str_cmp(sn, "reserved")
		||  !IS_SET(sk->skill_type, skill_type))
			continue;

		buf_printf(output, BUF_END, "%-19.18s", sn);	// notrans
		if (++col % 4 == 0)
			buf_append(output, "\n");
	}

	if (col % 4)
		buf_append(output, "\n");
}

const char *
fread_damtype(const char *ctx, rfile_t *fp)
{
	const char *dt = fread_sword(fp);
	if (IS_NULLSTR(dt))
		return dt;

	if (dt[0] != '+') {
		/*
		 * normalize damtype: add leading '+'
		 */
		const char *p = str_printf("+%s", dt);
		free_string(dt);
		dt = p;
	}
	C_STRKEY_CHECK(ctx, &skills, dt);
	return dt;
}

/*--------------------------------------------------------------------
 * spec_t
 */

avltree_t specs;

static void
spec_skill_init(spec_skill_t *spec_sk)
{
	spec_sk->sn = str_empty;
	spec_sk->level = 1;
	spec_sk->rating = 1;
	spec_sk->min = 1;
	spec_sk->adept = 75;
	spec_sk->max = 100;
}

static varr_info_t c_info_spec_skills =
{
	&varr_ops,

	(e_init_t) spec_skill_init,
	strkey_destroy,

	sizeof(spec_skill_t), 4
};

static void
spec_init(spec_t *spec)
{
	spec->spec_name = str_empty;
	spec->spec_class = 0;
	spec->spec_flags = 0;

	c_init(&spec->spec_skills, &c_info_spec_skills);
	trig_init(&spec->mp_trig);
	spec->mp_trig.trig_type = TRIG_SPEC;
}

static void
spec_destroy(spec_t *spec)
{
	free_string(spec->spec_name);
	c_destroy(&spec->spec_skills);
	trig_destroy(&spec->mp_trig);
}

avltree_info_t c_info_specs =
{
	&avltree_ops,

	(e_init_t) spec_init,
	(e_destroy_t) spec_destroy,

	MT_PVOID, sizeof(spec_t), ke_cmp_str
};

/*--------------------------------------------------------------------
 * form_index_t
 */

avltree_t forms;

static void
form_init(form_index_t *f)
{
	int i;
	f->name			= str_empty;
	mlstr_init2(&f->description, str_empty);
	mlstr_init2(&f->short_desc, str_empty);
	mlstr_init2(&f->long_desc, str_empty);
	f->damtype		= str_empty;
	f->damage[DICE_TYPE]	= 0;
	f->damage[DICE_NUMBER]	= 0;
	f->damage[DICE_BONUS]	= 1;
	f->hitroll		= 0;
	f->num_attacks		= 0;
	f->flags		= 0;
	f->affected_by		= 0;
	f->has_invis		= 0;
	f->has_detect		= 0;

	f->skill_spec	= str_empty;
	for (i = 0; i < MAX_RESIST; i++)
		f->resists[i] = RES_UNDEF;
	for (i = 0; i < MAX_STAT; i++)
		f->stats[i] = 10;
}

static void
form_destroy(form_index_t *f)
{
	free_string(f->name);
	mlstr_destroy(&f->description);
	mlstr_destroy(&f->short_desc);
	mlstr_destroy(&f->long_desc);
	free_string(f->damtype);
	free_string(f->skill_spec);
}

avltree_info_t c_info_forms =
{
	&avltree_ops,

	(e_init_t) form_init,
	(e_destroy_t) form_destroy,

	MT_PVOID, sizeof(form_index_t), ke_cmp_str
};

/*--------------------------------------------------------------------
 * liquid_t
 */

avltree_t liquids;

static void
liquid_init(liquid_t *lq)
{
	int i;

	gmlstr_init(&lq->lq_name);
	mlstr_init2(&lq->lq_color, str_empty);
	for (i = 0; i < MAX_COND; i++)
		lq->affect[i] = 0;
	lq->sip = 0;
}

static void
liquid_destroy(liquid_t *lq)
{
	gmlstr_destroy(&lq->lq_name);
	mlstr_destroy(&lq->lq_color);
}

avltree_info_t c_info_liquids =
{
	&avltree_ops,

	(e_init_t) liquid_init,
	(e_destroy_t) liquid_destroy,

	MT_PVOID, sizeof(liquid_t), ke_cmp_mlstr
};

/*--------------------------------------------------------------------
 * social_t
 */

varr socials;

static void
social_init(social_t *soc)
{
	soc->name = str_empty;
	soc->min_pos = 0;

	mlstr_init(&soc->found_char);
	mlstr_init(&soc->found_vict);
	mlstr_init(&soc->found_notvict);

	mlstr_init(&soc->noarg_char);
	mlstr_init(&soc->noarg_room);

	mlstr_init(&soc->self_char);
	mlstr_init(&soc->self_room);

	mlstr_init(&soc->notfound_char);
}

static void
social_destroy(social_t *soc)
{
	free_string(soc->name);

	mlstr_destroy(&soc->found_char);
	mlstr_destroy(&soc->found_vict);
	mlstr_destroy(&soc->found_notvict);

	mlstr_destroy(&soc->noarg_char);
	mlstr_destroy(&soc->noarg_room);

	mlstr_destroy(&soc->self_char);
	mlstr_destroy(&soc->self_room);

	mlstr_destroy(&soc->notfound_char);
}

varr_info_t c_info_socials =
{
	&varr_ops,

	(e_init_t) social_init,
	(e_destroy_t) social_destroy,

	sizeof(social_t), 8
};

/*--------------------------------------------------------------------
 * hint_t
 */

varr hints;

static void
hint_init(hint_t *t)
{
	mlstr_init(&t->phrase);
}

static void
hint_destroy(hint_t *t)
{
	mlstr_destroy(&t->phrase);
}

varr_info_t c_info_hints =
{
	&varr_ops,

	(e_init_t) hint_init,
	(e_destroy_t) hint_destroy,

	sizeof(hint_t), 4
};

/*--------------------------------------------------------------------
 * clan_t
 */

avltree_t clans;

static void
clan_init(clan_t *clan)
{
	clan->name = str_empty;
	clan->recall_vnum = 0;
	clan->skill_spec = str_empty;
	clan->clan_flags = 0;
	clan->altar_vnum = 0;
	clan->obj_vnum = 0;
	clan->mark_vnum = 0;
	clan->obj_ptr = NULL;
	clan->altar_ptr = NULL;
	clan->leader_list = str_empty;
	clan->member_list = str_empty;
	clan->second_list = str_empty;
	msgq_init(&clan->msgq_clan, MSGQ_LEN_CHAN);
}

static void
clan_destroy(clan_t *clan)
{
	free_string(clan->name);
	free_string(clan->skill_spec);
	free_string(clan->leader_list);
	free_string(clan->member_list);
	free_string(clan->second_list);
	msgq_destroy(&clan->msgq_clan);
}

avltree_info_t c_info_clans =
{
	&avltree_ops,

	(e_init_t) clan_init,
	(e_destroy_t) clan_destroy,

	MT_PVOID, sizeof(clan_t), ke_cmp_str
};

/*--------------------------------------------------------------------
 * glob_gmlstr
 */

avltree_t glob_gmlstr;

avltree_info_t c_info_gmlstr =
{
	&avltree_ops,

	(e_init_t) gmlstr_init,
	(e_destroy_t) gmlstr_destroy,

	MT_GMLSTR, sizeof(gmlstr_t), ke_cmp_mlstr,
};

/*--------------------------------------------------------------------
 * DESCRIPTOR_DATA
 */

DESCRIPTOR_DATA *descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *descriptor_free;

int desc_count;
int desc_free_count;

static void outbuf_init(outbuf_t *o, size_t size);
static void outbuf_destroy(outbuf_t *o);

DESCRIPTOR_DATA *
new_descriptor(int fd, int d_type, const char *ip)
{
	DESCRIPTOR_DATA *d;

	if (descriptor_free == NULL) {
		d = mem_alloc(MT_DESCRIPTOR, sizeof(*d));
	} else {
		d = descriptor_free;
		descriptor_free = descriptor_free->next;
		mem_validate(d);
		desc_free_count--;
	}

	desc_count++;
	memset(d, 0, sizeof(*d));
	d->descriptor = fd;
	d->d_type = d_type;
	d->ip = str_dup(ip);
	d->connected = CON_RESOLV;
	d->codepage = 0;

	/* mccp data init */
	d->bytes_sent	= 0;
	d->bytes_income	= 0;
	d->mccp_support	= 0;
	d->out_compress	= NULL;

	outbuf_init(&d->out_buf, 1024);
	outbuf_init(&d->snoop_buf, 0);
	d->mftp_data = str_empty;
	d->dvdata = dvdata_new();

	d->next	= descriptor_list;
	descriptor_list	= d;
	return d;
}

void
free_descriptor(DESCRIPTOR_DATA *d)
{
	if (!d)
		return;

	if (!mem_is(d, MT_DESCRIPTOR)) {
		printlog(LOG_BUG, "free_descriptor: d is not MT_DESCRIPTOR");
		return;
	}
	mem_invalidate(d);
	mem_untag(d, -1);

	desc_count--;
	desc_free_count++;

	free_string(d->host);
	free_string(d->ip);
	outbuf_destroy(&d->out_buf);
	outbuf_destroy(&d->snoop_buf);
	free_string(d->mftp_username);
	free_string(d->mftp_filename);
	free_string(d->mftp_data);
	dvdata_free(d->dvdata);

	d->next = descriptor_free;
	descriptor_free = d;
}

int dvdata_count;
int dvdata_real_count;

static void
outbuf_init(outbuf_t *o, size_t size)
{
	o->top = 0;
	if ((o->size = size) != 0)
		o->buf = malloc(o->size);
}

static void
outbuf_destroy(outbuf_t *o)
{
	free(o->buf);
}

/*--------------------------------------------------------------------
 * dvdata_t
 */

dvdata_t *
dvdata_new(void)
{
	dvdata_t *dv = calloc(1, sizeof(*dv));
	dv->prefix = str_empty;
	dv->pagelen = DEFAULT_PAGELEN;
	dv->prompt = str_dup(DEFAULT_PROMPT);
	dv->refcnt = 1;
	dvdata_count++;
	dvdata_real_count++;
	return dv;
}

dvdata_t *
dvdata_dup(dvdata_t *dv)
{
	dv->refcnt++;
	dvdata_count++;
	return dv;
}

void
dvdata_free(dvdata_t *dv)
{
	int i;

	dvdata_count--;
	if (--dv->refcnt > 0)
		return;

	free_string(dv->prompt);
	free_string(dv->prefix);

	for (i = 0; i < MAX_ALIAS; i++) {
		free_string(dv->alias[i]);
		free_string(dv->alias_sub[i]);
	}
	free(dv);
	dvdata_real_count--;
}

/*--------------------------------------------------------------------
 * mprog_t
 */

avltree_t mprogs;

int (*mprog_compile)(mprog_t *mp);
int (*mprog_execute)(mprog_t *mp,
		     void *arg1, void *arg2, void *arg3, void *arg4, void* arg5);

static void
mprog_init(mprog_t *mp)
{
	mp->name = str_empty;
	mp->type = MP_T_NONE;
	mp->status = MP_S_DIRTY;
	mp->flags = 0;
	mp->text = str_empty;
	mp->errbuf = NULL;
}

static void
mprog_destroy(mprog_t *mp)
{
	free_string(mp->name);
	free_string(mp->text);
	if (mp->errbuf != NULL)
		buf_free(mp->errbuf);
}

avltree_info_t c_info_mprogs = {
	&avltree_ops,

	(e_init_t) mprog_init,
	(e_destroy_t) mprog_destroy,

	MT_PVOID, sizeof(mprog_t), ke_cmp_csstr
};
