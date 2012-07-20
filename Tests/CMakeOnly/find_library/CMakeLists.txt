cmake_minimum_required(VERSION 2.8)
project(FindLibraryTest NONE)

set(CMAKE_FIND_DEBUG_MODE 1)

macro(test_find_library expected)
  get_filename_component(dir ${expected} PATH)
  get_filename_component(name ${expected} NAME)
  string(REGEX REPLACE "lib/?64" "lib" dir "${dir}")
  unset(LIB CACHE)
  find_library(LIB
    NAMES ${name}
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}
    NO_DEFAULT_PATH
    )
  if(LIB)
    # Convert to relative path for comparison to expected location.
    file(RELATIVE_PATH REL_LIB "${CMAKE_CURRENT_SOURCE_DIR}" "${LIB}")

    # Debugging output.
    if(CMAKE_FIND_DEBUG_MODE)
      message(STATUS "Library ${expected} searched as ${dir}, found as [${REL_LIB}].")
    endif()

    # Check and report failure.
    if(NOT "${REL_LIB}" STREQUAL "${expected}")
      message(SEND_ERROR "Library ${l} should have been [${expected}] but was [${REL_LIB}]")
    endif()
  else()
    message(SEND_ERROR "Library ${expected} searched as ${dir}, NOT FOUND!")
  endif()
endmacro()

set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)

set(CMAKE_SIZEOF_VOID_P 4)
foreach(lib
    lib/A/lib/libtest1.a
    lib/A/libtest1.a
    lib/libtest1.a
    lib/libtest2.a
    lib/libtest3.a
    lib/libtest3.a
    )
  test_find_library(${lib})
endforeach()

set(CMAKE_SIZEOF_VOID_P 8)
foreach(lib64
    lib/64/libtest2.a
    lib/A/lib64/libtest3.a
    lib/libtest3.a
    lib64/A/lib/libtest2.a
    lib64/A/lib64/libtest1.a
    lib64/A/libtest1.a
    lib64/libtest1.a
    )
  test_find_library(${lib64})
endforeach()
