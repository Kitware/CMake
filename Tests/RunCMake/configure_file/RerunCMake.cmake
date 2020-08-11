message("Running CMake on RerunCMake") # write to stderr if cmake reruns
add_subdirectory(RerunCMake)
# make sure CMakeCache.txt is newer than ConfigureFileOutput.txt
execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1)
