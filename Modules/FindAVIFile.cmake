# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindAVIFile
-----------

Finds `AVIFile <https://avifile.sourceforge.net/>`_ library and include paths.

AVIFile is a set of libraries for i386 machines to use various AVI codecs.
Support is limited beyond Linux.  Windows provides native AVI support, and so
doesn't need this library.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``AVIFile_FOUND``
  True if AVIFile is found.  For backward compatibility, the ``AVIFILE_FOUND``
  variable is also set to the same value.
``AVIFILE_LIBRARIES``
  The libraries to link against.
``AVIFILE_DEFINITIONS``
  Definitions to use when compiling.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may be also set:

``AVIFILE_INCLUDE_DIR``
  Directory containing ``avifile.h`` and other AVIFile headers.

Examples
^^^^^^^^

Finding AVIFile:

.. code-block:: cmake

  find_package(AVIFile)
#]=======================================================================]

if (UNIX)

  find_path(AVIFILE_INCLUDE_DIR avifile.h PATH_SUFFIXES avifile/include include/avifile include/avifile-0.7)
  find_library(AVIFILE_AVIPLAY_LIBRARY aviplay aviplay-0.7 PATH_SUFFIXES avifile/lib)

endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AVIFile DEFAULT_MSG AVIFILE_INCLUDE_DIR AVIFILE_AVIPLAY_LIBRARY)

if (AVIFile_FOUND)
    set(AVIFILE_LIBRARIES ${AVIFILE_AVIPLAY_LIBRARY})
    set(AVIFILE_DEFINITIONS "")
endif()

mark_as_advanced(AVIFILE_INCLUDE_DIR AVIFILE_AVIPLAY_LIBRARY)
