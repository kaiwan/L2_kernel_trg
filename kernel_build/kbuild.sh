#!/bin/bash
# kbuild.sh
# Simple kernel build script
# Lightly tested! YMMV
name=$(basename $0)
[ $# -ne 1 ] && {
    echo "Usage: ${name} path-to-kernel-source-tree-to-build"
    exit 1
}
KSRC=$1
[ ! -d ${KSRC} ] && {
    echo "${name}: dir ${KSRC} invalid?"
    exit 1
}

runcmd()
{
echo "[+] $@"
eval "$@"
[ $? -ne 0 ] && {
   echo " Command \"$@\" failed, aborting ..."
   exit 1
  }
}
 

### "main" here
cd ${KSRC} || exit 1
 
# Start with the 'tiniest' config...
#runcmd "make tinyconfig"

runcmd "make defconfig"
# ... and then add on the kmods we currently use
lsmod > /tmp/lsmod
runcmd "make LSMOD=/tmp/lsmod localmodconfig"
rm -f /tmp/lsmod
 
runcmd "ls -l .config"
# make oldconfig: "Update current config utilising a provided .config as base"
runcmd "make oldconfig"
 
CPU_CORES=$(nproc)
[ ${CPU_CORES} -lt 512 ] && JOBS=$((2*${CPU_CORES})) || {
	jobs=$(bc <<< "scale=0; (1.5*${CPU_CORES})")
	JOBS=${jobs::-2}   # rm the trailing '.0'
}
echo "jobs=${jobs}"
#exit 0

runcmd "time make -j${JOBS}"
runcmd "sudo make -j${JOBS} modules_install"
runcmd "sudo make install"
 
echo "[+] Done."
