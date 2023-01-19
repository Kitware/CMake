# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
TestForSTDNamespace
-------------------

Test for std:: namespace support

check if the compiler supports std:: on stl classes

::

  CMAKE_NO_STD_NAMESPACE - defined by the results
#]=======================================================================]

if(NOT DEFINED CMAKE_STD_NAMESPACE)
  message(CHECK_START "Check for STD namespace")
  try_compile(CMAKE_STD_NAMESPACE
    SOURCES ${CMAKE_ROOT}/Modules/TestForSTDNamespace.cxx
    )
  if (CMAKE_STD_NAMESPACE)
    message(CHECK_PASS "found")
    set (CMAKE_NO_STD_NAMESPACE 0 CACHE INTERNAL
         "Does the compiler support std::.")
  else ()
    message(CHECK_FAIL "not found")
    set (CMAKE_NO_STD_NAMESPACE 1 CACHE INTERNAL
       "Does the compiler support std::.")
  endif ()
endif()




