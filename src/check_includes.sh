#!/bin/sh
#
# $Id: check_includes.sh,v 1.3 2001-08-02 18:19:51 fjoe Exp $
#
# show files in core (sog/ and mod_core/) that #include module headers
#
check_includes()
{
	skip=$1
	shift
	egrep -l "(`grep -l "dynafun_decl.h" include/*.h | egrep -v "$skip" | sed -e 's^include/^^' | sed -e 's/ /\|/g' -e 's/\./\\\./g'`)" $* | sort | uniq
}

check_includes "dynafun_decl.h|core.h" mod_core/*.c
check_includes "dynafun_decl.h" sog/*.c sog/**/*.c
check_includes "dynafun_decl.h" mod_bootdb/*.c
