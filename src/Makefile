#
# $Id: Makefile,v 1.1 1998-06-06 10:55:10 fjoe Exp $
#
CC		= gcc
LD		= gcc
CFLAGS		=  -g -Wall -O
LFLAGS		=  -g
INSTALL_PATH	= /usr/local/muddy

OFILES = act_comm.o act_hera.o act_info.o act_move.o act_obj.o act_wiz.o \
         ban.o charset.o comm.o const.o db.o db2.o effects.o fight.o flags.o \
	 handler.o healer.o hometown.o interp.o lookup.o magic.o magic2.o \
	 martial_art.o mob_prog.o note.o obj_prog.o quest.o rating.o \
	 recycle.o resource.o save.o skills.o special.o tables.o update.o

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
#CFLAGS += -DSVR4
#OFILES += -lsocket -lnsl
#OFILES += strsep.o

muddy: $(OFILES)
	rm -f muddy
	$(LD) $(LFLAGS) -o muddy $(OFILES)

.c.o: 
	$(CC) -c $(CFLAGS) $<

depend:
	makedepend -- $(CFLAGS) -- `echo $(OFILES) | sed 's/\.o/.c/g'`

clean:
	rm -f *.o *.core muddy

areainstall:
	install -c -m 664 resource.h $(INSTALL_PATH)/area

bininstall:
	install -c -m 554 muddy $(INSTALL_PATH)/area

install: areainstall bininstall

