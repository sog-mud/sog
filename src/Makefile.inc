#
# $Id: Makefile.inc,v 1.28 2004-06-28 19:20:57 tatyana Exp $
#

#
# core version
#
ABI_VERSION=	10

#
# common defines
#
CC=		gcc
CXX=		g++
INCLUDES=	-I../include -I..

COPTS=		${INCLUDES}
CFLAGS=		-O -pipe -Wall $(COMPAT_CFLAGS)
LDFLAGS=	-export-dynamic

#
# additional flags for gcc 3.0.x
#CFLAGS+=	-std=c99

#
# BDECFLAGS minus -Wconversion, -ansi and -pedantic
CFLAGS+=	-W -Wbad-function-cast -Wcast-align\
		-Wcast-qual -Wchar-subscripts\
		-Wmissing-prototypes -Wnested-externs -Wpointer-arith\
		-Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings
CFLAGS+=	-Werror

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
# Doug Lea's malloc
DLMALLOC_SRCS=	malloc-2.6.5.c

.if !defined(MODULE)

BINDIR=		${INSTDIR}/bin

.else

PROG=		${MODULE}.so.${ABI_VERSION}
MODULE_ID!=	echo MOD_${MODULE} | tr '[a-z]' '[A-Z]'
CFLAGS+=	${MOD_CFLAGS} -DMODULE=${MODULE_ID}
LDFLAGS+=	${MOD_LDFLAGS}
CLEANFILES+=	${MODULE}.so.*

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
	     ${.CURDIR}/../mkdep.pl ${.CURDIR}/../mkutil.pl
	@${.CURDIR}/../mkdep.pl ${MODULE} ${DEPENDFILE} ${CDEPFILE} ${MODULES}

.include "${.CURDIR}/../Makefile.modules"

.endif

#
# some BSD make-specific stuff
NOOBJ=		yes
NOMAN=		yes

.if exists(${.CURDIR}/../Makefile.osdep)
.include "${.CURDIR}/../Makefile.osdep"
.endif

.if defined(WITH_LIBODBCXX)
CFLAGS+=	$(PTHREAD_CFLAGS)
.endif
