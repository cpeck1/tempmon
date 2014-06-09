#!/bin/bash
# /etc/init.d/tempmonDaemon

### BEGIN INIT INFO
# Provides:          tempmonClient
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: tempmon client service
# Description:       Monitors the temperature of the attached freezer monitor
### END INIT INFO

case "$1" in
    start)
	echo "Starting tempmon client"
	sh /home/pi/tempmon/tempmon.sh
	;;
    stop)
	echo "Stopping tempmon client"
	killall tempmon.sh
	;;
    *)
	echo "Usage: /etc/init.d/tempmonDaemon2 start|stop"
	exit 1
	;;
esac

exit 0
