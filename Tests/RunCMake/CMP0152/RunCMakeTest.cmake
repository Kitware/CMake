include(RunCMake)

if(NOT CMAKE_GENERATOR_NO_COMPILER_ENV)
  run_cmake(CMP0152-WARN)
  run_cmake(CMP0152-OLD)
  run_cmake(CMP0152-NEW)
endif()
