#!/bin/bash
DELAYSEC=.25
while [ true ]
do
	egrep '^R ' /proc/sched_debug # show only runnable/running
	sleep $DELAYSEC
done

	#egrep -A4 'tree-key' /proc/sched_debug |egrep -v 'task.*PID.*tree|----------|cpu#.'
