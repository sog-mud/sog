#
# $Id: Makefile,v 1.42 2001-08-02 18:34:49 fjoe Exp $
#

PROG=		sog
SRCS=		affect.c bootdb.c buffer.c clan.c class.c\
		cmd.c damtype.c dynafun.c flag.c hash.c\
		help.c hometown.c interp.c lang.c log.c material.c\
		memalloc.c mlstring.c module.c\
		module_decl.c namelist.c note.c objval.c race.c\
		rating.c recycle.c rwfile.c\
		special.c str.c string_edit.c strkey_hash.c\
		tables.c util.c varr.c vo_iter.c\
		$(COMM_SRCS) $(DB_SRCS)

.if defined(COMPAT_SRCS)
SRCS+=		${COMPAT_SRCS}
.endif

COMM_SRCS=	ban.c charset.c comm.c comm_act.c comm_colors.c comm_info.c\
		resolver.c

.PATH:		${.CURDIR}/.. ${.CURDIR}/../compat ${.CURDIR}/comm

.include <bsd.prog.mk>
