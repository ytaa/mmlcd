CC = gcc
CC_FLAGS = -Wall -Wextra -Werror -O2 -pthread
LD_FLAGS = -lpigpio -lrt -lpigpio

EXEC = lcdd

EXECDIR = out
BUILDDIR = build
SRCDIR = src
INCDIR = inc

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

DEPS = $(wildcard $(INCDIR)/*.h)

$(EXECDIR)/$(EXEC): $(OBJECTS)
	mkdir -p $(EXECDIR)
	$(CC) -o $(EXECDIR)/$(EXEC) $(OBJECTS) $(LD_FLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(BUILDDIR)
	$(CC) -c $(CC_FLAGS) -I $(INCDIR) $< -o $@

clean:
	rm -rf $(EXECDIR) $(BUILDDIR)

install: $(EXECDIR)/$(EXEC)
	cp $(EXECDIR)/$(EXEC) /usr/bin
	cp service/lcd.service /lib/systemd/system
	ln -fs /lib/systemd/system/lcd.service /etc/systemd/system