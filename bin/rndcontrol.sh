#!/bin/sh
# $Id: rndcontrol.sh,v 1.2.2.1 2001-09-23 18:18:57 fjoe Exp $

rndcontrol=/usr/sbin/rndcontrol
[ -x $rndcontrol ] && $rndcontrol -s 10 -s 14 > /dev/null && echo -n ' rnd'
