#
# $Id: Makefile.inc,v 1.11 2000-06-06 10:27:15 fjoe Exp $
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

#
# do not add deps on system includes
MKDEP_CPP_OPTS=	-MM
MKDEPCMD=	MKDEP_CPP_OPTS=${MKDEP_CPP_OPTS} mkdep

.if !defined(MODULE)

BINDIR=		${INSTDIR}/bin

.else

PROG=		${MODULE}.so.${ABI_VERSION}
MODULE_ID!=	echo MOD_${MODULE} | tr '[a-z]' '[A-Z]'
CFLAGS+=	${MOD_CFLAGS} -DMODULE=${MODULE_ID}
LDFLAGS+=	${MOD_LDFLAGS}

BINDIR=		${INSTDIR}/modules

CDEPFILE=	_depend.c
DEPCLEANFILES=	${DEPENDFILE}

cleandepend:	_SUBDIR
	rm -f ${DEPCLEANFILES}

.if !make(depend)
SRCS+=		${CDEPFILE}
.endif

.if make(cleandepend)
DEPCLEANFILES+=	${CDEPFILE}
.endif

${CDEPFILE}: ${DEPENDFILE} ${.CURDIR}/../Makefile.modules\
	     ${.CURDIR}/../makedep.pl ${.CURDIR}/../makeutil.pl
	${.CURDIR}/../makedep.pl ${MODULE} ${DEPENDFILE} ${CDEPFILE} ${MODULES}

.include "${.CURDIR}/../Makefile.modules"

.endif

#
# some BSD make-specific stuff
NOOBJ=		yes
NOMAN=		yes

.if exists(${.CURDIR}/../Makefile.osdep)
.include "${.CURDIR}/../Makefile.osdep"
.endif
