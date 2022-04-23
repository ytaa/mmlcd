CC = gcc
CC_FLAGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -O2 -pthread -fPIC 
LD_FLAGS = -shared
TARGET = libmmlcd.so

OUTDIR = out
BUILDDIR = build
SRCDIR = src
INCDIR = inc

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

DEPS = $(wildcard $(INCDIR)/*.h)

$(OUTDIR)/$(TARGET): $(OBJECTS)
	mkdir -p $(OUTDIR)
	$(CC) $(OBJECTS) -o $@ $(LD_FLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(BUILDDIR)
	$(CC) -c $(CC_FLAGS) -I $(INCDIR) $< -o $@

clean:
	rm -rf $(OUTDIR) $(BUILDDIR)

install: $(OUTDIR)/$(TARGET)
	cp $(OUTDIR)/$(TARGET) /usr/lib
	cp $(INCDIR)/libmmlcd.h /usr/include
	cp $(INCDIR)/libmmlcd_ipc.h /usr/include

uninstall:
	rm /usr/include/libmmlcd_ipc.h 
	rm /usr/include/libmmlcd.h 
	rm /usr/lib/$(TARGET) 