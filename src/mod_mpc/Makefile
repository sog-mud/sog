#
# $Id: Makefile,v 1.11 2001-09-02 17:57:46 fjoe Exp $
#

SRCS=		mpc.y mpc_dynafun.c mpc_iter.c mpc_lex.c mpc_c.c mpc_const.c
CLEANFILES+=	mpc_const.c

mpc_const.c:	../include/merc.h genconst.sh genconst.sed
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
