#!/bin/sh

if [ $# -ne 1 ]; then
	echo "Usage: $0 char-to-display"
	exit 1
fi

echo "PID: $$"
echo
while [ true ]
do
	echo -n $1
	#usleep 1000
	# do some junk, spend some time...
	for i in `seq 1 500`
	do
		tmp=$(($i+5))
	done
done

