#
# $Id: Makefile,v 1.19 1998-09-01 18:29:14 fjoe Exp $
#
CC		= gcc
LD		= gcc
INCLUDES	= -I . -I olc -I compat -I ../msgdb
CFLAGS		=  -g -Wall -O -pipe $(INCLUDES)
LFLAGS		=  -g
INSTALL_PATH	= /usr/local/muddy

OLC_OFILES = olc/olc.o olc/olc_area.o olc/olc_clan.o olc/olc_help.o\
	     olc/olc_mob.o olc/olc_mpcode.o olc/olc_room.o olc/olc_obj.o\
	     olc/olc_save.o

OFILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o auction.o\
         ban.o buffer.o charset.o clan.o class.o comm.o db.o\
	 db_area.o db_clan.o db_class.o db_skills.o\
	 effects.o fight.o flag.o gsn.o handler.o healer.o help.o hometown.o\
	 hunt.o interp.o log.o lookup.o magic.o magic2.o martial_art.o mem.o\
	 mlstring.o mob_cmds.o mob_prog.o namedp.o note.o obj_prog.o quest.o\
	 raffects.o rating.o recycle.o repair.o resource.o save.o skills.o\
	 special.o spellfn.o string_edit.o tables.o update.o util.o varr.o\
	 $(OLC_OFILES)

#
# for SUNOS
#CFLAGS += -DSUNOS
#LFLAGS += -lsocket -lnsl
#OFILES += compat/strsep.o compat/snprintf.o

#
# for FreeBSD
#CFLAGS += -DBSD44
#LFLAGS += -lcrypt

#
# for Unixware 2.x
#CFLAGS += -DSVR4 -DOLD_RAND
#LFLAGS += -lsocket -lnsl -lcrypt
#OFILES += compat/strsep.o

muddy: $(OFILES)
	rm -f muddy
	$(LD) $(LFLAGS) -o muddy $(OFILES)

.c.o: 
	$(CC) $(CFLAGS) -c $*.c -o $*.o

depend:
	mkdep $(CFLAGS) `echo $(OFILES) | sed 's/\.o/.c/g'`

clean:
	rm -f *.o *.core muddy

resinstall:
	install -c -m 664 ../msgdb/resource.h ../msgdb/lang.lst ../msgdb/*.lang $(INSTALL_PATH)/msgdb

ctags:
	ctags -dtw *.c *.h

bininstall:
	install -c -m 554 muddy $(INSTALL_PATH)/bin

install: muddy resinstall bininstall

