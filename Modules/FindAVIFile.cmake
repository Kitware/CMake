# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindAVIFile
-----------

Locate `AVIFILE <https://avifile.sourceforge.net/>`_
library and include paths.

AVIFILE is a set of libraries for
i386 machines to use various AVI codecs.  Support is limited beyond
Linux.  Windows provides native AVI support, and so doesn't need this
library.  This module defines

::

  AVIFILE_INCLUDE_DIR, where to find avifile.h , etc.
  AVIFILE_LIBRARIES, the libraries to link against
  AVIFILE_DEFINITIONS, definitions to use when compiling
  AVIFILE_FOUND, If false, don't try to use AVIFILE
#]=======================================================================]

if (UNIX)

  find_path(AVIFILE_INCLUDE_DIR avifile.h PATH_SUFFIXES avifile/include include/avifile include/avifile-0.7)
  find_library(AVIFILE_AVIPLAY_LIBRARY aviplay aviplay-0.7 PATH_SUFFIXES avifile/lib)

endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AVIFile DEFAULT_MSG AVIFILE_INCLUDE_DIR AVIFILE_AVIPLAY_LIBRARY)

if (AVIFILE_FOUND)
    set(AVIFILE_LIBRARIES ${AVIFILE_AVIPLAY_LIBRARY})
    set(AVIFILE_DEFINITIONS "")
endif()

mark_as_advanced(AVIFILE_INCLUDE_DIR AVIFILE_AVIPLAY_LIBRARY)
