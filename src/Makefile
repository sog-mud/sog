#
# $Id: Makefile,v 1.79 2001-01-18 22:19:15 fjoe Exp $
#

.include "${.CURDIR}/Makefile.modules"

SUBDIR=		sog ${MODULES}

HFILES=		abi_version.h module_decl.h

depend: ${HFILES} _SUBDIRUSE

cleandepend: _SUBDIRUSE
	rm -f ${HFILES} module_decl.c module-depend

abi_version.h: Makefile.inc
	@echo Creating abi_version.h
	@echo "#define" `grep ^ABI_VERSION Makefile.inc | sed s/=//` > abi_version.h

module_decl.h: Makefile.modules mkdecl.pl mkutil.pl
	@${.CURDIR}/mkdecl.pl ${MODULES}

ctags:
	ctags -dtw include/*.h sog/*.[ch] sog/*/*.[ch] mod_*/*.[ch]

gtags:
	gtags -co .

checktrans:
	ls mod_*/*.c sog/*.c sog/**/*.c |\
		egrep -v '^sog/tables.c|^mod_olc/' |\
		xargs ./checktrans.pl -u > checktrans.out

.include "Makefile.osdep"
.include <bsd.subdir.mk>
