include(RunCMake)

file(WRITE ${RunCMake_BINARY_DIR}/test_project/CMakeCache.txt [[
CACHE_STRING:STRING=cache string
CACHE_BOOL:BOOL=ON
CACHE_INTERNAL:INTERNAL=cache internal
]])

run_cmake(NewForm_Project)
run_cmake_command(NewForm_Script ${CMAKE_COMMAND} -DRunCMake_BINARY_DIR=${RunCMake_BINARY_DIR}
  -P "${RunCMake_SOURCE_DIR}/NewForm_Script.cmake")
run_cmake_command(OldForm_Script ${CMAKE_COMMAND} -DRunCMake_BINARY_DIR=${RunCMake_BINARY_DIR}
  -P "${RunCMake_SOURCE_DIR}/OldForm_Script.cmake")
