#
# $Id: Makefile,v 1.49 2001-08-28 16:37:41 avn Exp $
#

PROG=		sog
SRCS=		affect.c buffer.c class.c cmd.c colors.c\
		damtype.c db.c dynafun.c flag.c hash.c help.c hometown.c\
		lang.c log.c material.c memalloc.c mlstring.c module.c\
		module_decl.c mprog.c note.c objval.c race.c recycle.c\
		resolver.c rwfile.c sog.c special.c str.c strkey_hash.c\
		tables.c trig.c util.c varr.c vo_iter.c

.if defined(COMPAT_SRCS)
SRCS+=		${COMPAT_SRCS}
.endif

.PATH:		${.CURDIR}/.. ${.CURDIR}/../compat

afterdepend:
	touch -t 197001010000 sog

.include <bsd.prog.mk>
