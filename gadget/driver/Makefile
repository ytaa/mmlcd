CC = gcc
CC_FLAGS = -Wall -Wextra -Werror -O2
LD_FLAGS = -llcd

ROOT_DIR = /root
BIN_DIR = /usr/bin

DAEMON_DIR = daemon
DESC_DIR = descriptors
SYSTEMD_DIR = systemd
SCRIPTS_DIR = script
CONFIG_DIR = config

DESC_SOURCES = $(wildcard $(DESC_DIR)/*.txt)
DESC_BINS = $(DESC_SOURCES:$(DESC_DIR)/%.txt=$(DESC_DIR)/%.bin)

all: $(DESC_BINS) build_daemon

$(DESC_DIR)/%.bin: $(DESC_DIR)/%.txt
	cat $<| xxd -r -p - > $@

build_daemon:
	make -C $(DAEMON_DIR)

install_daemon:
	make -C $(DAEMON_DIR) install

install_desriptors:
	cp $(DESC_DIR)/*.bin $(ROOT_DIR)

install_scripts:
	ln -sf $(shell pwd)/$(SCRIPTS_DIR)/mmlcd_ffs_config.sh $(BIN_DIR)/mmlcd_ffs_config
	ln -sf $(shell pwd)/$(SCRIPTS_DIR)/mmlcd_ffs_enable.sh $(BIN_DIR)/mmlcd_ffs_enable

install_systemd:
	cp $(SYSTEMD_DIR)/ffs.socket /etc/systemd/system/
	cp $(SYSTEMD_DIR)/functionfs-mmlcd-daemon.service /etc/systemd/system/
	cp $(SYSTEMD_DIR)/run-ffs_mmlcdx.mount /etc/systemd/system/
	cp $(SYSTEMD_DIR)/run-ffs_mmlcd.mount /etc/systemd/system/
	cp $(SYSTEMD_DIR)/usb-gadget-ffs.service /etc/systemd/system/
	cp $(SYSTEMD_DIR)/udc.rules /etc/udev/rules.d/
	cp $(SYSTEMD_DIR)/usb-gadget.target /etc/systemd/system/

install_config:
	cp $(CONFIG_DIR)/mmlcd.scheme /etc/mmlcd/


install: install_desriptors install_scripts install_systemd install_config install_daemon

clean:
	rm -f $(DESC_DIR)/*.bin