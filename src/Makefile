#
# $Id: Makefile,v 1.60 1999-09-25 11:28:37 fjoe Exp $
#

SUBDIR = core dofun olc rspells sog spellfun

all: abi_version.h _SUBDIRUSE

install: abi_version.h _SUBDIRUSE

depend: abi_version.h _SUBDIRUSE

clean: _SUBDIRUSE
	rm -f abi_version.h

module_clean: _SUBDIRUSE

abi_version.h: Makefile.inc
	echo "#define" `grep ^ABI_VERSION Makefile.inc | sed s/=//` > abi_version.h

ctags:
	ctags -dtw include/*.h sog/comm/*.[ch] sog/db/*.[ch] sog/*.[ch]\
		core/*.[ch] dofun/*.[ch] spellfun/*.[ch] olc/*.[ch]\
		rspells/*.[ch]

gtags:
	gtags -co .

.include "Makefile.osdep"
.include <bsd.subdir.mk>
