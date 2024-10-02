cmake_minimum_required(VERSION 3.30)

include(RunCMake)

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(RunCMake_TEST_OPTIONS -DCMAKE_CONFIGURATION_TYPES=Debug)
else()
  set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
endif()

# Detect information from the toolchain:
# - CMAKE_C_LINK_LIBRARIES_PROCESSING
run_cmake(Inspect)
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

run_cmake(Unknown)

function(run_strategy case exe)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  if("DEDUPLICATION=ALL" IN_LIST CMAKE_C_LINK_LIBRARIES_PROCESSING)
    if("ORDER=REVERSE" IN_LIST CMAKE_C_LINK_LIBRARIES_PROCESSING)
      set(RunCMake-stderr-file ${case}-stderr-dedup-reverse.txt)
    else()
      set(RunCMake-stderr-file ${case}-stderr-dedup.txt)
    endif()
  endif()
  run_cmake(${case})
  unset(RunCMake-stderr-file)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --config Debug)
  if(exe)
    if("DEDUPLICATION=ALL" IN_LIST CMAKE_C_LINK_LIBRARIES_PROCESSING)
      if("ORDER=REVERSE" IN_LIST CMAKE_C_LINK_LIBRARIES_PROCESSING)
        set(RunCMake-stdout-file ${case}-run-stdout-dedup-reverse.txt)
      else()
        set(RunCMake-stdout-file ${case}-run-stdout-dedup.txt)
      endif()
    endif()
    run_cmake_command(${case}-run ${RunCMake_TEST_BINARY_DIR}/${exe})
    unset(RunCMake-stdout-file)
  endif()
endfunction()

run_strategy(Basic-PRESERVE_ORDER "main")
run_strategy(Basic-REORDER "main")

run_cmake(Duplicate-PRESERVE_ORDER)
run_cmake(Duplicate-REORDER)
