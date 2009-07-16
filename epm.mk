EPM_COMMAND = ./epm/epm -v
EPM_LINUX_LDFLAGS=-Wl,-rpath,/opt/drweb/lib
EPM_BSD_LDFLAGS=-Wl,-rpath,/usr/local/drweb/lib
EPM_SUN_LDFLAGS=-Wl,-R,/opt/drweb/lib
DEFAULT_CC=gcc
DEFAULT_CXX=g++
BSD_CC=gcc42
BSD_CXX=g++42

BUILD_EPM_CONF_STRING_COMMON = LDFLAGS=-Wl,-rpath,lib --enable-gui --with-softwaredir=/etc/drweb/software
BUILD_EPM_CONF_STRING_DEFAULT = $(BUILD_EPM_CONF_STRING_COMMON)
BUILD_EPM_CONF_STRING_BSD = $(BUILD_EPM_CONF_STRING_COMMON)

build-epm-bsd:
	cd epm && ( ./configure CC=$(BSD_CC) CXX=$(BSD_CXX) $(BUILD_EPM_CONF_STRING_BSD) ; $(MAKE) )

build-epm-default:
	cd epm && ( ./configure $(BUILD_EPM_CONF_STRING_DEFAULT) ; $(MAKE) )

result:
	rm -rf result
	[ -L ../result ] || ln -s packages/result ../result
