#!/bin/sh
name=$(basename $0)
KMOD=taskdtlv2

display_all()
{
echo "${name}: WARNING! displaying ALL threads task structure details can
result in the kernel log buffer getting overwritten (and it will take a while)"

i=1
IFS=$'\n'
for rec in $(ps -LA)
do
  [ $i -eq 1 ] && {  # skip ps's header
    let i=i+1
    continue
  }

  # 'grok' & extract it, and ...
  #pid=$(echo "${rec}" |awk '{print $1}')
  tid=$(echo "${rec}" |awk '{print $2}')
  echo "${i} :: TID = ${tid}"

  sudo bash -c "echo ${tid} > /dev/taskdtl"
  let i=i+1
done
dmesg
}

### "main" here
[ $# -ne 1 ] && {
 echo "Usage: ${name} {PID} | all
 If the parameter is the PID of a process (or thread), that entity's task structure
  details will be displayed
 If the parameter is 'all', this will display the task structure details of all
  threads currently alive"
 exit 1
}
[ ! -w /dev/taskdtl ] && {
 echo "${name}: device node /dev/taskdtl does not exist or not writeable? aborting..."
 exit 1
}
lsmod | grep -q ${KMOD} || {
 echo "${name}: kernel module \"${KMOD}\" not installed? inserting..."
 sudo rmmod ${KMOD}
 sudo dmesg -C
 sudo insmod ${KMOD}.ko
}

[ "$1" = "all" ] && {
 display_all
 exit 0
}
sudo bash -c "echo $1 > /dev/taskdtl"
dmesg
