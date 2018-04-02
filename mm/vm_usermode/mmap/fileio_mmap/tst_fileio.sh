#!/bin/sh

[ $# -ne 1 ] && {
 echo "Usage: $0 io-size-in-MB"
 exit 1
}

SRC=~/srcfile
DEST=~/destfile

let countKB=$1*1024/4   # because we're writing in units of 4KB @ a time..

rm -f ${SRC}
rm -f ${DEST}
sync

dd if=/dev/urandom of=${SRC} bs=4k count=$countKB
#ls -lh ${SRC}

# regular file i/o (read/write syscalls) with 'cp' test
echo "Regular file io test:"
time cp ${SRC} ${DEST}

unalias ls 2> /dev/null
ls -l ${SRC} ${DEST}
sleep 1
diff ${SRC} ${DEST} || {
 echo "reg: diff failed!"
 exit 1
}

rm -f ${SRC}
rm -f ${DEST}
sync

# mmap-ed file i/o with 'mmap_fileio' test
dd if=/dev/urandom of=${SRC} bs=4k count=$countKB
sleep 1

echo "mmap-ed file io test:"
time ./mmap_fileio ${SRC} ${DEST}
unalias ls 2> /dev/null
ls -l ${SRC} ${DEST}
sleep 1
diff ${SRC} ${DEST} || {
 echo "reg: diff failed!"
 exit 1
}

