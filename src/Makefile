#
# $Id: Makefile,v 1.53 1999-06-24 21:07:42 fjoe Exp $
#

.include "Makefile.inc"

SUBDIR = core dofun sog spellfun olc

ctags:
	ctags -dtw *.h comm/*.h compat/*.h db/*.h olc/*.h spellfun/*.c

gtags:
	gtags -co .

.include <bsd.subdir.mk>
