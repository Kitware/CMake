# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindDevIL
---------

Finds the Developer's Image Library, `DevIL <https://openil.sourceforge.net/>`_.

The DevIL package internally consists of the following libraries, all
distributed as part of the same release:

* The core Image Library (IL)

  This library is always required when working with DevIL, as it provides the
  main image loading and manipulation functionality.

* The Image Library Utilities (ILU)

  This library depends on IL and provides image filters and effects. It is only
  required if the application uses this extended functionality.

* The Image Library Utility Toolkit (ILUT)

  This library depends on both IL and ILU, and additionally provides an
  interface to OpenGL.  It is only needed if the application uses DevIL together
  with OpenGL.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following :ref:`Imported Targets`:

``DevIL::IL``
  .. versionadded:: 3.21

  Target encapsulating the core Image Library (IL) usage requirements, available
  if the DevIL package is found.

``DevIL::ILU``
  .. versionadded:: 3.21

  Target encapsulating the Image Library Utilities (ILU) usage requirements,
  available if the DevIL package is found.  This target also links to
  ``DevIL::IL`` for convenience, as ILU depends on the core IL library.

``DevIL::ILUT``
  .. versionadded:: 3.21

  Target encapsulating the Image Library Utility Toolkit (ILUT) usage
  requirements, available if the DevIL package and its ILUT library are found.
  This target also links to ``DevIL::ILU``, and transitively to ``DevIL::IL``,
  since ILUT depends on both.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``DevIL_FOUND``
  Boolean indicating whether the DevIL package is found, including the IL and
  ILU libraries.

``DevIL_ILUT_FOUND``
  .. versionadded:: 3.21

  Boolean indicating whether the ILUT library is found.  On most systems, ILUT
  is found when both IL and ILU are available.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``IL_INCLUDE_DIR``
  The directory containing the ``il.h``, ``ilu.h`` and ``ilut.h`` header files.

``IL_LIBRARIES``
  The full path to the core Image Library (IL).

``ILU_LIBRARIES``
  The full path to the ILU library.

``ILUT_LIBRARIES``
  The full path to the ILUT library.

Examples
^^^^^^^^

Finding the DevIL package and linking against the core Image Library (IL):

.. code-block:: cmake

  find_package(DevIL)
  target_link_libraries(app PRIVATE DevIL::IL)

Linking against the Image Library Utilities (ILU):

.. code-block:: cmake

  find_package(DevIL)
  target_link_libraries(app PRIVATE DevIL::ILU)

Linking against the Image Library Utility Toolkit (ILUT):

.. code-block:: cmake

  find_package(DevIL)
  target_link_libraries(app PRIVATE DevIL::ILUT)
#]=======================================================================]

# TODO: Add version support.
# Tested under Linux and Windows (MSVC)

include(FindPackageHandleStandardArgs)

find_path(IL_INCLUDE_DIR il.h
  PATH_SUFFIXES include IL
  DOC "The path to the directory that contains il.h"
)

#message("IL_INCLUDE_DIR is ${IL_INCLUDE_DIR}")

find_library(IL_LIBRARIES
  NAMES IL DEVIL
  PATH_SUFFIXES libx32 lib64 lib lib32
  DOC "The file that corresponds to the base il library."
)

#message("IL_LIBRARIES is ${IL_LIBRARIES}")

find_library(ILUT_LIBRARIES
  NAMES ILUT
  PATH_SUFFIXES libx32 lib64 lib lib32
  DOC "The file that corresponds to the il (system?) utility library."
)

#message("ILUT_LIBRARIES is ${ILUT_LIBRARIES}")

find_library(ILU_LIBRARIES
  NAMES ILU
  PATH_SUFFIXES libx32 lib64 lib lib32
  DOC "The file that corresponds to the il utility library."
)

#message("ILU_LIBRARIES is ${ILU_LIBRARIES}")

find_package_handle_standard_args(DevIL DEFAULT_MSG
                                  IL_LIBRARIES ILU_LIBRARIES
                                  IL_INCLUDE_DIR)
# provide legacy variable for compatibility
set(IL_FOUND ${DevIL_FOUND})

# create imported targets ONLY if we found DevIL.
if(DevIL_FOUND)
  # Report the ILUT found if ILUT_LIBRARIES contains valid path.
  if (ILUT_LIBRARIES)
    set(DevIL_ILUT_FOUND TRUE)
  else()
    set(DevIL_ILUT_FOUND FALSE)
  endif()

  if(NOT TARGET DevIL::IL)
    add_library(DevIL::IL UNKNOWN IMPORTED)
    set_target_properties(DevIL::IL PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${IL_INCLUDE_DIR}"
      IMPORTED_LOCATION "${IL_LIBRARIES}")
  endif()

  # DevIL Utilities target
  if(NOT TARGET DevIL::ILU)
    add_library(DevIL::ILU UNKNOWN IMPORTED)
    set_target_properties(DevIL::ILU PROPERTIES
      IMPORTED_LOCATION "${ILU_LIBRARIES}")
    target_link_libraries(DevIL::ILU INTERFACE DevIL::IL)
  endif()

  # ILUT (if found)
  if(NOT TARGET DevIL::ILUT AND DevIL_ILUT_FOUND)
    add_library(DevIL::ILUT UNKNOWN IMPORTED)
    set_target_properties(DevIL::ILUT PROPERTIES
      IMPORTED_LOCATION "${ILUT_LIBRARIES}")
    target_link_libraries(DevIL::ILUT INTERFACE DevIL::ILU)
  endif()
endif()
