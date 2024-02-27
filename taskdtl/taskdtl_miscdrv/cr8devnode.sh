#!/bin/bash
# cr8devnode.sh
# Simple utility script to create the device node for the miscdrv_rdwr 'misc'
# class device driver
name=$(basename $0)
OURMODNAME="taskdtl_miscdrv"
DEVNODE=/dev/taskdtl

MAJOR=10   # misc class is always major # 10
unalias dmesg 2>/dev/null
MINOR=$(dmesg |grep "${OURMODNAME}\:minor\=" |cut -d"=" -f2)
[ -z "${MINOR}" ] && {
  echo "${name}: failed to retrieve the minor #, aborting ..."
  exit 1
}
echo "minor number is ${MINOR}"

sudo rm -f ${DEVNODE}     # rm any stale instance
sudo mknod ${DEVNODE} c ${MAJOR} ${MINOR}
sudo chmod 0666 ${DEVNODE}
ls -l ${DEVNODE}
exit 0
