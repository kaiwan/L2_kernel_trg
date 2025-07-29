#!/bin/bash
# core_running_what.sh
name=$(basename $0)

# This is how it should be... so we generate the funcs!
[[ 0 -eq 1 ]] && {
# Show thread(s) running on cpu core 'n'  - func c'n'
function c0()
{
ps -eLF |awk '{ if($5==0) print $0}'
}

# "main" ..

numcores=$(($(nproc)-1))
[[ ${numcores} -le 0 ]] && numcores=1

SOURCED_SCRIPT=corefuncs
echo "Generating script \"${SOURCED_SCRIPT}\" for CPU cores 0 to ${numcores} now..."
rm -f ${SOURCED_SCRIPT}
for core in $(seq 0 ${numcores})
do
  #echo "core=${core}"
  echo "function c${core}()
{
ps -eLF |awk '{ if(\$5==${core}) print \$0}'
}" >> ${SOURCED_SCRIPT}

done
echo "Done.

To run:
Step 1. source ./${SOURCED_SCRIPT}
Step 2. run the builtin functions 'c0', 'c1', 'c2', ...
to see which threads are running on which core."
exit 0
