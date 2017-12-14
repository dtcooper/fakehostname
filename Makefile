CC:=gcc
CFLAGS:=-Wall -fPIC
LDLIBS:=-ldl
DESTDIR:=/usr/local
BINDIR:=bin
LIBDIR:=lib

PLATFORM:=$(shell uname -s)

ifeq ($(PLATFORM),Darwin)
	CFLAGS+=-dynamiclib -flat_namespace
	LIB_SUFFIX=dylib
else
	CFLAGS+=-shared
	LIB_SUFFIX=so
endif

SRC_FILE=libfakehostname.c
LIB_FILE=libfakehostname.$(LIB_SUFFIX)
SCRIPT_FILE=fakehostname

all: $(LIB_FILE)

$(LIB_FILE): $(SRC_FILE)
	$(CC) $(CFLAGS) $(SRC_FILE) -o $(LIB_FILE) $(LDLIBS)

install: all
	install $(LIB_FILE) $(DESTDIR)/$(LIBDIR)
	install $(SCRIPT_FILE) $(DESTDIR)/$(BINDIR)

uninstall:
	rm -f $(DESTDIR)/$(LIBDIR)/$(LIB_FILE) $(DESTDIR)/$(BINDIR)/$(SCRIPT_FILE)

clean:
	rm -f *.dylib *.so

test: all
	@./test.sh

.PHONY: all clean install uninstall test
