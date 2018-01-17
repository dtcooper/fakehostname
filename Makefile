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

FAKE_HOSTNAME_VERSION:=$(shell git describe --tags --always --dirty=-modified 2>/dev/null || echo dev-nogit)

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
DPKG_DIR=debian-pkg

# Use CDEFs to set a custom preload library name or search path
CDEFS:=-DLIB_LOCATIONS="\"$(LIB_LOCATIONS)\"" \
	-DLIB_FILE="\"$(LIB_FILE)\"" \
	-DENV_VARNAME_FAKE_HOSTNAME="\"$(ENV_VARNAME_FAKE_HOSTNAME)\"" \
	-DFAKE_HOSTNAME_VERSION="\"$(FAKE_HOSTNAME_VERSION)\""

.PHONY: all clean install uninstall test strip deb debug

all: $(LIB_FILE) $(CMD_FILE)

%.$(LIB_SUFFIX): %.c
	$(CC) $(CFLAGS) $(CLIBFLAGS) $(CDEFS) $< -o $@ $(LDLIBS)

$(CMD_FILE): $(CMD_FILE).c
	$(CC) $(CFLAGS) $(CDEFS) $< -o $@ $(LDLIBS)

%: %.c
	$(CC) $(CFLAGS) $(CDEFS) $< -o $@

$(EX_TIME_PRELOAD_FILE): LDLIBS=

debug: CDEFS+=-DENABLE_DEBUG
debug: all $(EX_NAME) $(EX_TIME_PRELOAD_FILE)

clean:
	rm -vrf *.so *.dylib *.deb $(CMD_FILE) $(EX_NAME) $(DPKG_DIR)

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

deb: CDEFS:=-DLIB_LOCATIONS="\"/usr/lib:.:/usr/local/lib\"" \
	-DLIB_FILE="\"$(LIB_FILE)\"" \
	-DENV_VARNAME_FAKE_HOSTNAME="\"$(ENV_VARNAME_FAKE_HOSTNAME)\"" \
	-DFAKE_HOSTNAME_VERSION="\"$(FAKE_HOSTNAME_VERSION)\""
deb: $(EX_NAME) all
	install -vd $(DPKG_DIR)/usr/bin
	install -vsm 0755 $(CMD_FILE) $(DPKG_DIR)/usr/bin

	install -vd $(DPKG_DIR)/usr/lib
	install -vsm 0755 $(LIB_FILE) $(DPKG_DIR)/usr/lib

	install -vd $(DPKG_DIR)/usr/share/fakehostname
	install -vTsm 0755 $(EX_NAME) $(DPKG_DIR)/usr/share/fakehostname/example-prog

	install -vd $(DPKG_DIR)/usr/share/doc/fakehostname
	install -vm 0644 README.md $(DPKG_DIR)/usr/share/doc/fakehostname
	install -vTm 0644 LICENSE $(DPKG_DIR)/usr/share/doc/fakehostname/copyright

	install -vd $(DPKG_DIR)/DEBIAN
	DEB_VER="$(FAKE_HOSTNAME_VERSION)"; \
	DEB_ARCH="$(shell dpkg --print-architecture)"; \
	DEB_NAME="fakehostname_$${DEB_VER}_$${DEB_ARCH}.deb"; \
	sed "s/<<VERSION>>/$$DEB_VER/" debian.control | sed "s/<<ARCH>>/$$DEB_ARCH/" \
		> $(DPKG_DIR)/DEBIAN/control; \
	fakeroot dpkg-deb -b $(DPKG_DIR) "fakehostname_$${DEB_VER}_$${DEB_ARCH}.deb";
endif
