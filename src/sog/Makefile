#
# $Id: Makefile,v 1.35 2000-06-05 12:06:26 fjoe Exp $
#

PROG=		sog
SRCS=		auction.c affect.c buffer.c cc_expr.c clan.c class.c\
		cmd.c damtype.c dynafun.c flag.c forms.c handler.c hash.c\
		help.c hometown.c interp.c lang.c liquid.c log.c material.c\
		memalloc.c mlstring.c mob_cmds.c mob_prog.c module.c\
		module_decl.c namelist.c note.c obj_prog.c objval.c race.c\
		rating.c recycle.c rfile.c save.c skills.c\
		socials.c spec.c special.c str.c string_edit.c strkey_hash.c\
		tables.c util.c varr.c vo_iter.c cc_parse.y cc_lex.l\
		$(COMM_SRCS) $(DB_SRCS)

.if defined(COMPAT_SRCS)
SRCS+=		${COMPAT_SRCS}
.endif

COMM_SRCS=	ban.c charset.c comm.c comm_act.c comm_colors.c comm_info.c\
		resolver.c

DB_SRCS=	db.c db_area.c db_cc_expr.c db_clan.c db_class.c db_cmd.c\
		db_damtype.c db_form.c db_hometown.c db_lang.c db_liquid.c\
		db_material.c db_race.c db_skills.c db_socials.c db_spec.c\
		db_system.c

.PATH:		.. ../compat comm db

.include <bsd.prog.mk>
