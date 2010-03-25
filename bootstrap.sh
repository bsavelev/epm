#!/bin/sh
#
# Init automake and autoconf environment
#
# $Id: bootstrap.sh,v 1.2.16.4 2009/10/27 16:08:20 bsavelev Exp $
#

old(){
autopoint -f
rm po/Makevars.template
rm po/Rules-quot
rm po/boldquot.sed
rm po/en@boldquot.header
rm po/en@quot.header
rm po/insert-header.sin
rm po/quot.sed

aclocal --force && \
autoconf --force && autoheader && automake --foreign --add-missing --copy --force
}


	rm -f ./install.sh  aclocal.m4
	autoheader
	aclocal -Im4
	autoconf
	automake -a -c --foreign
	touch po/remove-potcdate.sin