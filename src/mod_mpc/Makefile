#
# $Id: Makefile,v 1.1 2001-06-16 18:20:37 fjoe Exp $
#

.undef MODULE

.if !defined(MODULE)
PROG=		mpc
SRCS=		mpc.y mpc_c.c\
		hash.c varr.c str.c strkey_hash.c memalloc.c log.c util.c\
		dynafun.c buffer.c

NOMAN=		yes
NOOBJ=		yes

.include <bsd.prog.mk>

CFLAGS+=	-DMPTEST
YFLAGS+=	-p mp_
.PATH:	${.CURDIR}/../sog

.endif
