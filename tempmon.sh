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
d=/home/connor/workspace/tempmon
c=$d/client/
p=$d/UDPrecv.py
f=$d/tempmonlogs.txt

now=$(date)
echo "!!!PROGRAM LAUNCHED: $now!!!" >> $f

echo "---Discovering server IP" >> $f
python $p
cd $c
make

while [ true ];
do
    echo "---Reading local device" >> $f
    ./tempmon 
    a=$?

    echo "---Program exited with status code $a" >> $f
    
    # if the program executed successfully, wait 30 seconds
    if [ "$a" -eq 0 ]; then
	echo "---Next read in 30 seconds---" >> $f
	sleep 30

    # Unfortunately if this error is encountered only human intervention can
    # save it. To save the computer from beating itself over the head for days
    # on end, the process will terminate. Hopefully the server will report that
    # it hasn't updated
    elif [ "$a" -eq 1 ]; then
	echo "---FATAL: corrupt global file, terminating---" >> $f
	break

    # if the program terminated with a USB opening-related error, wait 60 
    # seconds
    elif [ "$a" -eq 2 ]; then
	echo "---Waiting for USB device---" >> $f
	sleep 60
    # if the program terminated with a USB reading-related error, wait a
    # minute before attempting to re-read; if this continues to happen, restart
    # the computer (restart disabled for now)
    elif [ "$a" -eq 3 ]; then
	echo "---USB Read failed---" >> $f
	    sleep 60

    # if the program terminated with a server-related error, re-run the server
    # discovery service to re-discover the server
    elif [ "$a" -eq 4 ]; then
	echo "---Attempting to locate server---" >> $f

	python $p 

	sleep 10
    fi
done

