drop table reset_list;
drop table triggers_list;
drop table practicer_list;
drop TABLE exit_flags_list;
drop table area_flags;
drop table area_flags_conf;
drop table room_exit_list;
drop table exit_table;
drop table obj_aff_bitvector_list;
drop table mob_aff_bitvector_list;
drop table room_aff_bitvector_list;
drop table shop_list;
drop table seller_list;
drop table obj_affect_data;
drop table mob_affect_data;
drop table room_affect_data;
drop table affect_where_conf;
drop table mobile_affected_list;
drop table mobile_detect_list;
drop table mobile_invis_list;
drop table mobile_mobflags_list;
drop table mobile_off_list;
drop table mobile_resist_list;
drop table mobile_act_list;
drop table help_list;
drop TABLE weapon_list;
drop table mobile_list;
drop table area_list;
drop table damclass_list;
drop table mobile_affected_conf;
drop table mobile_off_conf;
drop table mobile_id_conf;
drop table mobile_parts_conf;
drop table mobile_form_conf;
drop table pos_list;
drop table size_list;
drop table mobile_mobflags_conf;
drop table mobile_act_conf;
drop TABLE obj_exd_list;
drop table obj_objflags_list;
drop table obj_statflags_list;
drop table obj_wearflags_list; 
drop TABLE light_list;
drop TABLE wand_list;
drop TABLE staff_list;
drop TABLE armor_list;
drop TABLE potion_list;
drop TABLE pill_list;
drop TABLE scroll_list;
drop TABLE money_list;
drop TABLE furniture_list;
drop TABLE container_list;
drop TABLE fountain_list;
drop TABLE drink_con_list;
drop TABLE portal_list;
drop TABLE book_list;
drop TABLE herb_list;
drop TABLE weapon_flags_list;
drop TABLE furniture_flags_list;
drop TABLE eportal_flags_list;
drop TABLE portal_flags_list;
drop table object_list;
drop table obj_itemtypes_conf;
drop table obj_objflags_conf;
drop table obj_statflags_conf;
drop table obj_wearflags_conf; 
drop TABLE exit_flags_conf;
drop TABLE furniture_flags_conf;
drop TABLE portal_flags_conf;
drop TABLE book_class_conf;
drop TABLE fail_effects_conf;
drop TABLE cont_flags_conf;
drop TABLE weapon_class_conf;
drop TABLE weapon_flags_conf;
drop TABLE room_exd_list;
drop TABLE raffects_list;
drop TABLE roomflags_list;
drop TABLE room_list;
drop TABLE sector_types_conf;
drop TABLE room_flags_conf;
drop TABLE raffect_flags_conf;
drop TABLE exd_table;
drop table gmlstr_table;
drop table mlstring_table;
drop table skills_groups;
drop table mptrig_types;

-- global mlstring table
CREATE TABLE mlstring_table(
	mlstring_id serial PRIMARY KEY,
	nlang integer,
	text0 text,
	text1 text
);

CREATE TABLE gmlstr_table(
	gmlstr_id serial PRIMARY KEY,
	gender_id integer REFERENCES mlstring_table (mlstring_id) ON DELETE SET NULL ON UPDATE CASCADE,
	text_id integer REFERENCES mlstring_table (mlstring_id) ON DELETE SET NULL ON UPDATE CASCADE
);

-- global exd table
CREATE TABLE exd_table(
	exd_id serial PRIMARY KEY,
	exd_keyword text,
	description integer REFERENCES mlstring_table (mlstring_id) ON DELETE SET NULL ON UPDATE CASCADE
);

-- general area list
CREATE TABLE area_list(
	area_id integer PRIMARY KEY,
	-- name
	area_name text UNIQUE, 
	-- area credits
	area_credits text,
	-- area levels
	area_minlevel integer,
	area_maxlevel integer,
	-- area vnums
	area_minvnum integer,
	area_maxvnum integer,
	-- list of builders
	area_builders text,
	-- security
	area_security integer DEFAULT 0,
	-- reset message
	area_resetmsg integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	-- clan
	area_clan text
);

-- area flags conf
CREATE TABLE area_flags_conf(
	area_flag varchar(50) NOT NULL UNIQUE,
	flag_value integer PRIMARY KEY
);

-- area flags list
CREATE TABLE area_flags(
	area_id integer REFERENCES area_list (area_id) ON DELETE CASCADE ON UPDATE CASCADE,
	flag_value integer REFERENCES area_flags_conf (flag_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (area_id, flag_value)
);

-- size_list
CREATE TABLE size_list(
	size_name varchar(50) NOT NULL UNIQUE,
	size_value integer PRIMARY KEY
);

-- position_list
CREATE TABLE pos_list(
	position_name varchar(50) NOT NULL UNIQUE,
	position_value integer PRIMARY KEY
);

-- mobile's off conf
CREATE TABLE mobile_off_conf(
	off_flag_name varchar(50) NOT NULL UNIQUE,
	off_value integer PRIMARY KEY
);

-- mobile's id conf
CREATE TABLE mobile_id_conf(
	id_name varchar(50) NOT NULL UNIQUE,
	id_value integer PRIMARY KEY

);

-- mobile's affected_by conf
CREATE TABLE mobile_affected_conf(
	aff_name varchar(50) NOT NULL UNIQUE,
	aff_value integer PRIMARY KEY
);

-- mobile's acts conf
CREATE TABLE mobile_act_conf(
	act_name varchar(50) NOT NULL UNIQUE,
	act_value integer PRIMARY KEY
);

-- mobile's mobflags conf
CREATE TABLE mobile_mobflags_conf(
	mobflag_name varchar(50) NOT NULL UNIQUE,
	mobflag_value integer PRIMARY KEY
);

-- mobile's parts conf
CREATE TABLE mobile_parts_conf(
	part_name varchar(50) NOT NULL UNIQUE,
	part_value integer PRIMARY KEY
);

-- mobile's forms conf
CREATE TABLE mobile_form_conf(
	form_name varchar(50) NOT NULL UNIQUE,
	form_value integer PRIMARY KEY
);

-- damtype list
CREATE TABLE damclass_list(
	damclass_name varchar(50) NOT NULL UNIQUE,
	damclass_value integer PRIMARY KEY
);

-- general mobile list
CREATE TABLE mobile_list(
	-- vnum
	mob_vnum integer PRIMARY KEY,
	-- area
	area_id integer REFERENCES area_list (area_id) ON DELETE CASCADE ON UPDATE CASCADE,
	-- name
	name text DEFAULT NULL,
	-- level
	mob_level integer DEFAULT 0,
	-- alignment
	mob_align integer DEFAULT 0,
	-- hitroll
	mob_hitroll integer DEFAULT 0,
	-- wealth
	mob_wealth integer DEFAULT 0,
	-- race
	mob_race varchar(50),
	-- damage type
	mob_damtype text,
	-- size
	mob_size integer REFERENCES size_list (size_value) ON DELETE SET NULL ON UPDATE CASCADE,
	-- start position
	mob_startpos integer REFERENCES pos_list (position_value) ON DELETE SET NULL ON UPDATE CASCADE,
	-- default position
	mob_defpos integer REFERENCES pos_list (position_value) ON DELETE SET NULL ON UPDATE CASCADE,
	-- hit dice <h1>d<h2> + <h3>
	h1 integer DEFAULT 1,
	h2 integer DEFAULT 1,
	h3 integer DEFAULT 1,
	-- mana dice <m1>d<m2> + <m3>
	m1 integer DEFAULT 1,
	m2 integer DEFAULT 1,
	m3 integer DEFAULT 1,
	-- damage dice <d1>d<d2> + <d3>
	d1 integer DEFAULT 1,
	d2 integer DEFAULT 1,
	d3 integer DEFAULT 1,
	-- ac
	ac_pierce integer DEFAULT 0,
	ac_bash integer DEFAULT 0,
	ac_slash integer DEFAULT 0,
	ac_exotic integer DEFAULT 0,
	-- gender
	gender integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	-- description
	description integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	-- short description
	short_descr integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	-- long description
	long_descr integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	-- fvnum
	mob_fvnum integer,
	-- clan
	mob_clan text,
	-- mob invis level
	mob_invis_level integer,
	-- mob incog level
	mob_incog_level integer,
	-- material
	mob_material text
);

-- resistance list
CREATE TABLE mobile_resist_list(
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	damclass_value integer REFERENCES damclass_list (damclass_value) ON DELETE CASCADE ON UPDATE CASCADE,
	percent integer CHECK (percent <= 100 AND percent >= -100),
	PRIMARY KEY (mob_vnum, damclass_value)
);

-- act list
CREATE TABLE mobile_act_list(
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	act_value integer REFERENCES mobile_act_conf (act_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (mob_vnum, act_value)
);

-- off list
CREATE TABLE mobile_off_list(
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	off_value integer REFERENCES mobile_off_conf (off_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (mob_vnum, off_value)
);

-- invis list
CREATE TABLE mobile_invis_list(
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	id_value integer REFERENCES mobile_id_conf (id_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (mob_vnum, id_value)
);

-- detect list
CREATE TABLE mobile_detect_list(
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	id_value integer REFERENCES mobile_id_conf (id_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (mob_vnum, id_value)
);

-- affected_by list
CREATE TABLE mobile_affected_list(
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	aff_value integer REFERENCES mobile_affected_conf (aff_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (mob_vnum, aff_value)
);
-- mobflags_list
CREATE TABLE mobile_mobflags_list(
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	mobflag_value integer REFERENCES mobile_mobflags_conf (mobflag_value) ON DELETE CASCADE ON UPDATE CASCADE
);

-- obj stat flag conf
CREATE TABLE obj_statflags_conf(
	statflag_name varchar(50) NOT NULL UNIQUE,
	statflag_value integer PRIMARY KEY
);

-- item types conf
CREATE TABLE obj_itemtypes_conf(
	itemtype_name varchar(50) NOT NULL UNIQUE,
	itemtype_value integer PRIMARY KEY
);

-- obj obj flag conf
CREATE TABLE obj_objflags_conf(
	objflag_name varchar(50) NOT NULL UNIQUE,
	objflag_value integer PRIMARY KEY
);

-- obj wear flag conf
CREATE TABLE obj_wearflags_conf(
	wearflag_name varchar(50) NOT NULL UNIQUE,
	wearflag_value integer PRIMARY KEY
);

-- object list
CREATE TABLE object_list(
	obj_vnum integer PRIMARY KEY,
	obj_name text,
	obj_material text,
 	long_descr integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	short_descr integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	obj_level integer,
	obj_condition integer,
	obj_weight integer,
	obj_cost integer,
	obj_itemtype integer REFERENCES obj_itemtypes_conf (itemtype_value) ON DELETE CASCADE ON UPDATE CASCADE,
	obj_limit integer,
	obj_gender integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL
);

CREATE TABLE obj_statflags_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON UPDATE CASCADE ON DELETE CASCADE,
	statflag_value integer REFERENCES obj_statflags_conf (statflag_value) ON UPDATE CASCADE ON DELETE CASCADE,
	PRIMARY KEY (obj_vnum, statflag_value)
);

CREATE TABLE obj_objflags_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON UPDATE CASCADE ON DELETE CASCADE,
	objflag_value integer REFERENCES obj_objflags_conf (objflag_value) ON UPDATE CASCADE ON DELETE CASCADE,
	PRIMARY KEY (obj_vnum, objflag_value)
);

CREATE TABLE obj_wearflags_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON UPDATE CASCADE ON DELETE CASCADE,
	wearflag_value integer REFERENCES obj_wearflags_conf (wearflag_value) ON UPDATE CASCADE ON DELETE CASCADE,
	PRIMARY KEY (obj_vnum, wearflag_value)
);

CREATE TABLE sector_types_conf(
	sector_name varchar(50) NOT NULL UNIQUE,
	sector_value integer PRIMARY KEY
);

CREATE TABLE room_flags_conf(
	roomflag_name varchar(50) NOT NULL UNIQUE,
	roomflag_value integer PRIMARY KEY
);

CREATE TABLE raffect_flags_conf(
	raffect_name varchar (50) NOT NULL UNIQUE,
	raffect_value integer PRIMARY KEY
);

CREATE TABLE room_list(
	room_vnum integer PRIMARY KEY,
	room_name integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	room_description integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	room_sector integer REFERENCES sector_types_conf (sector_value) ON UPDATE CASCADE ON DELETE SET NULL,
	room_healrate integer DEFAULT 100,
	room_manarate integer DEFAULT 100
);

CREATE TABLE roomflags_list(
	room_vnum integer REFERENCES room_list (room_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	roomflag_value integer REFERENCES room_flags_conf (roomflag_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (room_vnum, roomflag_value)
);

CREATE TABLE raffects_list(
	room_vnum integer REFERENCES room_list (room_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	raffect_value integer REFERENCES raffect_flags_conf (raffect_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (room_vnum, raffect_value)
);

CREATE TABLE obj_exd_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	exd_id integer REFERENCES exd_table (exd_id) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (obj_vnum, exd_id)
);
	
CREATE TABLE room_exd_list(
	room_vnum integer REFERENCES room_list (room_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	exd_id integer REFERENCES exd_table (exd_id) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (room_vnum, exd_id)
);
	
CREATE TABLE exit_flags_conf(
	exitflag_name varchar(50) NOT NULL UNIQUE,
	exitflag_value integer PRIMARY KEY
);

CREATE TABLE furniture_flags_conf(
	furn_name varchar(50) NOT NULL UNIQUE,
	furn_value integer PRIMARY KEY
);

CREATE TABLE portal_flags_conf(
	portalflag_name varchar(50) NOT NULL UNIQUE,
	portalflag_value integer PRIMARY KEY
);

CREATE TABLE book_class_conf(
	bclass_name varchar(50) NOT NULL UNIQUE,
	bclass_value integer PRIMARY KEY
);

CREATE TABLE fail_effects_conf(
	fail_name varchar(50) NOT NULL UNIQUE,
	fail_value integer PRIMARY KEY
);

CREATE TABLE cont_flags_conf(
	contflag_name varchar(50) NOT NULL UNIQUE,
	contflag_value integer PRIMARY KEY
);

CREATE TABLE weapon_class_conf(
	weapon_name varchar(50) NOT NULL UNIQUE,
	weapon_value integer PRIMARY KEY
);

CREATE TABLE weapon_flags_conf(
	wflag_name varchar(50) NOT NULL UNIQUE,
	wflag_value integer PRIMARY KEY
);

CREATE TABLE light_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	light integer,
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE wand_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	spell_level integer,
	charges integer,
	charges_left integer,
	spell_name varchar(50),
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE staff_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	spell_level integer,
	charges integer,
	charges_left integer,
	spell_name varchar(50),
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE armor_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	ac_pierce integer,
	ac_bash integer,
	ac_slash integer,
	ac_exotic integer,
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE potion_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	spell_level integer,
	spell_1 varchar (50),
	spell_2 varchar (50),
	spell_3 varchar (50),
	spell_4 varchar (50),
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE pill_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	spell_level integer,
	spell_1 varchar (50),
	spell_2 varchar (50),
	spell_3 varchar (50),
	spell_4 varchar (50),
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE scroll_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	spell_level integer,
	spell_1 varchar (50),
	spell_2 varchar (50),
	spell_3 varchar (50),
	spell_4 varchar (50),
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE money_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	silver integer,
	gold integer,
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE weapon_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	weapon_name integer REFERENCES weapon_class_conf (weapon_value) ON DELETE SET NULL ON UPDATE CASCADE,
	d1 integer,
	d2 integer,
	dam_type varchar(50),
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE furniture_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	max_people integer,
	max_weight integer,
	heal_bonus integer,
	mana_bonus integer,
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE container_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	weight integer,
	key integer,
	capacity integer,
	mult integer,
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE fountain_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	total integer,
	left_liq integer,
	liquid varchar(50),
	poisoned bool,
	PRIMARY KEY (obj_vnum)
);
	
CREATE TABLE drink_con_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	total integer,
	left_liq integer,
	liquid varchar(50),
	poisoned bool,
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE portal_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	charges integer,
	to_vnum integer,
	key integer REFERENCES object_list (obj_vnum) ON DELETE SET NULL ON UPDATE CASCADE,
	PRIMARY KEY (obj_vnum)
);
	
CREATE TABLE book_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	book_class integer REFERENCES book_class_conf (bclass_value) ON DELETE SET NULL ON UPDATE CASCADE,
	spec text,
	chance integer,
	fail_effect integer REFERENCES fail_effects_conf (fail_value) ON DELETE SET NULL ON UPDATE CASCADE,
	message text,
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE herb_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	spell varchar(50),
	hit integer,
	mana integer,
	moves integer,
	PRIMARY KEY (obj_vnum)
);

CREATE TABLE weapon_flags_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	wflag_value integer REFERENCES weapon_flags_conf (wflag_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (obj_vnum, wflag_value)
);

CREATE TABLE furniture_flags_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	furn_value integer REFERENCES furniture_flags_conf (furn_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (obj_vnum, furn_value)
);

CREATE TABLE portal_flags_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	portalflag_value integer REFERENCES portal_flags_conf (portalflag_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (obj_vnum, portalflag_value)
);

CREATE TABLE eportal_flags_list(
	obj_vnum integer REFERENCES object_list (obj_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	exitflag_value integer REFERENCES exit_flags_conf (exitflag_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (obj_vnum, exitflag_value)
);

CREATE TABLE help_list(
	help_id serial PRIMARY KEY,
	area_id integer,
	help_level integer,
	help_keyword text,
	help_text integer REFERENCES mlstring_table (mlstring_id) ON DELETE SET NULL ON UPDATE CASCADE
);

CREATE TABLE shop_list(
	shop_id serial,
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	profit_buy integer,
	profit_sell integer,
	open_hour integer,
	close_hour integer,
	PRIMARY KEY (mob_vnum)
);

CREATE TABLE seller_list(
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	obj_itemtype integer REFERENCES obj_itemtypes_conf (itemtype_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (mob_vnum, obj_itemtype)
);

CREATE TABLE affect_where_conf(
	where_name varchar(50) UNIQUE NOT NULL,
	where_value integer PRIMARY KEY
);

CREATE TABLE obj_affect_data(
	affect_id serial PRIMARY KEY, 
	obj_vnum integer REFERENCES object_list (obj_vnum) ON UPDATE CASCADE ON DELETE CASCADE,
	aff_where integer REFERENCES affect_where_conf (where_value) ON DELETE SET NULL ON UPDATE CASCADE,
	aff_type text,
	aff_level integer,
	duration integer,
	location_i integer,
	location_s text,
	modifier integer
);

CREATE TABLE obj_aff_bitvector_list(
	affect_id integer REFERENCES obj_affect_data (affect_id) ON UPDATE CASCADE ON DELETE CASCADE,
	bitvector_value integer,
	PRIMARY KEY (affect_id, bitvector_value)
);

CREATE TABLE mob_affect_data(
	affect_id serial PRIMARY KEY, 
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON UPDATE CASCADE ON DELETE CASCADE,
	aff_where integer REFERENCES affect_where_conf (where_value) ON DELETE SET NULL ON UPDATE CASCADE,
	aff_type text,
	aff_level integer,
	duration integer,
	location_i integer,
	location_s text,
	modifier integer
);

CREATE TABLE mob_aff_bitvector_list(
	affect_id integer REFERENCES mob_affect_data (affect_id) ON UPDATE CASCADE ON DELETE CASCADE,
	bitvector_value integer,
	PRIMARY KEY (affect_id, bitvector_value)
);

CREATE TABLE room_affect_data(
	affect_id serial PRIMARY KEY, 
	room_vnum integer REFERENCES room_list (room_vnum) ON UPDATE CASCADE ON DELETE CASCADE,
	aff_where integer REFERENCES affect_where_conf (where_value) ON DELETE SET NULL ON UPDATE CASCADE,
	aff_type text,
	aff_level integer,
	duration integer,
	location_i integer,
	location_s text,
	modifier integer
);

CREATE TABLE room_aff_bitvector_list(
	affect_id integer REFERENCES room_affect_data (affect_id) ON UPDATE CASCADE ON DELETE CASCADE,
	bitvector_value integer,
	PRIMARY KEY (affect_id, bitvector_value)
);

CREATE TABLE exit_table(
	exit_id serial PRIMARY KEY,
	exit_size integer,
	key integer,
	keyword text,
	to_room integer,
	description integer REFERENCES mlstring_table (mlstring_id) ON UPDATE CASCADE ON DELETE SET NULL,
	short_descr integer REFERENCES gmlstr_table (gmlstr_id) ON UPDATE CASCADE ON DELETE SET NULL
);

CREATE TABLE room_exit_list(
	room_vnum integer PRIMARY KEY,
	north_exit integer,
	east_exit integer,
	south_exit integer,
	west_exit integer,
	up_exit integer,
	down_exit integer
);

CREATE TABLE exit_flags_list(
	exit_id integer REFERENCES exit_table (exit_id) ON DELETE CASCADE ON UPDATE CASCADE,
	exitflag_value integer REFERENCES exit_flags_conf (exitflag_value) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (exit_id, exitflag_value)
);

CREATE TABLE skills_groups(
	group_name text UNIQUE NOT NULL,
	group_id integer PRIMARY KEY
);

CREATE TABLE practicer_list(
	mob_vnum integer REFERENCES mobile_list (mob_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	group_id integer REFERENCES skills_groups (group_id) ON DELETE CASCADE ON UPDATE CASCADE,
	PRIMARY KEY (mob_vnum, group_id)
);

CREATE TABLE mptrig_types(
	mptrig_name text UNIQUE NOT NULL,
	mptrig_value integer PRIMARY KEY
);

CREATE TABLE triggers_list(
	trig_id serial PRIMARY KEY,
	vnum integer,
	vnum_type varchar(20),
	trig_type integer REFERENCES mptrig_types (mptrig_value) ON DELETE SET NULL ON UPDATE CASCADE,
	trig_prog text,
	trig_arg text,
	trig_text text
);

CREATE TABLE reset_list(
	room_vnum integer REFERENCES room_list (room_vnum) ON DELETE CASCADE ON UPDATE CASCADE,
	command varchar(1),
	arg0 integer,
	arg1 integer,
	arg2 integer,
	arg3 integer,
	arg4 integer
);
