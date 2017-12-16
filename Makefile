CC:=gcc
CFLAGS:=-Wall -Wextra -std=gnu99
LDLIBS:=-ldl
DESTDIR:=/usr/local
BINDIR:=bin
LIBDIR:=lib
LIB_LOCATIONS:=".:/usr/local/lib:/usr/lib"
ENV_VARNAME:=FAKE_HOSTNAME

PLATFORM:=$(shell uname -s)

ifeq ($(PLATFORM),Darwin)
	CLIBFLAGS:=-fPIC -dynamiclib -flat_namespace
	LIB_SUFFIX:=dylib
else
	CLIBFLAGS:=-fPIC -shared
	LIB_SUFFIX:=so
endif

LIB_SRC_FILE=libfakehostname.c
LIB_FILE:=libfakehostname.$(LIB_SUFFIX)
CMD_SRC_FILE=fakehostname.c
CMD_FILE:=fakehostname

# Use CDEFs to set a custom preload library name or search path
CDEFS:=-DLIB_LOCATIONS="\"$(LIB_LOCATIONS)\"" \
	-DLIB_FILE="\"$(LIB_FILE)\"" \
	-DENV_VARNAME="\"ENV_VARNAME\""

all: $(LIB_FILE) $(CMD_FILE)

$(LIB_FILE): $(LIB_SRC_FILE)
	$(CC) $(CFLAGS) $(CLIBFLAGS) $(CDEFS) $(LIB_SRC_FILE) -o $(LIB_FILE) $(LDLIBS)

$(CMD_FILE): $(CMD_SRC_FILE)
	$(CC) $(CFLAGS) $(CDEFS) $(CMD_SRC_FILE) -o $(CMD_FILE)

install: all
	@echo "Installing to $(DESTDIR)"
	install $(LIB_FILE) $(DESTDIR)/$(LIBDIR)/
	install $(SCRIPT_FILE) $(DESTDIR)/$(BINDIR)/

uninstall:
	rm -vf $(DESTDIR)/$(LIBDIR)/$(LIB_FILE) $(DESTDIR)/$(BINDIR)/$(SCRIPT_FILE)

clean:
	rm -vf *.dylib *.so fakehostname

test: all
	@./test.sh

.PHONY: all clean install uninstall test
