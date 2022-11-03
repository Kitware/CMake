include(RunCMake)

run_cmake_command(TargetMissing ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/TargetMissing.cmake)
run_cmake_command(ExecutableScripts ${CMAKE_COMMAND} -DSAMPLE_EXE=${SAMPLE_EXE} -P ${RunCMake_SOURCE_DIR}/ExecutableScripts.cmake)
