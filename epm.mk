EPM_LINUX_LDFLAGS=-Wl,-rpath,/opt/drweb/lib
EPM_BSD_LDFLAGS=-Wl,-rpath,/usr/local/drweb/lib
EPM_SUN_LDFLAGS=-Wl,-R,/opt/drweb/lib
DEFAULT_CC=gcc
DEFAULT_CXX=g++
BSD_CC=gcc42
BSD_CXX=g++42

BUILD_EPM_CONF_STRING_COMMON = LDFLAGS=-Wl,-rpath,lib
BUILD_EPM_CONF_STRING_DEFAULT = $(BUILD_EPM_CONF_STRING_COMMON) --with-softwaredir=/etc/drweb/software
BUILD_EPM_CONF_STRING_BSD = $(BUILD_EPM_CONF_STRING_COMMON) --with-softwaredir=/usr/local/etc/drweb/software

build-epm-bsd:
	cd epm && ( [ -f Makefile ] || ./configure CC=$(BSD_CC) CXX=$(BSD_CXX) $(BUILD_EPM_CONF_STRING_BSD) ; $(MAKE) )

build-epm-default:
	cd epm && ( [ -f Makefile ] || ./configure $(BUILD_EPM_CONF_STRING_DEFAULT) ; $(MAKE) )

.PHONY: result

result:
	rm -rf result
	[ -L ../result ] || ln -s packages/result ../result

BUILD_FLTK_CONF_STRING_COMMON = --prefix=$(CURDIR)/epm/fltk-install --enable-localjpeg --enable-localpng --enable-localzlib
BUILD_EPM_FLTK_ADD_STRING = FLTKCONFIG=$(CURDIR)/epm/fltk-install/bin/fltk-config --enable-gui

build-fltk-bsd:
	cd epm/fltk && ( [ -f Makefile ] || ./configure CC=$(BSD_CC) CXX=$(BSD_CXX) $(BUILD_FLTK_CONF_STRING_COMMON) ; $(MAKE) install )

build-fltk-default:
	cd epm/fltk && ( [ -f Makefile ] || ./configure $(BUILD_FLTK_CONF_STRING_COMMON) ; $(MAKE) install )

build-epm-gui-bsd: build-fltk-bsd
	cd epm && ( [ -f Makefile ] || ./configure CC=$(BSD_CC) CXX=$(BSD_CXX) $(BUILD_EPM_CONF_STRING_BSD) $(BUILD_EPM_FLTK_ADD_STRING) ; $(MAKE) )

build-epm-gui-default: build-fltk-default
	cd epm && ( [ -f Makefile ] || ./configure $(BUILD_EPM_CONF_STRING_DEFAULT) $(BUILD_EPM_FLTK_ADD_STRING) ; $(MAKE) )
