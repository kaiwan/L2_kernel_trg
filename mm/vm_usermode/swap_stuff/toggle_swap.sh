#!/bin/bash
# Scott Severance
# http://askubuntu.com/questions/1357/how-to-empty-swap-if-there-is-free-ram
# Small enhancements: Kaiwan NB, Nov14.

export name=$(basename $0)
source ./common.sh || {
 echo "$name: could not source common.sh , aborting..."
 exit 1
}

export SWAP_THRESHOLD_MB=175

function getData()
{
free_data="$(free)"
mem_data="$(echo "$free_data" | grep 'Mem:')"
free_mem="$(echo "$mem_data" | awk '{print $4}')"
buffers="$(echo "$mem_data" | awk '{print $6}')"
cache="$(echo "$mem_data" | awk '{print $7}')"
total_free=$((free_mem + buffers + cache))
used_swap="$(echo "$free_data" | grep 'Swap:' | awk '{print $3}')"
}

## The messages
MSG_SWAPOFF_ATTEMPT="${name} :: Swap usage high [${SWAP_THRESHOLD_MB} MB], attempting swapoff -a now..."
MSG_SWAP_FREED="${name} :: Freed SWAP : Free memory: $total_free kB ($((total_free / 1024)) MB). \
Used swap: $used_swap kB ($((used_swap / 1024)) MB)"
MSG_SWAPOFF_FAILED="${name} :: Swap usage high [${SWAP_THRESHOLD_MB} MB], swapoff -a cmd failed"
##

#function DesktopNotify()
#{
#	# Ubuntu : notify-send !
#	[ $# -ne 1 ] && MSG="<bug: no message parameter :)>" || MSG="$1"
#	notify-send --urgency=low "${MSG}"
#}

function toggleSwap()
{
if [[ $used_swap -lt $total_free ]]; then
	# Add a condition:
	# only free swap if above a threshold - knb
	USED_SWAP_MB=$((${used_swap}/1024))
	if [[ ${USED_SWAP_MB} -ge ${SWAP_THRESHOLD_MB} ]]; then
		DesktopNotify "${MSG_SWAPOFF_ATTEMPT}"
    	sudo swapoff -a && DesktopNotify "${MSG_SWAP_FREED}" || DesktopNotify "${MSG_SWAPOFF_FAILED}"
    	sudo swapon -a
	fi
else
    echo "Not enough free memory. Exiting."
    exit 1
fi
}

TMOUT_MIN=5
function main()
{
  while [ true ]
  do
    getData
    toggleSwap
	sleep $((${TMOUT_MIN}*60))
  done
}

[ $(id -u) -ne 0 ] && {
 echo "$name: need to run as root (sudo)"
 exit 1
}
DesktopNotify "${name} : starting...(timeout interval: ${TMOUT_MIN} min)"
main

