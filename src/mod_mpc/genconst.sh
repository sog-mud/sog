#!/bin/sh
#
# $Id: genconst.sh,v 1.3 2001-08-28 17:46:17 fjoe Exp $
#

echo "#include <stdlib.h>"
echo "#include <typedef.h>"
echo
echo "#include \"mpc_const.h\""
echo
echo "int_const_t ic_tab[] = {"
grep 'ACT_' ../include/merc.h | sed -f genconst.sed
grep 'TO_' ../include/sog.h | sed -f genconst.sed
echo
echo "	{ \"NULL\", 0 },"
echo "	{ NULL, 0 }"
echo "};"
