#
# $Id: Makefile,v 1.60.2.2 2000-06-01 10:10:12 fjoe Exp $
#

SUBDIR = sog mod_core mod_dofun mod_magic mod_olc mod_rspells

all: abi_version.h _SUBDIRUSE

install: abi_version.h _SUBDIRUSE

depend: abi_version.h _SUBDIRUSE

clean: _SUBDIRUSE
	rm -f abi_version.h

module_clean: _SUBDIRUSE

abi_version.h: Makefile.inc
	echo "#define" `grep ^ABI_VERSION Makefile.inc | sed s/=//` > abi_version.h

ctags:
	ctags -dtw include/*.h sog/*.[ch] sog/*/*.[ch] mod_*/*.[ch]

gtags:
	gtags -co .

.include "Makefile.osdep"
.include <bsd.subdir.mk>
