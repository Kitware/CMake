include(RunCMake)

# Do not let ccache modify paths checked by the test cases.
unset(ENV{CCACHE_BASEDIR})

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

  # Verify paths passed to compiler.
  unset(RunCMake_TEST_VARIANT_DESCRIPTION)
  run_symlink_test_case("${case}-exe" -S "${src}" -B "${bin}")
  if (RunCMake_GENERATOR MATCHES "Xcode")
    # The native build system may pass the real paths.
    set(RunCMake-stdout-file "generic-exe-build-stdout.txt")
  endif()
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command("${case}-exe-build" ${CMAKE_COMMAND} --build "${bin}")
endfunction ()

# Create the following structure:
#
#   .../none/source
#   .../none/binary
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/none")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/none/source")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/none/binary")
run_symlink_test(none-separate "source" "binary" "../source" "../binary")

# Create the following structure:
#
#   .../none/source
#   .../none/source/binary
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/none")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/none/source")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/none/source/binary")
run_symlink_test(none-bin_in_src "source" "source/binary" ".." "binary")

# Create the following structure:
#
#   .../none/binary
#   .../none/binary/source
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/none")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/none/binary")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/none/binary/source")
run_symlink_test(none-src_in_bin "binary/source" "binary" "source" "..")

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

# Create the following structure:
#
#   .../common_real/source
#   .../common_real/source/binary
#   .../common -> common_real
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/common_real")
file(REMOVE "${RunCMake_BINARY_DIR}/common")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/common_real/source")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/common_real/source/binary")
file(CREATE_LINK "common_real" "${RunCMake_BINARY_DIR}/common" SYMBOLIC)
run_symlink_test(common-bin_in_src "source" "source/binary" ".." "binary")

# Create the following structure:
#
#   .../common_real/binary
#   .../common_real/binary/source
#   .../common -> common_real
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/common_real")
file(REMOVE "${RunCMake_BINARY_DIR}/common")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/common_real/binary")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/common_real/binary/source")
file(CREATE_LINK "common_real" "${RunCMake_BINARY_DIR}/common" SYMBOLIC)
run_symlink_test(common-src_in_bin "binary/source" "binary" "source" "..")

# Create the following structure:
#
#   .../different_src/source_real
#   .../different_bin/binary_real
#   .../different/source -> ../different_src/source_real
#   .../different/binary -> ../different_bin/binary_real
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/different")
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/different_src")
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/different_bin")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/different")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/different_src/source_real")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/different_bin/binary_real")
file(CREATE_LINK "../different_src/source_real" "${RunCMake_BINARY_DIR}/different/source" SYMBOLIC)
file(CREATE_LINK "../different_bin/binary_real" "${RunCMake_BINARY_DIR}/different/binary" SYMBOLIC)
run_symlink_test(different-separate "source" "binary" "../../different/source" "../../different/binary")

# Create the following structure:
#
#   .../different_src/source_real
#   .../different_bin/binary_real
#   .../different/source -> ../different_src/source_real
#   .../different_src/source_real/binary -> ../../different_bin/binary_real
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/different")
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/different_src")
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/different_bin")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/different")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/different_src/source_real")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/different_bin/binary_real")
file(CREATE_LINK "../different_src/source_real" "${RunCMake_BINARY_DIR}/different/source" SYMBOLIC)
file(CREATE_LINK "../../different_bin/binary_real" "${RunCMake_BINARY_DIR}/different_src/source_real/binary" SYMBOLIC)
run_symlink_test(different-bin_in_src "source" "source/binary" "../../different/source" "binary")

# Create the following structure:
#
#   .../different_src/source_real
#   .../different_bin/binary_real
#   .../different/binary -> ../different_bin/binary_real
#   .../different_bin/binary_real/source -> ../../different_src/source_real
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/different")
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/different_src")
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/different_bin")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/different")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/different_src/source_real")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/different_bin/binary_real")
file(CREATE_LINK "../different_bin/binary_real" "${RunCMake_BINARY_DIR}/different/binary" SYMBOLIC)
file(CREATE_LINK "../../different_src/source_real" "${RunCMake_BINARY_DIR}/different_bin/binary_real/source" SYMBOLIC)
run_symlink_test(different-src_in_bin "binary/source" "binary" "source" "../../different/binary")

# Create the following structure:
#
#   .../asymmetric_real/path/binary
#   .../asymmetric/source
#   .../asymmetric/binary -> ../asymmetric_real/path/binary
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/asymmetric")
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/asymmetric_real")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/asymmetric/source")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/asymmetric_real/path/binary")
file(CREATE_LINK "../asymmetric_real/path/binary" "${RunCMake_BINARY_DIR}/asymmetric/binary" SYMBOLIC)
run_symlink_test(asymmetric-separate "source" "binary" "../../../asymmetric/source" "../binary")

# Create the following structure:
#
#   .../asymmetric_real/path/binary
#   .../asymmetric/source
#   .../asymmetric/source/binary -> ../../asymmetric_real/path/binary
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/asymmetric")
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/asymmetric_real")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/asymmetric/source")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/asymmetric_real/path/binary")
file(CREATE_LINK "../../asymmetric_real/path/binary" "${RunCMake_BINARY_DIR}/asymmetric/source/binary" SYMBOLIC)
run_symlink_test(asymmetric-bin_in_src "source" "source/binary" "../../../asymmetric/source" "binary")

# Create the following structure:
#
#   .../asymmetric_real/path/source
#   .../asymmetric/binary
#   .../asymmetric/binary/source -> ../../asymmetric_real/path/source
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/asymmetric")
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/asymmetric_real")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/asymmetric/binary")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/asymmetric_real/path/source")
file(CREATE_LINK "../../asymmetric_real/path/source" "${RunCMake_BINARY_DIR}/asymmetric/binary/source" SYMBOLIC)
run_symlink_test(asymmetric-src_in_bin "binary/source" "binary" "source" "../../../asymmetric/binary")
