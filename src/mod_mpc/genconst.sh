#!/bin/sh
#
# $Id: genconst.sh,v 1.5 2001-09-02 16:21:58 fjoe Exp $
#

echo "#include <stdlib.h>"
echo
echo "#include <typedef.h>"
echo "#include <memalloc.h>"
echo
echo "#include \"mpc_const.h\""
echo
echo "int_const_t ic_tab[] = {"
egrep 'ACT_|SEX_' ../include/merc.h | sed -f genconst.sed
grep 'TO_' ../include/sog.h | sed -f genconst.sed
echo
echo "	{ \"NULL\", 0, MT_PVOID },"
echo "	{ NULL, 0, MT_NONE }"
echo "};"
