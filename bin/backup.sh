#!/bin/sh

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
stamp=`LC_TIME=en_US.ISO_8859-1 date | awk '{ print $3"-"$2"-"$6 }'`

cd $SOG_HOME/player || exit 1
tar cvfz ../backup/backup-$stamp.tar.gz *
echo "$0: Backup complete ($SOG_HOME/backup/backup-$stamp.tar.gz)" >&2
