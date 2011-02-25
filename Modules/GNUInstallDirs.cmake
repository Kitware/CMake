# - Define GNU standard installation directories
# Provides install directory variables as defined for GNU software:
#  http://www.gnu.org/prep/standards/html_node/Directory-Variables.html
# Inclusion of this module defines the following variables:
#  CMAKE_INSTALL_<dir>      - destination for files of a given type
#  CMAKE_INSTALL_FULL_<dir> - corresponding absolute path
# where <dir> is one of:
#  BINDIR           - user executables (bin)
#  SBINDIR          - system admin executables (sbin)
#  LIBEXECDIR       - program executables (libexec)
#  SYSCONFDIR       - read-only single-machine data (etc)
#  SHAREDSTATEDIR   - modifiable architecture-independent data (com)
#  LOCALSTATEDIR    - modifiable single-machine data (var)
#  LIBDIR           - object code libraries (lib)
#  INCLUDEDIR       - C header files (include)
#  OLDINCLUDEDIR    - C header files for non-gcc (/usr/include)
#  DATAROOTDIR      - read-only architecture-independent data root (share)
#  DATADIR          - read-only architecture-independent data (DATAROOTDIR)
#  INFODIR          - info documentation (DATAROOTDIR/info)
#  LOCALEDIR        - locale-dependent data (DATAROOTDIR/locale)
#  MANDIR           - man documentation (DATAROOTDIR/man)
#  DOCDIR           - documentation root (DATAROOTDIR/doc/PROJECT_NAME)
# Each CMAKE_INSTALL_<dir> value may be passed to the DESTINATION options of
# install() commands for the corresponding file type.  If the includer does
# not define a value the above-shown default will be used and the value will
# appear in the cache for editing by the user.
# Each CMAKE_INSTALL_FULL_<dir> value contains an absolute path constructed
# from the corresponding destination by prepending (if necessary) the value
# of CMAKE_INSTALL_PREFIX.

#=============================================================================
# Copyright 2011 Nikita Krupen'ko <krnekit@gmail.com>
# Copyright 2011 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Installation directories
#
if(NOT DEFINED CMAKE_INSTALL_BINDIR)
  set(CMAKE_INSTALL_BINDIR "bin" CACHE PATH "user executables (bin)")
endif()

if(NOT DEFINED CMAKE_INSTALL_SBINDIR)
  set(CMAKE_INSTALL_SBINDIR "sbin" CACHE PATH "system admin executables (sbin)")
endif()

if(NOT DEFINED CMAKE_INSTALL_LIBEXECDIR)
  set(CMAKE_INSTALL_LIBEXECDIR "libexec" CACHE PATH "program executables (libexec)")
endif()

if(NOT DEFINED CMAKE_INSTALL_SYSCONFDIR)
  set(CMAKE_INSTALL_SYSCONFDIR "etc" CACHE PATH "read-only single-machine data (etc)")
endif()

if(NOT DEFINED CMAKE_INSTALL_SHAREDSTATEDIR)
  set(CMAKE_INSTALL_SHAREDSTATEDIR "com" CACHE PATH "modifiable architecture-independent data (com)")
endif()

if(NOT DEFINED CMAKE_INSTALL_LOCALSTATEDIR)
  set(CMAKE_INSTALL_LOCALSTATEDIR "var" CACHE PATH "modifiable single-machine data (var)")
endif()

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "object code libraries (lib)")
endif()

if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
  set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE PATH "C header files (include)")
endif()

if(NOT DEFINED CMAKE_INSTALL_OLDINCLUDEDIR)
  set(CMAKE_INSTALL_OLDINCLUDEDIR "/usr/include" CACHE PATH "C header files for non-gcc (/usr/include)")
endif()

if(NOT DEFINED CMAKE_INSTALL_DATAROOTDIR)
  set(CMAKE_INSTALL_DATAROOTDIR "share" CACHE PATH "read-only architecture-independent data root (share)")
endif()

if(NOT DEFINED CMAKE_INSTALL_DATADIR)
  set(CMAKE_INSTALL_DATADIR "${CMAKE_INSTALL_DATAROOTDIR}" CACHE PATH "read-only architecture-independent data (DATAROOTDIR)")
endif()

if(NOT DEFINED CMAKE_INSTALL_INFODIR)
  set(CMAKE_INSTALL_INFODIR "${CMAKE_INSTALL_DATAROOTDIR}/info" CACHE PATH "info documentation (DATAROOTDIR/info)")
endif()

if(NOT DEFINED CMAKE_INSTALL_LOCALEDIR)
  set(CMAKE_INSTALL_LOCALEDIR "${CMAKE_INSTALL_DATAROOTDIR}/locale" CACHE PATH "locale-dependent data (DATAROOTDIR/locale)")
endif()

if(NOT DEFINED CMAKE_INSTALL_MANDIR)
  set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_DATAROOTDIR}/man" CACHE PATH "man documentation (DATAROOTDIR/man)")
endif()

if(NOT DEFINED CMAKE_INSTALL_DOCDIR)
  set(CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}" CACHE PATH "documentation root (DATAROOTDIR/doc/PROJECT_NAME)")
endif()

mark_as_advanced(
  CMAKE_INSTALL_BINDIR
  CMAKE_INSTALL_SBINDIR
  CMAKE_INSTALL_LIBEXECDIR
  CMAKE_INSTALL_SYSCONFDIR
  CMAKE_INSTALL_SHAREDSTATEDIR
  CMAKE_INSTALL_LOCALSTATEDIR
  CMAKE_INSTALL_LIBDIR
  CMAKE_INSTALL_INCLUDEDIR
  CMAKE_INSTALL_OLDINCLUDEDIR
  CMAKE_INSTALL_DATAROOTDIR
  CMAKE_INSTALL_DATADIR
  CMAKE_INSTALL_INFODIR
  CMAKE_INSTALL_LOCALEDIR
  CMAKE_INSTALL_MANDIR
  CMAKE_INSTALL_DOCDIR
  )

# Result directories
#
foreach(dir
    BINDIR
    SBINDIR
    LIBEXECDIR
    SYSCONFDIR
    SHAREDSTATEDIR
    LOCALSTATEDIR
    LIBDIR
    INCLUDEDIR
    OLDINCLUDEDIR
    DATAROOTDIR
    DATADIR
    INFODIR
    LOCALEDIR
    MANDIR
    DOCDIR
    )
  if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_${dir}})
    set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_${dir}}")
  else()
    set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_${dir}}")
  endif()
endforeach()
