#
# $Id: Makefile,v 1.87 2003-09-08 16:27:07 fjoe Exp $
#

.include "${.CURDIR}/Makefile.modules"

SUBDIR=		sog ${MODULES}

HFILES=		abi_version.h module_decl.h

depend: ${HFILES} _SUBDIR

cleandepend: _SUBDIR
	rm -f ${HFILES} module_decl.c

abi_version.h: Makefile.inc
	@echo Creating abi_version.h
	@echo "#define" `grep ^ABI_VERSION Makefile.inc | sed s/=//` > abi_version.h

module_decl.h: Makefile.modules mkdecl.pl mkutil.pl
	@${.CURDIR}/mkdecl.pl ${MODULES}

ctags:
	ctags -dtw include/*.h sog/*.[ch] mod_*/*.[ch]

gtags:
	gtags -co .

CHECKTRANS_FILES!=	ls mod_*/*.c sog/*.c ../mpc/*.mpc |\
	egrep -v '^sog/tables.c$$|^mod_sog/info.c$$|/_depend.c$$' |\
	egrep -v '^mod_olc/|^mod_mpc/|^sog/objval.c$$' |\
	egrep -v '^sog/special.c$$'

checktrans: .PHONY
	./checktrans.pl -u ${CHECKTRANS_FILES}

.include "Makefile.osdep"
.include <bsd.subdir.mk>
