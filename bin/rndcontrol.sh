#!/bin/sh
rndcontrol=/usr/sbin/rndcontrol
[ -x $rndcontrol ] && $rndcontrol -s 10 -s 14 > /dev/null && echo -n ' rnd'
