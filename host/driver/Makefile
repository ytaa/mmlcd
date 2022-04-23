KDIR = /lib/modules/$(shell uname -r)
obj-m += mmlcd.o
mmlcd-objs := ./src/mmlcd.o ./src/mmlcd_dev.o
EXTRA_CFLAGS=-I$(PWD)/inc -DDEBUG
CONFIG_DYNAMIC_DEBUG=y

all: 
	make -C $(KDIR)/build M=$(PWD) modules
 
clean: 
	make -C $(KDIR)/build M=$(PWD) clean

.PHONY: all clean