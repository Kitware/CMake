# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindJPEG
# --------
#
# Find the JPEG library (libjpeg)
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``JPEG_FOUND``
#   If false, do not try to use JPEG.
# ``JPEG_INCLUDE_DIR``
#   where to find jpeglib.h, etc.
# ``JPEG_LIBRARIES``
#   the libraries needed to use JPEG.
# ``JPEG_VERSION``
#   the version of the JPEG library found
#
# Cache variables
# ^^^^^^^^^^^^^^^
#
# The following cache variables may also be set:
#
# ``JPEG_INCLUDE_DIR``
#   where to find jpeglib.h, etc.
# ``JPEG_LIBRARY``
#   where to find the JPEG library.

find_path(JPEG_INCLUDE_DIR jpeglib.h)

set(JPEG_NAMES ${JPEG_NAMES} jpeg libjpeg)
find_library(JPEG_LIBRARY NAMES ${JPEG_NAMES})

if(JPEG_INCLUDE_DIR AND EXISTS "${JPEG_INCLUDE_DIR}/jpeglib.h")
  file(STRINGS "${JPEG_INCLUDE_DIR}/jpeglib.h"
    jpeg_lib_version REGEX "^#define[\t ]+JPEG_LIB_VERSION[\t ]+.*")

  if (NOT jpeg_lib_version)
    # libjpeg-turbo sticks JPEG_LIB_VERSION in jconfig.h
    find_path(jconfig_dir jconfig.h)
    if (jconfig_dir)
      file(STRINGS "${jconfig_dir}/jconfig.h"
        jpeg_lib_version REGEX "^#define[\t ]+JPEG_LIB_VERSION[\t ]+.*")
    endif()
    unset(jconfig_dir)
  endif()

  string(REGEX REPLACE "^#define[\t ]+JPEG_LIB_VERSION[\t ]+([0-9]+).*"
    "\\1" JPEG_VERSION "${jpeg_lib_version}")
  unset(jpeg_lib_version)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(JPEG
  REQUIRED_VARS JPEG_LIBRARY JPEG_INCLUDE_DIR
  VERSION_VAR JPEG_VERSION)

if(JPEG_FOUND)
  set(JPEG_LIBRARIES ${JPEG_LIBRARY})
endif()

# Deprecated declarations.
set(NATIVE_JPEG_INCLUDE_PATH ${JPEG_INCLUDE_DIR})
if(JPEG_LIBRARY)
  get_filename_component(NATIVE_JPEG_LIB_PATH ${JPEG_LIBRARY} PATH)
endif()

mark_as_advanced(JPEG_LIBRARY JPEG_INCLUDE_DIR)
