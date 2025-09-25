# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindIcotool
-----------

Finds ``icotool``, command-line program for converting and creating Win32 icon
and cursor files.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``Icotool_FOUND``
  True if ``icotool`` has been found.  For backward compatibility, the
  ``ICOTOOL_FOUND`` variable is also set to the same value.
``ICOTOOL_VERSION_STRING``
  The version of ``icotool`` found.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``ICOTOOL_EXECUTABLE``
  The full path to the ``icotool`` tool.

Examples
^^^^^^^^

Finding ``icotool`` and executing it in a process to create ``.ico`` icon from
the source ``.png`` image located in the current source directory:

.. code-block:: cmake

  find_package(Icotool)
  if(Icotool_FOUND)
    execute_process(
      COMMAND
        ${ICOTOOL_EXECUTABLE} -c -o ${CMAKE_CURRENT_BINARY_DIR}/img.ico img.png
    )
  endif()
#]=======================================================================]

find_program(ICOTOOL_EXECUTABLE
  icotool
)

if(ICOTOOL_EXECUTABLE)
  execute_process(
    COMMAND ${ICOTOOL_EXECUTABLE} --version
    OUTPUT_VARIABLE _icotool_version
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if("${_icotool_version}" MATCHES "^icotool \\([^\\)]*\\) ([0-9\\.]+[^ \n]*)")
    set( ICOTOOL_VERSION_STRING
      "${CMAKE_MATCH_1}"
    )
  else()
    set( ICOTOOL_VERSION_STRING
      ""
    )
  endif()
  unset(_icotool_version)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Icotool
  REQUIRED_VARS ICOTOOL_EXECUTABLE
  VERSION_VAR ICOTOOL_VERSION_STRING
)

mark_as_advanced(
  ICOTOOL_EXECUTABLE
)
