#
# $Id: Makefile,v 1.11.2.1 2002-01-05 16:21:59 matrim Exp $
#

MODULE_NAME = sog
INSTALL_DIR = bin

COMM_CFILES = comm/ban.c comm/charset.c comm/comm.c comm/comm_act.c\
	      comm/comm_colors.c comm/comm_info.c comm/resolver.c comm/mccp.c

DB_CFILES = db/db.c db/db_area.c db/db_clan.c db/db_class.c db/db_cmd.c\
	    db/db_hometown.c db/db_lang.c db/db_msg.c db/db_race.c\
	    db/db_skills.c db/db_socials.c db/db_system.c db/db_rspells.c

CFILES = auction.c buffer.c chquest.c clan.c class.c cmd.c\
	 effects.c fight.c flag.c gsn.c handler.c help.c hometown.c\
	 interp.c lang.c log.c lookup.c magic.c\
	 memalloc.c mlstring.c mob_cmds.c mob_prog.c module.c msg.c namedp.c\
	 note.c obj_prog.c quest.c race.c raffect.c\
	 rating.c recycle.c religion.c save.c skills.c\
	 socials.c special.c str.c string_edit.c tables.c update.c\
	 util.c varr.c\
	 $(COMM_CFILES) $(DB_CFILES) $(COMPAT_CFILES)

.include "../Makefile.osdep"
