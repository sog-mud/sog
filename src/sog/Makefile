#
# $Id: Makefile,v 1.62 2004-06-28 19:32:16 tatyana Exp $
#

PROG=		sog
SRCS=		affect.c avltree.c buffer.c class.c cmd.c colors.c container.c\
		db.c dynafun.c flag.c help.c hometown.c lang.c log.c \
		material.c memalloc.c mlstring.c module.c module_decl.c msgq.c \
		note.c objval.c race.c recycle.c resolver.c rwfile.c sog.c \
		str.c tables.c trig.c util.c varr.c vo_iter.c ${COMPAT_SRCS}
LDADD=		$(COMPAT_LIBS) -lz

.PATH:		${.CURDIR}/.. ${.CURDIR}/../compat

.include <bsd.prog.mk>

.if defined(WITH_LIBODBCXX)
LDADD+=		-L/usr/local/lib -lodbc++ -lstdc++ $(PTHREAD_LIBS)
.endif
