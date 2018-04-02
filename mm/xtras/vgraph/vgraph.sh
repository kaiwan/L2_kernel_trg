#!/bin/bash
# vgraph.sh
# 
# Quick Description:
# "Draw" out, (somewhat) to scale, ranges of numbers in a vertically tiled 
# format. For eg.: the output of /proc/iomem, /proc/vmalloc, 
# /proc/<.pid>/maps, etc etc
# 
# We EXPECT as input a file; the file must be in CSV format with 3 columns;
# col 1 and col 2 are ASSumed to be in hexadecimal.
# (as of this very early ver at least). 
# FORMAT ::
#   [...]
# field1,field2,field3
# field1,field2,field3
#   [...]
#
# (As of now):
# field1: integer value (often an address of some sort)
# field2: integer value (often an address of some sort)
# field3: string: descriptive
#
# You must generate the file. For eg.
# you can do:
# awk '{print $1, $6}' /proc/self/maps
# -to get the output of the 'maps' file as 3 columns. Now you must of course
# change the delimiter to a comma ',' (CSV) (could use sed for this). 
# Then pass this script this file.
#
# TODO
# - convert to reqd format
# - check input file for correct format
# - write to SVG !
#
# Last Updated : 30july2017
# Created      : 26july2017
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


########### Globals follow #########################
export PHYADDR_HEX=1
export EMB=1  # simpler [no float point, etc]
DEBUG=0
gDELIM=","


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

#-------------------- p r e p _ f i l e -------------------------------
prep_file()
{
# Get rid of comment lines
sed --in-place '/^#/d' ${gINFILE}
} # end prep_file()

#------------------- g e t _ r a n g e _ i n f o ----------------------
get_range_info()
{
# Get the range: start - end
#  -the first and last numbers!
local int_start=$(head -n1 ${gINFILE} |cut -d"${gDELIM}" -f1 |sed 's/ //') # also trim
local int_end=$(tail -n1 ${gINFILE} |cut -d"${gDELIM}" -f2 |sed 's/ //')

# RELOOK : int value overflows here w/ large 64-bit # as input
# Fixed: use printf w/ %llu fmt
local start_dec=$(printf "%llu" 0x${int_start})   #$(echo $((16#${int_start})))
local end_dec=$(printf "%llu" 0x${int_end})
gTotalLen=$(printf "%llu" $((end_dec-start_dec)))
gFileLines=$(wc -l ${gINFILE} |awk '{print $1}')
decho "range: [${start_dec}-${end_dec}]: size=${gTotalLen}"
} # end get_range_info()

#---
# We require a 4d array to hold:
#          col0   col1   col2   col3
# row'n' [label],[size],[num1],[num2]
# HOWEVER, bash only actually supports 1d array; we thus treat a simple 1d
# array as an 'n'd (n=4) array! 
# So we just populate a 1d array like this:
#  [val1] [val2] [val3] [val4] [val5] [val6] [val7] [val8] [...]
# but INTERPRET it as 2d like so:
#  ([val1],[val2],[val3],[val4]) ([val5],[val6],[val7],[val8]) [...]
declare -a gArray
gRow=0
#---

#-----------------------s h o w A r r a y -----------------------------
showArray()
{
local i k
echo
decho "gRow = ${gRow}"
# TODO / FIXME : soft-code, rm the '4'
for ((i=0; i<${gRow}; i+=4))
do
    printf "[%s, " "${gArray[${i}]}"
	let k=i+1
    printf "%d," "${gArray[${k}]}" 
	let k=i+2
    printf "%x," "0x${gArray[${k}]}" 
	let k=i+3
    printf "%x]\n" "0x${gArray[${k}]}" 
done
} # end showArray()

SCALE_FACTOR=100000000 #20 #200
LIMIT_SCALE_SZ=10

#---------------------- g r a p h i t ---------------------------------
graphit()
{
local i k
local label sz num1 num2
local scaled_sz_fp scaled_sz_int
local szKB=0 szMB=0 szGB=0
local         LIN="+------------------------------------------------------+"
local ELLIPSE_LIN="~ .       .       .       .       .       .        .   ~"
local BOX_RT_SIDE="|                                                      |"
local oversized=0

#decho "gRow=${gRow}" 
#SCALE_FACTOR=$(((SCALE_FACTOR/gRow)*40))

# TODO - test and tweak for various gRow sizes.
# Best if we come up with a formula to calculate the scale factor.
if [ ${gRow} -le 20 ] ; then
 SCALE_FACTOR=20
elif [ ${gRow} -le 200 ] ; then
 SCALE_FACTOR=10000000
elif [ ${gRow} -le 500 ] ; then
 SCALE_FACTOR=100000000
fi

decho "gRow=${gRow}:SCALE_FACTOR=${SCALE_FACTOR}:LIMIT_SCALE_SZ=${LIMIT_SCALE_SZ}:len=${gTotalLen}"

# TODO / FIXME : soft-code, rm the '4'
for ((i=0; i<${gRow}; i+=4))
do
    #--- Retrieve values from the array
    label=${gArray[${i}]}  # col 1 [str: the label]
     #printf "%s: " "${gArray[${i}]}"
	let k=i+1
    sz=${gArray[${k}]}  # col 2 [int: the size]
     #printf "%d\n" "${gArray[${k}]}"
	let k=i+2
    num1=${gArray[${k}]}  # col 3 [int: the first number]
     #printf "%d\n" "${gArray[${k}]}"
	let k=i+3
    num2=${gArray[${k}]}  # col 4 [int: the second number]
     #printf "%d\n" "${gArray[${k}]}"
	#---

    scaled_sz_fp=$(bc <<< "scale=12; ${sz}/${gTotalLen}*100*${SCALE_FACTOR}")
    #scaled_sz_fp=$(bc <<< "scale=9; ${sz}/${gTotalLen}*100*${SCALE_FACTOR}")
	
	#[ ${scaled_sz_fp} -lt 1 ] && scaled_sz_fp=1
	# Convert fp to int
    if (( $(echo "${scaled_sz_fp} < 1" |bc -l) )); then
		scaled_sz_int=1
	else
		scaled_sz_int=$(LC_ALL=C printf "%.0f" "${scaled_sz_fp}")
	fi

	szKB=$((${sz}/1024))
	[ ${szKB} -ge 1024 ] && szMB=$(bc <<< "scale=2; ${szKB}/1024.0") || szMB=0
	# !EMB: if we try and use simple bash arithmetic comparison, we get a 
	# "integer expression expected" err; hence, use bc:
	szGB=0
	if (( $(echo "${szMB} > 1024" |bc -l) )); then
	  szGB=$(bc <<< "scale=2; ${szMB}/1024.0")
	fi
#	[ ${szKB} -ge 1024 ] && szMB=$((szKB/1024)) || szMB=0
#	[ ${szMB} -ge 1024 ] && szGB=$((szMB/1024)) || szGB=0

	[ 0 -eq 1 ] && {
	fg_cyan
	printf " {%.9f %d} " ${scaled_sz_fp} ${scaled_sz_int}
	color_reset
	}
	
	#--- Drawing :-p  !
	#fg_blue
	printf "%s %x\n" "${LIN}" "0x${num1}"
	printf "|%20s  [%d KB" ${label} ${szKB}
	if (( $(echo "${szKB} > 1024" |bc -l) )); then
	  tput bold; printf "  %6.2f MB" ${szMB}
	  if (( $(echo "${szMB} > 1024" |bc -l) )); then
	    printf "  %4.2f GB" ${szGB}
	  fi
	fi
	color_reset
	printf "]\n"

	# draw the sides of the 'box'
	[ ${scaled_sz_int} -gt ${LIMIT_SCALE_SZ} ] && {
		scaled_sz_int=${LIMIT_SCALE_SZ}
		oversized=1
	}
	let scaled_sz_int=scaled_sz_int-1  # no box side for single-line
	for ((x=1; x<${scaled_sz_int}; x++))
	do
		printf "%s\n" "${BOX_RT_SIDE}"
		if [ ${oversized} -eq 1 ] ; then
			[ ${x} -eq $(((LIMIT_SCALE_SZ-1)/2)) ] && printf "%s\n" "${ELLIPSE_LIN}"
		fi
	done
	#---

	#printf "%s\n" ${LIN}
	#color_reset
	#---
	oversized=0
done

#fg_blue
printf "%s %x\n" "${LIN}" "0x${num2}"
#color_reset
} # end graphit()

#------------------ i n t e r p r e t _ r e c -------------------------
# Interpret a record: a 'line' from the input stream:
# Format:
#  num1,num2,label
# eg.
#  00001000,00057fff,System RAM
# This above string will be passed as the parameter to this function.
interpret_rec()
{
#echo "num=$# p=$@"
local int_start=$(echo "${@}" |cut -d"${gDELIM}" -f1)
local int_end=$(echo "${@}" |cut -d"${gDELIM}" -f2)

# Skip comment lines
echo "${int_start}" | grep -q "^#" && return

local label=$(echo "${@}" |cut -d"${gDELIM}" -f3)
[ -z "${label}" ] && label=" [-unnamed-] "

# Convert hex to dec
local start_dec=$(printf "%llu" 0x${int_start})
local end_dec=$(printf "%llu" 0x${int_end})

#numspc=$(grep -o " " <<< ${pa_start} |wc -l)
 # ltrim: now get rid of the leading spaces

local sz=$(printf "%llu" $((end_dec-start_dec)))  # in bytes

#--- Populate the global array
gArray[${gRow}]=${label}
let gRow=gRow+1
gArray[${gRow}]=${sz}
let gRow=gRow+1
gArray[${gRow}]=${int_start}
let gRow=gRow+1
gArray[${gRow}]=${int_end}
let gRow=gRow+1

#disp ${numspc} ${pa_name} ${pa_start_dec} ${pa_end_dec} ${sz}
} # end interpret_rec()

#HDR1="                            /proc/iomem  ::  PHYSICAL ADDRESS SPACE"
#[ ${EMB} -eq 1 ] && HDR11="        [embedded ver]\n" || HDR11="\n"
#HDR2="      Region                                   Phy Addr"
#[ ${PHYADDR_HEX} -eq 0 ] && HDR21=" [decimal]" || HDR21=" [hex]"
#HDR22="                   Size\n"
#HDR3="                                         start       -       end   :       KB     MB     GB\n"
#gHDR="${HDR1}${HDR11}${HDR2}${HDR21}${HDR22}${HDR3}"
gHDR="    <Header line>    
"

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

 prep_file
 get_range_info
 export IFS=$'\n'
 local i=0
 printf "${gHDR}"

 # TODO - progress indication
 printf "%s: Processing, pl wait ...\n" "${name}"

 local REC
 for REC in $(cat ${gINFILE})
 do 
   #echo "REC: $REC"
   interpret_rec ${REC}
   printf "=== %06d / %06d\r" ${i} ${gFileLines}
   let i=i+1
   #[ $i -ge 18 ] && break
 done

 #showArray
 graphit

 #for SCALE_FACTOR in 200 400 600 800 1000 2000
 #do
 #   graphit
 #done
} # end start()


##### 'main' : execution starts here #####

[ 0 -eq 1 ] && {
 n=0x55e64cd85000
 printf "%d\n" $n
 exit 0
}

[ $# -ne 1 ] && {
  echo "Usage: ${name} input-CSV-filename(3 column format)"
  exit 1
}
[ ! -f $1 ] && {
  echo "${name}: input-CSV-filename \"$1\" invalid? Aborting..."
  exit 1
}
[ ! -r $1 ] && {
  echo "${name}: input-CSV-filename \"$1\" not readable? Aborting..."
  exit 1
}

gINFILE=$1
start
exit 0
