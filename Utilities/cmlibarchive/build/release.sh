#!/bin/sh +v

PATH=/usr/local/gnu-autotools/bin/:$PATH
export PATH

# BSD make's "OBJDIR" support freaks out the automake-generated
# Makefile.  Effectively disable it.
export MAKEOBJDIRPREFIX=/junk

# Start from the build directory, where the version file is located
if [ -f build/version ]; then
    cd build
fi

if [ \! -f version ]; then
    echo "Can't find version file"
    exit 1
fi

# Update the build number in the 'version' file.
# Separate number from additional alpha/beta/etc marker
MARKER=`cat version | sed 's/[0-9.]//g'`
# Bump the number
VN=`cat version | sed 's/[^0-9.]//g'`
# Reassemble and write back out
VN=$(($VN + 1))
rm -f version.old
mv version version.old
chmod +w version.old
echo $VN$MARKER > version
# Build out the string.
VS="$(($VN/1000000)).$(( ($VN/1000)%1000 )).$(( $VN%1000 ))$MARKER"

cd ..

# Substitute the integer version into Libarchive's archive.h
perl -p -i -e "s/^(#define\tARCHIVE_VERSION_NUMBER).*/\$1 $VN/" libarchive/archive.h
perl -p -i -e "s/^(#define\tARCHIVE_VERSION_STRING).*/\$1 \"libarchive $VS\"/" libarchive/archive.h
# Substitute the string version into tar and cpio Makefiles
perl -p -i -e "s/^(BSDTAR_VERSION_STRING)=.*/\$1=$VS/" tar/Makefile
perl -p -i -e "s/^(BSDCPIO_VERSION_STRING)=.*/\$1=$VS/" cpio/Makefile
# Substitute versions into configure.ac as well
perl -p -i -e 's/(m4_define\(\[LIBARCHIVE_VERSION_S\]),.*\)/$1,['"$VS"'])/' configure.ac
perl -p -i -e 's/(m4_define\(\[LIBARCHIVE_VERSION_N\]),.*\)/$1,['"$VN"'])/' configure.ac

# Add a version notice to NEWS
mv NEWS NEWS.bak
chmod +w NEWS.bak
echo > NEWS
echo `date +"%b %d, %Y:"` libarchive $VS released >> NEWS
cat NEWS.bak >> NEWS

# Clean up first
rm -rf /usr/obj`pwd`
(cd examples/minitar && make cleandir && make clean)
(cd libarchive && make cleandir && make clean)
(cd libarchive/test && make cleandir && make clean && make list.h)
(cd tar && make cleandir && make clean)

# Build the libarchive distfile
/bin/sh build/autogen.sh
./configure
make distcheck
