include(RunCMake)

run_cmake_command(ELF ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/ELF.cmake)

if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  run_cmake_command(XCOFF ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/XCOFF.cmake)
endif()
