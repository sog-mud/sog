#
# $Id: Makefile,v 1.7 1999-07-30 05:18:21 avn Exp $
#

.include "../Makefile.inc"

MODULE_NAME = sog
MODULE_MASK = sog
INSTALL_DIR = bin

CFLAGS += -DABI_VERSION=$(ABI_VERSION)

COMM_CFILES = comm/ban.c comm/charset.c comm/comm.c comm/comm_act.c\
	      comm/comm_colors.c comm/comm_info.c comm/resolver.c

DB_CFILES = db/db.c db/db_area.c db/db_clan.c db/db_class.c db/db_cmd.c\
	    db/db_hometown.c db/db_lang.c db/db_msg.c db/db_race.c\
	    db/db_skills.c db/db_socials.c db/db_system.c db/db_rspells.c

CFILES = auction.c buffer.c chquest.c clan.c class.c cmd.c\
	 effects.c fight.c flag.c gsn.c handler.c help.c hometown.c\
	 interp.c lang.c log.c lookup.c magic.c\
	 memalloc.c mlstring.c mob_cmds.c mob_prog.c module.c msg.c namedp.c\
	 note.c obj_prog.c quest.c race.c raffects.c\
	 rating.c recycle.c religion.c save.c skills.c\
	 socials.c special.c str.c string_edit.c tables.c update.c\
	 util.c varr.c\
	 $(COMM_CFILES) $(DB_CFILES) $(COMPAT_CFILES)

.include "../Makefile.rules"
