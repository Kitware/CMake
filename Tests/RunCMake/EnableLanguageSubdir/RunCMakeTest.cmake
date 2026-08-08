include(RunCMake)

function(configure_and_build case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  run_cmake(${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --target main)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OUTPUT_MERGE)
  unset(RunCMake_TEST_BINARY_DIR)
endfunction()

# CMP0220 policy behavior.  These cases are language-agnostic (they use CXX)
# and configure-only, so they run on every platform regardless of CMake_TEST_CUDA.

# NEW: language configuration propagates up out of the enabling scope.
run_cmake(CMP0220-NEW)
run_cmake(CMP0220-NEW-function)
run_cmake(CMP0220-NEW-multilevel)

# OLD: language configuration stays in the scope that enabled it.
run_cmake(CMP0220-OLD)

# MIXED: NEW and OLD set at different scope levels around the enabling scope.
run_cmake(CMP0220-MIXED-block)
run_cmake(CMP0220-MIXED-nested-block)

# WARN: whether the optional CMP0220 warning is emitted.
run_cmake(CMP0220-WARN)
run_cmake(CMP0220-WARN-default)
run_cmake(CMP0220-WARN-block)
run_cmake(CMP0220-WARN-top-level)

# These cases enable CXX in a subdirectory and verify that its implicit RC
# language state is usable from an ancestor scope.
if(CMake_TEST_RESOURCES)
  configure_and_build(CXXImplicitRC)
  run_cmake(CMP0220-MIXED-implicit-RC)
endif()

# The following cases enable CUDA in a subdirectory and build a target in an
# ancestor scope, so they require a working CUDA toolchain.
if(CMake_TEST_CUDA)
  foreach(case IN ITEMS CUDAParent CUDASibling)
    configure_and_build(${case})
  endforeach()
endif()
