# Simplest LKM Makefile
MODNAME = hello

#CC := gcc-12
obj-m	:= $(MODNAME).o

KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)
#ccflags-y += -DDEBUG

all: module

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
install:
	sudo $(MAKE) -C $(KDIR) M=$(PWD) modules_install
	sudo depmod
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
