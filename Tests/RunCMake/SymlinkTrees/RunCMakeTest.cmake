include(RunCMake)

function(run_symlink_test_case)
  file(REMOVE_RECURSE
    "${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt"
    "${RunCMake_TEST_BINARY_DIR}/CMakeFiles"
    )
  run_cmake_with_options(${ARGN})
endfunction()

# This function assumes that ``${RunCMake_BINARY_DIR}/${name}/source`` and
# ``${RunCMake_BINARY_DIR}/${name}/binary`` are set up properly prior to
# calling it.
function (run_symlink_test case src bin)
  string(REGEX REPLACE "-.*" "" name "${case}")
  set(RunCMake_TEST_NO_CLEAN TRUE)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/${name}/${src}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}/${bin}")
  configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt"
    "${RunCMake_TEST_SOURCE_DIR}/CMakeLists.txt"
    COPYONLY)

  # We explicitly pass the source directory argument for each case.
  set(RunCMake_TEST_NO_SOURCE_DIR 1)

  # Test running in binary directory.
  set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  # Emulate a shell using this directory.
  set(ENV{PWD} "${RunCMake_TEST_COMMAND_WORKING_DIRECTORY}")

  # Pass absolute path to the source tree, plain.
  set(RunCMake_TEST_VARIANT_DESCRIPTION " $abs/${name}/${src}")
  run_symlink_test_case("${case}" "${RunCMake_TEST_SOURCE_DIR}")
endfunction ()

# Create the following structure:
#
#   .../common_real/source
#   .../common_real/binary
#   .../common -> common_real
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/common_real")
file(REMOVE "${RunCMake_BINARY_DIR}/common")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/common_real/source")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/common_real/binary")
file(CREATE_LINK "common_real" "${RunCMake_BINARY_DIR}/common" SYMBOLIC)
run_symlink_test(common-separate "source" "binary")
