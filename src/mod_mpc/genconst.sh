#!/bin/sh
#
# $Id: genconst.sh,v 1.7 2001-09-07 15:40:17 fjoe Exp $
#

echo "#include <stdlib.h>"
echo
echo "#include <typedef.h>"
echo "#include <memalloc.h>"
echo
echo "#include \"mpc_const.h\""
echo
echo "int_const_t ic_tab[] = {"
egrep '#define (ACT_|AN_|SEX_|WEAR_|APPLY_|AFF_|TO_|SECT_)' ../include/merc.h | grep -v 'AFF_X_' | sed -f genconst.sed
egrep '#define (TO_|DAMF_)' ../include/sog.h | sed -f genconst.sed
egrep '#define (DAM_)' ../include/damtype.h | sed -f genconst.sed
echo
echo "	{ \"NULL\", 0, MT_PVOID },"
echo "	{ NULL, 0, MT_NONE }"
echo "};"
