#!/usr/bin/make -f

export LC_ALL := C
export SOURCE_DATE_EPOCH := 0

SHELL := /bin/sh
.SHELLFLAGS := -euc

DESTDIR ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share

EXE ?= sfxr$(EXEEXT)

PKG_CONFIG ?= pkg-config
INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= $(INSTALL)
INSTALL_DESKTOP ?= desktop-file-install

ifdef EMSCRIPTEN

EXEEXT ?= .html
CPPFLAGS ?=
CFLAGS ?= \
	-Oz \
	-flto \
	--closure 1 \
	-s ENVIRONMENT=web \
	-s STRICT=1 \
	-s WASM_BIGINT=1 \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s EXPORTED_RUNTIME_METHODS='["ccall"]' \
	-s EXPORTED_FUNCTIONS='["_main", "_malloc", "_free", "UTF8ToString"]' \
	-s USE_SDL=1 \
	-s USE_WEBGL2=1 \
	-s MIN_WEBGL_VERSION=2 \
	-s MAX_WEBGL_VERSION=2 \
	--shell-file ./shell.html \
	--preload-file ./font.tga \
	--preload-file ./ld48.tga
CXXFLAGS ?= \
	$(CFLAGS)
LDFLAGS ?=

else

CPPFLAGS ?=
CFLAGS ?= \
	-O3 \
	-flto \
	-fPIE \
	-fstack-protector-strong \
	-fstack-clash-protection \
	-fcf-protection=full
CXXFLAGS ?= \
	$(CFLAGS)
LDFLAGS ?= \
	-Wl,-pie \
	-Wl,-z,relro \
	-Wl,-z,defs \
	-Wl,-z,now \
	-Wl,-z,noexecstack

SFXR_SDL ?= sdl2
ifdef SFXR_SDL
CPPFLAGS += -DHAVE_SDL
CFLAGS += $(shell $(PKG_CONFIG) $(SFXR_SDL) --cflags)
LDFLAGS += $(shell $(PKG_CONFIG) $(SFXR_SDL) --libs)
endif

SFXR_GTK ?= gtk4
ifdef SFXR_GTK
CPPFLAGS += -DHAVE_GTK
CFLAGS += $(shell $(PKG_CONFIG) $(SFXR_GTK) --cflags)
LDFLAGS += $(shell $(PKG_CONFIG) $(SFXR_GTK) --libs)
endif

endif

CPPFLAGS += \
	-Wdate-time \
	-D_FORTIFY_SOURCE=2 \
	-DDATADIR=\"$(DESTDIR)$(DATADIR)\" \
	$(EXTRA_CPPFLAGS)
CFLAGS += \
	-frandom-seed=42 \
	-Wall \
	-Wextra \
	-Wformat \
	-Werror=format-security \
	$(EXTRA_CFLAGS)
CXXFLAGS += \
	$(EXTRA_CXXFLAGS)
LDFLAGS += \
	$(EXTRA_LDFLAGS)

.PHONY: all
all: $(EXE)

$(EXE): main.cpp sdlkit.h tools.h emfileutils.h
	$(CXX) $< $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@

.PHONY: install
install: $(EXE)
	$(INSTALL) -d '$(DESTDIR)$(BINDIR)'
	$(INSTALL) -d '$(DESTDIR)$(DATADIR)'/sfxr/
	$(INSTALL) -d '$(DESTDIR)$(DATADIR)'/applications/
	$(INSTALL) -d '$(DESTDIR)$(DATADIR)'/icons/hicolor/48x48/apps/
	$(INSTALL_PROGRAM) -m755 ./sfxr '$(DESTDIR)$(BINDIR)'
	$(INSTALL_DATA) -m644 ./*.tga ./*.bmp '$(DESTDIR)$(DATADIR)'/sfxr/
	$(INSTALL_DATA) -m644 ./sfxr.png '$(DESTDIR)$(DATADIR)'/icons/hicolor/48x48/apps/
	$(INSTALL_DESKTOP) --vendor '' --dir '$(DESTDIR)$(DATADIR)'/applications/ ./sfxr.desktop

.PHONY: install-strip
install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install

.PHONY: clean
clean:
	rm -f '$(EXE)'
