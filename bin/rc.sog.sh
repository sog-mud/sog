#!/bin/sh
# $Id: rc.sog.sh,v 1.2 2001-01-18 12:15:07 avn Exp $

sog_home=/home/sog
sog_sh=$sog_home/bin/sog.sh
test -x $sog_sh || exit 1
su - sog -c "nohup $sog_sh >/dev/null 2>&1 &"
echo -n " sog"
