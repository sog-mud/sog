#
# $Id: Makefile,v 1.24 1999-12-20 08:31:20 fjoe Exp $
#

MODULE_NAME = sog
INSTALL_DIR = bin

COMM_CFILES = comm/ban.c comm/charset.c comm/comm.c comm/comm_act.c\
	      comm/comm_colors.c comm/comm_info.c comm/resolver.c

DB_CFILES = db/db.c db/db_area.c db/db_cc_expr.c db/db_clan.c\
	    db/db_class.c db/db_cmd.c db/db_damtype.c db/db_hometown.c\
	    db/db_lang.c db/db_liquid.c db/db_material.c\
	    db/db_race.c db/db_skills.c db/db_socials.c db/db_spec.c\
	    db/db_system.c db/db_form.c

CFILES = auction.c affect.c buffer.c cc_expr.c chquest.c clan.c class.c cmd.c\
	 damtype.c effects.c fight.c flag.c handler.c hash.c help.c\
	 hometown.c interp.c lang.c liquid.c log.c magic.c material.c\
	 memalloc.c mlstring.c mob_cmds.c mob_prog.c module.c\
	 namelist.c note.c obj_prog.c objval.c quest.c race.c\
	 rating.c recycle.c religion.c rfile.c save.c skills.c\
	 socials.c spec.c special.c str.c string_edit.c strkey_hash.c\
	 tables.c update.c util.c varr.c forms.c\
	 $(COMM_CFILES) $(DB_CFILES) $(COMPAT_CFILES)

OFILES_ADD = cc_parse.o cc_lex.o
CLEAN_FILES = y.tab.h

.include "../Makefile.osdep"

cc_parse.o: cc_parse.y cc_lex.h ../include/typedef.h ../include/str.h\
	    ../include/log.h

cc_lex.o: cc_lex.l cc_lex.h ../include/typedef.h ../include/str.h\
	  ../include/varr.h ../include/cc_expr.h

