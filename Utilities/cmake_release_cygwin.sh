#!/bin/sh
#
# CMake Cygwin package creation script.  Run this in an empty
# directory from a separate CMake checkout.
#

CVS_TAG="-r Release-1-4"
PKG=cmake
VER=1.4.5
REL=1

CVSROOT=":pserver:anonymous@www.cmake.org:/cvsroot/CMake"
FULLPKG="${PKG}-${VER}-${REL}"

SELF_DIR=`cd \`echo "$0" | sed -n '/\//{s/\/[^\/]*$//;p;}'\`;pwd`

WriteREADME()
{
CYGVERSION=`uname -r`
cat > ${PKG}-${VER}/CYGWIN-PATCHES/cmake.README <<EOF
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
  unpack ${FULLPKG}-src.tar.bz2
    if you use setup to install this src package, it will be
	 unpacked under /usr/src automatically
  cd /usr/src
  ./${FULLPKG}.sh all

This will create:
  /usr/src/${FULLPKG}.tar.bz2
  /usr/src/${FULLPKG}-src.tar.bz2

-------------------------------------------

Port Notes:

<none>

------------------

Cygwin port maintained by: CMake Developers <cmake@www.cmake.org>

EOF
}

WriteSetupHint()
{
cat > ${PKG}-${VER}/CYGWIN-PATCHES/setup.hint <<EOF
# CMake setup.hint file for cygwin setup.exe program
category: Devel 
requires: libncurses5 cygwin 
sdesc: "A cross platform build manger" 
ldesc: "CMake is a cross platform build manager. It allows you to specify build parameters for C and C++ programs in a cross platform manner. For cygwin Makefiles will be generated. CMake is also capable of generating microsoft project files, nmake, and borland makefiles. CMake can also perform system inspection operations like finding installed libraries and header files." 
curr: ${VER}-${REL}
EOF
}

SourceTarball()
{
  cvs -z3 -d ${CVSROOT} export ${CVS_TAG} CMake &&
  mv CMake ${PKG}-${VER} &&
  tar cvjf ${PKG}-${VER}.tar.bz2 ${PKG}-${VER}
}

SourcePatch()
{
  mv ${PKG}-${VER} ${PKG}-${VER}-orig &&
  tar xvjf ${PKG}-${VER}.tar.bz2 &&
  mkdir -p ${PKG}-${VER}/CYGWIN-PATCHES &&
  WriteREADME &&
  WriteSetupHint &&
  (diff -urN "${PKG}-${VER}-orig" "${PKG}-${VER}" > "${FULLPKG}.patch")
  rm -rf ${PKG}-${VER} ${PKG}-${VER}-orig
}

CygwinScript()
{
  cp ${SELF_DIR}/cmake-cygwin-package.sh ./${FULLPKG}.sh
  chmod u+x ./${FULLPKG}.sh
}

Package()
{
  ./${FULLPKG}.sh all
}

SourceTarball && SourcePatch && CygwinScript && Package
