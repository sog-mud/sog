#
# $Id: Makefile,v 1.46 2001-08-13 18:23:57 fjoe Exp $
#

PROG=		sog
SRCS=		affect.c buffer.c class.c cmd.c colors.c\
		damtype.c db.c dynafun.c flag.c hash.c help.c hometown.c\
		lang.c log.c material.c memalloc.c mlstring.c module.c\
		module_decl.c note.c objval.c race.c recycle.c resolver.c\
		rwfile.c sog.c special.c str.c strkey_hash.c tables.c util.c\
		varr.c vo_iter.c

.if defined(COMPAT_SRCS)
SRCS+=		${COMPAT_SRCS}
.endif

.PATH:		${.CURDIR}/.. ${.CURDIR}/../compat

.include <bsd.prog.mk>
