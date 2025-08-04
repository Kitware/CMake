# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindMotif
---------

Finds Motif (or LessTif) graphical user interface toolkit:

.. code-block:: cmake

  find_package(Motif [...])

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``Motif_FOUND``
  Boolean indicating whether the Motif was found.  For backward compatibility,
  the ``MOTIF_FOUND`` variable is also set to the same value.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``MOTIF_INCLUDE_DIR``
  Include directories needed to use Motif.

``MOTIF_LIBRARIES``
  Libraries needed to link to Motif.

Examples
^^^^^^^^

Finding Motif and creating an imported interface target for linking it to a
project target:

.. code-block:: cmake

  find_package(Motif)

  if(Motif_FOUND AND NOT TARGET Motif::Motif)
    add_library(Motif::Motif INTERFACE IMPORTED)
    set_target_properties(
      Motif::Motif
      PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${MOTIF_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${MOTIF_LIBRARIES}"
    )
  endif()

  target_link_libraries(example PRIVATE Motif::Motif)
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
