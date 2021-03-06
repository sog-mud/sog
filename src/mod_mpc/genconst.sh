#!/bin/sh
#
# $Id: genconst.sh,v 1.19 2003-04-17 17:17:08 fjoe Exp $
#

echo "#include <stdio.h>"
echo
echo "#include <merc.h>"
echo
echo "#include \"mpc_const.h\""
echo
echo "int_const_t ic_tab[] = {"
egrep '#define (ACT_|AN_|SEX_|WEAR_|APPLY_|AFF_|TO_|SECT_|MAX_LEVEL|LEVEL_|WEAPON_|SIZE_|ITEM_|POS_|ETHOS_|ID_|ROOM_|DAM_|FORM_)' ../include/merc.h | egrep -v 'AFF_X_|WEAPON_IS|ID_ALL_INVIS' | sed -f genconst.sed
egrep '#define (ACT_|TO_|DAM_F_|XO_F_)' ../include/sog.h | egrep -v 'ACT_SPEECH\(|ACT_TO\(' | sed -f genconst.sed
egrep '#define (TRUE|FALSE)' ../include/typedef.h | sed -f genconst.sed
echo
echo "	{ \"NULL\", 0, MT_PVOID },"
echo "	{ NULL, 0, MT_NONE }"
echo "};"
