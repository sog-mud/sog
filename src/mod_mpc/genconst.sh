#!/bin/sh
#
# $Id: genconst.sh,v 1.10 2001-09-15 17:28:53 fjoe Exp $
#

echo "#include <stdio.h>"
echo
echo "#include <merc.h>"
echo
echo "#include \"mpc_const.h\""
echo
echo "int_const_t ic_tab[] = {"
egrep '#define (ACT_|AN_|SEX_|WEAR_|APPLY_|AFF_|TO_|SECT_|MAX_LEVEL|LEVEL_|WEAPON_|SIZE_|ITEM_)' ../include/merc.h | egrep -v 'AFF_X_|WEAPON_IS' | sed -f genconst.sed
egrep '#define (TO_|DAMF_|XO_F_)' ../include/sog.h | sed -f genconst.sed
egrep '#define (DAM_)' ../include/damtype.h | sed -f genconst.sed
echo
echo "	{ \"NULL\", 0, MT_PVOID },"
echo "	{ NULL, 0, MT_NONE }"
echo "};"
