KDIR = /lib/modules/$(shell uname -r)

CC = gcc
CC_FLAGS = -Wall -Wextra -Werror -Wno-error=unused-function -O2 -pthread -I$(KDIR)/source/tools/include -I$(KDIR)/source/include/uapi/linux/usb
LD_FLAGS = -lpthread -lsystemd -lmmlcd

EXEC = ffs-mmlcd

EXECDIR = out
BUILDDIR = build
SRCDIR = src
INCDIR = inc

INCDIRS_FLAGS = -I$(INCDIR) -I$(KDIR)/source/tools/include

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

DEPS = $(wildcard $(INCDIR)/*.h)

$(EXECDIR)/$(EXEC): $(OBJECTS)
	mkdir -p $(EXECDIR)
	$(CC) -o $(EXECDIR)/$(EXEC) $(OBJECTS) $(LD_FLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(BUILDDIR)
	$(CC) -c $(CC_FLAGS) $(INCDIRS_FLAGS) $< -o $@

clean:
	rm -rf $(EXECDIR) $(BUILDDIR)

install:
	systemctl stop functionfs-mmlcd-daemon || true 
	cp -f $(EXECDIR)/$(EXEC) /root/bin