include(RunCMake)

if (RunCMake_GENERATOR MATCHES "Visual Studio 9 2008")
  run_cmake(UnsupportedLinkerType)
  return()
endif()

run_cmake(InvalidLinkerType)

# look-up for LLVM linker
if (WIN32)
  set (LINKER_NAMES lld-link)
else()
  set(LINKER_NAMES ld.lld ld64.lld)
endif()
find_program(LLD_LINKER NAMES ${LINKER_NAMES})

macro(run_cmake_and_build test)
  run_cmake_with_options(${test} -DCMake_TEST_CUDA=${CMake_TEST_CUDA})
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(CMake_TEST_CUDA)
    string(APPEND "|${CMAKE_CUDA_USING_LINKER_LLD}")
  endif()
  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . --config Release --verbose ${ARGN})

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()

if(LLD_LINKER)
  block(SCOPE_FOR VARIABLES)
    set(CMAKE_VERBOSE_MAKEFILE TRUE)
    set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)
    set(CMAKE_CUDA_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)

    run_cmake_and_build(ValidLinkerType)
    run_cmake_and_build(CustomLinkerType)
  endblock()
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL "15.0")
  run_cmake_and_build(AppleClassic)
endif()
