# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindQuickTime
-------------

Locate QuickTime This module defines:

``QUICKTIME_LIBRARY``

``QUICKTIME_FOUND``
  if false, do not try to link to gdal
``QUICKTIME_INCLUDE_DIR``
  where to find the headers

``$QUICKTIME_DIR`` is an environment variable that would correspond to::

  ./configure --prefix=$QUICKTIME_DIR
#]=======================================================================]

find_path(QUICKTIME_INCLUDE_DIR QuickTime/QuickTime.h QuickTime.h
  HINTS
    ENV QUICKTIME_DIR
  PATH_SUFFIXES
    include
)
find_library(QUICKTIME_LIBRARY QuickTime
  HINTS
    ENV QUICKTIME_DIR
  PATH_SUFFIXES
    lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QuickTime DEFAULT_MSG QUICKTIME_LIBRARY QUICKTIME_INCLUDE_DIR)
