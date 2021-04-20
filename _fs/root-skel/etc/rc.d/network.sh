#!/bin/sh

. /etc/rc.subr

DEFCONF=/etc/rc.conf.d/network
CONF=/local/etc/rc.conf.d/network

ROUTES=/var/run/routes
RET=0


# Function evaluates network variable
network_var() {
	iface=$1
	var=$2
	eval echo "\$network_${iface}_${var}"
}


network_ifconfig() {
	iface=$1
	status="down"
	mode="dhcp"
	modefl="dynamic"
	ip=""
	mask=""

	rc_msg "  $iface" -n

	if [ "X$(network_var $iface enabled)" == "X1" ]; then
		status="up"
		mode="$(network_var $iface mode)"

		if [ "X$mode" == "Xstatic" ]; then
			modefl="-dynamic"
			ip="$(network_var $iface ip)"
			mask="$(network_var $iface mask)"

			rc_msg " $status $mode $ip netmask $mask" -n
			ifconfig_args="$status $modefl $ip netmask $mask"
		else
			modefl="dynamic"
			rc_msg " $status $mode" -n
			ifconfig_args="$status $modefl"
		fi
	else
		rc_msg " $status" -n
		ifconfig_args="$status"
	fi

	if ! ifconfig $iface $ifconfig_args >/dev/null 2>&1; then
		rc_msg "  [FAILED]" " " err
		RET=1
	else
		gw=$(network_var $iface gateway)
		metric=$(network_var $iface metric)

		if [ "X$status" == "Xup" ] && ( [ "X$gw" != "X" ] || [ "X$metric" != "X" ] ); then

			if [ "X$metric" == "X" ]; then
				metric="64"
			fi

			if [ "X$gw" == "X" ] || [ "X$mode" = "Xdhcp" ]; then
				route_args="metric $metric"
				gw="default"
			else
				route_args="gw $gw metric $metric"
			fi

			rc_msg " gw $gw metric $metric" -n
			if ! route add default $route_args $iface >/dev/null 2>&1; then
				rc_msg " [FAILED]" " " err
				RET=1
			else
				echo "default $route_args $iface" >> $ROUTES
				echo ""
			fi
		else
			echo ""
		fi
	fi
}


if [ ! -f $CONF ]; then
	rc_log "Network configuration not found! Switching to default configuration."
	cp $DEFCONF $CONF
fi
. $CONF

rc_msg "Configuring network"

touch $ROUTES

while read route; do
	route del $route
done < $ROUTES

rm -f $ROUTES

for i in $network_interfaces; do
	network_ifconfig $i
done

exit $RET
