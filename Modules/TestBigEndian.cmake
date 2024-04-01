# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
TestBigEndian
-------------

.. deprecated:: 3.20

  Supserseded by the :variable:`CMAKE_<LANG>_BYTE_ORDER` variable.

Check if the target architecture is big endian or little endian.

.. command:: test_big_endian

  .. code-block:: cmake

    test_big_endian(<var>)

  Stores in variable ``<var>`` either 1 or 0 indicating whether the
  target architecture is big or little endian.

#]=======================================================================]
include_guard()

include(CheckTypeSize)

function(TEST_BIG_ENDIAN VARIABLE)
  if(";${CMAKE_C_BYTE_ORDER};${CMAKE_CXX_BYTE_ORDER};${CMAKE_CUDA_BYTE_ORDER};${CMAKE_OBJC_BYTE_ORDER};${CMAKE_OBJCXX_BYTE_ORDER};" MATCHES ";(BIG_ENDIAN|LITTLE_ENDIAN);")
    set(order "${CMAKE_MATCH_1}")
    if(order STREQUAL "BIG_ENDIAN")
      set("${VARIABLE}" 1 PARENT_SCOPE)
    else()
      set("${VARIABLE}" 0 PARENT_SCOPE)
    endif()
  else()
    __TEST_BIG_ENDIAN_LEGACY_IMPL(is_big)
    set("${VARIABLE}" "${is_big}" PARENT_SCOPE)
  endif()
endfunction()

macro(__TEST_BIG_ENDIAN_LEGACY_IMPL VARIABLE)
  if(NOT DEFINED HAVE_${VARIABLE})
    message(CHECK_START "Check if the system is big endian")
    message(CHECK_START "Searching 16 bit integer")

    if(CMAKE_C_COMPILER_LOADED)
      set(_test_language "C")
    elseif(CMAKE_CXX_COMPILER_LOADED)
      set(_test_language "CXX")
    else()
      message(FATAL_ERROR "TEST_BIG_ENDIAN needs either C or CXX language enabled")
    endif()

    CHECK_TYPE_SIZE("unsigned short" CMAKE_SIZEOF_UNSIGNED_SHORT LANGUAGE ${_test_language})
    if(CMAKE_SIZEOF_UNSIGNED_SHORT EQUAL 2)
      message(CHECK_PASS "Using unsigned short")
      set(CMAKE_16BIT_TYPE "unsigned short")
    else()
      CHECK_TYPE_SIZE("unsigned int"   CMAKE_SIZEOF_UNSIGNED_INT LANGUAGE ${_test_language})
      if(CMAKE_SIZEOF_UNSIGNED_INT)
        message(CHECK_PASS "Using unsigned int")
        set(CMAKE_16BIT_TYPE "unsigned int")

      else()

        CHECK_TYPE_SIZE("unsigned long"  CMAKE_SIZEOF_UNSIGNED_LONG LANGUAGE ${_test_language})
        if(CMAKE_SIZEOF_UNSIGNED_LONG)
          message(CHECK_PASS "Using unsigned long")
          set(CMAKE_16BIT_TYPE "unsigned long")
        else()
          message(FATAL_ERROR "no suitable type found")
        endif()

      endif()

    endif()

    if(_test_language STREQUAL "CXX")
      set(_test_file TestEndianess.cpp)
    else()
      set(_test_file TestEndianess.c)
    endif()

    file(READ "${CMAKE_ROOT}/Modules/TestEndianess.c.in" TEST_ENDIANESS_FILE_CONTENT)
    string(CONFIGURE "${TEST_ENDIANESS_FILE_CONTENT}" TEST_ENDIANESS_FILE_CONTENT @ONLY)

     try_compile(HAVE_${VARIABLE}
      SOURCE_FROM_VAR "${_test_file}" TEST_ENDIANESS_FILE_CONTENT
      COPY_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestEndianess.bin" )

      if(HAVE_${VARIABLE})

        cmake_policy(PUSH)
        cmake_policy(SET CMP0159 NEW) # file(STRINGS) with REGEX updates CMAKE_MATCH_<n>

        file(STRINGS "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestEndianess.bin"
            CMAKE_TEST_ENDIANESS_STRINGS_LE LIMIT_COUNT 1 REGEX "THIS IS LITTLE ENDIAN")

        file(STRINGS "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/TestEndianess.bin"
            CMAKE_TEST_ENDIANESS_STRINGS_BE LIMIT_COUNT 1 REGEX "THIS IS BIG ENDIAN")

        cmake_policy(POP)

        # on mac, if there are universal binaries built both will be true
        # return the result depending on the machine on which cmake runs
        if(CMAKE_TEST_ENDIANESS_STRINGS_BE  AND  CMAKE_TEST_ENDIANESS_STRINGS_LE)
          if(CMAKE_SYSTEM_PROCESSOR MATCHES powerpc)
            set(CMAKE_TEST_ENDIANESS_STRINGS_BE TRUE)
            set(CMAKE_TEST_ENDIANESS_STRINGS_LE FALSE)
          else()
            set(CMAKE_TEST_ENDIANESS_STRINGS_BE FALSE)
            set(CMAKE_TEST_ENDIANESS_STRINGS_LE TRUE)
          endif()
          message(STATUS "TEST_BIG_ENDIAN found different results, consider setting CMAKE_OSX_ARCHITECTURES or CMAKE_TRY_COMPILE_OSX_ARCHITECTURES to one or no architecture !")
        endif()

        if(CMAKE_TEST_ENDIANESS_STRINGS_LE)
          set(${VARIABLE} 0 CACHE INTERNAL "Result of TEST_BIG_ENDIAN" FORCE)
          message(CHECK_PASS "little endian")
        endif()

        if(CMAKE_TEST_ENDIANESS_STRINGS_BE)
          set(${VARIABLE} 1 CACHE INTERNAL "Result of TEST_BIG_ENDIAN" FORCE)
          message(CHECK_PASS "big endian")
        endif()

        if(NOT CMAKE_TEST_ENDIANESS_STRINGS_BE  AND  NOT CMAKE_TEST_ENDIANESS_STRINGS_LE)
          message(CHECK_FAIL "TEST_BIG_ENDIAN found no result!")
          message(SEND_ERROR "TEST_BIG_ENDIAN found no result!")
        endif()
      else()
        message(CHECK_FAIL "failed")
        set(${VARIABLE})
      endif()
  endif()
endmacro()
