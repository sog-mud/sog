#
# $Id: Makefile,v 1.60.2.3 2002-08-02 07:23:42 fjoe Exp $
#

SUBDIR = sog mod_core mod_dofun mod_magic mod_olc mod_rspells

all: abi_version.h _SUBDIR

install: abi_version.h _SUBDIR

depend: abi_version.h _SUBDIR

clean: _SUBDIR
	rm -f abi_version.h

module_clean: _SUBDIR

abi_version.h: Makefile.inc
	echo "#define" `grep ^ABI_VERSION Makefile.inc | sed s/=//` > abi_version.h

ctags:
	ctags -dtw include/*.h sog/*.[ch] sog/*/*.[ch] mod_*/*.[ch]

gtags:
	gtags -co .

.include "Makefile.osdep"
.include <bsd.subdir.mk>
