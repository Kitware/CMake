# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindGnuTLS
----------

Finds the GNU Transport Layer Security library (GnuTLS).  The GnuTLS
package includes the main libraries (libgnutls and libdane), as well as the
optional gnutls-openssl compatibility extra library.  They are all distributed
as part of the same release.  This module checks for the presence of the main
libgnutls library and provides usage requirements for integrating GnuTLS into
CMake projects.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following :ref:`Imported Targets`:

``GnuTLS::GnuTLS``
  .. versionadded:: 3.16

  Target encapsulating the GnuTLS usage requirements, available if GnuTLS is
  found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``GnuTLS_FOUND``
  Boolean indicating whether the (requested version of) GnuTLS is found.  For
  backward compatibility, the ``GNUTLS_FOUND`` variable is also set to the same
  value.

``GNUTLS_VERSION``
  .. versionadded:: 3.16

  The version of GnuTLS found.

``GNUTLS_INCLUDE_DIRS``
  Include directories needed to use GnuTLS.

``GNUTLS_LIBRARIES``
  Libraries needed to link against to use GnuTLS.

``GNUTLS_DEFINITIONS``
  Compiler options required for using GnuTLS.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``GNUTLS_INCLUDE_DIR``
  The directory containing the ``gnutls/gnutls.h`` header file.

``GNUTLS_LIBRARY``
  The path to the GnuTLS library.

Deprecated Variables
^^^^^^^^^^^^^^^^^^^^

These variables are provided for backward compatibility:

``GNUTLS_VERSION_STRING``
  .. deprecated:: 3.16
    Superseded by ``GNUTLS_VERSION``.

  The version of GnuTLS found.

Examples
^^^^^^^^

Finding GnuTLS and linking it to a project target:

.. code-block:: cmake

  find_package(GnuTLS)
  target_link_libraries(project_target PRIVATE GnuTLS::GnuTLS)
#]=======================================================================]

if (GNUTLS_INCLUDE_DIR AND GNUTLS_LIBRARY)
  # in cache already
  set(gnutls_FIND_QUIETLY TRUE)
endif ()

if (NOT WIN32)
  # try using pkg-config to get the directories and then use these values
  # in the find_path() and find_library() calls
  # also fills in GNUTLS_DEFINITIONS, although that isn't normally useful
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_GNUTLS QUIET gnutls)
  endif()
  set(GNUTLS_DEFINITIONS ${PC_GNUTLS_CFLAGS_OTHER})
  set(GNUTLS_VERSION ${PC_GNUTLS_VERSION})
  # keep for backward compatibility
  set(GNUTLS_VERSION_STRING ${PC_GNUTLS_VERSION})
endif ()

find_path(GNUTLS_INCLUDE_DIR gnutls/gnutls.h
  HINTS
    ${PC_GNUTLS_INCLUDEDIR}
    ${PC_GNUTLS_INCLUDE_DIRS}
  )

find_library(GNUTLS_LIBRARY NAMES gnutls libgnutls
  HINTS
    ${PC_GNUTLS_LIBDIR}
    ${PC_GNUTLS_LIBRARY_DIRS}
  )

mark_as_advanced(GNUTLS_INCLUDE_DIR GNUTLS_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GnuTLS
                                  REQUIRED_VARS GNUTLS_LIBRARY GNUTLS_INCLUDE_DIR
                                  VERSION_VAR GNUTLS_VERSION_STRING)

if(GnuTLS_FOUND)
  set(GNUTLS_LIBRARIES    ${GNUTLS_LIBRARY})
  set(GNUTLS_INCLUDE_DIRS ${GNUTLS_INCLUDE_DIR})

  if(NOT TARGET GnuTLS::GnuTLS)
    add_library(GnuTLS::GnuTLS UNKNOWN IMPORTED)
    set_target_properties(GnuTLS::GnuTLS PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${GNUTLS_INCLUDE_DIRS}"
      INTERFACE_COMPILE_DEFINITIONS "${GNUTLS_DEFINITIONS}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${GNUTLS_LIBRARIES}")
  endif()
endif()
