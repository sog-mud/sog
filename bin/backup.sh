#!/bin/sh
# $Id: backup.sh,v 1.7 2003-05-20 19:07:49 fjoe Exp $

#
# determine our home
# all other paths are relative to $SOG_HOME
if ! SOG_HOME=$(expr $0 : "\(/.*\)/bin/backup\.sh\$"); then
	echo "$0: Cannot determine the SOG_HOME" >&2
	exit 1
fi
export SOG_HOME

#
# determine time stamp
stamp=`date +%F`

cd $SOG_HOME/player || exit 1
tar cvfj ../backup/backup-$stamp.tar.bz2 *
echo "$0: Backup complete ($SOG_HOME/backup/backup-$stamp.tar.bz2)" >&2
