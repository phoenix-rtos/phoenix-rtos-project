#!/bin/sh

. /etc/rc.subr

TELNETD=/usr/sbin/telnetd
PIDFILE=/var/run/telnetd.pid


start() {
	if [ -f $PIDFILE ]; then
		echo "Telnet server is already running"
		exit 1
	fi

	echo "Starting Telnet server"

	$TELNETD -l /bin/login & echo $! > $PIDFILE
}


stop() {
	if [ -f "$PIDFILE" ]; then
		echo "Stopping Telnet server"
		PID=$(cat $PIDFILE)
		kill $PID > /dev/null

		#hack alert - we need to poke telnet to exit
		nc localhost 23 > /dev/null

		rm $PIDFILE
	else
		rc_msg "Telnet server is not running" " " err
	fi;
}


case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	restart)
		stop
		sleep 1
		start
		;;
	*)
		echo "Unknown command, only start/stop are supported"
		;;
esac

exit $?
