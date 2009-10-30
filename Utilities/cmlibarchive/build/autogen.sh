#!/bin/sh


# Start from one level above the build directory
if [ -f version ]; then
    cd ..
fi

if [ \! -f build/version ]; then
    echo "Can't find source directory"
    exit 1
fi

set -xe
aclocal -I build/autoconf

# Note: --automake flag needed only for libtoolize from
# libtool 1.5.x; in libtool 2.2.x it is a synonym for --quiet
case `uname` in
Darwin) glibtoolize --automake -c;;
*) libtoolize --automake -c;;
esac
autoconf
autoheader
automake -a -c
