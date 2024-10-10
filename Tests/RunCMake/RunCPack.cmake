# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

include(RunCMake)

#[[
set(RunPack_GENERATORS ...)
run_cpack(<case>
  # general options
  [CONFIG <config>]     # Build/package given configuration (default "Release").
  [GENERATORS <gen>...] # Tell cpack to use the given generator(s).
  [SAMPLE <sample>]     # Use RunCPack/<sample> project (default <case>).

  # build step
  [BUILD]               # Build the test project before packaging.

  # package,cpack-<gen> steps
  [PACKAGE]             # Run cpack via buildsystem "package" target.
  [NO_CPACK]            # Do not run cpack directly.

  # verify step
  [NO_VERIFY]           # Do not run verify step.
  [GLOB <glob>...]      # Match expected package files with globbing patterns.
  [VERIFY <command>...] # Run custom verification command on each package file.
  )
#]]
function(run_cpack case)
  cmake_parse_arguments(PARSE_ARGV 1 run_cpack
    # Zero-value
    "BUILD;PACKAGE;NO_CPACK;NO_VERIFY"
    # One-value
    "CONFIG;SAMPLE"
    # Multi-value
    "GENERATORS;GLOB;VERIFY"
    )

  if(DEFINED RunCPack_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown arguments:\n  ${RunCPack_UNPARSED_ARGUMENTS}")
  endif()
  if(DEFINED RunCPack_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "Keywords missing values:\n  ${RunCPack_KEYWORDS_MISSING_VALUES}")
  endif()

  if(run_cpack_GENERATORS)
    set(RunCPack_GENERATORS "${run_cpack_GENERATORS}")
  elseif(NOT RunCPack_GENERATORS)
    message(FATAL_ERROR "RunCPack_GENERATORS not defined by caller!")
  endif()

  if(run_cpack_CONFIG)
    set(RunCPack_CONFIG "${run_cpack_CONFIG}")
  elseif(NOT RunCPack_CONFIG)
    set(RunCPack_CONFIG "Release")
  endif()

  if(run_cpack_SAMPLE)
    set(RunCPack_SAMPLE "${run_cpack_SAMPLE}")
  else()
    set(RunCPack_SAMPLE "${case}")
  endif()

  if(run_cpack_GLOB)
    set(RunCPack_GLOB "${run_cpack_GLOB}")
  endif()

  if(run_cpack_VERIFY)
    set(RunCPack_VERIFY ${run_cpack_VERIFY})
  endif()

  # Configure the sample project.
  set(RunCMake_TEST_SOURCE_DIR ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/RunCPack/${RunCPack_SAMPLE})
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  string(APPEND RunCMake_TEST_RAW_ARGS " \"-DCPACK_GENERATOR=${RunCPack_GENERATORS}\"")
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=${RunCPack_CONFIG})
  endif()
  run_cmake(${case}-cmake)
  unset(RunCMake_TEST_RAW_ARGS)
  set(RunCMake_TEST_NO_CLEAN 1)

  # Optionally build the project.
  if(run_cpack_BUILD)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    run_cmake_command(${case}-build
      "${CMAKE_COMMAND}" --build . --config "${RunCPack_CONFIG}")
    unset(RunCMake_TEST_OUTPUT_MERGE)
  endif()

  # Optionally package through the build system.
  if(run_cpack_PACKAGE)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    run_cmake_command(${case}-package
      "${CMAKE_COMMAND}" --build . --config "${RunCPack_CONFIG}" --target package)
    unset(RunCMake_TEST_OUTPUT_MERGE)
  endif()

  # Run cpack with each generator.
  if(NOT run_cpack_NO_CPACK)
    foreach(RunCPack_GENERATOR IN LISTS RunCPack_GENERATORS)
      run_cmake_command(${case}-cpack-${RunCPack_GENERATOR}
        "${CMAKE_CPACK_COMMAND}" -C "${RunCPack_CONFIG}" -G "${RunCPack_GENERATOR}")
    endforeach()
  endif()

  # Verify the resulting package files.
  if(NOT run_cpack_NO_VERIFY)
    set(RunCMake_TEST_RAW_ARGS " \"-Dglob=${RunCPack_GLOB}\" \"-Dverify=${RunCPack_VERIFY}\" -P \"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/RunCPack/verify.cmake\"")
    run_cmake_command(${case}-verify
      "${CMAKE_COMMAND}" -Ddir=${RunCMake_TEST_BINARY_DIR})
    unset(RunCMake_TEST_RAW_ARGS)
  endif()
endfunction()
