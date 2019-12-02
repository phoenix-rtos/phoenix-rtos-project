#!/bin/sh

. /etc/rc.subr

NTPD_DEF_CONF="/etc/rc.conf.d/ntpd"
NTPD_CONF="/local/etc/rc.conf.d/ntpd"

NTP_DEF_CONF="/etc/rc.conf.d/ntp"
NTP_CONF="/local/etc/rc.conf.d/ntp"

NTPD=/usr/sbin/ntpd
SYNCFILE="/var/run/ntpsync"

PIDFILE=/var/run/ntpd.pid

SLEEPTIME=660
MAX_UNSYNC_TIME=$((24*60)) # in minutes


start() {
	if [ ! -f $NTPD_CONF ]; then
		rc_log "$NTPD_CONF not found! Switching to default configuration."
		cp $NTPD_DEF_CONF $NTPD_CONF
	fi
	. $NTPD_CONF

	if [ ! -f $NTP_CONF ]; then
		rc_log "$NTP_CONF not found! Switching to default configuration."
		cp $NTP_DEF_CONF $NTP_CONF
	fi
	. $NTP_CONF

	PARAMS=""
	if [ ! -z "$PEERS" ]; then
		for server in $PEERS; do
			PARAMS="$PARAMS -p $server"
		done
	else
		rc_msg "NTP peer list is empty. Exiting..."
		exit 1
	fi

	if [ "$ENABLED" = "1" ]; then
		rc_msg "Starting NTP (peers $PEERS)"

		(
			while :; do
				if $NTPD $PARAMS -n -q 2>&1; then
					touch $SYNCFILE

					# Set RTC time
					hwclock -w

				else
					find "$SYNCFILE" -type f -mmin +$MAX_UNSYNC_TIME -exec rm "{}" \; >/dev/null 2>&1
				fi

				sleep $SLEEPTIME

			# pipe output to syslog
			done | logger -p daemon.info -t ntpd

		) & echo $! > $PIDFILE

	else
		rc_msg "NTP disabled. Exiting..."
		exit 1
	fi
}


stop() {
	if [ -f $PIDFILE ]; then
		PID=$(cat $PIDFILE)
		rm $PIDFILE >  /dev/null
		kill $PID
		rm -f $SYNCFILE
	else
		rc_msg "ntpd is not running" " " err
	fi
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
		start
 		;;
	*)
		rc_msg "Unknown command, only start/stop/restart are supported" " " err
		;;
esac

exit $?
