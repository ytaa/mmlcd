CC = gcc
CC_FLAGS = -Wall -Wextra -Werror -O2 -pthread
LD_FLAGS = 

EXEC = mmlcdd

EXECDIR = out
BUILDDIR = build
SRCDIR = src
INCDIR = inc

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

DEPS = $(wildcard $(INCDIR)/*.h)

ifndef TARGET_PLATFORM
 $(error In order to build this program you need to provide TARGET_PLATFORM variable)
endif

# Platform specific actions
ifeq ($(TARGET_PLATFORM), x86)
 $(info --- Building for x86 host ---)
else ifeq ($(TARGET_PLATFORM), rpi)
 $(info --- Building for Raspberry Pi gadget ---)
 LD_FLAGS += -lrt -lpigpio
 CC_FLAGS += -I$(INCDIR)/$(TARGET_PLATFORM)
 DEPS +=$(wildcard $(INCDIR)/$(TARGET_PLATFORM)/*.h)
else
 $(error Provided value for TARGET_PLATFORM '$(TARGET_PLATFORM)' variable is invalid)
endif

# TARGET_DIR validated - add related source files
HALSRCDIR = $(SRCDIR)/hal/$(TARGET_PLATFORM)
HALBUILDIR = $(BUILDDIR)/hal/$(TARGET_PLATFORM)
HALSOURCES += $(wildcard $(HALSRCDIR)/*.c)
HALOBJECTS += $(HALSOURCES:$(HALSRCDIR)/%.c=$(HALBUILDIR)/%.o)
$(info $(HALSOURCES))
$(info $(HALOBJECTS))
#$(error x)

$(EXECDIR)/$(EXEC): $(OBJECTS) $(HALOBJECTS)
	mkdir -p $(EXECDIR)
	$(CC) -o $(EXECDIR)/$(EXEC) $(OBJECTS) $(HALOBJECTS) $(LD_FLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(BUILDDIR)
	$(CC) -c $(CC_FLAGS) -I $(INCDIR) $< -o $@

$(HALBUILDIR)/%.o: $(HALSRCDIR)/%.c $(DEPS)
	mkdir -p $(HALBUILDIR)
	$(CC) -c $(CC_FLAGS) -I $(INCDIR) $< -o $@

clean:
	rm -rf $(EXECDIR) $(BUILDDIR) $(HALBUILDIR)

install: $(EXECDIR)/$(EXEC)
	cp $(EXECDIR)/$(EXEC) /usr/bin
	cp service/mmlcd.service /lib/systemd/system
	systemctl enable mmlcd

uninstall:
	systemctl disable mmlcd
	rm /lib/systemd/system/mmlcd.service
	rm /usr/bin/$(EXEC)
#ln -fs /lib/systemd/system/lcd.service /etc/systemd/system