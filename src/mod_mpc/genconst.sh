#!/bin/sh
#
# $Id: genconst.sh,v 1.1 2001-06-18 15:05:35 fjoe Exp $
#

echo "#include <stdlib.h>"
echo "#include <typedef.h>"
echo "#include \"mpc_const.h\""
echo "int_const_t ic_tab[] = {"
grep 'ACT_' ../include/merc.h | sed -f genconst.sed
echo "	{ NULL }"
echo "};"
