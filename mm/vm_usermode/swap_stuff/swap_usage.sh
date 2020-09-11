#!/bin/bash
# swap_usage.sh
# 
# Quick Description:
# 
# 
# Last Updated :
# Created      : 14 Nov 2014
# 
# Author:
# Kaiwan N Billimoria
# kaiwan -at- kaiwantech -dot- com
# kaiwanTECH
# 
# License:
# GPL / LGPL
# 
name=$(basename $0)
source ./common.sh || {
 echo "$name: could not source common.sh , aborting..."
 exit 1
}

SLPTM_MIN=1
tm=0
########### Functions follow #######################



main()
{
	SLPTM_SEC=$((SLPTM_MIN*60))
while [ true ]
do
#		date
	swapKB=$(free |grep "^Swap" |awk '{print $3}')
	echo "${tm} ${swapKB}"
	tm=$((${tm}+1))  # cumulative time elapsed in minutes
	sleep ${SLPTM_SEC}
done

}

##### execution starts here #####

main
exit 0
