#
# $Id: Makefile,v 1.56 1999-07-30 05:30:24 avn Exp $
#

.include "Makefile.inc"

SUBDIR = core dofun sog spellfun olc rspells

ctags:
	ctags -dtw include/*.h sog/comm/*.[ch] sog/db/*.[ch] sog/*.[ch]\
		core/*.[ch] dofun/*.[ch] spellfun/*.[ch] olc/*.[ch]\
		rspells/*.[ch]

gtags:
	gtags -co .

.include <bsd.subdir.mk>
