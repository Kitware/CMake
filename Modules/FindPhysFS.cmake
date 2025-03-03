# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindPhysFS
----------

Locate PhysFS library This module defines:

``PHYSFS_LIBRARY``
  the name of the library to link against
``PHYSFS_FOUND``
  if false, do not try to link to PHYSFS
``PHYSFS_INCLUDE_DIR``
  where to find physfs.h

``$PHYSFSDIR`` is an environment variable that would correspond to::

  ./configure --prefix=$PHYSFSDIR

used in building PHYSFS.
#]=======================================================================]

find_path(PHYSFS_INCLUDE_DIR physfs.h
  HINTS
    ENV PHYSFSDIR
  PATH_SUFFIXES include/physfs include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /opt
)

find_library(PHYSFS_LIBRARY
  NAMES physfs
  HINTS
    ENV PHYSFSDIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /opt
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PhysFS DEFAULT_MSG PHYSFS_LIBRARY PHYSFS_INCLUDE_DIR)
