# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
TestForANSIForScope
-------------------

This module checks whether the ``CXX`` compiler restricts the scope of variables
declared in a for-init-statement to the loop body.  In early C++ (pre-C++98),
variables declared in ``for(<init-statement> ...)`` could remain accessible
outside the loop after its body (``for() { <body> }``).

This module defines the following cache variable:

``CMAKE_NO_ANSI_FOR_SCOPE``
  A cache variable containing the result of the check.  It will be set to value
  ``0`` if the for-init-statement has restricted scope (``C++ 98`` and newer),
  and to value ``1`` if not (``ANSI C++``).

.. note::

  As of the ``C++ 98`` standard, variables declared in a for-init-statement are
  restricted to the loop body, making this behavior obsolete.

Examples
^^^^^^^^

Including this module will check the ``for()`` loop scope behavior and define
the ``CMAKE_NO_ANSI_FOR_SCOPE`` cache variable:

.. code-block:: cmake

  include(TestForANSIForScope)
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
