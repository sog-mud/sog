#
# $Id: Makefile,v 1.2 1999-06-24 20:35:04 fjoe Exp $
#

MODULE_NAME = sog

COMM_CFILES = comm/ban.c comm/charset.c comm/comm.c comm/comm_act.c\
	      comm/comm_colors.c comm/comm_info.c comm/resolver.c

DB_CFILES = db/db.c db/db_area.c db/db_clan.c db/db_class.c db/db_hometown.c\
	    db/db_lang.c db/db_msg.c db/db_race.c db/db_skills.c\
	    db/db_socials.c db/db_system.c

CFILES = auction.c buffer.c chquest.c clan.c class.c cmd.c\
	 effects.c fight.c flag.c gsn.c handler.c healer.c help.c hometown.c\
	 hunt.c interp.c lang.c log.c lookup.c magic.c\
	 memalloc.c mlstring.c mob_cmds.c mob_prog.c module.c msg.c namedp.c\
	 note.c obj_prog.c quest.c race.c raffects.c\
	 rating.c recycle.c religion.c rspells.c save.c skills.c\
	 socials.c special.c str.c string_edit.c tables.c update.c\
	 util.c varr.c\
	 $(COMM_CFILES) $(DB_CFILES)

install: $(MODULE_NAME)
	install $(INSTALL_FLAGS) $(MODULE_NAME) $(INSTALL_PATH)/bin

.include "../Makefile.inc"

