EPM_LINUX_LDFLAGS=-Wl,-rpath,/opt/drweb/lib
EPM_BSD_LDFLAGS=-Wl,-rpath,/usr/local/drweb/lib
EPM_SUN_LDFLAGS=-Wl,-R,/opt/drweb/lib

BUILD_EPM_CONF_STRING_COMMON = LDFLAGS=-Wl,-R,lib
BUILD_EPM_CONF_STRING_DEFAULT = $(BUILD_EPM_CONF_STRING_COMMON) --with-softwaredir=/etc/drweb/software
BUILD_EPM_CONF_STRING_BSD = $(BUILD_EPM_CONF_STRING_COMMON) --with-softwaredir=/usr/local/etc/drweb/software

configure-epm-bsd: Makefile
	cd epm && ./configure $(BUILD_EPM_CONF_STRING_BSD)
	touch $@

configure-epm-default: Makefile
	cd epm && ./configure $(BUILD_EPM_CONF_STRING_DEFAULT)
	touch $@

build-epm-default: configure-epm-default
	$(MAKE) -C epm
	
build-epm-bsd: configure-epm-bsd
	$(MAKE) -C epm

.PHONY: result

result:
	rm -rf result
	[ -L ../result ] || ln -s packages/result ../result

BUILD_FLTK_CONF_STRING_COMMON = --prefix=$(CURDIR)/epm/fltk-install \
				--enable-localjpeg --enable-localpng \
				--enable-localzlib --disable-xinerama \
				--disable-xdbe \
				--disable-threads --disable-gl
BUILD_EPM_FLTK_ADD_STRING = FLTKCONFIG=$(CURDIR)/epm/fltk-install/bin/fltk-config --enable-gui

build-fltk-bsd:
	cd epm/fltk && ( ./configure $(BUILD_FLTK_CONF_STRING_COMMON) ; $(MAKE) install )

build-fltk-default:
	cd epm/fltk && ( ./configure $(BUILD_FLTK_CONF_STRING_COMMON) ; $(MAKE) install )

build-epm-gui-bsd: build-fltk-bsd
	cd epm && ( [ -f Makefile ] || ./configure $(BUILD_EPM_CONF_STRING_BSD) $(BUILD_EPM_FLTK_ADD_STRING) ; $(MAKE) )

build-epm-gui-default: build-fltk-default
	cd epm && ( [ -f Makefile ] || ./configure $(BUILD_EPM_CONF_STRING_DEFAULT) $(BUILD_EPM_FLTK_ADD_STRING) ; $(MAKE) )
