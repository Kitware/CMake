cmake_minimum_required(VERSION 4.3)
project(TestDependencyAddTest C)

enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

add_executable(fake_discovery fake_discovery.c)
# Ensure the timestamp check uses the artifact path, not the target name.
set_target_properties(fake_discovery PROPERTIES OUTPUT_NAME renamed_discovery)

add_test(NAME ADD_TEST.command_genex
  COMMAND $<TARGET_FILE:fake_discovery> fake_discovery case_foo)
set_tests_properties(ADD_TEST.command_genex PROPERTIES
  LABELS label_one)

add_test(NAME ADD_TEST.command_target
  COMMAND "$<1:fake_discovery>" case_bar)
set_tests_properties(ADD_TEST.command_target PROPERTIES
  LABELS label_two)

add_test(NAME ADD_TEST.build_depends
  COMMAND ${CMAKE_COMMAND} -E true
  BUILD_DEPENDS fake_discovery)
set_tests_properties(ADD_TEST.build_depends PROPERTIES
  LABELS label_three)
