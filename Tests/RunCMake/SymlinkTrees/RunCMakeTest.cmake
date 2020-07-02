include(RunCMake)

# This function assumes that ``${RunCMake_BINARY_DIR}/${name}/source`` and
# ``${RunCMake_BINARY_DIR}/${name}/binary`` are set up properly prior to
# calling it.
function (run_symlink_test name)
  set(RunCMake_TEST_NO_CLEAN TRUE)
  configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt"
    "${RunCMake_BINARY_DIR}/${name}/source/CMakeLists.txt"
    COPYONLY)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/${name}/source")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}/binary")
  # Emulate a shell using this directory.
  set(ENV{PWD} "${RunCMake_TEST_BINARY_DIR}")
  set(RunCMake_TEST_OPTIONS
    "-Dinclude_dir:PATH=${CMAKE_CURRENT_LIST_DIR}")
  run_cmake("${name}_symlinks")
endfunction ()

# Create the following structure:
#
#   .../common_real/source
#   .../common_real/binary
#   .../common -> common_real
#
# In this case, CMake should act as if .../common *is* .../common_real for all
# computations except ``REALPATH``.  This supports the case where a system has
# a stable *symlink*, but not a stable target for that symlink.
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/common_real")
file(REMOVE "${RunCMake_BINARY_DIR}/common")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/common_real/source")
file(CREATE_LINK "common_real" "${RunCMake_BINARY_DIR}/common" SYMBOLIC)
run_symlink_test(common)
