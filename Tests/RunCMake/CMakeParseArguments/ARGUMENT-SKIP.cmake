# CMAKE_MINIMUM_REQUIRED_VERSION             2.8.12
# CMAKE_PARSE_ARGUMENTS_DEFAULT_SKIP_EMPTY   0
# CMAKE_PARSE_ARGUMENTS_(KEEP|SKIP)_EMPTY    SKIP
#     => SKIP

cmake_minimum_required(VERSION 2.8.12)

include(CMakeParseArguments)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY CMAKE_PARSE_ARGUMENTS_DEFAULT_SKIP_EMPTY 0)

macro(MY_INSTALL)
    set(options OPTIONAL FAST)
    set(oneValueArgs DESTINATION RENAME)
    set(multiValueArgs TARGETS CONFIGURATIONS)
    cmake_parse_arguments(MY_INSTALL "${options}" "${oneValueArgs}" "${multiValueArgs}" CMAKE_PARSE_ARGUMENTS_SKIP_EMPTY "${ARGN}")
endmacro()

my_install(DESTINATION "" TARGETS foo "" bar)

if(DEFINED MY_INSTALL_DESTINATION)
    message(FATAL_ERROR "DEFINED MY_INSTALL_DESTINATION")
endif()

if(NOT "${MY_INSTALL_TARGETS}" STREQUAL "foo;bar")
    message(FATAL_ERROR "NOT \"\${MY_INSTALL_TARGETS}\" STREQUAL \"foo;bar\"")
endif()
