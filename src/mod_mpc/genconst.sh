#!/bin/sh
#
# $Id: genconst.sh,v 1.8 2001-09-12 08:11:41 fjoe Exp $
#

echo "#include <stdio.h>"
echo
echo "#include <merc.h>"
echo
echo "#include \"mpc_const.h\""
echo
echo "int_const_t ic_tab[] = {"
egrep '#define (ACT_|AN_|SEX_|WEAR_|APPLY_|AFF_|TO_|SECT_|MAX_LEVEL|LEVEL_)' ../include/merc.h | grep -v 'AFF_X_' | sed -f genconst.sed
egrep '#define (TO_|DAMF_)' ../include/sog.h | sed -f genconst.sed
egrep '#define (DAM_)' ../include/damtype.h | sed -f genconst.sed
echo
echo "	{ \"NULL\", 0, MT_PVOID },"
echo "	{ NULL, 0, MT_NONE }"
echo "};"
