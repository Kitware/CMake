include(RunCMake)

run_cmake_command(TargetMissing ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/TargetMissing.cmake)
run_cmake_command(ExecutableScripts ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/ExecutableScripts.cmake)
