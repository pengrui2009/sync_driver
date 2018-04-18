ifneq ($(KERNELRELEASE),)
	obj-m := fsync.o
else
	#generate the path
	CDIR := $(shell pwd)
	KDIR := /lib/modules/$(shell uname -r)/build
	#compile object
all:
	make -C $(KDIR) M=$(CDIR) modules
clean:
	make -C $(KDIR) M=$(CDIR) clean
endif
