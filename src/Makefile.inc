#
# $Id: Makefile.inc,v 1.8 2000-06-05 12:06:14 fjoe Exp $
#

#
# core version
#
ABI_VERSION=	8

#
# common defines
#
CC=		gcc
INCLUDES=	-I../include -I..

COPTS=		${INCLUDES}
CFLAGS=		-Wall -O -pipe
LDFLAGS=	-export-dynamic

#
# debug flags
DEBUG_FLAGS=	-g
STRIP=		

MOD_CFLAGS=	-fPIC
MOD_LDFLAGS=	-shared

#
# SoG root
INSTDIR=	../..

.if defined(MODULE)
PROG=		${MODULE}.so.${ABI_VERSION}
MODULE_ID!=	echo MOD_${MODULE} | tr '[a-z]' '[A-Z]'
CFLAGS+=	${MOD_CFLAGS} -DMODULE=${MODULE_ID}
LDFLAGS+=	${MOD_LDFLAGS}
BINDIR=		${INSTDIR}/modules
.else
BINDIR=		${INSTDIR}/bin
.endif

#
# some BSD make-specific stuff
NOOBJ=		yes
NOMAN=		yes

.if exists(${.CURDIR}/../Makefile.osdep)
.include "${.CURDIR}/../Makefile.osdep"
.endif
