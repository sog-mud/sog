#include <odbc++/connection.h>
#include <odbc++/drivermanager.h>
#include <odbc++/preparedstatement.h>
#include <odbc++/statement.h>
#include <odbc++/resultset.h>
#include <odbc++/databasemetadata.h>

#include <cstdio>
#include <iostream>

extern "C" {
#include <merc.h>

#include "olc.h"
}

using namespace odbc;
using namespace std;

Connection *connect_db(void);

void dump_flag_table(Connection *con, flaginfo_t *tbl, const char *tbl_name,
		     const char *flag_name, const char *value_name);
void dump_flag_values(Connection *con, int id, flag_t flag, flaginfo_t *tbl,
		      const char *tbl_name, const char *id_column,
		      const char *value_column);

void dump_area_data(Connection *con);
void dump_mobile_data(Connection *con, AREA_DATA *pArea);
void dump_mob_resistances(Connection *con, MOB_INDEX_DATA *pMob);
void dump_object_data(Connection *con, AREA_DATA *pArea);
void dump_room_data(Connection *con, AREA_DATA *pArea);
void dump_exit_data(Connection *con, ROOM_INDEX_DATA *pRoom);
void dump_help_data(Connection *con, AREA_DATA *pArea);
void dump_affect_data(Connection * con, AFFECT_DATA *aff, int id,
		      const char *type);
void dump_object_vo(Connection *con, OBJ_INDEX_DATA *pObj);
void insertShop(Connection *con, SHOP_DATA *shop);
void insertExd(Connection *con, int id, ED_DATA *exd, const char *tbl_name,
	       const char *id_column);
void insertBitvector(Connection *con, int affect_id, AFFECT_DATA *aff,
		     const char *type);
int insertExit(Connection *con, EXIT_DATA *pExit);
void insertPracticer(Connection *con, MOB_INDEX_DATA *pMob);

extern "C" bool
dump_world(void)
{
	Connection *con = connect_db();

	if (con == NULL)
		return FALSE;

	/* area flags */
	dump_flag_table(con, area_flags,
	    "area_flags_conf", "area_flag", "flag_value");
	/* mobile flags */
	dump_flag_table(con, size_table,
	    "size_list", "size_name", "size_value");
	dump_flag_table(con, position_table,
	    "pos_list", "position_name", "position_value");
	dump_flag_table(con, off_flags,
	    "mobile_off_conf", "off_flag_name", "off_value");
	dump_flag_table(con, id_flags,
	    "mobile_id_conf", "id_name", "id_value");
	dump_flag_table(con, affect_flags,
	    "mobile_affected_conf", "aff_name", "aff_value");
	dump_flag_table(con, mob_act_flags,
	    "mobile_act_conf", "act_name", "act_value");
	dump_flag_table(con, mob_flags,
	    "mobile_mobflags_conf", "mobflag_name", "mobflag_value");
	dump_flag_table(con, part_flags,
	    "mobile_parts_conf", "part_name", "part_value");
	dump_flag_table(con, form_flags,
	    "mobile_form_conf", "form_name", "form_value");
	dump_flag_table(con, dam_classes,
	    "damclass_list", "damclass_name", "damclass_value");
	/* object flags */
	dump_flag_table(con, stat_flags,
	    "obj_statflags_conf", "statflag_name", "statflag_value");
	dump_flag_table(con, item_types,
	    "obj_itemtypes_conf", "itemtype_name", "itemtype_value");
	dump_flag_table(con, obj_flags,
	    "obj_objflags_conf", "objflag_name", "objflag_value");
	dump_flag_table(con, wear_flags,
	    "obj_wearflags_conf", "wearflag_name", "wearflag_value");
	dump_flag_table(con, furniture_flags,
	    "furniture_flags_conf", "furn_name", "furn_value");
	dump_flag_table(con, portal_flags,
	    "portal_flags_conf", "portalflag_name", "portalflag_value");
	dump_flag_table(con, book_class,
	    "book_class_conf", "bclass_name", "bclass_value");
	dump_flag_table(con, fail_effects,
	    "fail_effects_conf", "fail_name", "fail_value");
	dump_flag_table(con, cont_flags,
	    "cont_flags_conf", "contflag_name", "contflag_value");
	dump_flag_table(con, weapon_class,
	    "weapon_class_conf", "weapon_name", "weapon_value");
	dump_flag_table(con, weapon_type2,
	    "weapon_flags_conf", "wflag_name", "wflag_value");
	/* room flags */
	dump_flag_table(con, sector_types,
	    "sector_types_conf", "sector_name", "sector_value");
	dump_flag_table(con, room_flags,
	    "room_flags_conf", "roomflag_name", "roomflag_value");
	dump_flag_table(con, raffect_flags,
	    "raffect_flags_conf", "raffect_name", "raffect_value");
	/* exit flags */
	dump_flag_table(con, exit_flags,
	    "exit_flags_conf", "exitflag_name", "exitflag_value");
	/* affects flags */
	dump_flag_table(con, affect_where_types,
	    "affect_where_conf", "where_name", "where_value");

	try {
		dump_area_data(con);
	} catch (SQLException &e) {
		cerr << endl << e.getMessage() << endl;
		return FALSE;
	}

	con->commit();
	delete con;
	return TRUE;
}

Connection *
connect_db(void)
{
	Connection *con = NULL;

	try {
		con = DriverManager::getConnection("dsn=tatyana");
//		con->setAutoCommit(false);
	} catch (SQLException &e) {
		cerr << endl << e.getMessage() << endl;
		return NULL;
	}
	return con;
}

static void
setInt(PreparedStatement *pstmt, int idx, int num)
{
	if (num)
		pstmt->setInt(idx, num);
	else
		pstmt->setNull(idx, SQL_INTEGER);
}

static void
setString(PreparedStatement *pstmt, int idx, const char *str)
{
	if (!IS_NULLSTR(str))
		pstmt->setString(idx, str);
	else
		pstmt->setNull(idx, SQL_VARCHAR);
}

static void
setMlString(Connection *con, PreparedStatement *pstmt, int idx, mlstring *ml)
{
	int id;

	if (mlstr_null(ml)) {
		pstmt->setNull(idx, SQL_INTEGER);
		return;
	}

	PreparedStatement *pstmt2;
	if (ml->nlang == 0) {
		pstmt2 = con->prepareStatement(
		    "insert into mlstring_table (nlang, text0) values (?, ?)");
		pstmt2->setInt(1, ml->nlang);
		setString(pstmt2, 2, ml->u.str);
	} else {
		pstmt2 = con->prepareStatement(
		    "insert into mlstring_table (nlang, text0, text1) values (?, ?, ?)");
		pstmt2->setInt(1, ml->nlang);
		setString(pstmt2, 2, ml->u.lstr[0]);
		setString(pstmt2, 3, ml->u.lstr[1]);
	}
	pstmt2->executeUpdate();
	delete pstmt2;

	pstmt2 = con->prepareStatement(
	    "select currval('mlstring_table_mlstring_id_seq')");
	ResultSet *res = pstmt2->executeQuery();
	res->next();
	id = res->getInt(1);
	delete res;
	delete pstmt2;

	pstmt->setInt(idx, id);
}

void
setGmlStr(Connection *con, PreparedStatement *pstmt, int idx, gmlstr_t *gml)
{
	int gmlstr_id;

	if (mlstr_null(&gml->ml) && mlstr_null(&gml->gender)) {
		pstmt->setNull(idx, SQL_INTEGER);
		return;
	}

	PreparedStatement *stmt = con->prepareStatement
	    ("insert into gmlstr_table (gender_id, text_id) values (?, ?)");
	setMlString(con, stmt, 1, &gml->gender);
	setMlString(con, stmt, 2, &gml->ml);
	stmt->executeUpdate();
	delete stmt;

	stmt = con->prepareStatement(
	    "select currval('gmlstr_table_gmlstr_id_seq')");
	ResultSet *res = stmt->executeQuery();
	res->next();
	gmlstr_id = res->getInt(1);
	delete res;
	delete stmt;

	pstmt->setInt(idx, gmlstr_id);
}

void
insertExd(Connection *con, int id, ED_DATA *exd, const char *tbl_name,
	  const char *id_column)
{
	char stmt[MAX_INPUT_LENGTH];
	int exd_id;
	ED_DATA *ed;

	snprintf(stmt, sizeof(stmt),
	    "insert into %s (%s, exd_id) values (?, ?)",
	    tbl_name, id_column);

	for (ed = exd; ed != NULL; ed = ed->next) {
		PreparedStatement *pstmt = con->prepareStatement(
		    "insert into exd_table (exd_keyword, description) values (?, ?)");
		setString(pstmt, 1, exd->keyword);
		setMlString(con, pstmt, 2, &exd->description);

		pstmt->executeUpdate();
		delete pstmt;

		pstmt = con->prepareStatement(
		    "select currval('exd_table_exd_id_seq')");
		ResultSet *res = pstmt->executeQuery();
		res->next();
		exd_id = res->getInt(1);
		delete res;
		delete pstmt;

		pstmt = con->prepareStatement(stmt);
		pstmt->setInt(1, id);
		pstmt->setInt(2, exd_id);

		pstmt->executeUpdate();
		delete pstmt;
	}
}

void
insertShop(Connection *con, SHOP_DATA *shop)
{
	PreparedStatement *pstmt = con->prepareStatement
	    ("insert into shop_list (mob_vnum, profit_buy, profit_sell, open_hour, close_hour) values (?, ?, ?, ?, ?)");
	pstmt->setInt(1, shop->keeper);
	pstmt->setInt(2, shop->profit_buy);
	pstmt->setInt(3, shop->profit_sell);
	pstmt->setInt(4, shop->open_hour);
	pstmt->setInt(5, shop->close_hour);

	pstmt->executeUpdate();
	delete pstmt;

	for (int i = 0; i < MAX_TRADE; i++) {
		if (shop->buy_type[i] != 0) {
			pstmt = con->prepareStatement
			    ("insert into seller_list (mob_vnum, obj_itemtype) values (?, ?)");
			pstmt->setInt(1, shop->keeper);
			pstmt->setInt(2, shop->buy_type[i]);

			pstmt->executeUpdate();
			delete pstmt;
		}
	}
}

void
insertBitvector(Connection *con, int affect_id, AFFECT_DATA *aff,
		const char *type)
{
	where_t *w = where_lookup(aff->where);
	char tbl_name[MAX_INPUT_LENGTH];

	snprintf(tbl_name, sizeof(tbl_name), "%s_aff_bitvector_list", type);

	if (w != NULL && w->bit_table != NULL) {
		dump_flag_values(con, affect_id, aff->bitvector,
				 w->bit_table, tbl_name,
				 "affect_id", "bitvector_value");
	}

}

int
insertExit(Connection *con, EXIT_DATA *pExit)
{
	int id;

	PreparedStatement *pstmt = con->prepareStatement
	    ("insert into exit_table (exit_size, key, keyword, to_room, description, short_descr) values (?, ?, ?, ?, ?, ?)");

	setInt(pstmt, 1, pExit->size);
	setInt(pstmt, 2, pExit->key);
	setString(pstmt, 3, pExit->keyword);
	setInt(pstmt, 4, pExit->to_room.r->vnum);
	setMlString(con, pstmt, 5, &pExit->description);
	setGmlStr(con, pstmt, 6, &pExit->short_descr);

	pstmt->executeUpdate();
	delete pstmt;

	pstmt = con->prepareStatement(
	    "select currval('exit_table_exit_id_seq')");
	ResultSet *res = pstmt->executeQuery();
	res->next();
	id = res->getInt(1);
	delete res;
	delete pstmt;

	if (pExit->rs_flags) {
		dump_flag_values(con, id, pExit->rs_flags,
				exit_flags, "exit_flags_list",
				 "exit_id", "exitflag_value");
	}

	return id;
}

void
insertPracticer(Connection *con, MOB_INDEX_DATA *pMob)
{
	int *gr;
	PreparedStatement *pstmt = con->prepareStatement
	    ("insert into practicer_list (mob_vnum, group_id) values (?, ?)");

	C_FOREACH (int *, gr, &pMob->practicer) {
		pstmt->setInt(1, pMob->vnum);
		pstmt->setInt(2, *gr);
		pstmt->executeUpdate();
	}
	delete pstmt;
}
void
dump_exit_data(Connection *con, ROOM_INDEX_DATA *pRoom)
{
	int exit_id, door;
	EXIT_DATA *pExit;

	PreparedStatement *pstmt = con->prepareStatement
	    ("insert into room_exit_list (room_vnum, north_exit, east_exit, south_exit, west_exit, up_exit, down_exit) values (?, ?, ?, ?, ?, ?, ?)");

	pstmt->setInt(1, pRoom->vnum);
	for (door = 0; door < MAX_DIR; door++) {
		pExit = pRoom->exit[door];
		if (pExit != NULL) {
			exit_id = insertExit(con, pExit);
			pstmt->setInt(door + 2, exit_id);
		} else
			pstmt->setNull(door + 2, SQL_INTEGER);
	}
	pstmt->executeUpdate();
	delete pstmt;
}

void
dump_affect_data(Connection *con, AFFECT_DATA *aff, int id,
		 const char *type)
{
	char stmt[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;

	snprintf(stmt, sizeof(stmt),
	    "insert into %s_affect_data (%s_vnum, aff_where, aff_type, aff_level, duration, location_i, location_s, modifier) values (?, ?, ?, ?, ?, ?, ?, ?)",
	    type, type);

	PreparedStatement *pstmt = con->prepareStatement(stmt);

	for (paf = aff; paf != NULL; paf = paf->next) {
		pstmt->setInt(1, id);
		setInt(pstmt, 2, paf->where);
		setString(pstmt, 3, paf->type);
		setInt(pstmt, 4, paf->level);
		setInt(pstmt, 5, paf->duration);
		if (HAS_INT_LOCATION(paf)) {
			setInt(pstmt, 6, paf->location.i);
			pstmt->setNull(7, SQL_VARCHAR);
		} else {
			pstmt->setNull(6, SQL_INTEGER);
			setString(pstmt, 7, paf->location.s);
		}
		setInt(pstmt, 8, paf->modifier);

		pstmt->executeUpdate();

		if (paf->bitvector) {
			char stm2[MAX_INPUT_LENGTH];
			snprintf(stm2, sizeof(stm2),
			   "select currval('%s_affect_data_affect_id_seq')",
			   type);
			PreparedStatement *pstmt2 = con->prepareStatement(stm2);
			ResultSet *res = pstmt2->executeQuery();
			res->next();
			int id = res->getInt(1);
			delete res;
			delete pstmt2;
			insertBitvector(con, id, paf, "obj");
		}
	}
	delete pstmt;
}

void
dump_object_vo(Connection *con, OBJ_INDEX_DATA *pObj)
{
	PreparedStatement *pstmt;

	switch (pObj->item_type) {
	case ITEM_LIGHT:
		pstmt = con->prepareStatement(
		    "insert into light_list (obj_vnum, light) values (?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[2].i);
		break;
	case ITEM_WAND:
		pstmt = con->prepareStatement(
		    "insert into wand_list (obj_vnum, spell_level, charges, charges_left, spell_name) values (?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		pstmt->setInt(3, pObj->value[1].i);
		pstmt->setInt(4, pObj->value[2].i);
		setString(pstmt, 5, pObj->value[3].s);
		break;
	case ITEM_STAFF:
		pstmt = con->prepareStatement(
		    "insert into staff_list (obj_vnum, spell_level, charges, charges_left, spell_name) values (?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		pstmt->setInt(3, pObj->value[1].i);
		pstmt->setInt(4, pObj->value[2].i);
		setString(pstmt, 5, pObj->value[3].s);
		break;
	case ITEM_WEAPON:
		pstmt = con->prepareStatement(
		    "insert into weapon_list (obj_vnum, weapon_name, d1, d2, dam_type) values (?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		pstmt->setInt(3, pObj->value[1].i);
		pstmt->setInt(4, pObj->value[2].i);
		setString(pstmt, 5, pObj->value[3].s);
		dump_flag_values(con, pObj->vnum, pObj->value[4].i,
				 weapon_type2, "weapon_flags_list",
				 "obj_vnum", "wflag_value");
		break;
	case ITEM_ARMOR:
		pstmt = con->prepareStatement(
		    "insert into armor_list (obj_vnum, ac_pierce, ac_bash, ac_slash, ac_exotic) values (?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		pstmt->setInt(3, pObj->value[1].i);
		pstmt->setInt(4, pObj->value[2].i);
		pstmt->setInt(5, pObj->value[3].i);
		break;
	case ITEM_POTION:
		pstmt = con->prepareStatement(
		    "insert into potion_list (obj_vnum, spell_level, spell_1, spell_2, spell_3, spell_4) values (?, ?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		setString(pstmt, 3, pObj->value[1].s);
		setString(pstmt, 4, pObj->value[2].s);
		setString(pstmt, 5, pObj->value[3].s);
		setString(pstmt, 6, pObj->value[4].s);
		break;
	case ITEM_PILL:
		pstmt = con->prepareStatement(
		    "insert into pill_list (obj_vnum, spell_level, spell_1, spell_2, spell_3, spell_4) values (?, ?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		setString(pstmt, 3, pObj->value[1].s);
		setString(pstmt, 4, pObj->value[2].s);
		setString(pstmt, 5, pObj->value[3].s);
		setString(pstmt, 6, pObj->value[4].s);
		break;
	case ITEM_SCROLL:
		pstmt = con->prepareStatement(
		    "insert into scroll_list (obj_vnum, spell_level, spell_1, spell_2, spell_3, spell_4) values (?, ?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		setString(pstmt, 3, pObj->value[1].s);
		setString(pstmt, 4, pObj->value[2].s);
		setString(pstmt, 5, pObj->value[3].s);
		setString(pstmt, 6, pObj->value[4].s);
		break;
	case ITEM_FURNITURE:
		pstmt = con->prepareStatement(
		    "insert into furniture_list (obj_vnum, max_people, max_weight, heal_bonus, mana_bonus) values (?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		pstmt->setInt(3, pObj->value[1].i);
		pstmt->setInt(4, pObj->value[3].i);
		pstmt->setInt(5, pObj->value[4].i);
		dump_flag_values(con, pObj->vnum, pObj->value[2].i,
				 furniture_flags, "furniture_flags_list",
				 "obj_vnum", "furn_value");
		break;
	case ITEM_CONTAINER:
		pstmt = con->prepareStatement(
		    "insert into container_list (obj_vnum, weight, key, capacity, mult) values (?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		setInt(pstmt, 3, pObj->value[2].i);
		pstmt->setInt(4, pObj->value[3].i);
		pstmt->setInt(5, pObj->value[4].i);
		break;
	case ITEM_MONEY:
		pstmt = con->prepareStatement(
		    "insert into money_list (obj_vnum, silver, gold) values (?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		pstmt->setInt(3, pObj->value[1].i);
		break;
	case ITEM_FOUNTAIN:
		pstmt = con->prepareStatement(
		    "insert into fountain_list (obj_vnum, total, left_liq, liquid, poisoned) values (?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		pstmt->setInt(3, pObj->value[1].i);
		setString(pstmt, 4, pObj->value[2].s);
		if (pObj->value[3].i == 0)
			pstmt->setBoolean(5, false);
		else
			pstmt->setBoolean(5, true);
		break;
	case ITEM_DRINK_CON:
		pstmt = con->prepareStatement(
		    "insert into drink_con_list (obj_vnum, total, left_liq, liquid, poisoned) values (?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		pstmt->setInt(3, pObj->value[1].i);
		setString(pstmt, 4, pObj->value[2].s);
		if (pObj->value[3].i == 0)
			pstmt->setBoolean(5, false);
		else
			pstmt->setBoolean(5, true);
		break;
	case ITEM_PORTAL:
		pstmt = con->prepareStatement(
		    "insert into portal_list (obj_vnum, charges, to_vnum, key) values (?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		setInt(pstmt, 3, pObj->value[3].i);
		setInt(pstmt, 4, pObj->value[4].i);
		dump_flag_values(con, pObj->vnum, pObj->value[2].i,
				 portal_flags, "portal_flags_list",
				 "obj_vnum", "portalflag_value");
		dump_flag_values(con, pObj->vnum, pObj->value[1].i,
				 portal_flags, "eportal_flags_list",
				 "obj_vnum", "exitflag_value");
		break;
		break;
	case ITEM_BOOK:
		pstmt = con->prepareStatement(
		    "insert into book_list (obj_vnum, book_class, spec, chance, fail_effect, message) values (?, ?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		pstmt->setInt(2, pObj->value[0].i);
		setString(pstmt, 3, pObj->value[1].s);
		pstmt->setInt(4, pObj->value[2].i);
		pstmt->setInt(5, pObj->value[3].i);
		setString(pstmt, 6, pObj->value[4].s);
		break;
	case ITEM_HERB:
		pstmt = con->prepareStatement(
		    "insert into herb_list (obj_vnum, spell, hit, mana, moves) values (?, ?, ?, ?, ?)");
		pstmt->setInt(1, pObj->vnum);
		setString(pstmt, 2, pObj->value[0].s);
		pstmt->setInt(3, pObj->value[1].i);
		pstmt->setInt(4, pObj->value[2].i);
		pstmt->setInt(5, pObj->value[3].i);
		break;
	default:
		return;
	}
	pstmt->executeUpdate();
	delete pstmt;
}

void
dump_flag_values(Connection *con, int id, flag_t flag, flaginfo_t *tbl,
		 const char *tbl_name, const char *id_column,
		 const char *value_column)
{
	char stmt[MAX_INPUT_LENGTH];

	snprintf(stmt, sizeof(stmt),
	    "insert into %s (%s, %s) values (?, ?)",
	    tbl_name, id_column, value_column);

	int i;

	PreparedStatement *pstmt = con->prepareStatement(stmt);
	for (i = 1; tbl[i].name != NULL; i++) {
		if (IS_SET(flag, tbl[i].bit)) {
			pstmt->setInt(1, id);
			pstmt->setInt(2, tbl[i].bit);
			pstmt->executeUpdate();
		}
	}
	delete pstmt;
}

void
dump_flag_table(Connection *con, flaginfo_t *tbl, const char *tbl_name,
		const char *flag_name, const char *value_name)
{
	char stmt[MAX_INPUT_LENGTH];

	snprintf(stmt, sizeof(stmt),
	    "insert into %s (%s, %s) values (?, ?)",
	    tbl_name, flag_name, value_name);
	cout << "Inserting into " << tbl_name << "..";

	try {
		int i;

		PreparedStatement *pstmt = con->prepareStatement(stmt);
		for (i = 1; tbl[i].name != NULL; i++) {
			pstmt->setString(1, tbl[i].name);
			pstmt->setInt(2, tbl[i].bit);
			pstmt->executeUpdate();
		}
		delete pstmt;

		cout << " Done.." << endl;
	} catch (SQLException &e) {
		cerr << e.getMessage() << endl;
		cerr << " Failed.." << endl;
	}
}

void
dump_area_data(Connection *con)
{
	AREA_DATA *pArea;

	PreparedStatement *pstmt = con->prepareStatement(
	    "insert into area_list (area_id, area_name, area_credits, area_minlevel, area_maxlevel, area_minvnum, area_maxvnum, area_builders, area_security, area_resetmsg, area_clan) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	for (pArea = area_first; pArea; pArea = pArea->next) {
		pstmt->setInt(1, pArea->vnum);
		setString(pstmt, 2, pArea->name);
		setString(pstmt, 3, pArea->credits);
		pstmt->setInt(4, pArea->min_level);
		pstmt->setInt(5, pArea->max_level);
		pstmt->setInt(6, pArea->min_vnum);
		pstmt->setInt(7, pArea->max_vnum);
		setString(pstmt, 8, pArea->builders);
		pstmt->setInt(9, pArea->security);
		setMlString(con, pstmt, 10, &pArea->resetmsg);
		setString(pstmt, 11, pArea->clan);

		pstmt->executeUpdate();

		dump_flag_values(con, pArea->vnum,
				 pArea->area_flags, area_flags,
				 "area_flags", "area_id", "flag_value");

		dump_mobile_data(con, pArea);
		dump_object_data(con, pArea);
		dump_room_data(con, pArea);
		if (pArea->help_first != NULL)
			dump_help_data(con, pArea);

		cout << pArea->name << ": Done.." << endl;
	}
	delete pstmt;
}

void
dump_mobile_data(Connection *con, AREA_DATA *pArea)
{
	MOB_INDEX_DATA *pMob;
	int vnum;

	PreparedStatement *pstmt = con->prepareStatement
	    ("insert into mobile_list (mob_vnum, area_id, name, mob_level, mob_align, mob_hitroll, mob_wealth, mob_race, mob_damtype, mob_size, mob_startpos, mob_defpos, h1, h2, h3, m1, m2, m3, d1, d2, d3, ac_pierce, ac_bash, ac_slash, ac_exotic, gender, description, short_descr, long_descr, mob_fvnum, mob_clan, mob_invis_level, mob_incog_level, mob_material) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	cout << pArea->name << ": dump mobiles...";
	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pMob = get_mob_index(vnum)) != NULL) {
			pstmt->setInt(1, pMob->vnum);
			pstmt->setInt(2, pArea->vnum);
			setString(pstmt, 3, pMob->name);
			pstmt->setInt(4, pMob->level);
			pstmt->setInt(5, pMob->alignment);
			pstmt->setInt(6, pMob->hitroll);
			pstmt->setInt(7, pMob->wealth);
			setString(pstmt, 8, pMob->race);
			setString(pstmt, 9, pMob->damtype);
			pstmt->setInt(10, pMob->size);
			pstmt->setInt(11, pMob->start_pos);
			pstmt->setInt(12, pMob->default_pos);

			pstmt->setInt(13, pMob->hit[DICE_NUMBER]);
			pstmt->setInt(14, pMob->hit[DICE_TYPE]);
			pstmt->setInt(15, pMob->hit[DICE_BONUS]);

			pstmt->setInt(16, pMob->mana[DICE_NUMBER]);
			pstmt->setInt(17, pMob->mana[DICE_TYPE]);
			pstmt->setInt(18, pMob->mana[DICE_BONUS]);

			pstmt->setInt(19, pMob->damage[DICE_NUMBER]);
			pstmt->setInt(20, pMob->damage[DICE_TYPE]);
			pstmt->setInt(21, pMob->damage[DICE_BONUS]);

			pstmt->setInt(22, pMob->ac[AC_PIERCE]);
			pstmt->setInt(23, pMob->ac[AC_BASH]);
			pstmt->setInt(24, pMob->ac[AC_SLASH]);
			pstmt->setInt(25, pMob->ac[AC_EXOTIC]);

			setMlString(con, pstmt, 26, &pMob->gender);
			setMlString(con, pstmt, 27, &pMob->description);
			setMlString(con, pstmt, 28, &pMob->short_descr);
			setMlString(con, pstmt, 29, &pMob->long_descr);

			setInt(pstmt, 30, pMob->fvnum);
			setString(pstmt, 31, pMob->clan);
			setInt(pstmt, 32, pMob->invis_level);
			setInt(pstmt, 33, pMob->incog_level);

			setString(pstmt, 34, pMob->material);

			pstmt->executeUpdate();

			dump_flag_values(con, pMob->vnum,
			    pMob->affected_by, affect_flags,
			    "mobile_affected_list", "mob_vnum",
			    "aff_value");
			dump_flag_values(con, pMob->vnum,
			    pMob->has_invis, id_flags,
			    "mobile_invis_list", "mob_vnum",
			    "id_value");
			dump_flag_values(con, pMob->vnum,
			    pMob->has_detect, id_flags,
			    "mobile_detect_list", "mob_vnum",
			    "id_value");
			dump_flag_values(con, pMob->vnum, pMob->act,
			    mob_act_flags, "mobile_act_list",
			    "mob_vnum", "act_value");
			dump_flag_values(con, pMob->vnum,
			    pMob->mob_flags, mob_flags,
			    "mobile_mobflags_list",
			    "mob_vnum", "mobflag_value");
			dump_flag_values(con, pMob->vnum,
			    pMob->off_flags, off_flags,
			    "mobile_off_list",
			    "mob_vnum", "off_value");
			dump_mob_resistances(con, pMob);
			if (pMob->pShop != NULL)
				insertShop(con, pMob->pShop);
			if (pMob->affected != NULL) {
				dump_affect_data(con, pMob->affected,
				    pMob->vnum, "mob");
			}
			if (&pMob->practicer != NULL)
				insertPracticer(con, pMob);
		}
	}
	cout << " Done.." << endl;
	delete pstmt;
}

void
dump_mob_resistances(Connection *con, MOB_INDEX_DATA *pMob)
{
	PreparedStatement *pstmt = con->prepareStatement
	    ("insert into mobile_resist_list (mob_vnum, damclass_value, percent) values (?, ?, ?)");
	for (int i = 0; i < MAX_RESIST; i++) {
		if (pMob->resists[i] == RES_UNDEF)
			continue;

		pstmt->setInt(1, pMob->vnum);
		pstmt->setInt(2, i);
		pstmt->setInt(3, pMob->resists[i]);

		pstmt->executeUpdate();
	}
	delete pstmt;
}

void
dump_object_data(Connection *con, AREA_DATA *pArea)
{
	OBJ_INDEX_DATA *pObj;
	int vnum;

	PreparedStatement *pstmt = con->prepareStatement
	    ("insert into object_list (obj_vnum, obj_name, obj_material, long_descr, short_descr, obj_level, obj_condition, obj_weight, obj_cost, obj_itemtype, obj_limit, obj_gender) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	cout << pArea->name << ": dump objects...";
	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pObj = get_obj_index(vnum)) != NULL) {
			pstmt->setInt(1, pObj->vnum);
			setString(pstmt, 2, pObj->name);
			setString(pstmt, 3, pObj->material);
			setMlString(con, pstmt, 4, &pObj->description);
			setMlString(con, pstmt, 5, &pObj->short_descr);
			pstmt->setInt(6, pObj->level);
			pstmt->setInt(7, pObj->condition);
			pstmt->setInt(8, pObj->weight);
			pstmt->setInt(9, pObj->cost);
			pstmt->setInt(10, pObj->item_type);
			setInt(pstmt, 11, pObj->limit);
			setMlString(con, pstmt, 12, &pObj->gender);

			pstmt->executeUpdate();
			dump_flag_values(con, pObj->vnum,
					 pObj->stat_flags, stat_flags,
					 "obj_statflags_list",
					 "obj_vnum", "statflag_value");
			dump_flag_values(con, pObj->vnum,
					 pObj->obj_flags, obj_flags,
					 "obj_objflags_list",
					 "obj_vnum", "objflag_value");
			dump_flag_values(con, pObj->vnum,
					 pObj->wear_flags, obj_flags,
					 "obj_wearflags_list",
					 "obj_vnum", "wearflag_value");
			if (pObj->ed != NULL) {
				insertExd(con, pObj->vnum, pObj->ed,
					  "obj_exd_list", "obj_vnum");
			}
			dump_object_vo(con, pObj);

			if (pObj->affected != NULL) {
				dump_affect_data(con, pObj->affected,
				    pObj->vnum, "obj");
			}
		}
	}
	delete pstmt;
	cout << " Done.. " << endl;
}

void dump_room_data(Connection *con, AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoom;
	int vnum;

	PreparedStatement *pstmt = con->prepareStatement
	    ("insert into room_list (room_vnum, room_name, room_description, room_sector, room_healrate, room_manarate) values (?, ?, ?, ?, ?, ?)");

	cout << pArea->name << ": dump rooms...";
	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pRoom = get_room_index(vnum)) != NULL) {
			pstmt->setInt(1, pRoom->vnum);
			setMlString(con, pstmt, 2, &pRoom->name);
			setMlString(con, pstmt, 3, &pRoom->description);
			pstmt->setInt(4, pRoom->sector_type);
			pstmt->setInt(5, pRoom->heal_rate);
			pstmt->setInt(6, pRoom->mana_rate);

			pstmt->executeUpdate();

			dump_flag_values(con, pRoom->vnum,
					 pRoom->room_flags, room_flags,
					 "roomflags_list",
					 "room_vnum", "roomflag_value");
			dump_flag_values(con, pRoom->vnum,
					 pRoom->affected_by,
					 raffect_flags,
					 "raffects_list",
					 "room_vnum", "raffect_value");
			if (pRoom->ed != NULL) {
				insertExd(con, pRoom->vnum, pRoom->ed,
					  "room_exd_list", "room_vnum");
			}
			if (pRoom->affected != NULL) {
				dump_affect_data(con, pRoom->affected,
				    pRoom->vnum, "room");
			}
			dump_exit_data(con, pRoom);
		}
	}
	delete pstmt;
	cout << " Done.. " << endl;
}

void
dump_help_data (Connection *con, AREA_DATA *pArea)
{
	HELP_DATA *help;

	PreparedStatement *pstmt = con->prepareStatement
	    ("insert into help_list (area_id, help_level, help_keyword, help_text) values (?, ?, ?, ?)");

	cout << pArea->name << ": dump helps...";
	for (help = pArea->help_first; help != NULL ; help = help->next_in_area) {
		pstmt->setInt(1, pArea->vnum);
		pstmt->setInt(2, help->level);
		setString(pstmt, 3, help->keyword);
		setMlString(con, pstmt, 4, &help->text);

		pstmt->executeUpdate();
	}
	delete pstmt;
	cout << " Done.. " << endl;
}


