CC:=gcc
CFLAGS:=-Wall -Wextra -Wpedantic -std=gnu99 -O2
LDLIBS:=-ldl
DESTDIR:=/usr/local
BINDIR:=bin
LIBDIR:=lib
LIB_LOCATIONS:=".:/usr/local/lib:/usr/lib"
ENV_VARNAME_FAKE_HOSTNAME:=FAKE_HOSTNAME
VPATH=src:extras

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

EX_NAME=example
EX_TIME_PRELOAD_NAME=time_preload
EX_TIME_PRELOAD_FILE=$(EX_TIME_PRELOAD_NAME).$(LIB_SUFFIX)

# Use CDEFs to set a custom preload library name or search path
CDEFS:=-DLIB_LOCATIONS="\"$(LIB_LOCATIONS)\"" \
	-DLIB_FILE="\"$(LIB_FILE)\"" \
	-DENV_VARNAME_FAKE_HOSTNAME="\"$(ENV_VARNAME_FAKE_HOSTNAME)\""

.PHONY: all clean install uninstall test strip deb verbose

all: $(LIB_FILE) $(CMD_FILE)

%.$(LIB_SUFFIX): %.c
	$(CC) $(CFLAGS) $(CLIBFLAGS) $(CDEFS) $< -o $@ $(LDLIBS)

%: %.c
	$(CC) $(CFLAGS) $(CDEFS) $< -o $@

verbose: CDEFS+=-DENABLE_VERBOSE=1
verbose: all

$(EX_NAME): CDEFS=
$(EX_TIME_PRELOAD_FILE): CDEFS=
$(EX_TIME_PRELOAD_FILE): LDLIBS=

clean:
	rm -vrf *.so *.dylib *.deb $(CMD_FILE) $(EX_NAME) debian-pkg

install: all
	@echo "Installing to $(DESTDIR)"
	install $(LIB_FILE) $(DESTDIR)/$(LIBDIR)/
	install $(CMD_FILE) $(DESTDIR)/$(BINDIR)/

uninstall:
	rm -vf $(DESTDIR)/$(LIBDIR)/$(LIB_FILE) $(DESTDIR)/$(BINDIR)/$(CMD_FILE)

test: $(EX_NAME) $(EX_TIME_PRELOAD_FILE) $(LIB_FILE) $(CMD_FILE)
	@./extras/test.sh

strip: all
ifeq ($(PLATFORM),Darwin)
strip:
	strip $(LIB_FILE) $(CMD_FILE) || true
else
strip:
	strip -s $(LIB_FILE) $(CMD_FILE)

deb: $(EX_NAME) all
	mkdir -p debian-pkg/usr/bin
	cp -v $(CMD_FILE) debian-pkg/usr/bin
	strip -s debian-pkg/usr/bin/$(CMD_FILE)

	mkdir -p debian-pkg/usr/lib
	cp -v $(LIB_FILE) debian-pkg/usr/lib
	strip -s debian-pkg/usr/lib/$(LIB_FILE)

	mkdir -p debian-pkg/usr/share/doc/fakehostname
	cp -v README.md debian-pkg/usr/share/doc/fakehostname
	cp -v LICENSE debian-pkg/usr/share/doc/fakehostname/copyright
	cp -v $(EX_NAME) debian-pkg/usr/share/doc/fakehostname/example-prog
	strip -s debian-pkg/usr/share/doc/fakehostname/example-prog

	mkdir -p debian-pkg/DEBIAN
	DEB_VER="$(shell git describe --tags --always --dirty 2>/dev/null || echo unknown)"; \
	DEB_ARCH="$(shell dpkg --print-architecture)"; \
	DEB_NAME="fakehostname_$${DEB_VER}_$${DEB_ARCH}.deb"; \
	sed "s/<<VERSION>>/$$DEB_VER/" debian.control | sed "s/<<ARCH>>/$$DEB_ARCH/" \
		> debian-pkg/DEBIAN/control; \
	fakeroot dpkg-deb -b debian-pkg "fakehostname_$${DEB_VER}_$${DEB_ARCH}.deb"; \
	cp -v "$$DEB_NAME" "fakehostname-latest_$${DEB_ARCH}.deb"
endif
