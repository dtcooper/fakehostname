CC:=gcc
CFLAGS:=-Wall -Wextra -Wpedantic -std=gnu99 -O2
LDLIBS:=-ldl
DESTDIR:=/usr/local
BINDIR:=bin
LIBDIR:=lib
LIB_LOCATIONS:=".:/usr/local/lib:/usr/lib"
ENV_VARNAME_FAKE_HOSTNAME:=FAKE_HOSTNAME
VPATH=src

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
	-DENV_VARNAME_FAKE_HOSTNAME="\"$(ENV_VARNAME_FAKE_HOSTNAME)\""

all: $(LIB_FILE) $(CMD_FILE)
example: $(LIB_FILE) $(CMD_FILE)

%.$(LIB_SUFFIX): %.c
	$(CC) $(CFLAGS) $(CLIBFLAGS) $(CDEFS) $< -o $@ $(LDLIBS)

%: %.c
	$(CC) $(CFLAGS) $(CDEFS) $< -o $@

install: all
	@echo "Installing to $(DESTDIR)"
	install $(LIB_FILE) $(DESTDIR)/$(LIBDIR)/
	install $(CMD_FILE) $(DESTDIR)/$(BINDIR)/

uninstall:
	rm -vf $(DESTDIR)/$(LIBDIR)/$(LIB_FILE) $(DESTDIR)/$(BINDIR)/$(CMD_FILE)

clean:
	rm -vf $(LIB_NAME).so $(LIB_NAME).dylib $(CMD_FILE) example

test: example
	@./src/test.sh

.PHONY: all clean install uninstall test
