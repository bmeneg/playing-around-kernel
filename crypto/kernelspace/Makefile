ifeq ($(KERNELVERSION),)
	PWD := $(shell pwd)
	KERNELDIR := /usr/lib/modules/$(shell uname -r)/build/

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
else
	obj-m += sync.o
	obj-m += async.o
endif

PHONY: clean
clean:
	rm *.ko *.o *.mod .*.mod.* modules.* Module*

