#
# $Id: Makefile,v 1.54 1999-06-28 10:13:02 fjoe Exp $
#

.include "Makefile.inc"

SUBDIR = core dofun sog spellfun olc

ctags:
	ctags -dtw include/*.h sog/comm/*.[ch] sog/db/*.[ch] sog/*.[ch]\
		core/*.[ch] dofun/*.[ch] spellfun/*.[ch] core/*.[ch]

gtags:
	gtags -co .

.include <bsd.subdir.mk>
