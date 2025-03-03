# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
TestForANSIForScope
-------------------

Check for ANSI for scope support

Check if the compiler restricts the scope of variables declared in a
for-init-statement to the loop body.

::

  CMAKE_NO_ANSI_FOR_SCOPE - holds result
#]=======================================================================]

if(NOT DEFINED CMAKE_ANSI_FOR_SCOPE)
  message(CHECK_START "Check for ANSI scope")
  try_compile(CMAKE_ANSI_FOR_SCOPE
    SOURCES ${CMAKE_ROOT}/Modules/TestForAnsiForScope.cxx
    )
  if (CMAKE_ANSI_FOR_SCOPE)
    message(CHECK_PASS "found")
    set (CMAKE_NO_ANSI_FOR_SCOPE 0 CACHE INTERNAL
      "Does the compiler support ansi for scope.")
  else ()
    message(CHECK_FAIL "not found")
    set (CMAKE_NO_ANSI_FOR_SCOPE 1 CACHE INTERNAL
      "Does the compiler support ansi for scope.")
  endif ()
endif()
