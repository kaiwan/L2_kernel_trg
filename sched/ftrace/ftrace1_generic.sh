#!/bin/bash
# Ftrace demo via a bash script
# We ftrace a 'sleep 1'
#
# (c) Kaiwan NB, kaiwanTECH
# May16

reset_ftrc()
{
 echo 0 > tracing_on
 echo nop > current_tracer
 echo 1 > options/latency-format
 echo 0 > options/context-info
 echo 0 > options/userstacktrace
 echo 0 > tracing_max_latency # reset

 echo "" > set_ftrace_filter
 echo "" > set_ftrace_notrace
 echo "" > set_ftrace_pid
 echo 2048 > buffer_size_kb
}

init_ftrc()
{
 echo function_graph > current_tracer
 echo 1 > options/latency-format
 echo 1 > options/context-info
 echo funcgraph-proc > trace_options
 echo 1 > options/userstacktrace

 echo "" > set_ftrace_filter
 echo "" > set_ftrace_notrace
 echo "" > set_ftrace_pid
 echo 20480 > buffer_size_kb
}

PFX=/sys/kernel/debug
TRC=${PFX}/tracing
TMPFILE=/tmp/trc.txt

## "main" here
[ `id -u` -ne 0 ] && {
 echo "Need to be root."
 exit 1
}

echo "Checking for ftrace support ..."
mount | grep debugfs > /dev/null 2>&1 || {
 echo "debugfs not mounted? Pl mount the debugfs filesystem & retry. Aborting..."
 exit 1
}
TRC=$(mount|grep debugfs |awk '{print $3}')
[ ! -d ${TRC} ] && {
 echo "\"${TRC}\" not present? Aborting..."
 exit 2
}

TRC=${TRC}/tracing
cd ${TRC} || exit 3
echo "ftrace ok, init ..."
reset_ftrc
init_ftrc
#echo "Available tracers:"
#cat available_tracers

echo "Running ..."

 # trace

 echo 1 > tracing_on ; sleep 1 ; echo 0 > tracing_on 
 cp trace ${TMPFILE}

echo "Done. Trace file in ${TMPFILE}"
ls -lh ${TMPFILE}
reset_ftrc
