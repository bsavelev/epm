#!/bin/sh
#
# Init automake and autoconf environment
#
# $Id: bootstrap.sh,v 1.2.16.4 2009/10/27 16:08:20 bsavelev Exp $
#

aclocal --force && \
autoconf
