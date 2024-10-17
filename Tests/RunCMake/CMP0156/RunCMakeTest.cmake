include(RunCMake)

# CMP0156 control how libraries are specified for the link step
# a sensible configuration is how circular dependency is handled

if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(OPTIONS -DCMAKE_BUILD_TYPE=Release)
endif()

macro(run_cmake_and_build test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  run_cmake_with_options(${test} ${OPTIONS})
  set(RunCMake_TEST_NO_CLEAN TRUE)
  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . --config Release)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_OUTPUT_MERGE)
endmacro()

run_cmake(CMP0156-WARN)
run_cmake_and_build(CMP0156-OLD)
run_cmake_and_build(CMP0156-NEW)

if (CMAKE_C_COMPILER_ID STREQUAL "AppleClang"
    AND CMAKE_C_COMPILER_VERSION GREATER_EQUAL "15.0")
  # special case for Apple: with CMP0156=OLD, linker will warning on duplicate libraries
  run_cmake_and_build(CMP0156-OLD-AppleClang)
  run_cmake_and_build(CMP0156-NEW-AppleClang)
endif()


run_cmake_and_build(CMP0156-NEW-Imported)

if (CMAKE_C_COMPILER_ID STREQUAL "AppleClang"
    AND CMAKE_C_COMPILER_VERSION GREATER_EQUAL "15.0")
  # special case for Apple: FIXME(#26284): linker will warning on duplicate libraries
  run_cmake_and_build(CMP0156-NEW-AppleClang-Imported)
endif()
