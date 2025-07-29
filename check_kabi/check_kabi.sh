#!/bin/bash
# kabi = kernel ABI ; the distro specifies stable KABI's in the speicfied kabi
# file; they typically guarantee these K ABI's (APIs/structs/etc) remain stable
# and usable.
# It's the "official list of kernel symbols with guaranteed stable ABI for the
# <whatever> (eg. CentOS/RHEL 8) kernel series. It’s used for ABI compliance
# checks so external kernel modules can remain compatible across minor kernel updates."
name=$(basename $0)
die()
{
echo >&2 "FATAL: $*" ; exit 1
}
warn()
{
echo >&2 "WARNING: $*"
}
usage() {
 echo "Usage: ${name} your-module.ko stable-kernel-abi-file"
}


#--- 'main' 
[[ $# -ne 2 ]] && {
  usage ; exit 1
}
VEROBOSE=0
MYMOD="$1"
KABI_REF="$2"

[[ ! -f ${MYMOD} ]] && die "Couldn't locate your specified module \"${MYMOD}"\"
[[ ! -f ${KABI_REF} ]] && die "Couldn't locate the specified stable-kernel-abi file \"${KABI_REF}"\"

# what if the kmod's compressed (zstd); uncompress it
ext="${MYMOD##*.}"
#echo "ext:$ext"
[[ "${ext}" = "zst" ]] && {
  rm -f /tmp/mymod.ko
  zstd -d ${MYMOD} -o /tmp/mymod.ko >/dev/null || die "zstd uncompression on module failed"
  MYMOD=/tmp/mymod.ko
}

for sym in $(nm -u ${MYMOD} | awk '{print $2}'); do
    #if grep -w "$sym" /usr/src/kernels/$(uname -r)/Module.kabi_x86_64 >/dev/null; then
    if grep -w "${sym}" ${KABI_REF} >/dev/null; then
        [[ ${VERBOSE} -eq 1 ]] && echo "[OK] ${sym} is in stable KABI"
    else
        echo "[WARN] ${sym} is NOT in stable KABI"
    fi
done

