#
# $Id: Makefile,v 1.55 1999-07-30 05:18:12 avn Exp $
#

.include "Makefile.inc"

SUBDIR = core dofun sog spellfun olc rspells

ctags:
	ctags -dtw include/*.h sog/comm/*.[ch] sog/db/*.[ch] sog/*.[ch]\
		core/*.[ch] dofun/*.[ch] spellfun/*.[ch] core/*.[ch]

gtags:
	gtags -co .

.include <bsd.subdir.mk>
