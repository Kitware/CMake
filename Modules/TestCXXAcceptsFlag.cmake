# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
TestCXXAcceptsFlag
------------------

.. deprecated:: 3.0

  See :module:`CheckCXXCompilerFlag`.

Check if the CXX compiler accepts a flag.

.. code-block:: cmake

 CHECK_CXX_ACCEPTS_FLAG(<flags> <variable>)

``<flags>``
 the flags to try
``<variable>``
 variable to store the result
#]=======================================================================]

macro(CHECK_CXX_ACCEPTS_FLAG FLAGS  VARIABLE)
  if(NOT DEFINED ${VARIABLE})
    message(CHECK_START "Checking to see if CXX compiler accepts flag ${FLAGS}")
    try_compile(${VARIABLE}
      SOURCES ${CMAKE_ROOT}/Modules/DummyCXXFile.cxx
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${FLAGS}
      )
    if(${VARIABLE})
      message(CHECK_PASS "yes")
    else()
      message(CHECK_FAIL "no")
    endif()
  endif()
endmacro()
