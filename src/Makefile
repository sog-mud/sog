#
# $Id: Makefile,v 1.57 1999-09-11 13:09:12 fjoe Exp $
#

.include "Makefile.inc"

SUBDIR = core dofun sog spellfun olc rspells

all: abi_version.h _SUBDIRUSE

install: abi_version.h _SUBDIRUSE

depend: abi_version.h _SUBDIRUSE

clean: _SUBDIRUSE
	rm -f abi_version.h

module_clean: _SUBDIRUSE

abi_version.h: Makefile.rules
	echo "#define" `grep ^ABI_VERSION Makefile.rules | sed s/=//` > abi_version.h

ctags:
	ctags -dtw include/*.h sog/comm/*.[ch] sog/db/*.[ch] sog/*.[ch]\
		core/*.[ch] dofun/*.[ch] spellfun/*.[ch] olc/*.[ch]\
		rspells/*.[ch]

gtags:
	gtags -co .

.include <bsd.subdir.mk>
