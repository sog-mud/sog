#
# $Id: Makefile,v 1.57 2002-11-20 20:15:26 fjoe Exp $
#

PROG=		sog
SRCS=		affect.c avltree.c buffer.c class.c cmd.c colors.c container.c\
		db.c dynafun.c flag.c help.c hometown.c lang.c log.c \
		material.c memalloc.c mlstring.c module.c module_decl.c msgq.c \
		note.c objval.c race.c recycle.c resolver.c rwfile.c sog.c \
		str.c tables.c trig.c util.c varr.c vo_iter.c

.if defined(COMPAT_SRCS)
SRCS+=		${COMPAT_SRCS}
.endif

.PATH:		${.CURDIR}/.. ${.CURDIR}/../compat

.include <bsd.prog.mk>
