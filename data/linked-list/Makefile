ifeq ($(KERNELVERSION),)
	PWD := $(shell pwd)
	KERNELDIR := /usr/lib/modules/$(shell uname -r)/build/

default: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

else
	obj-m += linked-list.o
endif
