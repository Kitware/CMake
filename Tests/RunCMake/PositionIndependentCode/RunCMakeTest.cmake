include(RunCMake)

run_cmake(Conflict1)
run_cmake(Conflict2)
run_cmake(Conflict3)
run_cmake(Conflict4)
run_cmake(Conflict5)
run_cmake(Conflict6)
run_cmake(Debug)
run_cmake(Genex1)
run_cmake(Genex2)

set(RunCMake_TEST_OPTIONS "-DPIE_SUPPORTED=${RunCMake_BINARY_DIR}/PIESupported.cmake")
run_cmake(CheckPIESupported)
include ("${RunCMake_BINARY_DIR}/PIESupported.cmake" OPTIONAL)

if (PIE_SUPPORTED OR NO_PIE_SUPPORTED)
  if (CMAKE_SYSTEM_NAME MATCHES "^(Linux|(Free|Net|Open)BSD)$")
    # try to locate readelf needed for validation
    find_program (READELF NAMES readelf)
  endif()
  if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # try to locate otool needed for validation
    find_program (OTOOL NAMES otool)
  endif()

  if ((READELF OR OTOOL) AND
      (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
    macro(run_cmake_target test subtest)
      set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
      set(RunCMake_TEST_NO_CLEAN 1)
      set(RunCMake_TEST_CONFIG Release)
      run_cmake_command(${test}-${subtest} ${CMAKE_COMMAND} --build . --config Release --target ${subtest} ${ARGN})

      unset(RunCMake_TEST_BINARY_DIR)
      unset(RunCMake_TEST_NO_CLEAN)
    endmacro()

    set(RunCMake_TEST_SOURCE_DIR "${RunCMake_SOURCE_DIR}")
    set(RunCMake_TEST_OUTPUT_MERGE TRUE)
    if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
      set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
    endif()

    run_cmake(PIE)
    if (PIE_SUPPORTED)
      run_cmake_target(PIE pie_on)
    endif()
    if (NO_PIE_SUPPORTED)
      run_cmake_target(PIE pie_off)
    endif()

    run_cmake(CMP0083)
    run_cmake_target(CMP0083 cmp0083_ref)

    # retrieve default mode
    include("${RunCMake_SOURCE_DIR}/PIE_validator.cmake")
    include("${RunCMake_BINARY_DIR}/CMP0083-build/Release/CMP0083_config.cmake")
    check_executable("${cmp0083_ref}" cmp0083_ref_mode)

    if ((cmp0083_ref_mode STREQUAL "PIE" AND NO_PIE_SUPPORTED)
        OR (cmp0083_ref_mode STREQUAL "NO_PIE" AND PIE_SUPPORTED))
      run_cmake_target(CMP0083 cmp0083_new)
    endif()
    run_cmake_target(CMP0083 cmp0083_old)

    unset(RunCMake_TEST_SOURCE_DIR)
    unset(RunCMake_TEST_OPTIONS)
    unset(RunCMake_TEST_OUTPUT_MERGE)
  endif()
endif()
