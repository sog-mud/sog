#
# $Id: Makefile,v 1.4 1999-06-24 08:05:03 fjoe Exp $
#

.include "../Makefile.inc"

CFLAGS += $(SO_CFLAGS)
LFLAGS += $(SO_LFLAGS)

CFILES = spellfun.c magic.c magic2.c
OFILES = spellfun.o magic.o magic2.o

all: spellfun.so

spellfun.so: $(OFILES)
	$(LD) -o spellfun.so $(LFLAGS) $(SOLFLAGS) $(OFILES)

depend:
	rm -f .depend
	mkdep $(CFLAGS) $(CFILES)

clean:
	rm -f spellfun.so $(OFILES)

install: spellfun.so
	install -c -m 770 spellfun.so $(INSTALL_PATH)/../modules
