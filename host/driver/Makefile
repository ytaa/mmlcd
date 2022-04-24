KDIR = /lib/modules/$(shell uname -r)
MOD_NAME = mmlcd
obj-m += $(MOD_NAME).o
mmlcd-objs := ./src/mmlcd.o ./src/mmlcd_dev.o
EXTRA_CFLAGS=-I$(PWD)/inc -DDEBUG
CONFIG_DYNAMIC_DEBUG=y

all: 
	make -C $(KDIR)/build M=$(PWD) modules
 
clean: 
	make -C $(KDIR)/build M=$(PWD) clean

install:
	cp $(MOD_NAME).ko $(KDIR)/kernel/drivers/misc
 ifeq (, $(shell grep $(MOD_NAME) /etc/modules))
 	$(shell echo $(MOD_NAME) >> /etc/modules)
 endif


.PHONY: all clean install