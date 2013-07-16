
cmake_minimum_required(VERSION 2.8)

project(CompileDefinitions)

if ("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 6")
    add_definitions(-DNO_SPACES_IN_DEFINE_VALUES)
    set(NO_SPACES_IN_DEFINE_VALUES 1)
endif()

# Use compile flags to tell executables which config is built
# without depending on the compile definitions functionality.
foreach(c DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  set(CMAKE_C_FLAGS_${c} "${CMAKE_C_FLAGS_${c}} -DTEST_CONFIG_${c}")
  set(CMAKE_CXX_FLAGS_${c} "${CMAKE_CXX_FLAGS_${c}} -DTEST_CONFIG_${c}")
endforeach()

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
  "BUILD_CONFIG_NAME=\"$<CONFIGURATION>\""
  )

add_subdirectory(add_definitions_command)
add_subdirectory(target_prop)
add_subdirectory(add_definitions_command_with_target_prop)

add_executable(CompileDefinitions runtest.c)

if(NOT BORLAND AND NOT "${CMAKE_GENERATOR}" MATCHES "Visual Studio 7$")
  # Borland, VS70 IDE: ;
  # The Borland compiler will simply not accept a non-escaped semicolon
  # on the command line.  If it is escaped \; then the escape character
  # shows up in the preprocessing output too.
  #
  # The VS 7.0 IDE separates definitions on semicolons and commas with
  # no regard for quotes.  Fortunately VS 7.1 and above are okay.
  set(SEMICOLON "\;")
endif()

set_property(
  TARGET CompileDefinitions
  APPEND PROPERTY COMPILE_DEFINITIONS
  "SEMICOLON_STRING1=\"foo${SEMICOLON}bar\""
  )
target_compile_definitions(CompileDefinitions PRIVATE
  "SEMICOLON_STRING2=\"foo${SEMICOLON}bar\""
)
add_definitions("-DSEMICOLON_STRING3=\"foo${SEMICOLON}bar\"")

add_library(emptyTarget empty.cpp)
set_property(
  TARGET emptyTarget
  APPEND PROPERTY COMPILE_DEFINITIONS
  "SEMICOLON_STRING4=\"foo${SEMICOLON}bar\""
  )

get_target_property(_roundtrip emptyTarget COMPILE_DEFINITIONS)
set_property(
  TARGET CompileDefinitions
  APPEND PROPERTY COMPILE_DEFINITIONS
  "${_roundtrip}"
  )

if (SEMICOLON)
  set_property(
    TARGET CompileDefinitions
    APPEND PROPERTY COMPILE_DEFINITIONS
    TEST_SEMICOLON
  )
endif()
