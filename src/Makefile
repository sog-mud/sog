#
# $Id: Makefile,v 1.8 1998-07-03 15:18:38 fjoe Exp $
#
CC		= gcc
LD		= gcc
CFLAGS		=  -g -Wall -O
LFLAGS		=  -g
INSTALL_PATH	= /usr/local/muddy

OFILES = act_comm.o act_hera.o act_info.o act_move.o act_obj.o act_wiz.o \
         ban.o bit.o buffer.o charset.o comm.o const.o db.o db2.o effects.o \
	 fight.o flags.o handler.o healer.o hometown.o interp.o log.o lookup.o \
	 magic.o magic2.o martial_art.o mem.o mob_cmds.o mob_prog.o note.o \
	 obj_prog.o olc.o olc_act.o olc_mpcode.o olc_save.o quest.o raffects.o \
	 rating.o recycle.o resource.o save.o skills.o special.o string_edit.o \
	 tables.o update.o util.o

#
# for SUNOS
#CFLAGS += -DSUNOS
#LFLAGS += -lsocket -lnsl
#OFILES += strsep.o snprintf.o

#
# for FreeBSD
#CFLAGS += -DBSD44
#LFLAGS += -lcrypt

#
# for Unixware 2.x
#CFLAGS += -DSVR4 -DOLD_RAND
#LFLAGS += -lsocket -lnsl -lcrypt
#OFILES += strsep.o

muddy: $(OFILES)
	rm -f muddy
	$(LD) $(LFLAGS) -o muddy $(OFILES)

.c.o: 
	$(CC) -c $(CFLAGS) $<

depend:
	mkdep $(CFLAGS) `echo $(OFILES) | sed 's/\.o/.c/g'`

clean:
	rm -f *.o *.core muddy

areainstall:
	install -c -m 664 resource.h $(INSTALL_PATH)/area

bininstall:
	install -c -m 554 muddy $(INSTALL_PATH)/area

install: areainstall bininstall

