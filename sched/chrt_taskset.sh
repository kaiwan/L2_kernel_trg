#!/bin/bash
# Display sched policy & priority of all processes

which chrt || {
	echo "'chrt' not installed?"
	exit 1
}
which taskset || {
	echo "'taskset' not installed?"
	exit 1
}

for p in $(ps -A|awk '{print $1}')
do
	chrt -p $p
	taskset -p $p
	echo
done

