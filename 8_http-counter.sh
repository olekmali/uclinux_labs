#!/bin/sh

### BEGIN INIT INFO
# Provides:          http-counter server
# Required-Start:    $local_fs $remote_fs $network
# Required-Stop:     $local_fs $remote_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: http-counter server
# Description:       http-counter http-based server on TCP port 88
### END INIT INFO

PROCESS=http-counter
DAEMON=/usr/sbin/$PROCESS
LOGDIR=/var/log/http-counter
USER=counter

# Create the data folder if needed
test -f $DAEMON || exit 0
if [ ! -d $LOGDIR ]; then
	mkdir $LOGDIR
	chown -R $USER $LOGDIR || echo "Note: user $USER was not set up"
	chgrp -R $USER $LOGDIR || echo "Note: group $USER was not set up"
fi


start() {
	echo -n "Starting HTTP Counter: "
	$DAEMON
	if [ $? != 0 ]; then
		echo "FAILED"
		exit 1
	else
		echo "done"
	fi
}

stop() {
	echo -n "Stopping HTTP Counter: "
	killall $PROCESS
	echo "done"
}

case "$1" in
	start)
	start
	;;

	stop)
	stop
	;;

	restart|force-reload)
	stop
	sleep 1
	start
	;;

    *)
	echo "Usage: /etc/init.d/http-counter {start|stop|restart}"
	exit 1
	;;
esac

exit 0
