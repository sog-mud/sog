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

#
# corefile count limit
SOG_CORELIM=10

#
# here we go
#
umask 002
index=0
cd $SOG_HOME

if [ -r $SOG_SHUTDOWN ]; then
	rm $SOG_SHUTDOWN
fi

while [ 1 ]
do
	if [ -r $SOG_LASTLOG ]; then
		index=`cat $SOG_LASTLOG`
	fi

	while [ 1 ]
	do
		logfile=`printf "log/%05d.log" $index`
		if [ -r $logfile ]
		then
			index=$(($index + 1))
		else
			break
		fi
	done

	echo $(($index+1)) > $SOG_LASTLOG
	bin/$SOG_BIN >$logfile 2>&1
	exitcode=$?

	avail=`df -k $SOG_HOME | tail -1 | awk '{ print $4 }'`
	corecount=`ls corefiles/core.* | wc -l`
	if [ -r $SOG_CORE -a $corecount -le $SOG_CORELIM ]; then
		chmod g+rw $SOG_CORE
		if [ $avail -gt 65535 ]; then
			mv $SOG_CORE corefiles/core.$index
			ln bin/$SOG_BIN corefiles/$SOG_BIN.$index
		else
			echo `date` "Low space (${avail}k) on disk: corefile not renamed" >> $logfile
		fi
		sleep 5
		continue
	fi

	if [ -r $SOG_SHUTDOWN ]; then
		break
	fi

	if [ $exitcode -ne 0 ]; then
		break
	fi
	sleep 5
done
