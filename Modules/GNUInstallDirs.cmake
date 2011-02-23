# - Define the installation directories conforming to GNU standards.
# This module defines installation directories, as described by GNU standards.
# For more information about this standards, see the following site:
# http://www.gnu.org/prep/standards/html_node/Directory-Variables.html
#
# Module defines variables for installation directories and for resulting paths
# of this directories.
#
# Installation directories appointed to use during install. If such variable
# contains relative path, during install resulting path will be created by
# prepending it with CMAKE_INSTALL_PREFIX. If it contains absolute path, it
# would be taken as is.
# This variables only set if not defined already, thus allowing to override them
# and are stored in cache.
#   CMAKE_INSTALL_BINDIR              - user executables (bin)
#   CMAKE_INSTALL_SBINDIR             - system admin executables (sbin)
#   CMAKE_INSTALL_LIBEXECDIR          - program executables (libexec)
#   CMAKE_INSTALL_SYSCONFDIR          - read-only single-machine data (etc)
#   CMAKE_INSTALL_SHAREDSTATEDIR      - modifiable architecture-independent data (com)
#   CMAKE_INSTALL_LOCALSTATEDIR       - modifiable single-machine data (var)
#   CMAKE_INSTALL_LIBDIR              - object code libraries (lib)
#   CMAKE_INSTALL_INCLUDEDIR          - C header files (include)
#   CMAKE_INSTALL_OLDINCLUDEDIR       - C header files for non-gcc (/usr/include)
#   CMAKE_INSTALL_DATAROOTDIR         - read-only architecture-independent data root (share)
#   CMAKE_INSTALL_DATADIR             - read-only architecture-independent data (DATAROOTDIR)
#   CMAKE_INSTALL_INFODIR             - info documentation (DATAROOTDIR/info)
#   CMAKE_INSTALL_LOCALEDIR           - locale-dependent data (DATAROOTDIR/locale)
#   CMAKE_INSTALL_MANDIR              - man documentation (DATAROOTDIR/man)
#   CMAKE_INSTALL_DOCDIR              - documentation root (DATAROOTDIR/doc/PROJECT_NAME)
# Resulting directories derived from installation directories and always
# contains absolute paths.
# This variables computed in runtime and not stored in cache.
#   CMAKE_INSTALL_FULL_BINDIR         - user executables full path (PREFIX/BINDIR)
#   CMAKE_INSTALL_FULL_SBINDIR        - system admin executables full path (PREFIX/SBINDIR)
#   CMAKE_INSTALL_FULL_LIBEXECDIR     - program executables full path (PREFIX/LIBEXECDIR)
#   CMAKE_INSTALL_FULL_SYSCONFDIR     - read-only single-machine data fill path (PREFIX/SYSCONFDIR)
#   CMAKE_INSTALL_FULL_SHAREDSTATEDIR - modifiable architecture-independent data full path (PREFIX/SHAREDSTATEDIR)
#   CMAKE_INSTALL_FULL_LOCALSTATEDIR  - modifiable single-machine data full path (PREFIX/LOCALSTATEDIR)
#   CMAKE_INSTALL_FULL_LIBDIR         - object code libraries full path (PREFIX/LIBDIR)
#   CMAKE_INSTALL_FULL_INCLUDEDIR     - C header files full path (PREFIX/INCLUDEDIR)
#   CMAKE_INSTALL_FULL_OLDINCLUDEDIR  - C header files for non-gcc full path (/usr/include)
#   CMAKE_INSTALL_FULL_DATAROOTDIR    - read-only architecture-independent data root full path (PREFIX/DATAROOTDIR)
#   CMAKE_INSTALL_FULL_DATADIR        - read-only architecture-independent data full path (PREFIX/DATADIR)
#   CMAKE_INSTALL_FULL_INFODIR        - info documentation full path (PREFIX/INFODIR)
#   CMAKE_INSTALL_FULL_LOCALEDIR      - locale-dependent data full path (PREFIX/LOCALEDIR)
#   CMAKE_INSTALL_FULL_MANDIR         - man documentation full path (PREFIX/MANDIR)
#   CMAKE_INSTALL_FULL_DOCDIR         - documentation root full path (PREFIX/DOCDIR)

#=============================================================================
# Copyright 2011 Nikita Krupen'ko <krnekit@gmail.com>
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
