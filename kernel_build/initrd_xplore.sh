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
source ./common.sh || {
 echo "$name: could not source common.sh , aborting..."
 exit 1
}

########### Functions follow #######################


main()
{
TMPDIR=tmp.$$
DEST=initrd_copy.img
DEST_DIR=initrd_content

mkdir -p ${TMPDIR} || exit 1
cp $1 ${TMPDIR}/${DEST} || exit 1
cd ${TMPDIR}
mv ${DEST} ${DEST}.gz
gzip -d ${DEST}.gz
mkdir -p ${DEST_DIR}
cd ${DEST_DIR}
cpio -i < ../${DEST}
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
INITRD_SRC=$1
main ${INITRD_SRC}
exit 0
