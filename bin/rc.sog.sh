#!/bin/sh
sog_home=/home/sog2
sog_sh=$sog_home/bin/sog.sh
[ -x $sog_sh ] || exit 1

case "$1" in
start)
	su - sog -c "nohup $sog_sh >/dev/null 2>&1 &"
	echo -n " sog2"
	;;
stop)
	;;
*)
	echo "Usage: `basename $0` {start|stop}" >&2
	;;
esac

exit 0
