# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindQuickTime
-------------

Finds the QuickTime multimedia framework, which provides support for video,
audio, and interactive media.

.. note::

  This module is for the QuickTime framework, which has been deprecated by Apple
  and is no longer supported.  On Apple systems, use AVFoundation and AVKit
  instead.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``QuickTime_FOUND``
  Boolean indicating whether the QuickTime is found.  For backward
  compatibility, the ``QUICKTIME_FOUND`` variable is also set to the same value.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``QUICKTIME_LIBRARY``
  The path to the QuickTime library.

``QUICKTIME_INCLUDE_DIR``
  Directory containing QuickTime headers.

Hints
^^^^^

This module accepts the following variables:

``QUICKTIME_DIR``
  Environment variable that can be set to help locate a QuickTime library
  installed in a custom location.  It should point to the installation
  destination that was used when configuring, building, and installing QuickTime
  library: ``./configure --prefix=$QUICKTIME_DIR``.

Examples
^^^^^^^^

Finding QuickTime library:

.. code-block:: cmake

  find_package(QuickTime)
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
