#!/bin/sh
# c program return values:
#
# 0 - the program executed successfully
# 1 - file read-related error
# 2 - USB opening-related error
# 3 - USB reading-related error
# 4 - server-related error

# To make this a daemon process, move tempmonDaemon.sh to /etc/init.d, make sure
# to chmod 755 both tempmonDaemon.sh and this file, change the directory in
# tempmonDaemon.sh to match this file's directory, and run the command:
#
# sudo update-rc.d tempmonDaemon.sh defaults
#
# To install:
# : change 'd' below to match the directory containing this file
# : sudo mv tempmonDaemon.sh /etc/init.d/
# : sudo chmod 755 /etc/init.d/tempmonDaemon.sh
# : sudo chmod 755 tempmon.sh
# : 
d=${PWD}
c=$d/client/tempmon
p=$d/UDPrecv.py
f=$d/tempmonlogs.txt

now=$(date)
#python $p
cd $d/client
make

echo "Waiting for network to become available"
while [ true ];
do
    ping -c 1 www.google.ca
    a=$?
    if [ "$a" -eq 0 ]; then
	echo "Network avalable"
	break;
    else
	sleep 5
    fi
done

while [ true ];
do
    echo "---Reading local device"
    $c
    a=$?

    # if the program executed successfully, wait 30 seconds
    if [ "$a" -eq 0 ]; then
	sleep 30

    # Unfortunately if this error is encountered only human intervention can
    # save it. To save the computer from beating itself over the head for days
    # on end, the process will terminate. Hopefully the server will report that
    # it hasn't updated
    elif [ "$a" -eq 1 ]; then
	break

    # if the program terminated with a USB opening-related error, wait 60 
    # seconds
    elif [ "$a" -eq 2 ]; then
	sleep 60
    # if the program terminated with a USB reading-related error, wait a
    # minute before attempting to re-read; if this continues to happen, restart
    # the computer (restart disabled for now)
    elif [ "$a" -eq 3 ]; then
	    sleep 60

    # if the program terminated with a server-related error, re-run the server
    # discovery service to re-discover the server
    elif [ "$a" -eq 4 ]; then
	python $p 

	sleep 10
    fi
done

