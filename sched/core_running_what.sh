#!/bin/bash
# core_running_what.sh
name=$(basename $0)

# Show thread(s) running on cpu core 'n'  - func c'n'
function c0()
{
ps -eLF |awk '{ if($5==0) print $0}'
}
function c1()
{
ps -eLF |awk '{ if($5==1) print $0}'
}
function c2()
{
ps -eLF |awk '{ if($5==2) print $0}'
}
function c3()
{
ps -eLF |awk '{ if($5==3) print $0}'
}

# "main" ..
echo "Run this script as
 source ${name}
and then run the builtin functions 'c0', 'c1', 'c2', ...
to see which threads are running on which core."
