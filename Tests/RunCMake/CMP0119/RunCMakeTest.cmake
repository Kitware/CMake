include(RunCMake)

function(run_CMP0119 status)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0119-${status}-build)
  run_cmake(CMP0119-${status})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CMP0119-${status}-build "${CMAKE_COMMAND}" --build . --config Debug)
endfunction()

if(NOT RunCMake_GENERATOR MATCHES "Visual Studio|Xcode" AND
    NOT CMAKE_C_COMPILER_ID MATCHES "(Borland|Embarcadero|Watcom|OrangeC)")
  run_CMP0119(WARN)
  run_CMP0119(OLD)
endif()
if((CMAKE_C_COMPILER_ID MATCHES "(GNU|LCC|Clang|MSVC|Borland|Embarcadero|Intel|TI)")
  # FIXME(OrangeC#1137): OrangeC 7 no longer accepts custom file extensions with -x
  OR (CMAKE_C_COMPILER_ID STREQUAL "OrangeC" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 7))
  run_CMP0119(NEW)
endif()
