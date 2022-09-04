CC = gcc
CC_FLAGS = -Wall -Wextra -Werror -O2
LD_FLAGS = -lmmlcd

EXEC = mmlcd_btn_handler

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
	cp $(EXECDIR)/$(EXEC) /usr/local/bin

uninstall:
	rm /usr/local/bin/$(EXEC)