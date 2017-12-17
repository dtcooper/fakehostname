CC:=gcc
CFLAGS:=-Wall -Wextra -std=gnu99 -O2
LDLIBS:=-ldl
DESTDIR:=/usr/local
BINDIR:=bin
LIBDIR:=lib
LIB_LOCATIONS:=".:/usr/local/lib:/usr/lib"
ENV_VARNAME_FAKE_HOSTNAME:=FAKE_HOSTNAME

PLATFORM:=$(shell uname -s)

ifeq ($(PLATFORM),Darwin)
	CLIBFLAGS:=-fPIC -dynamiclib -flat_namespace
	LIB_SUFFIX:=dylib
else
	CLIBFLAGS:=-fPIC -shared
	LIB_SUFFIX:=so
endif

LIB_NAME=libfakehostname
LIB_SRC_FILE=$(LIB_NAME).c
LIB_FILE:=$(LIB_NAME).$(LIB_SUFFIX)
CMD_NAME=fakehostname
CMD_SRC_FILE=$(CMD_NAME).c
CMD_FILE:=$(CMD_NAME)

# Use CDEFs to set a custom preload library name or search path
CDEFS:=-DLIB_LOCATIONS="\"$(LIB_LOCATIONS)\"" \
	-DLIB_FILE="\"$(LIB_FILE)\"" \
	-DENV_VARNAME_FAKE_HOSTNAME="\"ENV_VARNAME_FAKE_HOSTNAME\""

all: $(LIB_FILE) $(CMD_FILE)

$(LIB_FILE): $(LIB_SRC_FILE)
	$(CC) $(CFLAGS) $(CLIBFLAGS) $(CDEFS) $(LIB_SRC_FILE) -o $(LIB_FILE) $(LDLIBS)

$(CMD_FILE): $(CMD_SRC_FILE)
	$(CC) $(CFLAGS) $(CDEFS) $(CMD_SRC_FILE) -o $(CMD_FILE)

install: all
	@echo "Installing to $(DESTDIR)"
	install $(LIB_FILE) $(DESTDIR)/$(LIBDIR)/
	install $(CMD_FILE) $(DESTDIR)/$(BINDIR)/

uninstall:
	rm -vf $(DESTDIR)/$(LIBDIR)/$(LIB_FILE) $(DESTDIR)/$(BINDIR)/$(CMD_FILE)

clean:
	rm -vf $(LIB_NAME).so $(LIB_NAME).dylib $(CMD_FILE) test_cmd

test_cmd: test_cmd.c
	$(CC) $(CFLAGS) test_cmd.c -o test_cmd

test: all test_cmd
	@./test.sh

.PHONY: all clean install uninstall test
