
include(RunCMake)

macro(run_cmake_target test subtest target)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-${subtest} ${CMAKE_COMMAND} --build . --target ${target} ${ARGN})

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()


run_cmake(bad_SHELL_usage)

if(RunCMake_GENERATOR MATCHES "(Ninja|Makefile)")
  run_cmake(LINKER_expansion)

  run_cmake_target(LINKER_expansion LINKER linker)
  run_cmake_target(LINKER_expansion LINKER_SHELL linker_shell)
  run_cmake_target(LINKER_expansion LINKER_CONSUMER linker_consumer)
endif()

# Some environments are excluded because they are not able to honor verbose mode
if (RunCMake_GENERATOR MATCHES "Makefiles|Ninja|Xcode|Visual Studio"
    AND NOT CMAKE_C_COMPILER_ID STREQUAL "Intel")
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)

  foreach(policy IN ITEMS OLD NEW)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/LINKER_expansion2-CMP0181-${policy}-build)
    run_cmake_with_options(LINKER_expansion2 -DCMP0181=${policy})

    run_cmake_target(LINKER_expansion2-CMP0181-${policy} EXE_LINKER_FLAGS exe_linker_flags --verbose)
    run_cmake_target(LINKER_expansion2-CMP0181-${policy} SHARED_LINKER_FLAGS shared_linker_flags --verbose)
    run_cmake_target(LINKER_expansion2-CMP0181-${policy} MODULE_LINKER_FLAGS module_linker_flags --verbose)
  endforeach()
endif()
