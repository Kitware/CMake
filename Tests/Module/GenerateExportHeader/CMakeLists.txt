cmake_minimum_required(VERSION 2.8.5 FATAL_ERROR)

project(GenerateExportHeader)

# Prevent timeout on Watcom by not running the tests.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES Watcom)
  file(WRITE
    "${CMAKE_CURRENT_BINARY_DIR}/main.cxx"
    "int main() { return 0; }
  "
  )

  add_executable(
    GenerateExportHeader
    "${CMAKE_CURRENT_BINARY_DIR}/main.cxx"
  )
  return()
endif()

include(CheckCXXCompilerFlag)

set( CMAKE_INCLUDE_CURRENT_DIR ON )

macro(TEST_FAIL value msg)
  if (${value})
    message (SEND_ERROR "Test fail:" "${msg}\n" ${Out} )
  endif ()
endmacro()

macro(TEST_PASS value msg)
  if (NOT ${value})
    message (SEND_ERROR "Test fail:" "${msg}\n" ${Out} )
  endif ()
endmacro()

check_cxx_compiler_flag(-Werror HAS_WERROR_FLAG)

if(HAS_WERROR_FLAG)
  set(ERROR_FLAG "-Werror")
else()
  # MSVC
  # And intel on windows?
  # http://software.intel.com/en-us/articles/how-to-handle-warnings-message-in-compiler/?wapkw=%28compiler+warning+message%29
  check_cxx_compiler_flag("/WX" HAS_WX_FLAG)
  if(HAS_WX_FLAG)
    set(ERROR_FLAG "/WX")
  else()
    # Sun CC
    # http://www.acsu.buffalo.edu/~charngda/sunstudio.html
    check_cxx_compiler_flag("-errwarn=%all" HAS_ERRWARN_ALL)
    if (HAS_ERRWARN_ALL)
      set(ERROR_FLAG "-errwarn=%all")
    else()
    endif()
  endif()
endif()

set(DEPS
  libshared
  libstatic
  lib_shared_and_static
)

foreach(DEP ${DEPS})
  try_compile(Result ${CMAKE_CURRENT_BINARY_DIR}/${DEP}_build
    ${CMAKE_CURRENT_SOURCE_DIR}/${DEP}
    ${DEP}
    OUTPUT_VARIABLE Out
  )
  if (NOT Result)
    message("OUTPUT: ${Out}")
  endif()
endforeach()

# The _do_build macro is called from a child scope, where
# the current source and binary dir are different. Save them here
# for use in the macro.
set(TEST_TOP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_TOP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})


# We seem to get race conditions is writing this stuff to the same file at least on MinGW
# So to write to separate source and build directories, we use a count to differentiate.
set (COUNT 0)
macro(_do_build Include Library LibrarySource Source Languages)

  math(EXPR COUNT "${COUNT} + 1" )

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/src.cpp" "#include \"${Include}\"\n"
    "int main() { ${Source}; }\n"
  )

  if ("${Library}" STREQUAL "static_variant")
    set(CONDITIONAL_STATIC_DEFINE "add_definitions(-DLIBSHARED_AND_STATIC_STATIC_DEFINE)\n")
  endif()

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}/CMakeLists.txt"
    "cmake_minimum_required(VERSION 2.8)\n"

    "project(compiletest ${Languages})\n"

    "set(CMAKE_INCLUDE_CURRENT_DIR ON)\n"

    "set(CMAKE_RUNTIME_OUTPUT_DIRECTORY \"\${CMAKE_CURRENT_BINARY_DIR}\")\n"

    "include(GenerateExportHeader)\n"

    "add_compiler_export_flags()\n"

    "if(NOT \"${ERROR_FLAG}\" STREQUAL \"\")\n"
    "  add_definitions(${ERROR_FLAG})\n"
    "endif()\n"

    "include(\"${TEST_TOP_BINARY_DIR}/${LibrarySource}_build/Targets.cmake\")\n"

    "include_directories(\"${TEST_TOP_SOURCE_DIR}/${LibrarySource}\"\n"
    "                    \"${TEST_TOP_BINARY_DIR}/${LibrarySource}_build\")\n"

    "${CONDITIONAL_STATIC_DEFINE}"

    "add_executable(compiletest src.cpp)\n"
    "target_link_libraries(compiletest ${Library})\n"
  )

  try_compile(Result ${CMAKE_CURRENT_BINARY_DIR}/fail${COUNT}
    ${CMAKE_CURRENT_BINARY_DIR}/test${COUNT}
    compiletest
    OUTPUT_VARIABLE Out
  )
endmacro()

macro(build_fail Include Library LibrarySource Source Message Languages)
  _do_build(${Include} ${Library} ${LibrarySource} "${Source}" "${Languages}")
  test_fail(Result ${Message})
endmacro()

macro(build_pass Include Library LibrarySource Source Message Languages)
  _do_build(${Include} ${Library} ${LibrarySource} "${Source}" "${Languages}")
  test_pass(Result ${Message})
endmacro()

include(GenerateExportHeader)

add_compiler_export_flags()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

message("#### COMPILER_HAS_DEPRECATED: " ${COMPILER_HAS_DEPRECATED})
message("#### COMPILER_HAS_HIDDEN_VISIBILITY: " ${COMPILER_HAS_HIDDEN_VISIBILITY})
message("#### WIN32: " ${WIN32})
message("#### HAS_WERROR_FLAG: " ${HAS_WERROR_FLAG})

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
add_subdirectory(nodeprecated)
add_subdirectory(prefix)

if (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES Clang))
  # We deliberately call deprecated methods, and test for that elsewhere.
  # No need to clutter the test output with warnings.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

if(MSVC AND COMPILER_HAS_DEPRECATED)
  add_definitions(/wd4996)
endif()

add_executable(GenerateExportHeader exportheader_test.cpp)

target_link_libraries(GenerateExportHeader ${link_libraries})
