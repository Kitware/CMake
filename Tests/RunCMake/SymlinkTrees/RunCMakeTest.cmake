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
function (run_symlink_test case src bin src_from_bin bin_from_src)
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

  # Pass absolute path to the source tree, with -S.
  set(RunCMake_TEST_VARIANT_DESCRIPTION " -S $abs/${name}/${src}")
  run_symlink_test_case("${case}" -S "${RunCMake_TEST_SOURCE_DIR}")

  # Pass relative path to the source tree, plain.
  set(RunCMake_TEST_VARIANT_DESCRIPTION " ${src_from_bin}")
  run_symlink_test_case("${case}" "${src_from_bin}")

  # Pass relative path to the source tree, with -S.
  set(RunCMake_TEST_VARIANT_DESCRIPTION " -S ${src_from_bin}")
  run_symlink_test_case("${case}" -S "${src_from_bin}")

  # Test running in source directory.
  set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}")
  # Emulate a shell using this directory.
  set(ENV{PWD} "${RunCMake_TEST_COMMAND_WORKING_DIRECTORY}")

  # Pass absolute path to the binary tree with -B.
  set(RunCMake_TEST_VARIANT_DESCRIPTION " -B $abs/${name}/${bin}")
  run_symlink_test_case("${case}" -B "${RunCMake_TEST_BINARY_DIR}")

  # Pass relative path to the binary tree with -B.
  set(RunCMake_TEST_VARIANT_DESCRIPTION " -B ${bin_from_src}")
  run_symlink_test_case("${case}" -B "${bin_from_src}")

  # Test running in another directory.
  set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY "${RunCMake_BINARY_DIR}/${name}")
  # Emulate a shell using this directory.
  set(ENV{PWD} "${RunCMake_TEST_COMMAND_WORKING_DIRECTORY}")

  # Pass absolute paths to the source and binary trees.
  set(RunCMake_TEST_VARIANT_DESCRIPTION " -S $abs/${name}/${src} -B $abs/${name}/${bin}")
  run_symlink_test_case("${case}" -S "${RunCMake_TEST_SOURCE_DIR}" -B "${RunCMake_TEST_BINARY_DIR}")

  # Pass relative paths to the source and binary trees.
  set(RunCMake_TEST_VARIANT_DESCRIPTION " -S ${src} -B ${bin}")
  run_symlink_test_case("${case}" -S "${src}" -B "${bin}")

  # Pass relative paths to the source and binary trees.
  set(RunCMake_TEST_VARIANT_DESCRIPTION " -S ../${name}/${src} -B ../${name}/${bin}")
  run_symlink_test_case("${case}" -S "../${name}/${src}" -B "../${name}/${bin}")
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
run_symlink_test(common-separate "source" "binary" "../source" "../binary")
