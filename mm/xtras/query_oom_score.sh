#!/bin/sh

IFS=$'\n'
for token in $(ps -eo pid,comm)
do
 echo "${token}"
 pid=$(echo "${token}" |awk '{print $1}' |xargs echo)
 name=$(echo "${token}"|awk '{print $2}' |xargs echo)
 echo "PID: ${pid} nm: ${name}"
 #cat /proc/${cleanpid}/oom_score
 #cat /proc/${cleanpid}/oom_score_adj
 echo
done

