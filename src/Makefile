#
# $Id: Makefile,v 1.73 2000-06-06 08:39:53 fjoe Exp $
#

# sog should be the first!
SUBDIR=		sog\
		mod_core mod_cc_fun mod_dofun mod_events mod_effects\
		mod_fight mod_magic mod_olc mod_quest mod_update

HFILES=		abi_version.h module_decl.h

depend: ${HFILES} _SUBDIRUSE

cleandepend: _SUBDIRUSE
	rm -f ${HFILES} module_decl.c

abi_version.h: Makefile.inc
	@echo Creating abi_version.h
	@echo "#define" `grep ^ABI_VERSION Makefile.inc | sed s/=//` > abi_version.h

module_decl.h: Makefile makedecl.pl
	@${.CURDIR}/makedecl.pl ${SUBDIR}

.if make(depend) || make(module-depend)
SUBDIR+=	make-module-depend
.endif

DEPENDS!=	echo ${SUBDIR} | sed 's^ ^/.depend ^g'

module-depend: ${DEPENDS}
	@${.CURDIR}/makedep.pl ${SUBDIR}

ctags:
	ctags -dtw include/*.h sog/*.[ch] sog/*/*.[ch] mod_*/*.[ch]

gtags:
	gtags -co .

.include "Makefile.osdep"
.include <bsd.subdir.mk>
