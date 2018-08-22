# Makefile for 2.6 kernel LKMs
# This Makefile idiom from the Rubini & Corbet (LDD3) book.

ifneq ($(KERNELRELEASE),)
	obj-m       := kvalloc.o

# turn on debug mode
EXTRA_CFLAGS += -DDEBUG

else

#########################################
# To support cross-compiling for the ARM:
# For ARM, invoke make as:
# make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- 
ifeq ($(ARCH),arm)
# Update 'KDIR' below to point to the ARM Linux kernel source tree
KDIR ?= ~/linux-3.1.5
else
KDIR ?= /lib/modules/$(shell uname -r)/build 
endif
#########################################

	PWD         := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
endif
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean

