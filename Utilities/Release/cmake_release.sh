#!/bin/sh
#=============================================================================
#
# Program:   CMake - Cross-Platform Makefile Generator
# Module:    $RCSfile$
# Language:  C++
# Date:      $Date$
# Version:   $Revision$
#
# Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
# See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.
#
#    This software is distributed WITHOUT ANY WARRANTY; without even 
#    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
#    PURPOSE.  See the above copyright notices for more information.
#
#=============================================================================

#
# CMake UNIX Release Script.
#
# Run with no arguments for documentation.
#

# Release version number.
TAG="Release-1-6-7"
VERSION="1.6.7"
RELEASE="1"
PREVIOUS_VERSION="1.4.7"
PREVIOUS_RELEASE="1"

# CVSROOT setting used to check out CMake.
CVSROOT=":pserver:anonymous@www.cmake.org:/cvsroot/CMake"
CVSROOT_GREP=":pserver:anonymous@www.cmake.org:[0-9]*/cvsroot/CMake"

# CMake release root directory.
RELEASE_ROOT_NAME="CMakeReleaseRoot"
RELEASE_ROOT="${HOME}/${RELEASE_ROOT_NAME}"

# Installation prefix used during tarball creation.  Tarballs are
# relative to the installation prefix and do not include this in their
# paths.
PREFIX="/usr/local"

# Directory relative to PREFIX where documentation should be placed.
DOC_DIR="/doc/cmake"

# No default compiler.  The config file must provide it.
CC=""
CXX=""
CFLAGS=""
CXXFLAGS=""

# Details of remote invocation.
[ -z "$REMOTE" ] && SELF="$0"

#-----------------------------------------------------------------------------
usage()
{
    cat <<EOF
CMake Release Script Usage:
  $0 [command]

Typical usage:
  Cleanup remote host release directory:

    $0 remote <host> clean

  Create binary release tarball:

    $0 remote_binary <host>

  Create source release tarball:

    $0 remote_source <host>

  Upload tarballs:

    $0 upload

  Create and upload cygwin package:

    $0 cygwin_package
    $0 cygwin_upload

Available commands:

EOF
    cat "$0" | awk '
/^#--*$/               { doc=1; text="" }

/(^#$|^#[^-].*$)/     {
  if(doc)
    {
    if(text != "") { text = sprintf("%s  %s\n", text, $0) }
    else           { text = sprintf("  %s\n", $0) }
    }
}

/^[A-Za-z0-9_]*\(\)$/ {
  doc=0;
  if(text != "") { printf("%s:\n%s\n", $0, text) }
}
'
}

#-----------------------------------------------------------------------------
error_log()
{
    echo "An error has been logged to $1:" &&
    cat "$1" &&
    return 1
}

#-----------------------------------------------------------------------------
check_host()
{
    HOST="$1"
    if [ -z "$HOST" ]; then
        echo "Must specify host."
        return 1
    fi
}

#-----------------------------------------------------------------------------
# Run a command on the specified remote host.
#
#  remote <host> [command]
#
# Only one level of remote invocation is allowed.  The <host>
# specification must be a valid ssh destination with public
# key authentication and no password.
remote()
{
    if [ ! -z "$REMOTE" ]; then
        echo "Cannot do recursive remote calls."
        return 1
    fi
    check_host "$1" || return 1
    shift
    RTASK="'$1'"; shift; for i in "$@"; do RTASK="${RTASK} '$i'"; done
    RESULT=0
    echo "------- Running remote task on $HOST. -------" &&
    (echo "REMOTE=\"1\"" &&
        (echo TASK=\"`(eval echo '${RTASK}') | (sed 's/"/\\\\"/g')`\") &&
        cat $SELF) | ssh -e none "$HOST" /bin/sh  || RESULT=1
    echo "-------- Remote task on $HOST done.  --------" &&
    return $RESULT
}

#-----------------------------------------------------------------------------
# Copy tarballs from the specified host.
#
#  remote_copy <host> [EXPR]
#
# The <host> specification must be a valid ssh destination
# with public key authentication and no password.  Only
# files matching the given expression are copied.  If
# no expression is given, "*" is used.
remote_copy()
{
    check_host "$1" || return 1
    EXPR="$2"
    [ ! -z "$EXPR" ] || EXPR="*"
    echo "------- Copying tarballs from $HOST. -------" &&
    scp "$HOST:${RELEASE_ROOT_NAME}/Tarballs/${EXPR}" . &&
    echo "---- Done copying tarballs from $HOST. -----"
}

#-----------------------------------------------------------------------------
remote_copy_source()
{
    check_host "$1" || return 1
    remote_copy "$HOST" "cmake-${VERSION}.tar*"
}

#-----------------------------------------------------------------------------
remote_copy_binary()
{
    check_host "$1" || return 1
    remote_copy "$HOST" "cmake-${VERSION}-*"
}

#-----------------------------------------------------------------------------
# Create source tarballs on the specified host and copy them locally.
#
#  remote_source <host>
#
# The <host> specification must be a valid ssh destination
# with public key authentication and no password.
remote_source()
{
    check_host "$1" || return 1
    remote "$HOST" source_tarball &&
    remote_copy_source "$HOST"
}

#-----------------------------------------------------------------------------
# Create binary tarballs on the specified host and copy them locally.
#
#  remote_binary <host>
#
# The <host> specification must be a valid ssh destination
# with public key authentication and no password.
remote_binary()
{
    check_host "$1" || return 1
    remote "$HOST" binary_tarball &&
    remote_copy_binary "$HOST"
}

#-----------------------------------------------------------------------------
# Upload any tarballs in the current directory to the CMake FTP site.
#
#  upload
#
# The user must be able to ssh to kitware@www.cmake.org with public
# key authentication and no password.
upload()
{
    echo "------- Copying tarballs to www.cmake.org. -------"
    scp cmake-${VERSION}*tar.* kitware@www.cmake.org:/projects/FTP/pub/cmake
    echo "---- Done copying tarballs to www.cmake.org. -----"
}

#-----------------------------------------------------------------------------
setup()
{
    [ -z "${DONE_setup}" ] || return 0 ; DONE_setup="yes"
    mkdir -p ${RELEASE_ROOT}/Logs &&
    echo "Entering ${RELEASE_ROOT}" &&
    cd ${RELEASE_ROOT}
}

#-----------------------------------------------------------------------------
# Remove the release root directory.
#
#  clean
#
clean()
{
    cd "${HOME}" &&
    echo "Cleaning up ${RELEASE_ROOT}" &&
    rm -rf "${RELEASE_ROOT_NAME}"
}

#-----------------------------------------------------------------------------
cvs_login()
{
    [ -z "${DONE_cvs_login}" ] || return 0 ; DONE_cvs_login="yes"
    setup || return 1
    (
        if [ -f "${HOME}/.cvspass" ]; then
            CVSPASS="${HOME}/.cvspass"
        else
            CVSPASS=""
        fi
        if [ -z "`grep \"$CVSROOT_GREP\" ${CVSPASS} /dev/null`" ]; then
            echo "cmake" | cvs -q -z3 -d $CVSROOT login
        else
            echo "Already logged in."
        fi
    ) >Logs/cvs_login.log 2>&1 || error_log Logs/cvs_login.log
}

#-----------------------------------------------------------------------------
utilities()
{
    [ -z "${DONE_utilities}" ] || return 0 ; DONE_utilities="yes"
    cvs_login || return 1
    (
        if [ -d "ReleaseUtilities/CVS" ]; then
            cd ReleaseUtilities && cvs -z3 -q update -dAP
        else
            rm -rf CheckoutTemp &&
            mkdir CheckoutTemp &&
            cd CheckoutTemp &&
            cvs -q -z3 -d $CVSROOT co -r ${TAG} CMake/Utilities/Release &&
            mv CMake/Utilities/Release ../ReleaseUtilities &&
            cd .. &&
            rm -rf CheckoutTemp
        fi
    ) >Logs/utilities.log 2>&1 || error_log Logs/utilities.log
}

#-----------------------------------------------------------------------------
config()
{
    [ -z "${DONE_config}" ] || return 0 ; DONE_config="yes"
    utilities || return 1
    CONFIG_FILE="config_`uname`"
    echo "Loading ${CONFIG_FILE} ..."
    . ${RELEASE_ROOT}/ReleaseUtilities/${CONFIG_FILE} >Logs/config.log 2>&1 || error_log Logs/config.log
    if [ -z "${CC}" ] || [ -z "${CXX}" ] || [ -z "${PLATFORM}" ]; then
        echo "${CONFIG_FILE} should specify CC, CXX, and PLATFORM." &&
        return 1
    fi
    export CC CXX CFLAGS CXXFLAGS PATH LD_LIBRARY_PATH
}

#-----------------------------------------------------------------------------
checkout()
{
    [ -z "${DONE_checkout}" ] || return 0 ; DONE_checkout="yes"
    config || return 1
    echo "Exporting cmake from cvs ..." &&
    (
        rm -rf cmake-${VERSION} &&
        rm -rf CheckoutTemp &&
        mkdir CheckoutTemp &&
        cd CheckoutTemp &&
        cvs -q -z3 -d $CVSROOT export -r ${TAG} CMake &&
        mv CMake ../cmake-${VERSION} &&
        cd .. &&
        rm -rf CheckoutTemp
    ) >Logs/checkout.log 2>&1 || error_log Logs/checkout.log
}

#-----------------------------------------------------------------------------
# Create source tarballs for CMake.
#
#  source_tarball
#
source_tarball()
{
    [ -z "${DONE_source_tarball}" ] || return 0 ; DONE_source_tarball="yes"
    config || return 1
    [ -d "cmake-${VERSION}" ] || checkout || return 1
    echo "Creating source tarballs ..." &&
    (
        mkdir -p Tarballs &&
        rm -rf Tarballs/cmake-${VERSION}.tar* &&
        tar cvf Tarballs/cmake-${VERSION}.tar cmake-${VERSION} &&
        gzip -c Tarballs/cmake-${VERSION}.tar >Tarballs/cmake-${VERSION}.tar.gz &&
        compress Tarballs/cmake-${VERSION}.tar
    ) >Logs/source_tarball.log 2>&1 || error_log Logs/source_tarball.log
}

#-----------------------------------------------------------------------------
write_cache()
{
    cat > CMakeCache.txt <<EOF
BUILD_TESTING:BOOL=OFF
EOF
}

#-----------------------------------------------------------------------------
cache()
{
    [ -z "${DONE_cache}" ] || return 0 ; DONE_cache="yes"
    config || return 1
    echo "Writing CMakeCache.txt ..." &&
    (
        rm -rf "cmake-${VERSION}-${PLATFORM}" &&
        mkdir -p "cmake-${VERSION}-${PLATFORM}" &&
        cd "cmake-${VERSION}-${PLATFORM}" &&
        write_cache
    ) >Logs/cache.log 2>&1 || error_log Logs/cache.log
}

#-----------------------------------------------------------------------------
configure()
{
    [ -z "${DONE_configure}" ] || return 0 ; DONE_configure="yes"
    config || return 1
    [ -d "cmake-${VERSION}" ] || checkout || return 1
    cache || return 1
    echo "Running configure ..." &&
    (
        cd "cmake-${VERSION}-${PLATFORM}" &&
        ../cmake-${VERSION}/configure --prefix=${PREFIX}
    ) >Logs/configure.log 2>&1 || error_log Logs/configure.log
}

#-----------------------------------------------------------------------------
build()
{
    [ -z "${DONE_build}" ] || return 0 ; DONE_build="yes"
    config || return 1
    if [ ! -d "cmake-${VERSION}-${PLATFORM}/Bootstrap" ]; then
        configure || return 1
    fi
    echo "Running make ..." &&
    (
        cd "cmake-${VERSION}-${PLATFORM}" &&
        make
    ) >Logs/build.log 2>&1 || error_log Logs/build.log
}

#-----------------------------------------------------------------------------
tests()
{
    [ -z "${DONE_tests}" ] || return 0 ; DONE_tests="yes"
    config || return 1
    [ -f "cmake-${VERSION}-${PLATFORM}/Source/ccmake" ] || build || return 1
    echo "Running tests ..." &&
    (
        cd "cmake-${VERSION}-${PLATFORM}" &&
        rm -rf Tests &&
        ./Source/ctest -V
    ) >Logs/tests.log 2>&1 || error_log Logs/tests.log
}

#-----------------------------------------------------------------------------
install()
{
    [ -z "${DONE_install}" ] || return 0 ; DONE_install="yes"
    config || return 1
    [ -d "cmake-${VERSION}-${PLATFORM}/Tests/Simple" ] || tests || return 1
    echo "Running make install ..." &&
    (
        rm -rf Install &&
        cd "cmake-${VERSION}-${PLATFORM}" &&
        make install DESTDIR="${RELEASE_ROOT}/Install"
    ) >Logs/install.log 2>&1 || error_log Logs/install.log
}

#-----------------------------------------------------------------------------
strip()
{
    [ -z "${DONE_strip}" ] || return 0 ; DONE_strip="yes"
    config || return 1
    [ -f "Install/usr/local/bin/ccmake" ] || install || return 1
    echo "Stripping executables ..." &&
    (
        strip Install${PREFIX}/bin/*
    ) >Logs/strip.log 2>&1 || error_log Logs/strip.log
}

#-----------------------------------------------------------------------------
manifest()
{
    [ -z "${DONE_manifest}" ] || return 0 ; DONE_manifest="yes"
    config || return 1
    [ -f "Install/usr/local/bin/ccmake" ] || install || return 1
    echo "Writing MANIFEST ..." &&
    (
        mkdir -p Install${PREFIX}${DOC_DIR} &&
        rm -rf Install${PREFIX}${DOC_DIR}/MANIFEST &&
        touch Install${PREFIX}${DOC_DIR}/MANIFEST &&
        cd Install${PREFIX} &&
        FILES=`find . -type f |sed 's/^\.\///'` &&
        cd ${RELEASE_ROOT} &&
        (cat >> Install${PREFIX}${DOC_DIR}/MANIFEST <<EOF
${FILES}
EOF
        ) &&
        rm -rf Install/README &&
        (cat > Install/README <<EOF
CMake $VERSION binary for $PLATFORM

Extract the file "cmake-${VERSION}-${PLATFORM}-files.tar" into your
destination directory (typically /usr/local).  The following files will
be extracted:

${FILES}

EOF
        )
    ) >Logs/manifest.log 2>&1 || error_log Logs/manifest.log
}

#-----------------------------------------------------------------------------
# Create binary tarballs for CMake.
#
#  binary_tarball
#
binary_tarball()
{
    [ -z "${DONE_binary_tarball}" ] || return 0 ; DONE_binary_tarball="yes"
    config || return 1
    strip || return 1
    manifest || return 1
    echo "Creating binary tarballs ..." &&
    (
        mkdir -p Tarballs &&
        rm -rf Install/cmake-${VERSION}-${PLATFORM}-files.tar &&
        (
            cd Install${PREFIX} &&
            tar cvf ${RELEASE_ROOT}/Install/cmake-${VERSION}-${PLATFORM}-files.tar *
        ) &&
        rm -rf Tarballs/cmake-${VERSION}-${PLATFORM}.tar* &&
        (
            cd Install &&
            tar cvf ${RELEASE_ROOT}/Tarballs/cmake-${VERSION}-${PLATFORM}.tar cmake-${VERSION}-${PLATFORM}-files.tar README
        ) &&
        (
            cd Tarballs &&
            gzip -c cmake-${VERSION}-${PLATFORM}.tar >cmake-${VERSION}-${PLATFORM}.tar.gz &&
            compress cmake-${VERSION}-${PLATFORM}.tar
        )
    ) >Logs/binary_tarball.log 2>&1 || error_log Logs/binary_tarball.log
}

#-----------------------------------------------------------------------------
cygwin_source_tarball()
{
    [ -z "${DONE_cygwin_source_tarball}" ] || return 0 ; DONE_cygwin_source_tarball="yes"
    config || return 1
    [ -d "cmake-${VERSION}" ] || checkout || return 1
    echo "Creating cygwin source tarball ..." &&
    (
        mkdir -p Cygwin &&
        rm -rf Cygwin/cmake-${VERSION}.tar.bz2 &&
        tar cvjf Cygwin/cmake-${VERSION}.tar.bz2 cmake-${VERSION}
    ) >Logs/cygwin_source_tarball.log 2>&1 || error_log Logs/cygwin_source_tarball.log
}

#-----------------------------------------------------------------------------
cygwin_source_patch()
{
    [ -z "${DONE_cygwin_source_patch}" ] || return 0 ; DONE_cygwin_source_patch="yes"
    config || return 1
    [ -d "cmake-${VERSION}" ] || checkout || return 1
    echo "Creating source patch for cygwin ..." &&
    (
        mkdir -p Cygwin &&
        rm -rf Cygwin/Patched &&
        mkdir -p Cygwin/Patched &&
        (tar c cmake-${VERSION} | (cd Cygwin/Patched; tar x)) &&
        cd Cygwin/Patched &&
        mkdir -p cmake-${VERSION}/CYGWIN-PATCHES &&
        (
            CYGVERSION=`uname -r`
            cat > cmake-${VERSION}/CYGWIN-PATCHES/cmake.README <<EOF
cmake
--------------------------------------
Runtime requirements:
  cygwin-${CYGVERSION} or newer

Build requirements
  cygwin-${CYGVERSION} or newer
  make

Canonical homepage:
  http://www.cmake.org

Canonical download:
  ftp://www.cmake.org/pub/cmake/

------------------------------------

Build instructions:
  unpack cmake-${VERSION}-${RELEASE}-src.tar.bz2
    if you use setup to install this src package, it will be
	 unpacked under /usr/src automatically
  cd /usr/src
  ./cmake-${VERSION}-${RELEASE}.sh all

This will create:
  /usr/src/cmake-${VERSION}-${RELEASE}.tar.bz2
  /usr/src/cmake-${VERSION}-${RELEASE}-src.tar.bz2

-------------------------------------------

Port Notes:

<none>

------------------

Cygwin port maintained by: CMake Developers <cmake@www.cmake.org>

EOF
        ) &&
        (
            cat > cmake-${VERSION}/CYGWIN-PATCHES/setup.hint <<EOF
# CMake setup.hint file for cygwin setup.exe program
category: Devel 
requires: libncurses6 cygwin 
sdesc: "A cross platform build manger" 
ldesc: "CMake is a cross platform build manager. It allows you to specify build parameters for C and C++ programs in a cross platform manner. For cygwin Makefiles will be generated. CMake is also capable of generating microsoft project files, nmake, and borland makefiles. CMake can also perform system inspection operations like finding installed libraries and header files." 
prev: ${PREVIOUS_VERSION}-${PREVIOUS_RELEASE}
curr: ${VERSION}-${RELEASE}
EOF
        ) &&
        dos2unix cmake-${VERSION}/CYGWIN-PATCHES/setup.hint &&
        cp cmake-${VERSION}/CYGWIN-PATCHES/setup.hint ../setup.hint &&
        (diff -urN "../../cmake-${VERSION}" "cmake-${VERSION}" > "../cmake-${VERSION}-${RELEASE}.patch"; [ "$?" = "1" ])
    ) >Logs/cygwin_source_patch.log 2>&1 || error_log Logs/cygwin_source_patch.log
}

#-----------------------------------------------------------------------------
cygwin_package_script()
{
    [ -z "${DONE_cygwin_package_script}" ] || return 0 ; DONE_cygwin_package_script="yes"
    utilities || return 1
    echo "Creating cygwin packaging script ..." &&
    (
        mkdir -p Cygwin &&
        cp ReleaseUtilities/cygwin-package.sh.in Cygwin/cmake-${VERSION}-${RELEASE}.sh &&
        chmod u+x Cygwin/cmake-${VERSION}-${RELEASE}.sh
    ) >Logs/cygwin_package_script.log 2>&1 || error_log Logs/cygwin_package_script.log
}

#-----------------------------------------------------------------------------
# Create the CMake cygwin package files.
#
#  cygwin_package
#
# This command should be run from a cygwin prompt.
cygwin_package()
{
    [ -z "${DONE_cygwin_package}" ] || return 0 ; DONE_cygwin_package="yes"
    config || return 1
    [ -f "Cygwin/cmake-${VERSION}.tar.bz2" ] || cygwin_source_tarball || return 1
    [ -f "Cygwin/cmake-${VERSION}-${RELEASE}.patch" ] || cygwin_source_patch || return 1
    [ -f "Cygwin/cmake-${VERSION}-${RELEASE}.sh" ] || cygwin_package_script || return 1
    echo "Running cygwin packaging script ..." &&
    (
        rm -rf Cygwin/Package &&
        mkdir -p Cygwin/Package &&
        cd Cygwin/Package &&
        cp ../setup.hint . &&
        cp ../cmake-${VERSION}.tar.bz2 . &&
        cp ../cmake-${VERSION}-${RELEASE}.patch . &&
        cp ../cmake-${VERSION}-${RELEASE}.sh . &&
        ./cmake-${VERSION}-${RELEASE}.sh all
    ) >Logs/cygwin_package.log 2>&1 || error_log Logs/cygwin_package.log
}

#-----------------------------------------------------------------------------
# Upload the CMake cygwin package files.
#
#  cygwin_upload
#
# This should be run after "cygwin_package".
cygwin_upload()
{
    setup || return 1
    echo "------- Copying cywgin packages to www.cmake.org. -------"
    scp Cygwin/Package/cmake-${VERSION}-${RELEASE}-src.tar.bz2 \
        Cygwin/Package/cmake-${VERSION}-${RELEASE}.tar.bz2 \
        Cygwin/Package/setup.hint \
        kitware@www.cmake.org:/projects/FTP/pub/cmake/cygwin
    echo "---- Done copying cygwin packages to www.cmake.org. -----"
}

#-----------------------------------------------------------------------------
win32_zipfile()
{
    setup || return 1
    echo "Creating windows non-admin install zip file ..." &&
    (
        mkdir -p Win32 &&
        cd "c:/Program Files" &&
        rm -rf cmake-${VERSION}-x86-win.zip &&
        zip -r cmake-${VERSION}-x86-win.zip CMake \
            -x CMake/INSTALL.LOG -x CMake/UNWISE.EXE -x CMake/WiseUpdt.exe &&
        mv cmake-${VERSION}-x86-win.zip ${RELEASE_ROOT}/Win32
    ) >Logs/win32_zipfile.log 2>&1 || error_log Logs/win32_zipfile.log
}

#-----------------------------------------------------------------------------
win32_upload()
{
    setup || return 1
    echo "------- Copying windows zip file to www.cmake.org. -------"
    scp Win32/cmake-${VERSION}-x86-win.zip \
        kitware@www.cmake.org:/projects/FTP/pub/cmake
    echo "---- Done copying windows zip file to www.cmake.org. -----"
}

#-----------------------------------------------------------------------------
# Install CMake into the OSX package directory structure.
#
#  osx_install
#
# This will build CMake if it is not already built.
osx_install()
{
    [ -z "${DONE_osx_install}" ] || return 0 ; DONE_osx_install="yes"
    config || return 1
    [ -f "cmake-${VERSION}-${PLATFORM}/Source/ccmake" ] || build || return 1
    if [ -z "${WX_RESOURCES}" ]; then
        echo "${CONFIG_FILE} should specify WX_RESOURCES."
        return 1
    fi
    echo "Running make install for OSX package ..." &&
    (
        rm -rf OSX &&
        mkdir -p OSX/Package_Root/Applications &&
        mkdir -p OSX/Resources/PreFlight &&
        mkdir -p OSX/Resources/PostFlight &&
        (
            cd "cmake-${VERSION}-${PLATFORM}" &&
            make install DESTDIR="${RELEASE_ROOT}/OSX/Package_Root"
        ) &&
        cp cmake-${VERSION}/Copyright.txt OSX/Resources/License.txt &&
        cp -r cmake-${VERSION}-${PLATFORM}/Source/CMake.app OSX/Package_Root/Applications &&
        echo "APPL????" > OSX/Package_Root/Applications/CMake.app/Contents/PkgInfo &&
        cp "${WX_RESOURCES}" OSX/Package_Root/Applications/CMake.app/Contents/Resources/wxCMakeSetup.rsrc
    ) >Logs/osx_install.log 2>&1 || error_log Logs/osx_install.log
}

#-----------------------------------------------------------------------------
run()
{
    CMD="'$1'"; shift; for i in "$@"; do CMD="${CMD} '$i'"; done
    eval "$CMD"
}

# Determine task and evaluate it.
if [ -z "$TASK" ] && [ -z "$REMOTE" ] ; then
    if [ -z "$1" ]; then
        usage
    else
        run "$@"
    fi
else
    [ -z "$TASK" ] || eval run "$TASK"
fi
