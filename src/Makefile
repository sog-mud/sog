#
# $Id: Makefile,v 1.81 2001-07-08 16:36:12 fjoe Exp $
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

CHECKTRANS_FILES!=	ls mod_*/*.c sog/*.c sog/**/*.c |\
	egrep -v '^sog/tables.c$$|^sog/comm/comm_info.c$$|/_depend.c$$' |\
	egrep -v '^sog/cc_parse.c$$|^sog/cc_lex.c$$' |\
	egrep -v '^mod_olc/|^mod_mpc/|^sog/objval.c$$' |\
	egrep -v '^sog/obj_prog.c$$|^sog/mob_prog.c$$|^sog/mob_cmds.c$$|^sog/special.c$$'

checktrans: .PHONY
	./checktrans.pl -u ${CHECKTRANS_FILES}

.include "Makefile.osdep"
.include <bsd.subdir.mk>
