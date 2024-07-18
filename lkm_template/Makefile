# Makefile
# ***************************************************************
# Brief Description:
# A 'better' Makefile template for Linux LKMs (Loadable Kernel Modules); besides
# the 'usual' targets (the build, install and clean), we incorporate targets to
# do useful (and indeed required) stuff like:
#  - adhering to kernel coding style (indent+checkpatch)
#  - several static analysis targets (via sparse, gcc, flawfinder, cppcheck)
#  - two _dummy_ dynamic analysis targets (KASAN, LOCKDEP); just to remind you!
#  - a packaging (.tar.xz) target and
#  - a help target.
#
# To get started, just type:
#  make help
#
# For details on this so-called 'better' Makefile, please refer my earlier book
# 'Linux Kernel Programming', Packt, Mar 2021, Ch 5 section 'A "better" Makefile
# template for your kernel modules'.

#------------------------------------------------------------------
# Set FNAME_C to the kernel module name source filename (without .c)
# This enables you to use this Makefile as a template; just update this variable!
# As well, the DEBUG_MODE variable (see it below) can be set to 'y' or 'n' (no being
# the default)
FNAME_C := lkm_template
ifeq ($(FNAME_C),)
  $(error You MUST pass the C file like this:\
make FNAME_C=csrc-filename-without-.c target-name)
else
  $(info [FYI: FNAME_C = $(FNAME_C))
endif
#------------------------------------------------------------------

# To support cross-compiling for kernel modules:
# For architecture (cpu) 'arch', invoke make as:
#  make ARCH=<arch> CROSS_COMPILE=<cross-compiler-prefix>
# The KDIR var is set to a sample path below; you're expected to update it on
# your box to the appropriate path to the kernel src tree for that arch.
ifeq ($(ARCH),arm)
  # *UPDATE* 'KDIR' below to point to the ARM Linux kernel source tree on your box
  export KDIR ?= /big/kernels/linux-4.19.94
  #KDIR ?= ~/rpi_work/kernel_rpi/linux
else ifeq ($(ARCH),arm64)
  # *UPDATE* 'KDIR' below to point to the ARM64 (Aarch64) Linux kernel source
  # tree on your box
  export KDIR ?= ~/kernel/linux-5.4
else ifeq ($(ARCH),powerpc)
  # *UPDATE* 'KDIR' below to point to the PPC64 Linux kernel source tree on your box
  export KDIR ?= ~/kernel/linux-5.0
else
  # 'KDIR' is the Linux 'kernel headers' package on your host system; this is
  # usually an x86_64, but could be anything, really (f.e. building directly
  # on a Raspberry Pi implies that it's the host)
  export KDIR ?= /lib/modules/$(shell uname -r)/build
endif

# Compiler
CC     := $(CROSS_COMPILE)gcc
#CC     := clang

PWD            := $(shell pwd)
obj-m          += ${FNAME_C}.o

#--- Debug or production mode?
# Set the DEBUG_MODE variable accordingly to y/n resp.
# NOTE:
# It's important to understand that, actually, debug info is *always* going to
# be generated when you build a module on a debug kernel, a kernel where
# CONFIG_DEBUG_INFO=y . This makes the setting of the ccflags-y (or the older
# EXTRA_CFLAGS) variable mostly redundant (besides the still useful -DDEBUG).
# Also, realize that the macro named 'DEBUG' is turned on by many CONFIG_*DEBUG*
# options; hence, we use a different macro name, viz, DEBUG_MODE.
DEBUG_MODE := y
DBG_STRIP := y

ifeq (${DEBUG_MODE}, y)
 $(info 'DEBUG' build)
else
 $(info 'Production' build)
endif

ifeq (${DEBUG_MODE}, y)
# https://www.kernel.org/doc/html/latest/kbuild/makefiles.html#compilation-flags
# EXTRA_CFLAGS deprecated; use ccflags-y
# As such, we're told that we can't use anything but -O2 for kernel code;
# -Og though, works & can help for debugging code so use it when 'debug' is on!
  ccflags-y   += -DDEBUG -Og -g -ggdb -gdwarf-4 -Wall -fno-omit-frame-pointer -fvar-tracking-assignments
  DBG_STRIP := n
  $(info KDIR = $(KDIR)])
else
  INSTALL_MOD_STRIP := 1
  ccflags-y   += -UDEBUG -Wall
  $(info KDIR = $(KDIR))
endif
# We always keep the dynamic debug facility enabled; this allows us to turn
# dynamically turn on/off debug printk's later... To disable it simply comment
# out the following line
ccflags-y   += -DDYNAMIC_DEBUG_MODULE
KMODDIR ?= /lib/modules/$(shell uname -r)
STRIP := ${CROSS_COMPILE}strip

# Gain access to kernel configs:
# Try to copy the kernel config from a few locations..
$(shell sudo cp /boot/config-$(shell uname -r) $(KDIR)/.config 2>/dev/null)
#$(shell sudo zcat /proc/config.gz > $(KDIR)/.config 2>/dev/null)
include $(KDIR)/.config

# Strip the module? Note:
# a) Only strip debug symbols else it won't load correctly
# b) WARNING! Don't strip modules when using CONFIG_MODULE_SIG* crytographic security
ifdef CONFIG_MODULE_SIG
  DBG_STRIP := n
endif
ifdef CONFIG_MODULE_SIG_ALL
  DBG_STRIP := n
endif
ifdef CONFIG_MODULE_SIG_FORCE
  DBG_STRIP := n
endif

# gcc-10 issue:
#ccflags-y  += $(call cc-option,--allow-store-data-races)

.PHONY: all clean help install tarxz-pkg checkpatch code-style indent \
sa sa_cppcheck sa_gcc sa_flawfinder sa_sparse

# Set VERBOSE to 1 to run make in verbose mode (make V=1 ...)
VERBOSE = 0

all:
	@echo
	@echo '--- Building : KDIR=${KDIR} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} ccflags-y=${ccflags-y} ---'
	@${CC} --version|head -n1
	@echo
	make V=$(VERBOSE) -C $(KDIR) M=$(PWD) modules
ifeq (${DBG_STRIP}, y)
	$(info Stripping module (as it's a debug build -AND- CONFIG_MODULE_SIG* is Off)])
	${STRIP} ./${FNAME_C}.ko
else
	$(info NOT stripping module (as it's either a debug build -OR- CONFIG_MODULE_SIG* is On)])
endif
install:
	@echo
	@echo "--- installing ---"
	@echo " [First, invoking the 'make' ]"
	# Build - call 'make' (it also takes care of stripping the module when required)
	make V=$(VERBOSE)
	@echo
	@echo " [Now for the 'sudo make install' ]"
	sudo make V=$(VERBOSE) -C $(KDIR) M=$(PWD) modules_install
	@echo " sudo depmod"
	sudo depmod
clean:
	@echo
	@echo "--- cleaning ---"
	@echo
	make V=$(VERBOSE) -C $(KDIR) M=$(PWD) clean
# from 'indent'
	rm -f *~

# Any usermode programs to build? Insert the build target(s) here

#--------------- More (useful) targets! -------------------------------
INDENT := indent

# code-style : "wrapper" target over the following kernel code style targets
code-style:
	make V=$(VERBOSE) indent
	make V=$(VERBOSE) checkpatch

# indent- "beautifies" C code - to conform to the the Linux kernel
# coding style guidelines.
# Note! original source file(s) is overwritten, so we back it up.
indent:
ifeq (,$(shell which indent))
	$(error ERROR: install indent first)
endif
	@echo
	@echo "--- applying kernel code style indentation with indent ---"
	@echo
	mkdir bkp 2> /dev/null; cp -f *.[chsS] bkp/
	${INDENT} -linux --line-length95 *.[chsS]
	  # add source files as required

# Detailed check on the source code styling / etc
checkpatch:
	make V=$(VERBOSE) clean
	@echo
	@echo "--- kernel code style check with checkpatch.pl ---"
	@echo
	$(KDIR)/scripts/checkpatch.pl --no-tree -f --max-line-length=95 *.[ch]
	  # add source files as required

#--- Static Analysis
# sa : "wrapper" target over the following kernel static analyzer targets
sa:
	make V=$(VERBOSE) sa_sparse
	make V=$(VERBOSE) sa_gcc
	make V=$(VERBOSE) sa_flawfinder
	make V=$(VERBOSE) sa_cppcheck

# static analysis with sparse
sa_sparse:
ifeq (,$(shell which sparse))
	$(error ERROR: install sparse first)
endif

	make V=$(VERBOSE) clean
	@echo
	@echo "--- static analysis with sparse ---"
	@echo
# if you feel it's too much, use C=1 instead
# NOTE: deliberately IGNORING warnings from kernel headers!
	make V=$(VERBOSE) -Wsparse-all C=2 CHECK="/usr/bin/sparse --os=linux --arch=$(ARCH)" -C $(KDIR) M=$(PWD) modules 2>&1 |egrep -v "^\./include/.*\.h|^\./arch/.*\.h"

# static analysis with gcc
sa_gcc:
	make V=$(VERBOSE) clean
	@echo
	@echo "--- static analysis with gcc ---"
	@echo
	make V=$(VERBOSE) W=1 -C $(KDIR) M=$(PWD) modules

# static analysis with flawfinder
sa_flawfinder:
ifeq (,$(shell which flawfinder))
	$(error ERROR: install flawfinder first)
endif
	make V=$(VERBOSE) clean
	@echo
	@echo "--- static analysis with flawfinder ---"
	@echo
	flawfinder *.[ch]

# static analysis with cppcheck
sa_cppcheck:
ifeq (,$(shell which cppcheck))
	$(error ERROR: install cppcheck first)
endif
	make V=$(VERBOSE) clean
	@echo
	@echo "--- static analysis with cppcheck ---"
	@echo
	cppcheck -v --force --enable=all -i .tmp_versions/ -i *.mod.c -i bkp/ --suppress=missingIncludeSystem .

# Packaging; just tar.xz as of now
PKG_NAME := ${FNAME_C}
tarxz-pkg:
	rm -f ../${PKG_NAME}.tar.xz 2>/dev/null
	make V=$(VERBOSE) clean
	@echo
	@echo "--- packaging ---"
	@echo
	tar caf ../${PKG_NAME}.tar.xz *
	ls -l ../${PKG_NAME}.tar.xz
	@echo '=== package created: ../$(PKG_NAME).tar.xz ==='
	@echo 'Tip: when extracting, to extract into a dir of the same name as the tar file,'
	@echo ' do: tar -xvf ${PKG_NAME}.tar.xz --one-top-level'

help:
	@echo '=== Makefile Help : additional targets available ==='
	@echo
	@echo 'TIP: type make <tab><tab> to show all valid targets'
	@echo '(this can result in your being asked for your password as theres'
	@echo 'a sudo <cmd> embedded within)'
	@echo

	@echo '--- 'usual' kernel LKM targets ---'
	@echo 'typing "make" or "all" target : builds the kernel module object (the .ko)'
	@echo 'install     : installs the kernel module(s) to INSTALL_MOD_PATH (default here: /lib/modules/$(shell uname -r)/)'
	@echo 'clean       : cleanup - remove all kernel objects, temp files/dirs, etc'

	@echo
	@echo '--- kernel code style targets ---'
	@echo 'code-style : "wrapper" target over the following kernel code style targets'
	@echo ' indent     : run the $(INDENT) utility on source file(s) to indent them as per the kernel code style'
	@echo ' checkpatch : run the kernel code style checker tool on source file(s)'

	@echo
	@echo '--- kernel static analyzer targets ---'
	@echo 'sa         : "wrapper" target over the following kernel static analyzer targets'
	@echo ' sa_sparse     : run the static analysis sparse tool on the source file(s)'
	@echo ' sa_gcc        : run gcc with option -W1 ("Generally useful warnings") on the source file(s)'
	@echo ' sa_flawfinder : run the static analysis flawfinder tool on the source file(s)'
	@echo ' sa_cppcheck   : run the static analysis cppcheck tool on the source file(s)'
	@echo 'TIP: use coccinelle as well (requires spatch): https://www.kernel.org/doc/html/v4.15/dev-tools/coccinelle.html'

	@echo
	@echo '--- kernel dynamic analysis targets ---'
	@echo 'da_kasan   : DUMMY target: this is to remind you to run your code with the dynamic analysis KASAN tool enabled; requires configuring the kernel with CONFIG_KASAN On, rebuild and boot it'
	@echo 'da_lockdep : DUMMY target: this is to remind you to run your code with the dynamic analysis LOCKDEP tool (for deep locking issues analysis) enabled; requires configuring the kernel with CONFIG_PROVE_LOCKING On, rebuild and boot it'
	@echo 'TIP: best to build a debug kernel with several kernel debug config options turned On, boot via it and run all your test cases'

	@echo
	@echo '--- misc targets ---'
	@echo 'tarxz-pkg  : tar and compress the LKM source files as a tar.xz into the dir above; allows one to transfer and build the module on another system'
	@echo ' Tip: when extracting, to extract into a dir of the same name as the tar file,'
	@echo '  do: tar -xvf ${PKG_NAME}.tar.xz --one-top-level'
	@echo 'help       : this help target'
