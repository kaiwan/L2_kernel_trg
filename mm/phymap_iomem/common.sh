#!/bin/sh
#------------------------------------------------------------------
# common.sh
#
# Common convenience routines
# 
# (c) Kaiwan N Billimoria
# kaiwan -at- kaiwantech -dot- com
# MIT / GPL v2
#------------------------------------------------------------------
# The SEALS Opensource Project
# SEALS : Simple Embedded Arm Linux System
# Maintainer : Kaiwan N Billimoria
# kaiwan -at- kaiwantech -dot- com
# Project URL:
# https://github.com/kaiwan/seals

export TOPDIR=$(pwd)
ON=1
OFF=0

### UPDATE for your box
PFX=.
source ${PFX}/err_common.sh || {
 echo "$name: could not source ${PFX}/err_common.sh, aborting..."
 exit 1
}
source ${PFX}/color.sh || {
 echo "$name: could not source ${PFX}/color.sh, aborting..."
 exit 1
}

# If we're not in a GUI (X Windows) display, abort (reqd for yad)
check_gui()
{
 which xdpyinfo > /dev/null 2>&1 || {
   FatalError "xdpyinfo (package x11-utils) does not seem to be installed. Aborting..."
 }
 xdpyinfo >/dev/null 2>&1 || {
   FatalError "Sorry, we're not running in a GUI display environment. Aborting..."
 }
 which xrandr > /dev/null 2>&1 || {
   FatalError "xrandr (package x11-server-utils) does not seem to be installed. Aborting..."
 }

 #--- Screen Resolution stuff
 res_w=$(xrandr --current | grep '*' | uniq | awk '{print $1}' | cut -d 'x' -f1)
 res_h=$(xrandr --current | grep '*' | uniq | awk '{print $1}' | cut -d 'x' -f2)
 centre_x=$((($res_w/3)+0))
 centre_y=$((($res_h/3)-100))
 CAL_WIDTH=$((($res_w/3)+200))
 CAL_HT=$(($res_h/3))
}


# genLogFilename
# Generates a logfile name that includes the date/timestamp
# Format:
#  ddMmmYYYY[_HHMMSS]
# Parameter(s)
# #$1 : String to prefix to log filename, null okay as well [required]
#  $1 : Include time component or not [required]
#    $1 = 0 : Don't include the time component (only date) in the log filename
#    $1 = 1 : include the time component in the log filename
genLogFilename()
{
 [ $1 -eq 0 ] && log_filename=$(date +%d%b%Y)
 [ $1 -eq 1 ] && log_filename=$(date +%d%b%Y_%H%M%S)
 echo ${log_filename}
}

# mysudo
# Simple front end to gksudo/sudo
# Parameter(s):
#  $1 : descriptive message
#  $2 ... $n : command to execute
mysudo()
{
[ $# -lt 2 ] && {
 #echo "Usage: mysudo "
 return
}
local msg=$1
shift
local cmd="$@"
aecho "${LOGNAME}: ${msg}"
sudo --preserve-env sh -c "${cmd}"
}

# check_root_AIA
# Check whether we are running as root user; if not, exit with failure!
# Parameter(s):
#  None.
# "AIA" = Abort If Absent :-)
check_root_AIA()
{
	if [ `id -u` -ne 0 ]; then
		Echo "Error: need to run as root! Aborting..."
		exit 1
	fi
}

# check_file_AIA
# Check whether the file, passed as a parameter, exists; if not, exit with failure!
# Parameter(s):
#  $1 : Pathname of file to check for existence. [required]
# "AIA" = Abort If Absent :-)
# Returns: 0 on success, 1 on failure
check_file_AIA()
{
	[ $# -ne 1 ] && return 1
	[ ! -f $1 ] && {
		Echo "Error: file \"$1\" does not exist. Aborting..."
		exit 1
	}
}

# check_folder_AIA
# Check whether the directory, passed as a parameter, exists; if not, exit with failure!
# Parameter(s):
#  $1 : Pathname of folder to check for existence. [required]
# "AIA" = Abort If Absent :-)
# Returns: 0 on success, 1 on failure
check_folder_AIA()
{
	[ $# -ne 1 ] && return 1
	[ ! -d $1 ] && {
		Echo "Error: folder \"$1\" does not exist. Aborting..."
		exit 1
	}
}

# check_folder_createIA
# Check whether the directory, passed as a parameter, exists; if not, create it!
# Parameter(s):
#  $1 : Pathname of folder to check for existence. [required]
# "IA" = If Absent :-)
# Returns: 0 on success, 1 on failure
check_folder_createIA()
{
	[ $# -ne 1 ] && return 1
	[ ! -d $1 ] && {
		Echo "Folder \"$1\" does not exist. Creating it..."
		mkdir -p $1	&& return 0 || return 1
	}
}


# GetIP
# Extract IP address from ifconfig output
# Parameter(s):
#  $1 : name of network interface (string)
# Returns: IPaddr on success, non-zero on failure
GetIP()
{
	[ $# -ne 1 ] && return 1
	ifconfig $1 >/dev/null 2>&1 || return 2
	ifconfig $1 |grep 'inet addr'|awk '{print $2}' |cut -f2 -d':'
}

# get_yn_reply
# User's reply should be Y or N.
# Returns:
#  0  => user has answered 'Y'
#  1  => user has answered 'N'
get_yn_reply()
{
aecho -n "Type Y or N please (followed by ENTER) : "
str="${@}"
while true
do
   aecho ${str}
   read reply

   case "$reply" in
   	y | yes | Y | YES ) 	return 0
			;;
   	n* | N* )		return 1
			;;	
   	*) aecho "What? Pl type Y / N"
   esac
done
}

# MountPartition
# Mounts the partition supplied as $1
# Parameters:
#  $1 : device node of partition to mount
#  $2 : mount point
# Returns:
#  0  => mount successful
#  1  => mount failed
MountPartition()
{
[ $# -ne 2 ] && {
 aecho "MountPartition: parameter(s) missing!"
 return 1
}

DEVNODE=$1
[ ! -b ${DEVNODE} ] && {
 aecho "MountPartition: device node $1 does not exist?"
 return 1
}

MNTPT=$2
[ ! -d ${MNTPT} ] && {
 aecho "MountPartition: folder $2 does not exist?"
 return 1
}

mount |grep ${DEVNODE} >/dev/null || {
 #echo "The partition is not mounted, attempting to mount it now..."
 mount ${DEVNODE} -t auto ${MNTPT} || {
  wecho "Could not mount the '$2' partition!"
  return 1
 }
}
return 0
}

## is_kernel_thread
# Param: PID
# Returns:
#   1 if $1 is a kernel thread, 0 if not, 127 on failure.
is_kernel_thread()
{
[ $# -ne 1 ] && {
 aecho "is_kernel_thread: parameter missing!" 1>&2
 return 127
}

prcs_name=$(ps aux |awk -v pid=$1 '$2 == pid {print $11}')
#echo "prcs_name = ${prcs_name}"
[ -z ${prcs_name} ] && {
 wecho "is_kernel_thread: could not obtain process name!" 1>&2
 return 127
}

firstchar=$(echo "${prcs_name:0:1}")
#echo "firstchar = ${firstchar}"
len=${#prcs_name}
let len=len-1
lastchar=$(echo "${prcs_name:${len}:1}")
#echo "lastchar = ${lastchar}"
[ ${firstchar} = "[" -a ${lastchar} = "]" ] && return 1 || return 0
}

#---------------- n u m t h r e a d s ---------------------------------
# Given a process name, calculates the # of threads currently alive
# within it _and_ other processes of the same name.
# Params:
# $1 = process name
# Ret: # of threads curr alive in the given process
#  -the 'return' is an artifact; we echo the value; the caller is to
#   pick it up using the usual quoting technique.
numthreads()
{
local pid nthrd total_thrds
#for pidrec in $(ps -LA|egrep "^$1")
# Loop over all processes by given name
for pid in $(pgrep "$1")
do
  #echo "pid: ${pid}"
  # get # threads within this pid
  nthrd=$(ls /proc/${pid}/task |wc -w)
  #echo "${pid}:${nthrd}"
  let total_thrds+=nthrd
done
echo "${total_thrds}"
} # end numthreads()

#------------------- p r _ s z _ h u m a n ----------------------------
# Prints given numeric KB value in MB, GB as required.
# Requires: bc
# Parameters:
# $1 = label
# $2 = size in KB
#
# TODO - on embedded systems it's unlikely 'bc' will be available; 
#  must do without it..
pr_sz_human()
{
[ -z $2 ] && return
which bc >/dev/null || return

local szKB=$2
local szMB=0 szGB=0 szTB=0 szPB=0 szEB=0 szZB=0 szYB=0 szBB=0  szGoB=0 

#echo "p1 = $1 ; p2 = $2 ; @ = $@"

local verbose=0
[ ${verbose} -eq 1 ] && {
 printf "%30s:%9s:%9s:%9s:%9s:%9s:%9s:%9s:%9s:%9s:%9s\n" " " "KB" "MB" "GB" "PB" "TB" "EB" "ZB" "YB" "BB" "GoB"
}

  [ ${szKB} -ge 1024 ] && szMB=$(bc <<< "scale=2; ${szKB}/1024.0")      # MB : megabytes : 10^6
  # !EMB: if we try and use simple bash arithmetic comparison, we get a
  # "integer expression expected" err; hence, use bc:
  if (( $(echo "${szMB} > 1024" |bc -l) )); then                        # GB : gigabytes : 10^9
    szGB=$(bc <<< "scale=2; ${szMB}/1024.0")
  fi
  if (( $(echo "${szGB} > 1024" |bc -l) )); then                        # TB : terabytes : 10^12
    szTB=$(bc <<< "scale=2; ${szGB}/1024.0")
  fi
  if (( $(echo "${szTB} > 1024" |bc -l) )); then                        # PB : petabytes : 10^15
    szPB=$(bc <<< "scale=2; ${szTB}/1024.0")
  fi
  if (( $(echo "${szPB} > 1024" |bc -l) )); then                        # EB : exabytes : 10^18
    szEB=$(bc <<< "scale=2; ${szPB}/1024.0")
  fi
  if (( $(echo "${szEB} > 1024" |bc -l) )); then                        # ZB : zettabytes : 10^21
    szZB=$(bc <<< "scale=2; ${szEB}/1024.0")
  fi
  if (( $(echo "${szZB} > 1024" |bc -l) )); then                        # YB : yottabytes : 10^24
    szYB=$(bc <<< "scale=2; ${szZB}/1024.0")
  fi
  if (( $(echo "${szYB} > 1024" |bc -l) )); then                        # BB : brontobytes : 10^27
    szBB=$(bc <<< "scale=2; ${szYB}/1024.0")
  fi
  if (( $(echo "${szBB} > 1024" |bc -l) )); then                        # GB [?] : geopbytes : 10^30
    szGoB=$(bc <<< "scale=2; ${szBB}/1024.0")
  fi

  printf "%-25s:%9ld" "$1" ${szKB}
  if (( $(echo "${szMB} > 0" |bc -l) )); then           # print MB
    printf ":%9.2f" ${szMB}
  fi
  if (( $(echo "${szGB} > 0" |bc -l) )); then           # print GB
    printf ":%9.2f" ${szGB}
  fi
  if (( $(echo "${szTB} > 0" |bc -l) )); then           # print TB
    printf ":%9.2f" ${szTB}
  fi
  if (( $(echo "${szPB} > 0" |bc -l) )); then           # print PB
    printf ":%9.2f" ${szPB}
  fi
  if (( $(echo "${szEB} > 0" |bc -l) )); then           # print EB
    printf ":%9.2f" ${szEB}
  fi
  if (( $(echo "${szZB} > 0" |bc -l) )); then           # print ZB
    printf ":%9.2f" ${szZB}
  fi
  if (( $(echo "${szYB} > 0" |bc -l) )); then           # print YB
    printf ":%9.2f" ${szYB}
  fi
  if (( $(echo "${szBB} > 0" |bc -l) )); then           # print BB
    printf ":%9.2f" ${szBB}
  fi
  if (( $(echo "${szGoB} > 0" |bc -l) )); then           # print GoB
    printf ":%9.2f" ${szGoB}
  fi
  printf "\n"
}
