#
# $Id: Makefile,v 1.43 1999-02-12 18:14:32 fjoe Exp $
#
CC		= gcc
LD		= gcc
INCLUDES	= -I .

CFLAGS		= -pg -g -Wall -O -pipe $(INCLUDES)
LFLAGS		= -pg -g
LIBS		=
INSTALL_PATH	= ..

#
# for SUNOS
#CFLAGS	+= -DSUNOS
#LIBS	+= -lsocket -lnsl
#CFILES	+= compat/strsep.c compat/snprintf.c
#OFILES	+= strsep.o snprintf.o

#
# for FreeBSD
CFLAGS	+= -DBSD44 
LIBS	+= -lcrypt

#
# for Linux
#CFLAGS	+= -DLINUX -DNOCRYPT
#LIBS	+=

#
# for Unixware 2.x
#CFLAGS	+= -DSVR4 -DOLD_RAND
#LIBS	+= -lsocket -lnsl -lcrypt
#CFILES	+= compat/strsep.c
#OFILES	+= strsep.o

#
# Generally Makefile should not be edited below this line
#
COMM_CFILES = comm/ban.c comm/charset.c comm/comm.c comm/comm_act.c\
	      comm/comm_colors.c comm/comm_info.c comm/resolver.c
COMM_OFILES = ban.o charset.o comm.o comm_act.o\
	      comm_colors.o comm_info.o resolver.o

DB_CFILES = db/cmd.c db/db.c db/db_area.c db/db_clan.c db/db_class.c\
	    db/db_skills.c db/db_lang.c db/db_race.c\
	    db/gsn.c db/lang.c db/msg.c db/spellfn.c db/word.c
DB_OFILES = cmd.o db.o db_area.o db_clan.o db_class.o\
	    db_skills.o db_lang.o db_race.o\
	    gsn.o lang.o msg.o spellfn.o word.o

OLC_CFILES = olc/olc.c olc/olc_area.c olc/olc_clan.c olc/olc_help.c\
	     olc/olc_lang.c olc/olc_mob.c olc/olc_mpcode.c olc/olc_msg.c\
	     olc/olc_room.c olc/olc_obj.c olc/olc_save.c olc/olc_word.c
OLC_OFILES = olc.o olc_area.o olc_clan.o olc_help.o\
	     olc_lang.o olc_mob.o olc_mpcode.o olc_msg.o\
	     olc_room.o olc_obj.o olc_save.o olc_word.o

CFILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c auction.c\
         buffer.c clan.c class.c\
	 effects.c fight.c flag.c handler.c healer.c help.c hometown.c\
	 hunt.c interp.c log.c lookup.c magic.c magic2.c martial_art.c mem.c\
	 mlstring.c mob_cmds.c mob_prog.c namedp.c note.c obj_prog.c quest.c\
	 race.c raffects.c rating.c recycle.c religion.c repair.c\
	 save.c skills.c special.c str.c string_edit.c\
	 tables.c update.c util.c varr.c\
	 $(COMM_CFILES) $(DB_CFILES) $(OLC_CFILES)

OFILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o auction.o\
         buffer.o clan.o class.o\
	 effects.o fight.o flag.o handler.o healer.o help.o hometown.o\
	 hunt.o interp.o log.o lookup.o magic.o magic2.o martial_art.o mem.o\
	 mlstring.o mob_cmds.o mob_prog.o namedp.o note.o obj_prog.o quest.o\
	 race.o raffects.o rating.o recycle.o religion.o repair.o\
	 save.o skills.o special.o str.o string_edit.o\
	 tables.o update.o util.o varr.o\
	 $(COMM_OFILES) $(DB_OFILES) $(OLC_OFILES)

muddy: $(OFILES)
	rm -f muddy
	$(LD) $(LFLAGS) -o muddy $(OFILES) $(LIBS)

.c.o: 
	$(CC) $(CFLAGS) -c $<

depend:
	mkdep $(CFLAGS) $(CFILES)

clean:
	rm -f *.o *.core muddy

ctags:
	ctags -dtw $(CFILES) *.h

install: muddy
	install -Cp -m 770 muddy $(INSTALL_PATH)/bin

