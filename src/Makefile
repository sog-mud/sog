#
# $Id: Makefile,v 1.50 1999-06-22 14:29:41 fjoe Exp $
#

.include "Makefile.inc"

SUBDIR = spellfun

COMM_CFILES = comm/ban.c comm/charset.c comm/comm.c comm/comm_act.c\
	      comm/comm_colors.c comm/comm_info.c comm/resolver.c
COMM_OFILES = ban.o charset.o comm.o comm_act.o\
	      comm_colors.o comm_info.o resolver.o

DB_CFILES = db/db.c db/db_area.c db/db_clan.c db/db_class.c db/db_hometown.c\
	    db/db_lang.c db/db_msg.c db/db_race.c db/db_skills.c\
	    db/db_socials.c db/db_system.c\
	    db/cmd.c db/gsn.c db/hometown.c db/lang.c db/memalloc.c\
	    db/msg.c db/socials.c
DB_OFILES = db.o db_area.o db_clan.o db_class.o db_hometown.o\
	    db_lang.o db_msg.o db_race.o db_skills.o\
	    db_socials.o db_system.o\
	    cmd.o gsn.o hometown.o lang.o memalloc.o\
	    msg.o socials.o

OLC_CFILES = olc/olc.c olc/olc_area.c olc/olc_clan.c olc/olc_help.c\
	     olc/olc_lang.c olc/olc_mob.c olc/olc_mpcode.c olc/olc_msg.c\
	     olc/olc_room.c olc/olc_rule.c olc/olc_obj.c olc/olc_save.c\
	     olc/olc_social.c
OLC_OFILES = olc.o olc_area.o olc_clan.o olc_help.o\
	     olc_lang.o olc_mob.o olc_mpcode.o olc_msg.o\
	     olc_room.o olc_rule.o olc_obj.o olc_save.o\
	     olc_social.o

CFILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c auction.c\
         buffer.c cast.c chquest.c clan.c class.c dl.c\
	 effects.c fight.c flag.c handler.c healer.c help.c\
	 hunt.c interp.c log.c lookup.c martial_art.c\
	 mlstring.c mob_cmds.c mob_prog.c namedp.c note.c obj_prog.c quest.c\
	 race.c raffects.c rating.c recycle.c religion.c repair.c rspells.c\
	 save.c skills.c special.c str.c string_edit.c\
	 tables.c update.c util.c varr.c\
	 $(COMM_CFILES) $(DB_CFILES) $(OLC_CFILES) $(COMPAT_CFILES)

OFILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o auction.o\
         buffer.o cast.o chquest.o clan.o class.o dl.o\
	 effects.o fight.o flag.o handler.o healer.o help.o\
	 hunt.o interp.o log.o lookup.o martial_art.o\
	 mlstring.o mob_cmds.o mob_prog.o namedp.o note.o obj_prog.o quest.o\
	 race.o raffects.o rating.o recycle.o religion.o repair.o rspells.o\
	 save.o skills.o special.o str.o string_edit.o\
	 tables.o update.o util.o varr.o\
	 $(COMM_OFILES) $(DB_OFILES) $(OLC_OFILES) $(COMPAT_OFILES)

all: sog
	@for i in $(SUBDIR); do \
		(echo "===> $${i}"; \
		cd ${.CURDIR}/$${i}; \
		${MAKE} ${.TARGET}); \
	done

sog: $(OFILES)
	rm -f sog
	$(LD) $(LFLAGS) -o sog $(OFILES) $(LIBS)

.c.o: 
	$(CC) $(CFLAGS) -c $<

depend:
	rm -f .depend
	mkdep $(CFLAGS) $(CFILES)
	@for i in $(SUBDIR); do \
		(echo "===> $${i}"; \
		cd ${.CURDIR}/$${i}; \
		${MAKE} ${.TARGET}); \
	done

clean:
	rm -f *.o *.core sog
	@for i in $(SUBDIR); do \
		(echo "===> $${i}"; \
		cd ${.CURDIR}/$${i}; \
		${MAKE} ${.TARGET}); \
	done

ctags:
	ctags -dtw $(CFILES) *.h comm/*.h compat/*.h db/*.h olc/*.h spellfun/*.c

gtags:
	gtags -co .

install: sog
	install -c -m 770 sog $(INSTALL_PATH)/bin
	@for i in $(SUBDIR); do \
		(echo "===> $${i}"; \
		cd ${.CURDIR}/$${i}; \
		${MAKE} ${.TARGET}); \
	done
