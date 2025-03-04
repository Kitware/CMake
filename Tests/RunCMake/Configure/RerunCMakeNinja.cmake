set(input  ${CMAKE_CURRENT_BINARY_DIR}/input.txt)
set(stamp  ${CMAKE_CURRENT_BINARY_DIR}/stamp.txt)
file(READ ${input} content)
file(WRITE ${stamp} "${content}")

# Add enough subdirectories to make the total list of paths to 'cmake_install.cmake'
# files exceed the Windows command-line length limit.
set(length 0)
foreach(i RANGE 1 1000)
  if(length GREATER_EQUAL 32678)
    break()
  endif()
  add_subdirectory(RerunCMakeNinja RerunCMakeNinja${i})
  string(LENGTH "${CMAKE_CURRENT_BINARY_DIR}/RerunCMakeNinja${i}/cmake_install.cmake" subdir_length)
  math(EXPR length "${length} + ${subdir_length}")
endforeach()
