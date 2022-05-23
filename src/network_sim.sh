#!/bin/bash
#
# tc uses the following units when passed as a parameter.
# kbps: Kilobytes per second
# mbps: Megabytes per second
# kbit: Kilobits per second
# mbit: Megabits per second
# bps: Bytes per second
# Amounts of data can be specified in:
# kb or k: Kilobytes
# mb or m: Megabytes
# mbit: Megabits
# kbit: Kilobits
# To get the byte figure from bits, divide the number by 8 bit
# Name of the traffic control command.


if [ -z $2 ]
then
    BANDWIDTH=100mbit
else
    BANDWIDTH=$2mbit
fi

TC=/sbin/tc
IF=lo # The network interface
IP=127.0.0.1 # IP address of the machine we are controlling
DNLD=$BANDWIDTH #100mbit # Download limit (in Megabits)
UPLD=$BANDWIDTH #100mbit # Upload limit (in Megabits)
RTT=40ms # RTT (in mega bits)

# Filter options for limiting the intended interface.
U32="$TC filter add dev $IF protocol ip parent 1:0 prio 1 u32"
start() {
echo
echo "Bandwidth set to:" $DNLD

# We'll use Hierarchical Token Bucket (HTB) to shape bandwidth.
# For detailed configuration options, please consult Linux man page.
$TC qdisc add dev $IF root handle 1: htb default 30
$TC class add dev $IF parent 1: classid 1:1 htb rate $DNLD ceil $DNLD
$TC class add dev $IF parent 1: classid 1:2 htb rate $UPLD ceil $UPLD
$U32 match ip dst $IP/32 flowid 1:1
$U32 match ip src $IP/32 flowid 1:2
$TC qdisc add dev $IF parent 1:1 netem delay $RTT
$TC qdisc add dev $IF parent 1:2 netem delay $RTT
}

stop() {
# Stop the bandwidth shaping.
$TC qdisc del dev $IF root
}

restart() {
# Self-explanatory.
stop
sleep 1
start
}

show() {
# Display status of traffic control status.
$TC -s qdisc ls dev $IF
}

case "$1" in

start)
echo -n "Starting bandwidth shaping: "
start
echo "done"
;;

stop)
echo -n "Stopping bandwidth shaping: "
stop
echo "done"
;;

restart)
echo -n "Restarting bandwidth shaping: "
restart
echo "done"
;;

show)
echo "Bandwidth shaping status for $IF:"
show
echo ""
;;

*)
pwd=$(pwd)
echo "Usage: tc.bash {start|stop|restart|show}"
;;

esac
exit 0
