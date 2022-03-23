include(RunCMake)

if(NOT CMAKE_GENERATOR_NO_COMPILER_ENV)
  run_cmake(CMP0132-WARN)
  run_cmake(CMP0132-OLD)
  run_cmake(CMP0132-NEW)
endif()
