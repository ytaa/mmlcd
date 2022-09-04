LIB_DIR = lib
DAEMON_DIR = daemon
CLI_DIR = cli

DIRS = $(LIB_DIR) $(DAEMON_DIR) $(CLI_DIR)

all:
	$(foreach dir,$(DIRS),make -C $(dir); sudo -E make -C $(dir) install;)

install:
	$(foreach dir,$(DIRS),sudo -E make -C $(dir) install;)

clean:
	$(foreach dir,$(DIRS),make -C $(dir) clean;)