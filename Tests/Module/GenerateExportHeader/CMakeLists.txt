cmake_minimum_required(VERSION 2.8.5 FATAL_ERROR)

project(GenerateExportHeader)

set( CMAKE_INCLUDE_CURRENT_DIR ON )

macro(TEST_FAIL value msg)
  if (${value})
    message (SEND_ERROR "Test fail:" ${msg} ${Out} )
  endif ()
endmacro()

macro(TEST_PASS value msg)
  if (NOT ${value})
    message (SEND_ERROR "Test fail:" ${msg} ${Out} )
  endif ()
endmacro()

# We seem to get race conditions is writing this stuff to the same file at least on MinGW
# So to write to separate source and build directories, we use a count to differentiate.
set (COUNT 0)
macro(_do_build Include Library LibrarySource Source)

  math(EXPR COUNT "${COUNT} + 1" )

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/src.cpp" "#include \"${Include}\"\n"
    "int main() { ${Source}; }\n"
  )

  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../${LibrarySource}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}")

  if ("${Library}" STREQUAL "static_variant")
    set(CONDITIONAL_STATIC_DEFINE "add_definitions(-DLIBSHARED_AND_STATIC_STATIC_DEFINE)\n")
  endif()

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"
    "cmake_minimum_required(VERSION 2.8)\n"

    "project(compiletest)\n"

    "set(CMAKE_INCLUDE_CURRENT_DIR ON)\n"

    "set(CMAKE_RUNTIME_OUTPUT_DIRECTORY \${CMAKE_CURRENT_BINARY_DIR})\n"

    "include(GenerateExportHeader)\n"

    "add_compiler_export_flags()\n"

    "if(CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES Clang))\n"
    "  add_definitions(-Werror)\n"
    "else()\n"
    "  if(MSVC)\n"
         # Treat deprecation warnings as errors.
    "    add_definitions(/we4996)\n"
    "  endif()\n"
    "endif()\n"

    "if(MSVC)\n"
    "  add_definitions(-DCOMPILER_IS_MSVC)\n"
    "endif()\n"

    "add_subdirectory(${LibrarySource})\n"

    "include_directories(${LibrarySource} \${CMAKE_CURRENT_BINARY_DIR}/${LibrarySource})\n"

    "${CONDITIONAL_STATIC_DEFINE}"

    "add_executable(compiletest src.cpp)\n"
    "target_link_libraries(compiletest ${Library})\n"
  )

  try_compile(Result ${CMAKE_CURRENT_BINARY_DIR}/fail${COUNT}
    ${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}
    compilefail
    OUTPUT_VARIABLE Out
  )
endmacro()

macro(build_fail Include Library LibrarySource Source Message)
  _do_build(${Include} ${Library} ${LibrarySource} "${Source}")
  if(COMPILER_HAS_HIDDEN_VISIBILITY OR WIN32 OR (${CMAKE_CXX_COMPILER_ID} MATCHES Clang))
    test_fail(Result ${Message})
  else()
    test_pass(Result ${Message})
  endif()
endmacro()

macro(build_pass Include Library LibrarySource Source Message)
  _do_build(${Include} ${Library} ${LibrarySource} "${Source}")
  test_pass(Result ${Message})
endmacro()

include(GenerateExportHeader)

add_compiler_export_flags()

if (MSVC)
  add_definitions(-DCOMPILER_IS_MSVC)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(link_libraries)
macro(macro_add_test_library name)
  add_subdirectory(${name})
  include_directories(${name}
            ${${name}_BINARY_DIR} # For the export header.
  )
  list(APPEND link_libraries ${name})
  add_subdirectory(${name}test)
endmacro()

macro_add_test_library(libshared)
macro_add_test_library(libstatic)
add_subdirectory(lib_shared_and_static)
add_subdirectory(lib_shared_and_statictest)

add_subdirectory(override_symbol)

if (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES Clang))
  # We deliberately call deprecated methods, and test for that elsewhere.
  # No need to clutter the test output with warnings.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

if(MSVC)
  add_definitions(/wd4996)
endif()

add_executable(GenerateExportHeader exportheader_test.cpp)

target_link_libraries(GenerateExportHeader ${link_libraries})
