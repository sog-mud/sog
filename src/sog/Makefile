#
# $Id: Makefile,v 1.45 2001-08-05 16:36:55 fjoe Exp $
#

PROG=		sog
SRCS=		affect.c buffer.c class.c cmd.c colors.c\
		damtype.c db.c dynafun.c flag.c hash.c help.c hometown.c\
		interp.c lang.c log.c material.c memalloc.c mlstring.c module.c\
		module_decl.c note.c objval.c race.c recycle.c rwfile.c\
		special.c str.c strkey_hash.c tables.c util.c varr.c vo_iter.c\
		$(COMM_SRCS)

.if defined(COMPAT_SRCS)
SRCS+=		${COMPAT_SRCS}
.endif

COMM_SRCS=	ban.c charset.c comm.c comm_info.c\
		resolver.c

.PATH:		${.CURDIR}/.. ${.CURDIR}/../compat ${.CURDIR}/comm

.include <bsd.prog.mk>
