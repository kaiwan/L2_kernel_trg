ifeq ($(KERNELRELEASE),)  

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

PWD := $(shell pwd)  

.PHONY: build clean  

build:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

else

  EXTRA_CFLAGS += -DDEBUG #-DUSE_FTRACE_PRINT
  $(info @@@ Building for ARCH=${ARCH}, CROSS_COMPILE=${CROSS_COMPILE}, KERNELRELEASE=${KERNELRELEASE}; EXTRA_CFLAGS=${EXTRA_CFLAGS} @@@)
  obj-m := slab_custom.o  

endif

