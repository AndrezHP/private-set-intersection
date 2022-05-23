#!/bin/sh

uflag=
pflag=
eflag=
cflag=
hflag=
logflag=

while getopts 'helpuc' OPTION
do
	case $OPTION in
	p)	pflag="-D PRIME"
		;;
	e)	eflag="-D ECC"
		;;
	l)	logflag="-D LOG"
		;;
	u)	uflag="-D UB"
		;;
	c)	cflag="-D CUCKOO"
		;;
	h)	hflag="-D HASH"
		;;
	?)	printf "Wrong argmuents"
		exit 2
		;;
	esac
done
shift $(($OPTIND - 1))

g++ client.cpp -o client -O2 $logflag $eflag $pflag $uflag $cflag $hflag -lsodium -lgmp
g++ server.cpp -o server -O2 $logflag $eflag $pflag $uflag $cflag $hflag -lsodium -lgmp
