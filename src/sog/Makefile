#
# $Id: Makefile,v 1.11.2.8 2004-02-22 21:55:26 fjoe Exp $
#

PROG=		sog
SRCS=		auction.c buffer.c chquest.c clan.c class.c cmd.c\
		effects.c fight.c fishing.c flag.c gsn.c handler.c help.c\
		hometown.c imc.c interp.c lang.c log.c lookup.c magic.c\
		memalloc.c mlstring.c mob_cmds.c mob_prog.c module.c msg.c\
		msgq.c namedp.c note.c obj_prog.c quest.c race.c raffect.c\
		rating.c recycle.c religion.c save.c skills.c\
		socials.c special.c str.c string_edit.c tables.c update.c\
		util.c varr.c vo_iter.c\
		${COMM_SRCS} ${DB_SRCS} ${COMPAT_SRCS}
COMM_SRCS=	ban.c charset.c comm.c comm_act.c\
		comm_colors.c comm_info.c resolver.c mccp.c
DB_SRCS=	db.c db_area.c db_clan.c db_class.c db_cmd.c\
		db_hometown.c db_item.c db_lang.c db_msg.c db_race.c\
		db_skills.c db_socials.c db_system.c db_rspells.c
LDADD=		-lz

.PATH:		${.CURDIR}/comm ${.CURDIR}/db ${.CURDIR}/../compat

.include <bsd.prog.mk>
