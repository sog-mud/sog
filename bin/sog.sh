#!/bin/sh
uid=`id -P | awk -F: '{ print $1 }'`
if [ "$uid" != "sog" ]; then
	echo "SoG must be run under user \`sog'"
	exit 1
fi
nohup ./autorun >/dev/null 2>&1 &
