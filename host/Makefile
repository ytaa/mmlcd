KDIR = /lib/modules/$(shell uname -r)
MOD_NAME = mmlcd
obj-m += $(MOD_NAME).o
mmlcd-objs := ./src/mmlcd.o ./src/mmlcd_dev.o ./src/mmlcd_proc.o
EXTRA_CFLAGS=-I$(PWD)/inc -DDEBUG
CONFIG_DYNAMIC_DEBUG=y

all: 
	make -C $(KDIR)/build M=$(PWD) modules
 
clean: 
	make -C $(KDIR)/build M=$(PWD) clean

install:
# copy module to kernel directories
	cp $(MOD_NAME).ko $(KDIR)/kernel/drivers/misc
	cp $(MOD_NAME).ko $(KDIR)/kernel
# install module
	rmmod $(MOD_NAME).ko | true
	insmod $(MOD_NAME).ko
# refresh module dependencies
	depmod -a


.PHONY: all clean install