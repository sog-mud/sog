#
# $Id: Makefile,v 1.22 2004-06-28 19:32:16 tatyana Exp $
#

MODULE=		olc
SRCS=		olc.c olc_area.c olc_clan.c olc_class.c olc_cmd.c olc_damtype.c\
		olc_help.c olc_hint.c olc_lang.c olc_liquid.c olc_material.c\
		olc_mob.c olc_mprog.c olc_msg.c olc_obj.c olc_race.c\
		olc_room.c olc_rule.c olc_skill.c olc_social.c olc_spec.c\
		adjust.c

.include "../Makefile.osdep"

.if defined(WITH_LIBODBCXX)
SRCS+=		dump.cpp
.endif

.include <bsd.prog.mk>

.if defined(WITH_LIBODBCXX)
CFLAGS+=	-I/usr/local/include -DWITH_LIBODBCXX
.endif
