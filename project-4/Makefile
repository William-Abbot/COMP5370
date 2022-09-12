SHELL := /bin/bash
CC := gcc
CFLAGS := -m32 -static -U_FORTIFY_SOURCE -DCOOKIE=$(shell cat cookie) -Wno-implicit-function-declaration
LDFLAGS :=
LDLIBS :=

targets := target6 target7

.PHONY: default checksudo clean install uninstall

default: checkcookie checksudo $(targets) permissions

checkcookie:
	@if ! test -f "cookie"; then \
		echo -e "Cookie not set. Please run './setcookie <uniqname>'.\n"; \
		exit 1; \
	fi

checksudo:
	@if [ $(shell id -u) -ne 0 ]; then \
		echo -e "You are not root. Please run 'sudo make'.\n"; \
		exit 1; \
	fi

target6: CFLAGS += -fstack-protector-all -z execstack -DSOLUTION="\"sol6.py\""
target7: CFLAGS += -fstack-protector-all -z execstack -DSOLUTION="\"sol7.py\""

target6: target6.c helper.c
target7: target7.c helper.c

clean:
	rm -f $(targets) cookie

permissions:
	sudo chown root:$(SUDO_USER) target{6,7}
	sudo chmod 6777 target{6,7}
	@for t in target[2-6]; do \
		if [ `stat -c '%a' $$t` -ne 6777 ]; then \
			echo -e "Setuid permission could not be set. Make sure your files are in a native Linux folder and not a VirtualBox shared folder.\n"; \
			rm -f $(targets); \
			exit 1; \
		fi; \
	done
