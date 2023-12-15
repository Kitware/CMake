
# This tests setting the TEST_LAUNCHER target property from the
# CMAKE_TEST_LAUNCHER variable.

# -DCMAKE_TEST_LAUNCHER=/path/to/pseudo_test_launcher is passed to this
# test

project(test_launcher LANGUAGES C)

add_executable(target_with_test_launcher simple_src_exiterror.cxx)
set_target_properties(target_with_test_launcher PROPERTIES LINKER_LANGUAGE C)
get_property(launcher TARGET target_with_test_launcher
             PROPERTY TEST_LAUNCHER)
if(NOT "${launcher}" MATCHES "pseudo_test_launcher")
  message(SEND_ERROR "Default TEST_LAUNCHER property not set")
endif()

set_property(TARGET target_with_test_launcher
             PROPERTY TEST_LAUNCHER "another_test_launcher")
get_property(launcher TARGET target_with_test_launcher
             PROPERTY TEST_LAUNCHER)
if(NOT "${launcher}" MATCHES "another_test_launcher")
  message(SEND_ERROR
    "set_property/get_property TEST_LAUNCHER is not consistent")
endif()

unset(CMAKE_TEST_LAUNCHER CACHE)
add_executable(target_without_test_launcher simple_src_exiterror.cxx)
set_target_properties(target_without_test_launcher PROPERTIES LINKER_LANGUAGE C)
get_property(launcher TARGET target_without_test_launcher
             PROPERTY TEST_LAUNCHER)
if(NOT "${launcher}" STREQUAL "")
  message(SEND_ERROR "Default TEST_LAUNCHER property not set to null")
endif()

add_executable(target_with_empty_test_launcher simple_src_exiterror.cxx)
set_target_properties(target_with_empty_test_launcher PROPERTIES LINKER_LANGUAGE C)
set_property(TARGET target_with_empty_test_launcher PROPERTY TEST_LAUNCHER "")

enable_testing()
add_test(NAME test_target_with_empty_test_launcher COMMAND target_with_empty_test_launcher)
