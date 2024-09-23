#!/bin/bash
SCHED_FILE=/sys/kernel/debug/sched/debug
DELAYSEC=.25
[[ $(id -u) -ne 0 ]] && {
  echo "need root to view ${SCHED_FILE}" ; exit 1
}
while [ true ]
do
	egrep '^ *R ' ${SCHED_FILE} # show only runnable/running
	#egrep '^R ' /proc/sched_debug # show only runnable/running
	sleep $DELAYSEC
done

	#egrep -A4 'tree-key' /proc/sched_debug |egrep -v 'task.*PID.*tree|----------|cpu#.'
