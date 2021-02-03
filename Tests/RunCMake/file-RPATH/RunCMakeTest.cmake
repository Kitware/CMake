include(RunCMake)

if(HAVE_ELF_H)
  run_cmake_command(ELF ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/ELF.cmake)
endif()
