cmake_minimum_required(VERSION 2.6)
project(EnvironmentProj)

add_executable(Environment main.cxx)

enable_testing()

add_test(Environment1 Environment)
add_test(Environment2 Environment)
add_test(EchoEnvironment1 ${CMAKE_COMMAND} -E environment)
add_test(EchoEnvironment2 ${CMAKE_COMMAND} -E environment)
add_test(EchoEnvironment3 ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/check_mod.cmake")

# Make sure "CMAKE_ENV.*Happy Thanksgiving" is in the output of
# the "1" tests:
#
set_tests_properties(Environment1 EchoEnvironment1 PROPERTIES
  ENVIRONMENT "CMAKE_ENVIRONMENT_TEST_VAR=Happy Thanksgiving!"
  PASS_REGULAR_EXPRESSION "CMAKE_ENV.*Happy Thanksgiving"
)

# Make sure "CMAKE_ENV.*Happy Thanksgiving" is *NOT* in the output of
# the "2" tests:
#
set_tests_properties(Environment2 EchoEnvironment2 PROPERTIES
  FAIL_REGULAR_EXPRESSION "CMAKE_ENV.*Happy Thanksgiving"
)

set_property(TEST EchoEnvironment3
  PROPERTY ENVIRONMENT_MODIFICATION
    # Variables expected to be unset.
    "UNSET_EXPLICIT=set:value"
    "UNSET_EXPLICIT=unset:"
    "UNSET_VIA_RESET=set:value"
    "UNSET_VIA_RESET=reset:"

    # Direct settings.
    "DIRECT=set:old"
    "DIRECT=set:new"

    # String manipulation.
    "STRING_MANIP=set:-core-"
    "STRING_MANIP=string_append:post-"
    "STRING_MANIP=string_prepend:-pre"
    "STRING_MANIP=string_append:suffix"
    "STRING_MANIP=string_prepend:prefix"

    # String manipulation on non-existent.
    "STRING_DNE=string_append:post-"
    "STRING_DNE=string_prepend:-pre"
    "STRING_DNE=string_append:suffix"
    "STRING_DNE=string_prepend:prefix"

    # Path manipulation.
    "PATH_MANIP=set:core"
    "PATH_MANIP=path_list_append:post"
    "PATH_MANIP=path_list_prepend:pre"
    "PATH_MANIP=path_list_append:suffix"
    "PATH_MANIP=path_list_prepend:prefix"

    # Path manipulation on non-existent.
    "PATH_DNE=path_list_append:post"
    "PATH_DNE=path_list_prepend:pre"
    "PATH_DNE=path_list_append:suffix"
    "PATH_DNE=path_list_prepend:prefix"

    # CMake list manipulation.
    "CMAKE_LIST_MANIP=set:core"
    "CMAKE_LIST_MANIP=cmake_list_append:post"
    "CMAKE_LIST_MANIP=cmake_list_prepend:pre"
    "CMAKE_LIST_MANIP=cmake_list_append:suffix"
    "CMAKE_LIST_MANIP=cmake_list_prepend:prefix"

    # CMake list manipulation on non-existent.
    "CMAKE_LIST_DNE=cmake_list_append:post"
    "CMAKE_LIST_DNE=cmake_list_prepend:pre"
    "CMAKE_LIST_DNE=cmake_list_append:suffix"
    "CMAKE_LIST_DNE=cmake_list_prepend:prefix"
)
