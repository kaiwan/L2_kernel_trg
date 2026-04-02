#!/bin/bash
# initrd_xplore.sh
# 
# Quick Description:
# Extract a given initrd image.
# 
# Kaiwan N Billimoria
# kaiwan -at- kaiwantech -dot- com
# kaiwanTECH
# MIT License.
name=$(basename $0)

die() {
 echo 2>&1 "FATAL: ${name}: $@" ; exit 1
}
DEBUG=1
decho() {
 [[ ${DEBUG} -eq 1 ]] && echo "$@"
}

# unpack_initramfs
# Parameters:
#  $1 : initramfs source file (.cpio.gz)
unpack_initramfs()
{
local TMPDIR=initrd_$$
local DEST=initrd_copy.img
local DEST_DIR=initrd_content
local TYPE

mkdir -p ${TMPDIR} || exit 1
cp $1 ${TMPDIR}/${DEST} || exit 1
cd ${TMPDIR}

unmkinitramfs ${DEST} .
return
}

##### execution starts here #####

# args
[ $# -ne 1 ] && {
  echo "Usage: ${name} initrd-img-file"
  exit 1
}
[ ! -f $1 ] && {
  echo "${name}: initrd-img-file $1 not found? Perms issue? Aborting..."
  exit 1
}
unpack_initramfs ${1}
exit 0
