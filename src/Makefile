#
# $Id: Makefile,v 1.71 2000-06-05 12:06:14 fjoe Exp $
#

# sog should be the first!
SUBDIR=		sog\
		mod_core mod_cc_expr mod_dofun mod_events mod_effects\
		mod_fight mod_magic mod_olc mod_quest mod_update

HFILES=		abi_version.h module_decl.h

depend: ${HFILES} _SUBDIRUSE

clean: _SUBDIRUSE
	rm -f ${HFILES} module_decl.c

abi_version.h: Makefile.inc
	echo "#define" `grep ^ABI_VERSION Makefile.inc | sed s/=//` > abi_version.h

module_decl.h: Makefile makedecl.pl
	@${.CURDIR}/makedecl.pl ${SUBDIR}

.if make(depend) || make(module-depend)
SUBDIR+=	make-module-depend
.endif

DEPENDS!=	echo ${SUBDIR} | sed 's^ ^/.depend ^g'

module-depend: ${DEPENDS}
	@echo ${DEPENDS}

ctags:
	ctags -dtw include/*.h sog/*.[ch] sog/*/*.[ch] mod_*/*.[ch]

gtags:
	gtags -co .

.include "Makefile.osdep"
.include <bsd.subdir.mk>
