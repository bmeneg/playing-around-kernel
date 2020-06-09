ifeq ($(KERNELVERSION),)
	PWD := $(shell pwd)
	KERNELDIR := ~/git/linux/

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
else
	ccflags-y := -g3 -O0
	obj-m += sync.o
	obj-m += async.o
endif

PHONY: clean
clean:
	rm *.ko *.o *.mod .*.mod.* modules.* Module*

