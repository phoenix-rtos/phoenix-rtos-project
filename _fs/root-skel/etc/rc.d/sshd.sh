#!/bin/sh

. /etc/rc.subr

DROPBEAR=/sbin/dropbear
PIDFILE=/var/run/dropbear.pid


start() {
	rc_msg "Starting SSH server" -n
	if ! $DROPBEAR -R -B -P $PIDFILE; then
		rc_msg " [FAILED]" " " err
		exit 1
	fi
	echo ""
}


stop() {
	if [ -f "$PIDFILE" ]; then
		PID=$(cat $PIDFILE)

		echo "Stopping SSH server"
		#FIXME: the only way to kill dropbear for now until we fix read/write interruption
		kill $PID
		/bin/scp localhost:drop . 2&> /dev/null
	else
		rc_msg "SSH server is not running" " " err
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
		echo "Unknown command, only start/stop are supported" " " err
	;;
esac

exit $?
