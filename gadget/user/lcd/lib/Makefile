CC = gcc
CC_FLAGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -O2 -pthread -fPIC 
LD_FLAGS = -shared -lpigpio -lrt

TARGET = liblcd.so

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
	cp $(INCDIR)/liblcd.h /usr/include
	cp $(INCDIR)/liblcd_ipc.h /usr/include