cmake_minimum_required(VERSION 4.3)
set(CMAKE_TEST_BUILD_DEPENDS ON)

project(TestDependencyCase NONE)

enable_testing()

# Test names that differ only in case each get a working test_prep target,
# even where the build tool's target names are case-insensitive.
foreach(case IN ITEMS "Mixed;upper" "mixed;lower" "All;all")
  list(GET case 0 test)
  list(GET case 1 dependency)
  add_custom_target(${dependency}_case_dependency
    COMMAND ${CMAKE_COMMAND} -E touch ${dependency}_case_dependency-built.txt)
  set_property(TARGET ${dependency}_case_dependency
    PROPERTY EXCLUDE_FROM_ALL TRUE)
  add_test(NAME ${test}
    COMMAND ${CMAKE_COMMAND} -E true
    BUILD_DEPENDS ${dependency}_case_dependency)
endforeach()
