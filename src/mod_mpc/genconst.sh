#!/bin/sh
#
# $Id: genconst.sh,v 1.2 2001-08-14 16:06:57 fjoe Exp $
#

echo "#include <stdlib.h>"
echo "#include <typedef.h>"
echo "#include \"mpc_const.h\""
echo "int_const_t ic_tab[] = {"
grep 'ACT_' ../include/merc.h | sed -f genconst.sed
echo "	{ NULL, 0 }"
echo "};"
