#!/bin/bash
trap 'pkill t1; pkill t2; pkill t3' INT QUIT EXIT

[ $# -ne 1 ] && {
  echo "Usage: $0 0|1
 0 => show Only our three runnable/running processes t1, t2, t3
 1 => show system-wide runnable/running processes "
  exit 1
}
SCHED_FILE=/sys/kernel/debug/sched/debug
[[ $(id -u) -ne 0 ]] && {
  echo "need root to view ${SCHED_FILE}" ; exit 1
}

taskset -c 02 ./t1 a > /dev/null &
taskset -c 02 ./t2 a > /dev/null &
taskset -c 02 ./t3 a > /dev/null &

HDR="
            task   PID         tree-key  switches  prio     exec-runtime         sum-exec        sum-sleep       Task Group
-------------------------------------------------------------------------------------------------------------------------------------"
echo "${HDR}"
[ $1 -eq 0 ] && ./show_rq.sh |grep "t[1-3]" || [ $1 -eq 1 ] && ./show_rq.sh

