This source directory serves as a 'template' for Linux LKMs (Loadable Kernel
Modules). Please do go through the files in it carefully and follow the
conventions and style; particularly, see and follow the various Makefile
targets.

By 'follow', what we really mean is, use them! They help you perform key sanity
checks on your code, via code-style checking, static and dynamic analysis tools.

The output of the `make help` below shows the targets:

`
$ make help
=== Makefile Help : additional targets available ===

TIP: Type make <tab><tab> to show all valid targets
FYI: KDIR=/lib/modules/6.5.6-200.fc38.x86_64/build ARCH= CROSS_COMPILE= ccflags-y="-UDEBUG -DDYNAMIC_DEBUG_MODULE" MYDEBUG=n DBG_STRIP=n

--- usual kernel LKM targets ---
typing "make" or "all" target : builds the kernel module object (the .ko)
install     : installs the kernel module(s) to INSTALL_MOD_PATH (default here: /lib/modules/6.5.6-200.fc38.x86_64/).
            : Takes care of performing debug-only symbols stripping iff MYDEBUG=n and not using module signature
nsdeps      : namespace dependencies resolution; for possibly importing namespaces
clean       : cleanup - remove all kernel objects, temp files/dirs, etc

--- kernel code style targets ---
code-style : "wrapper" target over the following kernel code style targets
 indent     : run the indent utility on source file(s) to indent them as per the kernel code style
 checkpatch : run the kernel code style checker tool on source file(s)

--- kernel static analyzer targets ---
sa         : "wrapper" target over the following kernel static analyzer targets
 sa_sparse     : run the static analysis sparse tool on the source file(s)
 sa_gcc        : run gcc with option -W1 ("Generally useful warnings") on the source file(s)
 sa_flawfinder : run the static analysis flawfinder tool on the source file(s)
 sa_cppcheck   : run the static analysis cppcheck tool on the source file(s)
TIP: use Coccinelle as well: https://www.kernel.org/doc/html/v6.1/dev-tools/coccinelle.html

--- kernel dynamic analysis targets ---
da_kasan   : DUMMY target: this is to remind you to run your code with the dynamic analysis KASAN tool enabled; requires configuring the kernel with CONFIG_KASAN On, rebuild and boot it
da_lockdep : DUMMY target: this is to remind you to run your code with the dynamic analysis LOCKDEP tool (for deep locking issues analysis) enabled; requires configuring the kernel with CONFIG_PROVE_LOCKING On, rebuild and boot it
TIP: Best to build a debug kernel with several kernel debug config options turned On, boot via it and run all your test cases

--- misc targets ---
tarxz-pkg  : tar and compress the LKM source files as a tar.xz into the dir above; allows one to transfer and build the module on another system
        TIP: When extracting, to extract into a directory with the same name as the tar file, do this:
              tar -xvf lkm_template.tar.xz --one-top-level
help       : this help target
$` 

-----------------------------------------------------------------------
**TIP**
On Ubuntu 22.04 or earlier, it's possible that the module build fails with this error:

`gcc-11: error: unrecognized command-line option ‘-ftrivial-auto-var-init=zero’`

It's a limitation within GCC ver 11; to fix it, install gcc-12, set CC to it in the Makefile and retry:

`sudo apt install gcc-12`

In the Makefile, add this line:

`CC := gcc-12 `

and then rebuild...


A few generic important rules / a quick Checklist for the Programmer
----------------------------------------------------------------------------------
DID YOU REMEMBER TO

✔ Rule #1 : Check all APIs for their failure case

✔ Rule #2 : Compile with warnings on (-Wall -Wextra) and eliminate all
   warnings as far as is possible

✔ Rule #3 : Never trust [user] input; validate it

✔ Rule #4 : Use assertions in your code

✔ Rule #5 : Eliminate unused (or dead) code from the codebase immediately

✔ Rule #6 : Test thoroughly; ideally, 100% code coverage is the objective.
   Take the time and trouble to learn to use these powerful kernel-space tools:
   memory checkers (KASAN, kernel memory leak detector, KCSAN, UBSAN, etc),
   static and dynamic analyzers, security checkers (checksec), fuzzers, etc

✔ Rule #7 : Do NOT Assume Anything
   (“ASSUME : makes an ASS out of U and ME” :-) )

Also see:
LOW-LEVEL SOFTWARE DESIGN: https://kaiwantech.wordpress.com/2017/01/03/low-level-software-design/


----------------------------------------------------------------------------------
FYI, the checkpatch.pl Perl script used here is part of the Linux kernel source;
you can always download it from here:

https://github.com/torvalds/linux/blob/master/scripts/checkpatch.pl
... or access it on your local system via

` ${KDIR}/scripts/checkpatch.pl`

; where `KDIR` is the location of your local kernel source tree (could even
point to the kernel headers tree).
