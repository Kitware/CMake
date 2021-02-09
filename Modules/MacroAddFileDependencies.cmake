# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
MacroAddFileDependencies
------------------------

.. deprecated:: 3.14

::

  MACRO_ADD_FILE_DEPENDENCIES(<source> <files>...)

Do not use this command in new code.  It is just a wrapper around:

.. code-block:: cmake

  set_property(SOURCE <source> APPEND PROPERTY OBJECT_DEPENDS <files>...)

Instead use the :command:`set_property` command to append to the
:prop_sf:`OBJECT_DEPENDS` source file property directly.

#]=======================================================================]

macro (MACRO_ADD_FILE_DEPENDENCIES _file)

  set_property(SOURCE "${_file}" APPEND PROPERTY OBJECT_DEPENDS "${ARGN}")

endmacro ()
