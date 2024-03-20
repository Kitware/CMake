include(RunCMake)

# Detect information from the toolchain:
# - CMAKE_C_STANDARD_DEFAULT
# - CMAKE_CXX_STANDARD_DEFAULT
run_cmake(Inspect)
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

function(run_StdLatest lang)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/StdLatest-${lang}-build)
  run_cmake(StdLatest-${lang})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(StdLatest-${lang}-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

if(NOT CMake_NO_C_STANDARD AND DEFINED CMAKE_C_STANDARD_DEFAULT)
  run_StdLatest(C)
endif()
if(NOT CMake_NO_CXX_STANDARD AND DEFINED CMAKE_CXX_STANDARD_DEFAULT)
  run_StdLatest(CXX)
endif()
if(CMake_TEST_CUDA)
  run_StdLatest(CUDA)
endif()
if(CMake_TEST_HIP)
  run_StdLatest(HIP)
endif()
if(CMake_TEST_OBJC)
  run_StdLatest(OBJC)
  run_StdLatest(OBJCXX)
endif()
