/*
 * $Id: recycle.c,v 1.126 2001-08-22 20:45:50 fjoe Exp $
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

#include <sys/types.h>
#include <sys/time.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <merc.h>
#include <rwfile.h>

flag_t		mud_options;

time_t		boot_time;
time_t		current_time;	/* time of this pulse */

TIME_INFO_DATA	time_info;
WEATHER_DATA	weather_info;
AUCTION_DATA	auction;
rating_t	rating_table[RATING_TABLE_SIZE];

ROOM_INDEX_DATA	*top_affected_room = NULL;
CHAR_DATA	*top_affected_char = NULL;
OBJ_DATA	*top_affected_obj = NULL;

int		reboot_counter = 1440;
int		rebooter = 0;

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

struct fd_set	in_set;
struct fd_set	out_set;
struct fd_set	exc_set;

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
const char LANG_CONF		[] = "lang.conf";	// notrans
const char CMD_CONF		[] = "cmd.conf";	// notrans
const char DAMTYPE_CONF		[] = "damtype.conf";	// notrans
const char MATERIALS_CONF	[] = "materials.conf";	// notrans
const char LIQUIDS_CONF		[] = "liquids.conf";	// notrans
const char FORMS_CONF		[] = "forms.conf";	// notrans
const char CC_EXPR_CONF		[] = "cc_expr.conf";	// notrans
const char UHANDLERS_CONF	[] = "uhandlers.conf";	// notrans

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
	return obj;
}

void
free_obj(OBJ_DATA *obj)
{
	if (!obj)
		return;

	if (!mem_is(obj, MT_OBJ)) {
		log(LOG_BUG, "free_obj: obj is not MT_OBJ");
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

void pc_skill_init(pc_skill_t *pc_sk)
{
	pc_sk->sn = str_empty;
	pc_sk->percent = 0;
}

static varrdata_t v_sk_affected =
{
	sizeof(saff_t), 1,
	(e_init_t) saff_init,
	(e_destroy_t) saff_destroy,
	NULL
};

static varrdata_t v_learned =
{
	sizeof(pc_skill_t), 8,
	(e_init_t) pc_skill_init,
	strkey_destroy,
	NULL
};

static varrdata_t v_specs =
{
	sizeof(const char *), 2,
	strkey_init,
	strkey_destroy,
	NULL
};

CHAR_DATA *
char_new(MOB_INDEX_DATA *pMobIndex)
{
	CHAR_DATA *ch;
	int i;

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
		ch->perm_stat[i] = 15;
		ch->mod_stat[i] = 0;
	}

	for (i = 0; i < MAX_RESIST; i++)
		ch->res_mod[i] = 0;

	ch->luck		= 50;
	ch->luck_mod		= 0;

	varr_init(&ch->sk_affected, &v_sk_affected);

	if (pMobIndex) {
		ch->pMobIndex = pMobIndex;
		ch->chan = CHAN_NOSHOUT | CHAN_NOMUSIC;
	} else {
		PC_DATA *pc = PC(ch);

		pc->add_age = 0;
		pc->logon = current_time;
		pc->version = PFILE_VERSION;
		pc->buffer = buf_new(0);

		varr_init(&pc->learned, &v_learned);
		varr_init(&pc->specs, &v_specs);

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
		pc->plr_flags = PLR_NOSUMMON | PLR_NOCANCEL;
		pc->www_show_flags = WSHOW_RACE | WSHOW_SEX | WSHOW_TITLE;
		pc->bank_s = 0;
		pc->bank_g = 0;
		pc->hints_level = HINT_ALL;
		ch->comm = COMM_COMBINE | COMM_PROMPT;
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
		log(LOG_BUG, "free_char: ch is not MT_CHAR");
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
		varr_destroy(&pc->specs);
		varr_destroy(&pc->learned);

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
	}

	if (ch->carrying != NULL) {
		log(LOG_BUG, "char_free: ch->carrying != NULL");
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
		log(LOG_BUG, "reset_add: `after' reset not found");
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

int room_count;
ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

ROOM_INDEX_DATA *
new_room_index(void)
{
	ROOM_INDEX_DATA *pRoom;

        pRoom = mem_alloc(MT_ROOM, sizeof(*pRoom));
	memset(pRoom, 0, sizeof(*pRoom));
	pRoom->heal_rate = 100;
	pRoom->mana_rate = 100;

        room_count++;
	return pRoom;
}

void
free_room_index(ROOM_INDEX_DATA *pRoom)
{
	int door;
	RESET_DATA *pReset;

	if (!mem_is(pRoom, MT_ROOM))
		return;

	mlstr_destroy(&pRoom->name);
	mlstr_destroy(&pRoom->description);

	for (door = 0; door < MAX_DIR; door++) {
		if (pRoom->exit[door])
			free_exit(pRoom->exit[door]);
	}

	ed_free(pRoom->ed);

	for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
		reset_free(pReset);

	room_count--;
	mem_free(pRoom);
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *
get_room_index(int vnum)
{
	ROOM_INDEX_DATA *pRoomIndex;

	if (vnum <= 0)
		return NULL;

	for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH];
	     pRoomIndex; pRoomIndex = pRoomIndex->next)
		if (pRoomIndex->vnum == vnum)
			return pRoomIndex;

	return NULL;
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

int obj_index_count;
OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];

OBJ_INDEX_DATA *
new_obj_index(void)
{
	OBJ_INDEX_DATA *pObj;

        pObj = mem_alloc(MT_OBJ_INDEX, sizeof(*pObj));
	memset(pObj, 0, sizeof(*pObj));

	pObj->name		= str_dup(str_empty);
	pObj->item_type		= ITEM_TRASH;
	objval_init(pObj->item_type, pObj->value);
	pObj->material		= str_dup("unknown");
	pObj->condition		= 100;
	pObj->limit		= -1;
	mlstr_init2(&pObj->gender, flag_string(gender_table, SEX_NEUTRAL));
	trig_init_list(&pObj->mp_trigs);

        obj_index_count++;
	return pObj;
}

void
free_obj_index(OBJ_INDEX_DATA *pObj)
{
	if (!mem_is(pObj, MT_OBJ_INDEX))
		return;

	free_string(pObj->name);
	free_string(pObj->material);
	mlstr_destroy(&pObj->gender);
	mlstr_destroy(&pObj->short_descr);
	mlstr_destroy(&pObj->description);

	aff_free_list(pObj->affected);
	ed_free(pObj->ed);
	objval_destroy(pObj->item_type, pObj->value);
	trig_destroy_list(&pObj->mp_trigs);

	obj_index_count--;
	mem_free(pObj);
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *
get_obj_index(int vnum)
{
	OBJ_INDEX_DATA *pObjIndex;

	if (vnum <= 0)
		return NULL;

	for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH];
	     pObjIndex; pObjIndex = pObjIndex->next)
		if (pObjIndex->vnum == vnum)
			return pObjIndex;

	return NULL;
}

/*--------------------------------------------------------------------
 * MOB_INDEX_DATA
 */

int mob_index_count;
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];

MOB_INDEX_DATA *
new_mob_index(void)
{
	MOB_INDEX_DATA *pMob;
	int i;

        pMob = mem_alloc(MT_MOB_INDEX, sizeof(*pMob));
	memset(pMob, 0, sizeof(*pMob));

	pMob->name		= str_dup(str_empty);
	pMob->race		= str_dup("human");
	pMob->material		= str_dup("unknown");
	pMob->size		= SIZE_MEDIUM;
	pMob->start_pos		= POS_STANDING;
	pMob->default_pos	= POS_STANDING;
	pMob->damtype		= str_empty;
	mlstr_init2(&pMob->gender, flag_string(gender_table, SEX_NEUTRAL));
	for (i = 0; i < MAX_RESIST; i++)
		pMob->resists[i] = RES_UNDEF;
	trig_init_list(&pMob->mp_trigs);

	mob_index_count++;
	return pMob;
}

void
free_mob_index(MOB_INDEX_DATA *pMob)
{
	if (!mem_is(pMob, MT_MOB_INDEX))
		return;

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
	aff_free_list(pMob->affected);
	trig_destroy_list(&pMob->mp_trigs);

	mob_index_count--;
	mem_free(pMob);
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *
get_mob_index(int vnum)
{
	MOB_INDEX_DATA *pMobIndex;

	if (vnum <= 0)
		return NULL;

	for (pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH];
	     pMobIndex; pMobIndex = pMobIndex->next)
		if (pMobIndex->vnum == vnum)
			return pMobIndex;

	return NULL;
}

/*--------------------------------------------------------------------
 * skill_t
 */

hash_t skills;

hashdata_t h_skills =
{
	sizeof(skill_t), 1,
	(e_init_t) skill_init,
	(e_destroy_t) skill_destroy,
	(e_cpy_t) skill_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_mlstr
};

static void	evf_init	(evf_t *);
static void	evf_destroy	(evf_t *);
static evf_t *	evf_cpy		(evf_t *, evf_t *);

static varrdata_t v_evf =
{
	sizeof(evf_t), 1,
	(e_init_t) evf_init,
	(e_destroy_t) evf_destroy,
	(e_cpy_t) evf_cpy
};

void
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
	mlstr_init2(&sk->msg_off, str_empty);
	mlstr_init2(&sk->msg_obj, str_empty);
	sk->skill_flags = 0;
	sk->restrict_race = str_empty;
	sk->group = 0;
	sk->skill_type = 0;
	varr_init(&sk->events, &v_evf);
}

skill_t *
skill_cpy(skill_t *dst, const skill_t *src)
{
	gmlstr_cpy(&dst->sk_name, &src->sk_name);
	dst->fun_name = str_qdup(src->fun_name);
	dst->fun = src->fun;
	dst->target = src->target;
	dst->min_pos = src->min_pos;
	dst->min_mana = src->min_mana;
	dst->rank = src->rank;
	dst->beats = src->beats;
	gmlstr_cpy(&dst->noun_damage, &src->noun_damage);
	mlstr_cpy(&dst->msg_off, &src->msg_off);
	mlstr_cpy(&dst->msg_obj, &src->msg_obj);
	dst->skill_flags = src->skill_flags;
	dst->restrict_race = str_qdup(src->restrict_race);
	dst->group = src->group;
	dst->skill_type = src->skill_type;
	varr_cpy(&dst->events, &src->events);
	return dst;
}

void
skill_destroy(skill_t *sk)
{
	gmlstr_destroy(&sk->sk_name);
	free_string(sk->fun_name);
	gmlstr_destroy(&sk->noun_damage);
	mlstr_destroy(&sk->msg_off);
	mlstr_destroy(&sk->msg_obj);
	free_string(sk->restrict_race);
	varr_destroy(&sk->events);
}

static void *
skills_dump_cb(void *p, va_list ap)
{
	skill_t *sk = (skill_t *) p;

	BUFFER *output = va_arg(ap, BUFFER *);
	int skill_type = va_arg(ap, int);
	int *pcol = va_arg(ap, int *);

	const char *sn = gmlstr_mval(&sk->sk_name);

	if (!str_cmp(sn, "reserved")
	||  (skill_type >= 0 && sk->skill_type != skill_type))
		return NULL;

	buf_printf(output, BUF_END, "%-19.18s", sn);		// notrans
	if (++(*pcol) % 4 == 0)
		buf_append(output, "\n");
	return 0;
}

void
skills_dump(BUFFER *output, int skill_type)
{
	int col = 0;
	hash_foreach(&skills, skills_dump_cb, output, skill_type, &col);
	if (col % 4)
		buf_append(output, "\n");
}

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

static evf_t *
evf_cpy(evf_t *dst, evf_t *src)
{
	dst->event = src->event;
	dst->fun_name = str_qdup(src->fun_name);
	dst->fun = src->fun;
	return dst;
}

/*--------------------------------------------------------------------
 * spec_t
 */

hash_t specs;

hashdata_t h_specs =
{
	sizeof(spec_t), 1,
	(e_init_t) spec_init,
	(e_destroy_t) spec_destroy,
	(e_cpy_t) spec_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

static void		spec_skill_init(spec_skill_t *spec_sk);
static spec_skill_t *	spec_skill_cpy(spec_skill_t *, const spec_skill_t *);

static varrdata_t v_spec_skills =
{
	sizeof(spec_skill_t), 4,
	(e_init_t) spec_skill_init,
	strkey_destroy,
	(e_cpy_t) spec_skill_cpy,
};

void
spec_init(spec_t *spec)
{
	spec->spec_name = str_empty;
	spec->spec_class = 0;

	varr_init(&spec->spec_skills, &v_spec_skills);
	trig_init(&spec->mp_trig);
	spec->mp_trig.trig_type = TRIG_SPEC;
}

spec_t *
spec_cpy(spec_t *dst, const spec_t *src)
{
	dst->spec_name = str_qdup(src->spec_name);
	dst->spec_class = src->spec_class;
	varr_cpy(&dst->spec_skills, &src->spec_skills);
	trig_cpy(&dst->mp_trig, &src->mp_trig);
	return dst;
}

void
spec_destroy(spec_t *spec)
{
	free_string(spec->spec_name);
	varr_destroy(&spec->spec_skills);
	trig_destroy(&spec->mp_trig);
}

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

static spec_skill_t *
spec_skill_cpy(spec_skill_t *dst, const spec_skill_t *src)
{
	dst->sn = str_qdup(src->sn);
	dst->level = src->level;
	dst->rating = src->rating;
	dst->min = src->min;
	dst->adept = src->adept;
	dst->max = src->max;
	return dst;
}

/*--------------------------------------------------------------------
 * form_index_t
 */

hash_t forms;

hashdata_t h_forms =
{
	sizeof(form_index_t), 1,
	(e_init_t) form_init,
	(e_destroy_t) form_destroy,
	(e_cpy_t) form_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

void
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
	f->skill_spec	= str_empty;
	for (i = 0; i < MAX_RESIST; i++)
		f->resists[i] = RES_UNDEF;
	for (i = 0; i < MAX_STAT; i++)
		f->stats[i] = 10;
}

form_index_t *
form_cpy(form_index_t *dst, const form_index_t *src)
{
	int i;
	dst->name		= str_qdup(src->name);
	mlstr_cpy(&dst->description, &src->description);
	mlstr_cpy(&dst->short_desc, &src->short_desc);
	mlstr_cpy(&dst->long_desc, &src->long_desc);
	dst->damtype		= str_qdup(src->damtype);
	dst->hitroll		= src->hitroll;
	dst->num_attacks	= src->num_attacks;
	dst->skill_spec		= str_qdup(src->skill_spec);
	dst->flags		= src->flags;
	dst->damage[DICE_TYPE]	= src->damage[DICE_TYPE];
	dst->damage[DICE_NUMBER]= src->damage[DICE_NUMBER];
	dst->damage[DICE_BONUS]	= src->damage[DICE_BONUS];
	for (i = 0; i < MAX_RESIST; i++)
		dst->resists[i] = src->resists[i];
	for (i = 0; i < MAX_STAT; i++)
		dst->stats[i] = src->stats[i];

	return dst;
}

void
form_destroy(form_index_t *f)
{
	free_string(f->name);
	mlstr_destroy(&f->description);
	mlstr_destroy(&f->short_desc);
	mlstr_destroy(&f->long_desc);
	free_string(f->damtype);
	free_string(f->skill_spec);
}

/*--------------------------------------------------------------------
 * liquid_t
 */

hash_t liquids;

hashdata_t h_liquids =
{
	sizeof(liquid_t), 1,
	(e_init_t) liquid_init,
	(e_destroy_t) liquid_destroy,
	(e_cpy_t) liquid_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_mlstr
};

void
liquid_init(liquid_t *lq)
{
	int i;

	gmlstr_init(&lq->lq_name);
	mlstr_init2(&lq->lq_color, str_empty);
	for (i = 0; i < MAX_COND; i++)
		lq->affect[i] = 0;
	lq->sip = 0;
}

liquid_t *
liquid_cpy(liquid_t *dst, const liquid_t *src)
{
	int i;

	gmlstr_cpy(&dst->lq_name, &src->lq_name);
	mlstr_cpy(&dst->lq_color, &src->lq_color);
	for (i = 0; i < MAX_COND; i++)
		dst->affect[i] = src->affect[i];
	dst->sip = src->sip;
	return dst;
}

void
liquid_destroy(liquid_t *lq)
{
	gmlstr_destroy(&lq->lq_name);
	mlstr_destroy(&lq->lq_color);
}

/*--------------------------------------------------------------------
 * social_t
 */

varr socials;

varrdata_t v_socials =
{
	sizeof(social_t), 8,
	(e_init_t) social_init,
	(e_destroy_t) social_destroy,
	NULL
};

void
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

void
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

/*--------------------------------------------------------------------
 * hint_t
 */

varr hints;

varrdata_t v_hints =
{
	sizeof(hint_t), 4,
	(e_init_t) hint_init,
	(e_destroy_t) hint_destroy,
	NULL
};

void
hint_init(hint_t *t)
{
	mlstr_init(&t->phrase);
}

void
hint_destroy(hint_t *t)
{
	mlstr_destroy(&t->phrase);
}

/*--------------------------------------------------------------------
 * clan_t
 */

hash_t clans;

hashdata_t h_clans =
{
	sizeof(clan_t), 1,
	(e_init_t) clan_init,
	(e_destroy_t) clan_destroy,
	(e_cpy_t) clan_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

void
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
}

clan_t *
clan_cpy(clan_t *dst, const clan_t *src)
{
	dst->name = str_qdup(src->name);
	dst->recall_vnum = src->recall_vnum;
	dst->skill_spec = str_qdup(src->skill_spec);
	dst->clan_flags = src->clan_flags;
	dst->altar_vnum = src->altar_vnum;
	dst->obj_vnum = src->obj_vnum;
	dst->mark_vnum = src->mark_vnum;
	dst->obj_ptr = src->obj_ptr;
	dst->altar_ptr = src->altar_ptr;
	dst->leader_list = str_qdup(src->leader_list);
	dst->member_list = str_qdup(src->member_list);
	dst->second_list = str_qdup(src->second_list);
	return dst;
}

void
clan_destroy(clan_t *clan)
{
	free_string(clan->name);
	free_string(clan->skill_spec);
	free_string(clan->leader_list);
	free_string(clan->member_list);
	free_string(clan->second_list);
}

/*--------------------------------------------------------------------
 * glob_gmlstr
 */

hash_t		glob_gmlstr;

hashdata_t h_glob_gmlstr =
{
	sizeof(gmlstr_t), 1,
	(e_init_t) gmlstr_init,
	(e_destroy_t) gmlstr_destroy,
	(e_cpy_t) gmlstr_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_mlstr
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
new_descriptor(int fd)
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
	d->connected = CON_GET_CODEPAGE;
	d->codepage = 0;
	outbuf_init(&d->out_buf, 1024);
	outbuf_init(&d->snoop_buf, 0);
	d->dvdata = dvdata_new();

	return d;
}

void
free_descriptor(DESCRIPTOR_DATA *d)
{
	if (!d)
		return;

	if (!mem_is(d, MT_DESCRIPTOR)) {
		log(LOG_BUG, "free_descriptor: d is not MT_DESCRIPTOR");
		return;
	}
	mem_invalidate(d);

	desc_count--;
	desc_free_count++;

	free_string(d->host);
	free_string(d->ip);
	outbuf_destroy(&d->out_buf);
	outbuf_destroy(&d->snoop_buf);
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

