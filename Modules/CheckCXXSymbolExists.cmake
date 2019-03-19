# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckCXXSymbolExists
--------------------

Check if a symbol exists as a function, variable, or macro in C++

.. command:: CHECK_CXX_SYMBOL_EXISTS

  .. code-block:: cmake

    CHECK_CXX_SYMBOL_EXISTS(<symbol> <files> <variable>)

  Check that the ``<symbol>`` is available after including given header
  ``<files>`` and store the result in a ``<variable>``.  Specify the list of
  files in one argument as a semicolon-separated list.
  ``CHECK_CXX_SYMBOL_EXISTS()`` can be used to check in C++ files, as
  opposed to ``CHECK_SYMBOL_EXISTS()``, which works only for ``C``.

  If the header files define the symbol as a macro it is considered
  available and assumed to work.  If the header files declare the symbol
  as a function or variable then the symbol must also be available for
  linking.  If the symbol is a type or enum value it will not be
  recognized (consider using :module:`CheckTypeSize`
  or :module:`CheckCSourceCompiles`).

The following variables may be set before calling this macro to modify
the way the check is run:

``CMAKE_REQUIRED_FLAGS``
  string of compile command line flags.
``CMAKE_REQUIRED_DEFINITIONS``
  a :ref:`;-list <CMake Language Lists>` of macros to define (-DFOO=bar).
``CMAKE_REQUIRED_INCLUDES``
  a :ref:`;-list <CMake Language Lists>` of header search paths to pass to
  the compiler.
``CMAKE_REQUIRED_LINK_OPTIONS``
  a :ref:`;-list <CMake Language Lists>` of options to add to the link command.
``CMAKE_REQUIRED_LIBRARIES``
  a :ref:`;-list <CMake Language Lists>` of libraries to add to the link
  command. See policy :policy:`CMP0075`.
``CMAKE_REQUIRED_QUIET``
  execute quietly without messages.
#]=======================================================================]

include_guard(GLOBAL)
include(CheckSymbolExists)

macro(CHECK_CXX_SYMBOL_EXISTS SYMBOL FILES VARIABLE)
  __CHECK_SYMBOL_EXISTS_IMPL("${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckSymbolExists.cxx" "${SYMBOL}" "${FILES}" "${VARIABLE}" )
endmacro()
