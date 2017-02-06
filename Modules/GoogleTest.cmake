# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
GoogleTest
----------

This module defines functions to help use the Google Test infrastructure.

.. command:: gtest_add_tests

  Automatically add tests with CTest by scanning source code for Google test
  macros.

  ::

    gtest_add_tests(<exe> <args> <files>...)

  ``<exe>``
    The path to the test executable.
  ``<args>``
    A ;-list of extra arguments to be passed to executable.  The entire
    list must be passed as a single argument.  Enclose it in quotes,
    or pass ``""`` for no arguments.
  ``<files>...``
    A list of source files to search for tests and test fixtures.
    Alternatively, use ``AUTO`` to specify that ``<exe>`` is the name
    of a CMake executable target whose sources should be scanned.

Example
^^^^^^^

.. code-block:: cmake

  include(GoogleTest)
  set(FooTestArgs --foo 1 --bar 2)
  add_executable(FooTest FooUnitTest.cc)
  gtest_add_tests(FooTest "${FooTestArgs}" AUTO)

#]=======================================================================]

function(gtest_add_tests executable extra_args)
  if(NOT ARGN)
    message(FATAL_ERROR "Missing ARGN: Read the documentation for GTEST_ADD_TESTS")
  endif()
  if(ARGN STREQUAL "AUTO")
    # obtain sources used for building that executable
    get_property(ARGN TARGET ${executable} PROPERTY SOURCES)
  endif()
  set(gtest_case_name_regex ".*\\( *([A-Za-z_0-9]+) *, *([A-Za-z_0-9]+) *\\).*")
  set(gtest_test_type_regex "(TYPED_TEST|TEST_?[FP]?)")
  foreach(source ${ARGN})
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${source})
    file(READ "${source}" contents)
    string(REGEX MATCHALL "${gtest_test_type_regex} *\\(([A-Za-z_0-9 ,]+)\\)" found_tests ${contents})
    foreach(hit ${found_tests})
      string(REGEX MATCH "${gtest_test_type_regex}" test_type ${hit})

      # Parameterized tests have a different signature for the filter
      if("x${test_type}" STREQUAL "xTEST_P")
        string(REGEX REPLACE ${gtest_case_name_regex}  "*/\\1.\\2/*" test_name ${hit})
      elseif("x${test_type}" STREQUAL "xTEST_F" OR "x${test_type}" STREQUAL "xTEST")
        string(REGEX REPLACE ${gtest_case_name_regex} "\\1.\\2" test_name ${hit})
      elseif("x${test_type}" STREQUAL "xTYPED_TEST")
        string(REGEX REPLACE ${gtest_case_name_regex} "\\1/*.\\2" test_name ${hit})
      else()
        message(WARNING "Could not parse GTest ${hit} for adding to CTest.")
        continue()
      endif()
      add_test(NAME ${test_name} COMMAND ${executable} --gtest_filter=${test_name} ${extra_args})
    endforeach()
  endforeach()
endfunction()
