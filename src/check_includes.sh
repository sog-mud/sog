#!/bin/sh
#
# $Id: check_includes.sh,v 1.4 2001-08-05 16:36:14 fjoe Exp $
#
# show files in core (sog/ and mod_core/) that #include module headers
#
check_includes()
{
	skip=$1
	shift
	egrep -l "(`grep -l "dynafun_decl.h" include/*.h | egrep -v "$skip" | sed -e 's^include/^^' | sed -e 's/ /\|/g' -e 's/\./\\\./g'`)" $* | sort | uniq
}

echo "===> #include \"xxx.h\" instead of #include <xxx.h>"
egrep -Ir `ls include/*.h | sed -e 's/include\//"/' -e 's/$/"/' | tr '\n' ' ' | sed -e 's/ $//' -e 's/ /|/g'` . | grep -v 'core_includes.sh'

echo
echo "===> core files that #include module headers"
check_includes "dynafun_decl.h|core.h" mod_core/*.c
check_includes "dynafun_decl.h" sog/*.c sog/**/*.c
check_includes "dynafun_decl.h" mod_bootdb/*.c

