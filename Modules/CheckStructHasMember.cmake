# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckStructHasMember
--------------------

Check if the given struct or class has the specified member variable

.. command:: check_struct_has_member

  .. code-block:: cmake

    check_struct_has_member(<struct> <member> <headers> <variable>
                            [LANGUAGE <language>])

  Check that the struct or class ``<struct>`` has the specified ``<member>``
  after including the given header(s) ``<headers>`` where the prototype should
  be declared. Specify the list of header files in one argument as a
  semicolon-separated list. The result is stored in an internal cache variable
  ``<variable>``.

  The options are:

  ``LANGUAGE <language>``
    Use the ``<language>`` compiler to perform the check.
    Acceptable values are ``C`` and ``CXX``.
    If not specified, it defaults to ``C``.

The following variables may be set before calling this macro to modify
the way the check is run:

.. include:: /module/CMAKE_REQUIRED_FLAGS.txt

.. include:: /module/CMAKE_REQUIRED_DEFINITIONS.txt

.. include:: /module/CMAKE_REQUIRED_INCLUDES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_DIRECTORIES.txt

.. include:: /module/CMAKE_REQUIRED_QUIET.txt

Example
^^^^^^^

.. code-block:: cmake

  include(CheckStructHasMember)

  check_struct_has_member("struct timeval" tv_sec sys/select.h
                          HAVE_TIMEVAL_TV_SEC LANGUAGE C)
#]=======================================================================]

include_guard(GLOBAL)
include(CheckSourceCompiles)

macro (CHECK_STRUCT_HAS_MEMBER _STRUCT _MEMBER _HEADER _RESULT)
  set(_INCLUDE_FILES)
  foreach (it ${_HEADER})
    string(APPEND _INCLUDE_FILES "#include <${it}>\n")
  endforeach ()

  if("x${ARGN}" STREQUAL "x")
    set(_lang C)
  elseif("x${ARGN}" MATCHES "^xLANGUAGE;([a-zA-Z]+)$")
    set(_lang "${CMAKE_MATCH_1}")
  else()
    message(FATAL_ERROR "Unknown arguments:\n  ${ARGN}\n")
  endif()

  set(_CHECK_STRUCT_MEMBER_SOURCE_CODE "
${_INCLUDE_FILES}
int main(void)
{
  (void)sizeof(((${_STRUCT} *)0)->${_MEMBER});
  return 0;
}
")

  if("${_lang}" STREQUAL "C")
    check_source_compiles(C "${_CHECK_STRUCT_MEMBER_SOURCE_CODE}" ${_RESULT})
  elseif("${_lang}" STREQUAL "CXX")
    check_source_compiles(CXX "${_CHECK_STRUCT_MEMBER_SOURCE_CODE}" ${_RESULT})
  else()
    message(FATAL_ERROR "Unknown language:\n  ${_lang}\nSupported languages: C, CXX.\n")
  endif()
endmacro ()

# FIXME(#24994): The following modules are included only for compatibility
# with projects that accidentally relied on them with CMake 3.26 and below.
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
