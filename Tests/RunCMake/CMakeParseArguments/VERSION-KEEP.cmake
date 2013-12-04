# CMAKE_MINIMUM_REQUIRED_VERSION             3.0.0
# CMAKE_PARSE_ARGUMENTS_DEFAULT_SKIP_EMPTY   UNSET
# CMAKE_PARSE_ARGUMENTS_(KEEP|SKIP)_EMPTY    UNSET
#     => KEEP

cmake_minimum_required(VERSION 2.8.12)
# This is a hack, required to test the behaviour in CMake 3.0.0 before
# it is actually released
set(CMAKE_MINIMUM_REQUIRED_VERSION 3.0.0)

include(CMakeParseArguments)

macro(MY_INSTALL)
    set(options OPTIONAL FAST)
    set(oneValueArgs DESTINATION RENAME)
    set(multiValueArgs TARGETS CONFIGURATIONS)
    cmake_parse_arguments(MY_INSTALL "${options}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")
endmacro()

my_install(DESTINATION "" TARGETS foo "" bar)

if(NOT DEFINED MY_INSTALL_DESTINATION)
    message(FATAL_ERROR "NOT DEFINED MY_INSTALL_DESTINATION")
elseif(NOT "${MY_INSTALL_DESTINATION}" STREQUAL "")
    message(FATAL_ERROR "NOT \"\${MY_INSTALL_DESTINATION}\" STREQUAL \"\"")
endif()

if(NOT "${MY_INSTALL_TARGETS}" STREQUAL "foo;;bar")
    message(FATAL_ERROR "NOT \"\${MY_INSTALL_TARGETS}\" STREQUAL \"foo;;bar\"")
endif()
