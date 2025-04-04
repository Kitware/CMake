# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
GNUInstallDirs
--------------

Define GNU standard installation directories

Provides install directory variables as defined by the
`GNU Coding Standards`_.

.. _`GNU Coding Standards`: https://www.gnu.org/prep/standards/html_node/Directory-Variables.html

Result Variables
^^^^^^^^^^^^^^^^

Inclusion of this module defines the following variables:

``CMAKE_INSTALL_<dir>``

  Destination for files of a given type.  This value may be passed to
  the ``DESTINATION`` options of  :command:`install` commands for the
  corresponding file type.  It should be a path relative to the installation
  prefix so that it can be converted to an absolute path in a relocatable way.

  While absolute paths are allowed, they are not recommended as they
  do not work with the ``cmake --install`` command's
  :option:`--prefix <cmake--install --prefix>` option, or with the
  :manual:`cpack <cpack(1)>` installer generators. In particular, there is no
  need to make paths absolute by prepending :variable:`CMAKE_INSTALL_PREFIX`;
  this prefix is used by default if the DESTINATION is a relative path.

``CMAKE_INSTALL_FULL_<dir>``

  The absolute path generated from the corresponding ``CMAKE_INSTALL_<dir>``
  value.  If the value is not already an absolute path, an absolute path
  is constructed typically by prepending the value of the
  :variable:`CMAKE_INSTALL_PREFIX` variable.  However, there are some
  `special cases`_ as documented below.

  These variables shouldn't be used in :command:`install` commands
  as they do not work with the ``cmake --install`` command's
  :option:`--prefix <cmake--install --prefix>` option, or with the
  :manual:`cpack <cpack(1)>` installer generators.

where ``<dir>`` is one of:

``BINDIR``
  user executables (``bin``)
``SBINDIR``
  system admin executables (``sbin``)
``LIBEXECDIR``
  program executables (``libexec``)
``SYSCONFDIR``
  read-only single-machine data (``etc``)
``SHAREDSTATEDIR``
  modifiable architecture-independent data (``com``)
``LOCALSTATEDIR``
  modifiable single-machine data (``var``)
``RUNSTATEDIR``
  .. versionadded:: 3.9
    run-time variable data (``LOCALSTATEDIR/run``)
``LIBDIR``
  object code libraries (``lib`` or ``lib64``)

  On Debian, this may be ``lib/<multiarch-tuple>`` when
  :variable:`CMAKE_INSTALL_PREFIX` is ``/usr``.
``INCLUDEDIR``
  C header files (``include``)
``OLDINCLUDEDIR``
  C header files for non-gcc (``/usr/include``)
``DATAROOTDIR``
  read-only architecture-independent data root (``share``)
``DATADIR``
  read-only architecture-independent data (``DATAROOTDIR``)
``INFODIR``
  info documentation (``DATAROOTDIR/info``)
``LOCALEDIR``
  locale-dependent data (``DATAROOTDIR/locale``)
``MANDIR``
  man documentation (``DATAROOTDIR/man``)
``DOCDIR``
  documentation root (``DATAROOTDIR/doc/PROJECT_NAME``)

If the includer does not define a value the above-shown default will be
used and the value will appear in the cache for editing by the user.

Special Cases
^^^^^^^^^^^^^

.. versionadded:: 3.4

The following values of :variable:`CMAKE_INSTALL_PREFIX` are special:

``/``

  For ``<dir>`` other than the ``SYSCONFDIR``, ``LOCALSTATEDIR`` and
  ``RUNSTATEDIR``, the value of ``CMAKE_INSTALL_<dir>`` is prefixed
  with ``usr/`` if it is not user-specified as an absolute path.
  For example, the ``INCLUDEDIR`` value ``include`` becomes ``usr/include``.
  This is required by the `GNU Coding Standards`_, which state:

    When building the complete GNU system, the prefix will be empty
    and ``/usr`` will be a symbolic link to ``/``.

``/usr``

  For ``<dir>`` equal to ``SYSCONFDIR``, ``LOCALSTATEDIR`` or
  ``RUNSTATEDIR``, the ``CMAKE_INSTALL_FULL_<dir>`` is computed by
  prepending just ``/`` to the value of ``CMAKE_INSTALL_<dir>``
  if it is not user-specified as an absolute path.
  For example, the ``SYSCONFDIR`` value ``etc`` becomes ``/etc``.
  This is required by the `GNU Coding Standards`_.

``/opt/...``

  For ``<dir>`` equal to ``SYSCONFDIR``, ``LOCALSTATEDIR`` or
  ``RUNSTATEDIR``, the ``CMAKE_INSTALL_FULL_<dir>`` is computed by
  *appending* the prefix to the value of ``CMAKE_INSTALL_<dir>``
  if it is not user-specified as an absolute path.
  For example, the ``SYSCONFDIR`` value ``etc`` becomes ``/etc/opt/...``.
  This is defined by the `Filesystem Hierarchy Standard`_.

  This behavior does not apply to paths under ``/opt/homebrew/...``.

.. _`Filesystem Hierarchy Standard`: https://refspecs.linuxfoundation.org/FHS_3.0/fhs/index.html

Macros
^^^^^^

.. command:: GNUInstallDirs_get_absolute_install_dir

  .. code-block:: cmake

    GNUInstallDirs_get_absolute_install_dir(absvar var dirname)

  .. versionadded:: 3.7

  Set the given variable ``absvar`` to the absolute path contained
  within the variable ``var``.  This is to allow the computation of an
  absolute path, accounting for all the special cases documented
  above.  While this macro is used to compute the various
  ``CMAKE_INSTALL_FULL_<dir>`` variables, it is exposed publicly to
  allow users who create additional path variables to also compute
  absolute paths where necessary, using the same logic.  ``dirname`` is
  the directory name to get, e.g. ``BINDIR``.

  .. versionchanged:: 3.20
    Added the ``<dirname>`` parameter.  Previous versions of CMake passed
    this value through the variable ``${dir}``.
#]=======================================================================]

cmake_policy(SET CMP0140 NEW)

# Convert a cache variable to PATH type

function(_GNUInstallDirs_cache_convert_to_path var description)
  get_property(cache_type CACHE ${var} PROPERTY TYPE)
  if(cache_type STREQUAL "UNINITIALIZED")
    file(TO_CMAKE_PATH "${${var}}" cmakepath)
    set_property(CACHE ${var} PROPERTY TYPE PATH)
    set_property(CACHE ${var} PROPERTY VALUE "${cmakepath}")
    set_property(CACHE ${var} PROPERTY HELPSTRING "${description}")
  endif()
endfunction()

# Create a cache variable with default for a path.
function(_GNUInstallDirs_cache_path var description)
  set(cmake_install_var "CMAKE_INSTALL_${var}")
  set(default "${_GNUInstallDirs_${var}_DEFAULT}")
  set(full_description "${description}")
  if(NOT DEFINED ${cmake_install_var})
    set(${cmake_install_var} "${default}" CACHE PATH "${full_description}")
  endif()
  _GNUInstallDirs_cache_convert_to_path("${cmake_install_var}" "${full_description}")
endfunction()

# Create a cache variable with not default for a path, with a fallback
# when unset; used for entries slaved to other entries such as
# DATAROOTDIR.
function(_GNUInstallDirs_cache_path_fallback var description)
  set(cmake_install_var "CMAKE_INSTALL_${var}")
  set(default "${_GNUInstallDirs_${var}_DEFAULT}")
  if(NOT ${cmake_install_var})
    set(${cmake_install_var} "" CACHE PATH "${description}")
    set(${cmake_install_var} "${default}")
  endif()
  _GNUInstallDirs_cache_convert_to_path("${cmake_install_var}" "${description}")
  return(PROPAGATE ${cmake_install_var})
endfunction()

# Installation directories
#

# Set the standard default values before any special handling
set(_GNUInstallDirs_BINDIR_DEFAULT "bin")
set(_GNUInstallDirs_SBINDIR_DEFAULT "sbin")
set(_GNUInstallDirs_LIBEXECDIR_DEFAULT "libexec")
set(_GNUInstallDirs_SYSCONFDIR_DEFAULT "etc")
set(_GNUInstallDirs_SHAREDSTATEDIR_DEFAULT "com")
set(_GNUInstallDirs_LOCALSTATEDIR_DEFAULT "var")
set(_GNUInstallDirs_LIBDIR_DEFAULT "lib")
set(_GNUInstallDirs_INCLUDEDIR_DEFAULT "include")
set(_GNUInstallDirs_OLDINCLUDEDIR_DEFAULT "/usr/include")
set(_GNUInstallDirs_DATAROOTDIR_DEFAULT "share")

# We check if the variable was manually set and not cached, in order to
# allow projects to set the values as normal variables before including
# GNUInstallDirs to avoid having the entries cached or user-editable. It
# replaces the "if(NOT DEFINED CMAKE_INSTALL_XXX)" checks in all the
# other cases.
# If CMAKE_INSTALL_LIBDIR is defined, if _libdir_set is false, then the
# variable is a normal one, otherwise it is a cache one.
get_property(_libdir_set CACHE CMAKE_INSTALL_LIBDIR PROPERTY TYPE SET)
if(NOT DEFINED CMAKE_INSTALL_LIBDIR OR (_libdir_set
    AND DEFINED _GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX
    AND NOT "${_GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX}" STREQUAL "${CMAKE_INSTALL_PREFIX}"))
  # If CMAKE_INSTALL_LIBDIR is not defined, it is always executed.
  # Otherwise:
  #  * if _libdir_set is false it is not executed (meaning that it is
  #    not a cache variable)
  #  * if _GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX is not defined it is
  #    not executed
  #  * if _GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX and
  #    CMAKE_INSTALL_PREFIX are the same string it is not executed.
  #    _GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX is updated after the
  #    execution, of this part of code, therefore at the next inclusion
  #    of the file, CMAKE_INSTALL_LIBDIR is defined, and the 2 strings
  #    are equal, meaning that the if is not executed the code the
  #    second time.

  set(_LIBDIR_DEFAULT "lib")
  # Override this default 'lib' with 'lib64' iff:
  #  - we are on Linux system but NOT cross-compiling
  #  - we are NOT on debian
  #  - we are NOT building for conda
  #  - we are on a 64 bits system
  # reason is: amd64 ABI: https://github.com/hjl-tools/x86-psABI/wiki/X86-psABI
  # For Debian with multiarch, use 'lib/${CMAKE_LIBRARY_ARCHITECTURE}' if
  # CMAKE_LIBRARY_ARCHITECTURE is set (which contains e.g. "i386-linux-gnu"
  # and CMAKE_INSTALL_PREFIX is "/usr"
  # See http://wiki.debian.org/Multiarch
  if(DEFINED _GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX)
    set(__LAST_LIBDIR_DEFAULT "lib")
    # __LAST_LIBDIR_DEFAULT is the default value that we compute from
    # _GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX, not a cache entry for
    # the value that was last used as the default.
    # This value is used to figure out whether the user changed the
    # CMAKE_INSTALL_LIBDIR value manually, or if the value was the
    # default one. When CMAKE_INSTALL_PREFIX changes, the value is
    # updated to the new default, unless the user explicitly changed it.
  endif()
  if (NOT DEFINED CMAKE_SYSTEM_NAME OR NOT DEFINED CMAKE_SIZEOF_VOID_P)
    message(AUTHOR_WARNING
      "Unable to determine default CMAKE_INSTALL_LIBDIR directory because no target architecture is known. "
      "Please enable at least one language before including GNUInstallDirs.")
  endif()

  if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|GNU)$"
      AND NOT CMAKE_CROSSCOMPILING)
    unset(__system_type_for_install)
    if(DEFINED ENV{CONDA_BUILD} AND DEFINED ENV{PREFIX})
      set(conda_prefix "$ENV{PREFIX}")
      cmake_path(ABSOLUTE_PATH conda_prefix NORMALIZE)
      if("${CMAKE_INSTALL_PREFIX}" STREQUAL conda_prefix)
        set(__system_type_for_install "conda")
      endif()
    elseif(DEFINED ENV{CONDA_PREFIX})
      set(conda_prefix "$ENV{CONDA_PREFIX}")
      cmake_path(ABSOLUTE_PATH conda_prefix NORMALIZE)
      if("${CMAKE_INSTALL_PREFIX}" STREQUAL conda_prefix AND
         NOT ("${CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/?$" OR
              "${CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/local/?$"))
        set(__system_type_for_install "conda")
      endif()
    endif()
    if(NOT __system_type_for_install)
      if (EXISTS "/etc/alpine-release")
        set(__system_type_for_install "alpine")
      elseif (EXISTS "/etc/arch-release")
        set(__system_type_for_install "arch linux")
      elseif (EXISTS "/etc/debian_version")
        set(__system_type_for_install "debian")
      endif()
    endif()

    if(__system_type_for_install STREQUAL "debian")
      if(CMAKE_LIBRARY_ARCHITECTURE)
        if("${CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/?$")
          set(_LIBDIR_DEFAULT "lib/${CMAKE_LIBRARY_ARCHITECTURE}")
        endif()
        if(DEFINED _GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX
            AND "${_GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/?$")
          set(__LAST_LIBDIR_DEFAULT "lib/${CMAKE_LIBRARY_ARCHITECTURE}")
        endif()
      endif()
    elseif(NOT DEFINED __system_type_for_install)
      # not debian, alpine, arch, or conda so rely on CMAKE_SIZEOF_VOID_P:
      if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        set(_LIBDIR_DEFAULT "lib64")
        if(DEFINED _GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX)
          set(__LAST_LIBDIR_DEFAULT "lib64")
        endif()
      endif()
    endif()
  endif()
  unset(__system_type_for_install)

  if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "${_LIBDIR_DEFAULT}" CACHE PATH "Object code libraries (${_LIBDIR_DEFAULT})")
  elseif(DEFINED __LAST_LIBDIR_DEFAULT
      AND "${__LAST_LIBDIR_DEFAULT}" STREQUAL "${CMAKE_INSTALL_LIBDIR}")
    set_property(CACHE CMAKE_INSTALL_LIBDIR PROPERTY VALUE "${_LIBDIR_DEFAULT}")
  endif()
endif()
_GNUInstallDirs_cache_convert_to_path(CMAKE_INSTALL_LIBDIR "Object code libraries (lib)")

# Save for next run
set(_GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL "CMAKE_INSTALL_PREFIX during last run")
unset(_libdir_set)
unset(__LAST_LIBDIR_DEFAULT)

_GNUInstallDirs_cache_path(BINDIR
  "User executables (bin)")
_GNUInstallDirs_cache_path(SBINDIR
  "System admin executables (sbin)")
_GNUInstallDirs_cache_path(LIBEXECDIR
  "Program executables (libexec)")
_GNUInstallDirs_cache_path(SYSCONFDIR
  "Read-only single-machine data (etc)")
_GNUInstallDirs_cache_path(SHAREDSTATEDIR
  "Modifiable architecture-independent data (com)")
_GNUInstallDirs_cache_path(LOCALSTATEDIR
  "Modifiable single-machine data (var)")

_GNUInstallDirs_cache_path(INCLUDEDIR
  "C header files (include)")
_GNUInstallDirs_cache_path(OLDINCLUDEDIR
  "C header files for non-gcc (/usr/include)")
_GNUInstallDirs_cache_path(DATAROOTDIR
  "Read-only architecture-independent data root (share)")

#-----------------------------------------------------------------------------
# Values whose defaults are relative to DATAROOTDIR.  Store empty values in
# the cache and store the defaults in local variables if the cache values are
# not set explicitly.  This auto-updates the defaults as DATAROOTDIR changes.

if(CMAKE_SYSTEM_NAME MATCHES "^(([^kF].*)?BSD|DragonFly)$")
  set(_GNUInstallDirs_INFODIR_DEFAULT "info")
  _GNUInstallDirs_cache_path(INFODIR
    "Info documentation (info)")
else()
  set(_GNUInstallDirs_INFODIR_DEFAULT "${CMAKE_INSTALL_DATAROOTDIR}/info")
  _GNUInstallDirs_cache_path_fallback(INFODIR
    "Info documentation (DATAROOTDIR/info)")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "^(([^k].*)?BSD|DragonFly)$" AND NOT CMAKE_SYSTEM_NAME MATCHES "^(FreeBSD)$")
  set(_GNUInstallDirs_MANDIR_DEFAULT "man")
  _GNUInstallDirs_cache_path(MANDIR
    "Man documentation (man)")
else()
  set(_GNUInstallDirs_MANDIR_DEFAULT "${CMAKE_INSTALL_DATAROOTDIR}/man")
  _GNUInstallDirs_cache_path_fallback(MANDIR
    "Man documentation (DATAROOTDIR/man)")
endif()

set(_GNUInstallDirs_DATADIR_DEFAULT "${CMAKE_INSTALL_DATAROOTDIR}")
set(_GNUInstallDirs_LOCALEDIR_DEFAULT "${CMAKE_INSTALL_DATAROOTDIR}/locale")
set(_GNUInstallDirs_DOCDIR_DEFAULT "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}")
set(_GNUInstallDirs_RUNSTATEDIR_DEFAULT "${CMAKE_INSTALL_LOCALSTATEDIR}/run")

_GNUInstallDirs_cache_path_fallback(DATADIR
  "Read-only architecture-independent data (DATAROOTDIR)")
_GNUInstallDirs_cache_path_fallback(LOCALEDIR
  "Locale-dependent data (DATAROOTDIR/locale)")
_GNUInstallDirs_cache_path_fallback(DOCDIR
  "Documentation root (DATAROOTDIR/doc/PROJECT_NAME)")
_GNUInstallDirs_cache_path_fallback(RUNSTATEDIR
  "Run-time variable data (LOCALSTATEDIR/run)")

# Unset all the defaults used
foreach(dir IN ITEMS
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
    MANDIR
    LOCALEDIR
    DOCDIR
    RUNSTATEDIR
)
  unset(_GNUInstallDirs_${dir}_DEFAULT)
endforeach()

#-----------------------------------------------------------------------------

mark_as_advanced(
  CMAKE_INSTALL_BINDIR
  CMAKE_INSTALL_SBINDIR
  CMAKE_INSTALL_LIBEXECDIR
  CMAKE_INSTALL_SYSCONFDIR
  CMAKE_INSTALL_SHAREDSTATEDIR
  CMAKE_INSTALL_LOCALSTATEDIR
  CMAKE_INSTALL_RUNSTATEDIR
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

macro(GNUInstallDirs_get_absolute_install_dir absvar var)
  set(GGAID_extra_args ${ARGN})
  list(LENGTH GGAID_extra_args GGAID_extra_arg_count)
  if(GGAID_extra_arg_count GREATER "0")
    list(GET GGAID_extra_args 0 GGAID_dir)
  else()
    # Historical behavior: use ${dir} from caller's scope
    set(GGAID_dir "${dir}")
    message(AUTHOR_WARNING
      "GNUInstallDirs_get_absolute_install_dir called without third argument. "
      "Using \${dir} from the caller's scope for compatibility with CMake 3.19 and below.")
  endif()

  if(NOT IS_ABSOLUTE "${${var}}")
    # Handle special cases:
    # - CMAKE_INSTALL_PREFIX == /
    # - CMAKE_INSTALL_PREFIX == /usr
    # - CMAKE_INSTALL_PREFIX == /opt/...
    if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/")
      if("${GGAID_dir}" STREQUAL "SYSCONFDIR" OR "${GGAID_dir}" STREQUAL "LOCALSTATEDIR" OR "${GGAID_dir}" STREQUAL "RUNSTATEDIR")
        set(${absvar} "/${${var}}")
      else()
        if (NOT "${${var}}" MATCHES "^usr/")
          set(${var} "usr/${${var}}")
        endif()
        set(${absvar} "/${${var}}")
      endif()
    elseif("${CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/?$")
      if("${GGAID_dir}" STREQUAL "SYSCONFDIR" OR "${GGAID_dir}" STREQUAL "LOCALSTATEDIR" OR "${GGAID_dir}" STREQUAL "RUNSTATEDIR")
        set(${absvar} "/${${var}}")
      else()
        set(${absvar} "${CMAKE_INSTALL_PREFIX}/${${var}}")
      endif()
    elseif("${CMAKE_INSTALL_PREFIX}" MATCHES "^/opt/" AND NOT "${CMAKE_INSTALL_PREFIX}" MATCHES "^/opt/homebrew/")
      if("${GGAID_dir}" STREQUAL "SYSCONFDIR" OR "${GGAID_dir}" STREQUAL "LOCALSTATEDIR" OR "${GGAID_dir}" STREQUAL "RUNSTATEDIR")
        set(${absvar} "/${${var}}${CMAKE_INSTALL_PREFIX}")
      else()
        set(${absvar} "${CMAKE_INSTALL_PREFIX}/${${var}}")
      endif()
    else()
      set(${absvar} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    endif()
  else()
    set(${absvar} "${${var}}")
  endif()

  unset(GGAID_dir)
  unset(GGAID_extra_arg_count)
  unset(GGAID_extra_args)
endmacro()

# Result directories
#
foreach(dir
    BINDIR
    SBINDIR
    LIBEXECDIR
    SYSCONFDIR
    SHAREDSTATEDIR
    LOCALSTATEDIR
    RUNSTATEDIR
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
  GNUInstallDirs_get_absolute_install_dir(CMAKE_INSTALL_FULL_${dir} CMAKE_INSTALL_${dir} ${dir})
endforeach()
