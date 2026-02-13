include(RunCMake)

function(build_project test)
  set(RunCMake_TEST_NO_CLEAN FALSE)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()

  run_cmake(${test})

  set(RunCMake_TEST_NO_CLEAN TRUE)
  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . --config Release)
endfunction()

#[[
These tests are not testing CPS as such, but rather are testing the way that
CMake maps configurations; specifically, which MAP_IMPORTED_CONFIG_<CONFIG> is
considered when selecting a configuration of a target required by another
target for which a non-standard configuration has been selected.

It happens that CMake's behavior is to always use the configuration of the
project being built, and not the selected configuration of the consumer of the
target for which configuration selection is occurring. This turns out to be
advantageous, as it makes it relatively simple to use chains of interface
targets to select along multiple configuration axes, since it is relatively
simple for projects to set CMAKE_MAP_IMPORTED_CONFIG_<CONFIG> to a list of
preferred values along each axis and be sure the same list will be used for
each level of selection.

This test/example uses the following hierarchy:
  target "animal", configurations "cat", "dog"
  targets "cat", "dog", configurations "tame", "wild"

The consuming project sets CMAKE_MAP_IMPORTED_CONFIG_<CONFIG> to the list of
desired values on each of the two axes, e.g. "CAT;TAME". This causes CMake to
select the "CAT" configuration of the "animal" target, which links "cat",
then, because CMake is still using MAP_IMPORTED_CONFIG_${CMAKE_BUILD_TYPE},
to select the "TAME" configuration of the "cat" target.

A more practical example is described by CPS, at
https://cps-org.github.io/cps/recommendations.html.

The main reason this is a "CPS" test is because CPS makes it possible to
provide targets which implement multiple configuration axes in a way that is
surprisingly usable by CMake consumers. However, CMake itself has no way of
generating such exports as of when this test is being added (CMake 4.3). Thus,
it is testing something which is suggested by CPS, but is unlikely to be
encountered in a CMake-created package. (In principle, however, a hand-written
CMake-script package description could produce equivalent imported targets.)
#]]

build_project(TestTameCat)
build_project(TestWildCat)
build_project(TestTameDog)
build_project(TestWildDog)
