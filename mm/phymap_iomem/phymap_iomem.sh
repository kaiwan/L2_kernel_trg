#!/bin/bash
# phymap_iomem.sh
# 
# Quick Description:
# /proc/iomem describes the _physical_ address space, in effect,
# it's a view of the physical memory map.
# 
# Last Updated : 25july2017
# Created      : 25july2017
# 
# Author:
# Kaiwan N Billimoria
# kaiwan -at- kaiwantech -dot- com
# kaiwanTECH
# 
# License:
# MIT License.
# 
name=$(basename $0)
source ./common.sh || {
 echo "${name}: fatal: could not source common.sh , aborting..."
 exit 1
}

#[ $# -ne 1 ] && {
#  echo "Usage: ${name} "
#  exit 1
#}

export TMPF=/tmp/.$$
export PHYADDR_HEX=1
export EMB=1  # simpler [no float point, etc]
DEBUG=1

########### Functions follow #######################

#------------------------- d i s p ------------------------------------
# eg. disp ${numspc} ${pa_name} ${pa_start_dec} ${pa_end_dec} ${sz}
# Params:
#  $1 : left indentation length
#  $2 : Region
#  $3 : start phy addr
#  $4 : end phy addr
#  $5 : size of region in bytes
disp()
{
local sp=$(($1+1))
local fmtname=$((30-${sp}))
local szKB=$(($5/1024))
local szMB=0
local szGB=0

[ ${PHYADDR_HEX} -eq 0 ] && {
  printf "%${sp}s%-${fmtname}s:%16d   %16d [%9d" \
		" " "${2}" "${3}" "${4}" ${szKB}
} || {
  printf "%${sp}s%-${fmtname}s:%16lx   %16lx [%9d" \
		" " "${2}" "${3}" "${4}" ${szKB}
}

# Calculate sizes in MB and GB if required
[ ${EMB} -eq 0 ] && {
  [ ${szKB} -ge 1024 ] && szMB=$(bc <<< "scale=2; ${szKB}/1024.0")
  # !EMB: if we try and use simple bash arithmetic comparison, we get a 
  # "integer expression expected" err; hence, use bc:
  if (( $(echo "${szMB} > 1024" |bc -l) )); then
    szGB=$(bc <<< "scale=2; ${szMB}/1024.0")
  fi

  if (( $(echo "${szMB} > 0" |bc -l) )); then
    printf "  %6.2f" ${szMB}
  fi
  if (( $(echo "${szGB} > 0" |bc -l) )); then
    printf "  %4.2f" ${szGB}
  fi
} || {  # embedded sys: simpler
  [ ${szKB} -ge 1024 ] && szMB=$((${szKB}/1024))
  [ ${szMB} -ge 1024 ] && szGB=$((${szMB}/1024))
  [ ${szMB} -gt 0 ] && printf "  %6d" ${szMB}
  [ ${szGB} -gt 0 ] && printf "  %4d" ${szGB}
}

printf "]\n"
} # end disp()

#------------------- g e t _ r a n g e _ i n f o ----------------------
get_range_info()
{
# Get the range: start - end
#  -the first and last addresses!
local par_start=$(head -n1 ${TMPF} |cut -d":" -f1) # |sed 's/ //') # par = phy addr range
local start=$(echo "${par_start}" |cut -d"-" -f1)
local par_end=$(tail -n1 ${TMPF} |cut -d":" -f1 |sed 's/ //') # par = phy addr range
local end=$(echo "${par_end}" |cut -d"-" -f2)

local start_dec=$(echo $((16#${start})))
local end_dec=$(echo $((16#${end})))
gTotalLen=$((end_dec-start_dec))
} # end get_range_info()

#---
# We require a 2d array to hold:
#          col0   col1
# row'n' [label],[size]
# HOWEVER, bash only actually supports 1d array; we thus treat a simple 1d array
# as a 2d array! 
# So we just populate a 1d array like this:
#  [val1] [val2] [val3] [val4] [val5][val6] [...]
# but INTERPRET it as 2d like so:
#  ([val1],[val2]) ([val3],[val4]) ([val5],[val6]) [...]
declare -a gArray
gRow=0
#---

showArray()
{
local i k
echo "gRow = ${gRow}"
for ((i=0; i<${gRow}; i+=2))
do
    printf "%s: " "${gArray[${i}]}"
	let k=i+1
    printf "%d\n" "${gArray[${k}]}" 
done
}

SCALE_FACTOR=50
LIMIT_SCALE_SZ=10

#---------------------- g r a p h i t ---------------------------------
graphit()
{
local i k
local label sz scaled_sz_fp scaled_sz_int
local szKB=0 szMB=0 szGB=0
local LIN="+------------------------------------+"
local ELLIPSE_LIN="~ . . . . . . . . . . . . . . . . .  ~"
local oversized=1

decho "len=${gTotalLen}"

for ((i=0; i<${gRow}; i+=2))
do
    label=${gArray[${i}]}
    #printf "%s: " "${gArray[${i}]}"
	let k=i+1
    sz=${gArray[${k}]}
    #printf "%d\n" "${gArray[${k}]}"
    scaled_sz_fp=$(bc <<< "scale=9; (${sz}/${gTotalLen})*100*${SCALE_FACTOR}")
	
	#[ ${scaled_sz_fp} -lt 1 ] && scaled_sz_fp=1
	# Convert fp to int
    if (( $(echo "${scaled_sz_fp} < 1" |bc -l) )); then
		scaled_sz_int=1
	else
		scaled_sz_int=$(LC_ALL=C printf "%.0f" "${scaled_sz_fp}")
	fi

	szKB=$((${sz}/1024))
	[ ${szKB} -ge 1024 ] && szMB=$((${szKB}/1024))
	[ ${szMB} -ge 1024 ] && szGB=$((${szMB}/1024))

	[ 0 -eq 1 ] && {
    printf "%s: %.6f: %d [%d KB" \
		"${label}" "${scaled_sz_fp}" "${scaled_sz_int}" "${szKB}"
	[ ${szMB} -gt 0 ] && printf "  %6d" ${szMB}
	[ ${szGB} -gt 0 ] && printf "  %4d" ${szGB}
	printf "]\n"
	}
	
	#--- Drawing :-p  !
	fg_blue
	printf "%s\n" ${LIN}
	printf "|%-20s  [%d KB] \n" ${label} ${szKB}
	  # TODO - indentation of label
	  #      - pr addresses

	# draw the sides of the 'box'
	[ ${scaled_sz_int} -gt ${LIMIT_SCALE_SZ} ] && {
		scaled_sz_int=${LIMIT_SCALE_SZ}
		oversized=1
	}
	for ((x=0; x<${scaled_sz_int}; x++))
	do
		printf "|                                    |\n"
		if [ ${oversized} -eq 1 ] ; then
			[ ${x} -eq $(((LIMIT_SCALE_SZ-1)/2)) ] && printf "%s\n" "${ELLIPSE_LIN}"
		fi
	done

	#printf "%s\n" ${LIN}
	color_reset
	oversized=0
done
printf "%s\n" ${LIN}

} # end graphit()

#------------------ i n t e r p r e t _ i o m e m _ r e c -------------
# Interpret a 'line' from output of /proc/iomem ; 
# eg. looks like this
#  00001000-00057fff : System RAM
interpret_iomem_rec()
{
#echo "num=$# p=$@"
local par=$(echo "${@}" |cut -d":" -f1) # par = phy addr range
local pa_name=$(echo "${@}" |cut -d":" -f2)

# pa_start:
local pa_start=$(echo "${par}" |cut -d"-" -f1)
# we need to count the indentation - the spaces on the left
numspc=$(grep -o " " <<< ${pa_start} |wc -l)
 # ltrim: now get rid of the leading spaces
local pa_start=$(echo "${pa_start}" |sed 's/^ *//')
local pa_end=$(echo "${par}" |cut -d"-" -f2)

local pa_start_dec=$(echo $((16#${pa_start})))
local pa_end_dec=$(echo $((16#${pa_end})))
local sz=$((pa_end_dec-pa_start_dec))  # in bytes

# Populate the global array
gArray[${gRow}]=${pa_name}
let gRow=gRow+1
gArray[${gRow}]=${sz}
let gRow=gRow+1

#disp ${numspc} ${pa_name} ${pa_start_dec} ${pa_end_dec} ${sz}
} # end interpret_iomem_rec()

HDR1="                            /proc/iomem  ::  PHYSICAL ADDRESS SPACE"
[ ${EMB} -eq 1 ] && HDR11="        [embedded ver]\n" || HDR11="\n"
HDR2="      Region                                   Phy Addr"
[ ${PHYADDR_HEX} -eq 0 ] && HDR21=" [decimal]" || HDR21=" [hex]"
HDR22="                   Size\n"
HDR3="                                         start       -       end   :       KB     MB     GB\n"
gHDR="${HDR1}${HDR11}${HDR2}${HDR21}${HDR22}${HDR3}"


arrtest()
{
declare -a arry
local r=5 c=2

for ((i=0; i<r; i++))
do
    for ((j=0; j<c; j++))
    do
        a[${i},${j}]=$RANDOM
    done
done

for ((i=0; i<r; i++))
do
    for ((j=0; j<c; j++))
    do
        printf "%d " ${a[${i},${j}]}
    done
	echo
done
}

#--------------------------- s t a r t --------------------------------
start()
{

[ 0 -eq 1 ] && {
 arrtest
 exit 0
}

 [ ${EMB} -eq 0 ] && {
   which bc >/dev/null || {
     echo "${name}: bc package missing, pl install. Aborting..."
     exit 1
   }
 }

 sudo cat /proc/iomem > ${TMPF}
 get_range_info
#exit 0

 export IFS=$'\n'
 local i=0
 printf "${gHDR}"

 local REC
 for REC in $(cat ${TMPF})
 do 
   #echo "REC: $REC"
   interpret_iomem_rec ${REC}
   let i=i+1
   #[ $i -ge 18 ] && break
 done

 #showArray
 graphit

 sudo rm -f ${TMPF}
} # end start()

##### 'main' : execution starts here #####

start
exit 0
