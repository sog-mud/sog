#!/bin/sh
# $Id: rndcontrol.sh,v 1.2 2001-01-18 12:15:07 avn Exp $

rndcontrol=/usr/sbin/rndcontrol
[ -x $rndcontrol ] && $rndcontrol -s 10 -s 14 > /dev/null && echo -n ' rnd'
