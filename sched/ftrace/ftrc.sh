#!/bin/bash
# ftrc.sh
# Simple wrapper to use kernel ftrace facility.

trap 'echo 0 > ${PFX}/tracing_on ; popd > /dev/null' INT QUIT

name=$(basename $0)
PFX=/sys/kernel/debug/tracing
TRACE_INTERVAL=5

if [ `id -u` -ne 0 ]; then
	echo "$name: need to be root."
	exit 1
fi

if [ $# -ne 1 ]; then
	echo "Usage: $name ftrace-interval-in-sec"
	exit 1
fi
TRACE_INTERVAL=$1

pushd . >/dev/null
cd ${PFX}

echo "Select tracer from the list:"
cat ${PFX}/available_tracers
read tracer
echo "tracer = $tracer"
#TODO- validity check
echo "${tracer}" > ${PFX}/current_tracer

echo -n "[current_tracer] Current Tracer is: "
cat ${PFX}/current_tracer
echo "[trace_options] Current Trace Options are: "
cat ${PFX}/trace_options
echo

if [ ${tracer} == "function_graph" ]; then
	echo "[set_graph_function] Current function(s) traced are: "
	cat /sys/kernel/debug/tracing/set_graph_function
	echo "Type in your own functions (space-separated); [Enter] keeps default: "
	read graph_funcs
	if [ -n "${graph_funcs}" ]; then
		for func in ${graph_funcs}
		do
			echo "function: $func"
			echo "$func" >> /sys/kernel/debug/tracing/set_graph_function
		done
		echo
		echo "New graph-traced functions are:"
		cat /sys/kernel/debug/tracing/set_graph_function
	fi
fi


echo -n "Confirm Trace options above and START trace? [Y/n]: "
read reply
if [[ $reply == "n" ]] || [[ $reply == "N" ]]; then
        echo "$name: aborting now..."
        exit 1
fi
echo
echo "Will now ftrace for $TRACE_INTERVAL seconds..."
echo "To manually Stop, ^C"
echo
echo "Starting trace now..."
echo 1 > ${PFX}/tracing_on

sleep $TRACE_INTERVAL
echo 0 > ${PFX}/tracing_on

#tail -f ${PFX}/trace >> /tmp/ftrace_log.txt
cat ${PFX}/trace > /tmp/ftrace_log.txt
popd > /dev/null

