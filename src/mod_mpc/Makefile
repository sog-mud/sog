#
# $Id: Makefile,v 1.14 2003-04-25 12:49:31 fjoe Exp $
#

SRCS=		mpc.y mpc_accessor.c mpc_dynafun.c mpc_iter.c mpc_lex.c\
		mpc_c.c mpc_const.c
CLEANFILES+=	mpc_const.c

mpc_const.c:	../include/merc.h ../include/sog.h genconst.sh genconst.sed
	sh genconst.sh > ${.TARGET}

.if defined(MPC)
PROG=		mpc
SRCS+=		hash.c varr.c str.c strkey_hash.c memalloc.c log.c util.c\
		dynafun.c buffer.c db.c

NOMAN=		yes
NOOBJ=		yes

CFLAGS+=	-DMPC
.PATH:		${.CURDIR}/../sog ${.CURDIR}/../sog/db

.else
MODULE=		mpc
SRCS+=		init_mpc.c
.endif

afterdepend:
	@(grep -q 'int yygrowstack()' mpc.c && \
	sed 's/int yygrowstack()/int yygrowstack(void)/' mpc.c > .mpc.c && \
	mv -f .mpc.c mpc.c) || true

.include <bsd.prog.mk>

YFLAGS+=	-p mpc_
#CFLAGS+=	-E
