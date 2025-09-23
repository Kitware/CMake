# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindMotif
---------

Finds Motif (or LessTif) graphical user interface toolkit.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``Motif_FOUND``
  Boolean indicating whether the Motif was found.  For backward compatibility,
  the ``MOTIF_FOUND`` variable is also set to the same value.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``MOTIF_LIBRARIES``
  Libraries needed to link to Motif.
``MOTIF_INCLUDE_DIR``
  Include directories needed to use Motif.

Examples
^^^^^^^^

Finding Motif:

.. code-block:: cmake

  find_package(Motif)
#]=======================================================================]

if(UNIX)
  find_path(MOTIF_INCLUDE_DIR
    Xm/Xm.h
    /usr/openwin/include
    )

  find_library(MOTIF_LIBRARIES
    Xm
    /usr/openwin/lib
    )

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Motif DEFAULT_MSG MOTIF_LIBRARIES MOTIF_INCLUDE_DIR)

mark_as_advanced(
  MOTIF_INCLUDE_DIR
  MOTIF_LIBRARIES
)
