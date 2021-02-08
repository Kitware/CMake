# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
MacroAddFileDependencies
------------------------

MACRO_ADD_FILE_DEPENDENCIES(<_file> depend_files...)

Using the macro MACRO_ADD_FILE_DEPENDENCIES() is discouraged.  There
are usually better ways to specify the correct dependencies.

MACRO_ADD_FILE_DEPENDENCIES(<_file> depend_files...) is just a
convenience wrapper around the OBJECT_DEPENDS source file property.
You can just use set_property(SOURCE <file> APPEND PROPERTY
OBJECT_DEPENDS depend_files) instead.
#]=======================================================================]

macro (MACRO_ADD_FILE_DEPENDENCIES _file)

  set_property(SOURCE "${_file}" APPEND PROPERTY OBJECT_DEPENDS "${ARGN}")

endmacro ()
