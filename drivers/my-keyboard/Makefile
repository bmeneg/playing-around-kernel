ifeq ($(KERNELVERSION),)
	KERNELDIR ?= /usr/lib/modules/$(shell uname -r)/build/
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
else
	obj-m := my-kbd.o
endif
