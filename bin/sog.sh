#!/bin/sh

#
# check uid
uid=`id -P | awk -F: '{ print $1 }'`
if [ "$uid" != "sog" ]; then
	echo "SoG must be run under user \`sog'"
	exit 1
fi

#
# locale settings for FreeBSD
LANG=ru_RU.KOI8-R; export LANG
# locale settings for Linux
#LANG=koi8-r; export LANG
LC_TIME=en_US.ISO_8859-1; export LC_TIME

#
# mud home -- all other paths and filenames are relative from $SOG_HOME
SOG_HOME=/usr/local/sog; export SOG_HOME
#SOG_HOME=/home/mud

#
# logging control
SOG_LASTLOG=tmp/lastlog; export SOG_LASTLOG

#
# mud binary executable
SOG_BIN=sog; export SOG_BIN

#
# shutdown flag file
SOG_SHUTDOWN=tmp/shutdown; export SOG_SHUTDOWN

#
# corefile name
SOG_CORE=sog.core; export SOG_CORE

nohup $SOG_BIN/bin/autorun >/dev/null 2>&1 &
