# General automake options.
AUTOMAKE_OPTIONS = foreign no-dependencies
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = po

EXTRA_PROGRAMS = setup uninst

bin_PROGRAMS = \
			epm \
			epminstall \
			mkepmlist \
			@GUIS@

noinst_LIBRARIES = libepm.a

libepm_a_SOURCES = \
			aix.c \
			bsd.c \
			deb.c \
			dist.c \
			file.c \
			inst.c \
			osx.c \
			pkg.c \
			portable.c \
			qprintf.c \
			rpm.c \
			run.c \
			setld.c \
			slackware.c \
			snprintf.c \
			string.c \
			support.c \
			swinstall.c \
			tar.c \
			config.h

setup_SOURCES	=	setup.cxx \
			setup2.cxx \
			gui-common.cxx

setup_LDADD = libepm.a @LIBINTL@ @GUILIBS@

setup_CXXFLAGS = @EPM_GUI_CXXFLAGS@

uninst_SOURCES	=	uninst.cxx \
			uninst2.cxx \
			gui-common.cxx

uninst_LDADD = libepm.a @LIBINTL@ @GUILIBS@

uninst_CXXFLAGS = @EPM_GUI_CXXFLAGS@

epm_SOURCES	=	epm.c

epm_LDADD = libepm.a

epminstall_SOURCES =	epminstall.c

epminstall_LDADD = libepm.a

mkepmlist_SOURCES =	mkepmlist.c

mkepmlist_LDADD = libepm.a

AM_CFLAGS = $(ARCHFLAGS) $(OPTIM) $(EPM_CFLAGS)
AM_CXXFLAGS = $(ARCHFLAGS) $(OPTIM) $(EPM_CXXFLAGS)


dummy: all epm.test.list
	rm -rf dummy
	./epm$(EXEEXT) -m dummy -a i386 epm epm.test.list
	cd dummy && gzip -dc *.tar.gz | tar -xpPF - || true
	cd dummy && mv -f epm.install epm.install.sh && \
	    mv -f epm.remove epm.remove.sh

.PHONY: dummy
