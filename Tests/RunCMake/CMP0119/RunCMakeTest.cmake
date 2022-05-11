include(RunCMake)

function(run_CMP0119 status)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0119-${status}-build)
  run_cmake(CMP0119-${status})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CMP0119-${status}-build "${CMAKE_COMMAND}" --build . --config Debug)
endfunction()

if(NOT RunCMake_GENERATOR MATCHES "Visual Studio|Xcode" AND
    NOT CMAKE_C_COMPILER_ID MATCHES "(Borland|Embarcadero|Watcom)")
  run_CMP0119(WARN)
  run_CMP0119(OLD)
endif()
if((CMAKE_C_COMPILER_ID MATCHES "(GNU|LCC|Clang|MSVC|Borland|Embarcadero|Intel|TI)"))
  run_CMP0119(NEW)
endif()
