include(RunCMake)

run_cmake_command(ELF ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/ELF.cmake)

if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  run_cmake_command(XCOFF ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/XCOFF.cmake)
endif()

run_cmake_command(TextCheck ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/TextCheck.cmake)
run_cmake_command(TextCheckEmpty ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/TextCheckEmpty.cmake)

run_cmake_command(TextChange ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/TextChange.cmake)
run_cmake_command(TextChangeEmpty ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/TextChangeEmpty.cmake)

run_cmake_command(TextSet ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/TextSet.cmake)
run_cmake_command(TextSetEmpty ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/TextSetEmpty.cmake)

run_cmake_command(TextRemove ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/TextRemove.cmake)
