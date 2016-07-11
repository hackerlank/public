#!/bin/bash

 

pidfile=/var/lock/subsys/`basename $0`.pid

if [ -f $pidfile ] && [ -e /proc/`cat $pidfile` ] ; then

    exit 1

fi

 

trap "rm -fr $pidfile ; exit 0" 1 2 3 15

echo $$ > $pidfile

 

maxfails=3

fails=0

success=0

 

while [ 1 ]

do

    /usr/bin/wget --timeout=3 --tries=1 http://127.0.0.1/ -q -O /dev/null

    if [ $? -ne 0 ] ; then

        let fails=$[$fails+1]

        success=0

    else

        fails=0

        let success=$[$success+1]

    fi

 

    if [ $fails -ge $maxfails ] ; then

        fails=0

        success=0

 

        #check keepalived is running ? try to stop it

        service keepalived status | grep running

        if [ $? -eq 0 ] ; then

            logger -is "local service fails $maxfails times ... try to stop keepalived."

            service keepalived stop 2>&1 | logger

        fi

    fi

 

    if [ $success -gt $maxfails ] ; then

        #check keepalived is stopped ? try to start it

        service keepalived status | grep stopped

        if [ $? -eq 0 ] ; then

            logger -is "service changes normal, try to start keepalived ."

            service keepalived start

        fi

        success=0

    fi

    sleep 3

done
