#!/bin/sh
rndcontrol=/usr/sbin/rndcontrol

[ -x $rndcontrol ] || exit 1

case "$1" in
start)
	$rndcontrol -s 10 -s 14 > /dev/null && echo -n ' rnd'
	;;
stop)
	;;
*)
	echo "Usage: `basename $0` {start|stop}" >&2
	;;
esac

exit 0
