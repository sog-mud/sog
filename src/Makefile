#
# $Id: Makefile,v 1.62 1999-11-22 14:54:21 fjoe Exp $
#

SUBDIR = sog core dofun events mod_cc_rulefun olc spellfun

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
		events/*.[ch]

gtags:
	gtags -co .

.include "Makefile.osdep"
.include <bsd.subdir.mk>
