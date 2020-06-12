ifeq ($(KERNELRELEASE),)  

  # To support cross-compiling for the ARM:
  # For ARM, invoke make as:
  # make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- 
  ifeq ($(ARCH),arm)
    # *UPDATE* 'KDIR' below to point to the ARM Linux kernel source tree on your box
    KDIR ?= ~/3.14.34
  else
    KDIR ?= /lib/modules/$(shell uname -r)/build 
  endif

PWD := $(shell pwd)  

.PHONY: build clean  

build:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

else

  $(info Building for ARCH ${ARCH} and KERNELRELEASE ${KERNELRELEASE}) 
  EXTRA_CFLAGS += -DDEBUG #-DUSE_FTRACE_PRINT
  obj-m :=    kmalloc_test.o

endif
