#!/bin/sh
#
# $Id: check_includes.sh,v 1.1 2001-06-26 17:32:46 fjoe Exp $
#
# show files in core (sog/) that #include module headers
#
egrep -l "(`grep -l "dynafun_decl.h" include/*.h | sed 's^include/^^' | xargs echo $* | sed 's/ /\|/g' | sed 's/\./\\\./g'`)" sog/*.c sog/*/*.c | sort | uniq
