#
# $Id: Makefile,v 1.70 2000-06-02 16:40:37 fjoe Exp $
#

SUBDIR = sog mod_core mod_cc_expr mod_dofun mod_events mod_effects mod_fight\
	 mod_magic mod_olc mod_quest mod_update

all: abi_version.h _SUBDIRUSE

install: abi_version.h _SUBDIRUSE

depend: abi_version.h _SUBDIRUSE

clean: _SUBDIRUSE
	rm -f abi_version.h

abi_version.h: Makefile.inc
	echo "#define" `grep ^ABI_VERSION Makefile.inc | sed s/=//` > abi_version.h

ctags:
	ctags -dtw include/*.h sog/*.[ch] sog/*/*.[ch] mod_*/*.[ch]

gtags:
	gtags -co .

.include "Makefile.osdep"
.include <bsd.subdir.mk>
