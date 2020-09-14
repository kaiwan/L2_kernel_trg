#!/bin/sh
# Query the scheduling attributes (policy and RT (static) priority) of 
# all processes currently alive on the system.
# Just a simple wrapper around chrt.
# 
# Tip: Pipe this o/p to grep for FIFO / RR tasks..
# Also note that a multithreaded process shows up as several same PIDs
#  (resolve these using ps -eLf - to see actual PIDs of threads).

for p in `ps -A -To pid`
do
#	ps -A|grep $p|awk '{print $4}'
	chrt -p $p
	taskset -p $p
done

