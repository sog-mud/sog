#
# $Id: Makefile,v 1.22 1998-09-19 10:17:24 fjoe Exp $
#
CC		= gcc
LD		= gcc
INCLUDES	= -I . -I olc -I compat -I db -I ../msgdb
CFLAGS		=  -g -Wall -O -pipe $(INCLUDES)
LFLAGS		=  -g
INSTALL_PATH	= /usr/local/muddy

#
# for SUNOS
#CFLAGS += -DSUNOS
#LFLAGS += -lsocket -lnsl
#CFILES += compat/strsep.c compat/snprintf.c
#OFILES += strsep.o snprintf.o

#
# for FreeBSD
CFLAGS += -DBSD44
LFLAGS += -lcrypt

#
# for Unixware 2.x
#CFLAGS += -DSVR4 -DOLD_RAND
#LFLAGS += -lsocket -lnsl -lcrypt
#CFILES += compat/strsep.c
#OFILES += strsep.o

#
# Generally Makefile should not be edited below this line
#
DB_CFILES = db/db.c db/db_area.c db/db_clan.c db/db_class.c db/db_skills.c
DB_OFILES = db.o db_area.o db_clan.o db_class.o db_skills.o

OLC_CFILES = olc/olc.c olc/olc_area.c olc/olc_clan.c olc/olc_help.c\
	     olc/olc_mob.c olc/olc_mpcode.c olc/olc_room.c olc/olc_obj.c\
	     olc/olc_save.c
OLC_OFILES = olc.o olc_area.o olc_clan.o olc_help.o\
	     olc_mob.o olc_mpcode.o olc_room.o olc_obj.o\
	     olc_save.o

CFILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c auction.c\
         ban.c buffer.c charset.c clan.c class.c comm.c\
	 effects.c fight.c flag.c gsn.c handler.c healer.c help.c hometown.c\
	 hunt.c interp.c log.c lookup.c magic.c magic2.c martial_art.c mem.c\
	 mlstring.c mob_cmds.c mob_prog.c namedp.c note.c obj_prog.c quest.c\
	 raffects.c rating.c recycle.c religion.c repair.c resolver.c\
	 resource.c save.c skills.c special.c spellfn.c string_edit.c\
	 tables.c update.c util.c varr.c\
	 $(DB_CFILES) $(OLC_CFILES)

OFILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o auction.o\
         ban.o buffer.o charset.o clan.o class.o comm.o\
	 effects.o fight.o flag.o gsn.o handler.o healer.o help.o hometown.o\
	 hunt.o interp.o log.o lookup.o magic.o magic2.o martial_art.o mem.o\
	 mlstring.o mob_cmds.o mob_prog.o namedp.o note.o obj_prog.o quest.o\
	 raffects.o rating.o recycle.o religion.o repair.o resolver.o\
	 resource.o save.o skills.o special.o spellfn.o string_edit.o\
	 tables.o update.o util.o varr.o\
	 $(DB_OFILES) $(OLC_OFILES)

muddy: $(OFILES)
	rm -f muddy
	$(LD) $(LFLAGS) -o muddy $(OFILES)

.c.o: 
	$(CC) $(CFLAGS) -c $<

depend:
	mkdep $(CFLAGS) $(CFILES)

clean:
	rm -f *.o *.core muddy

ctags:
	ctags -dtw *.c *.h

resinstall:
	install -c -m 664 ../msgdb/resource.h ../msgdb/lang.lst ../msgdb/*.lang $(INSTALL_PATH)/msgdb

bininstall: muddy 
	install -c -m 554 muddy $(INSTALL_PATH)/bin

install: bininstall resinstall

