#
# $Id: Makefile,v 1.3 2001-06-18 15:05:34 fjoe Exp $
#

.undef MODULE

SRCS=		mpc.y mpc_c.c mpc_const.c
CLEANFILES+=	mpc_const.c

mpc_const.c:	../include/merc.h genconst.sh genconst.sed
	sh genconst.sh > ${.TARGET}

.if !defined(MODULE)
PROG=		mpc
SRCS+=		hash.c varr.c str.c strkey_hash.c memalloc.c log.c util.c\
		dynafun.c buffer.c db.c

NOMAN=		yes
NOOBJ=		yes

.include <bsd.prog.mk>

CFLAGS+=	-DMPC
.PATH:		${.CURDIR}/../sog ${.CURDIR}/../sog/db

.endif

YFLAGS+=	-p mp_
