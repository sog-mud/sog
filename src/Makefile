#
# $Id: Makefile,v 1.18 1998-08-17 18:47:01 fjoe Exp $
#
CC		= gcc
LD		= gcc
INCLUDES	= -I . -I olc -I compat
CFLAGS		=  -g -Wall -O -pipe $(INCLUDES)
LFLAGS		=  -g
INSTALL_PATH	= /usr/local/muddy

OLC_OFILES = olc/olc.o olc/olc_area.o olc/olc_help.o olc/olc_mob.o\
	     olc/olc_mpcode.o olc/olc_room.o olc/olc_obj.o olc/olc_save.o

OFILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o auction.o\
         ban.o bit.o buffer.o charset.o clan.o comm.o const.o db.o db2.o\
	 effects.o fight.o handler.o healer.o hometown.o hunt.o\
	 interp.o log.o lookup.o magic.o magic2.o martial_art.o mem.o\
	 mlstring.o mob_cmds.o mob_prog.o note.o obj_prog.o quest.o raffects.o\
	 rating.o recycle.o repair.o resource.o save.o skills.o special.o\
	 string_edit.o update.o util.o $(OLC_OFILES)

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
	install -c -m 664 resource.h lang.lst *.lang $(INSTALL_PATH)/area

bininstall:
	install -c -m 554 muddy $(INSTALL_PATH)/area

install: muddy resinstall bininstall

