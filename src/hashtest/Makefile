#
# $Id: Makefile,v 1.3 2003-04-24 13:36:53 fjoe Exp $
#

PROG=		hashtest
SRCS=		hashtest.c avltree.c container.c memalloc.c varr.c str.c

.include <bsd.prog.mk>

.PATH:		${.CURDIR}/../sog
CFLAGS+=	-DTEST

test::
	./hashtest foobar 100000 1000
