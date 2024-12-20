include(RunCMake)

block()
  set(source ${RunCMake_SOURCE_DIR}/project)
  run_cmake_command(dont-set-file ${CMAKE_COMMAND} -S ${source})
  run_cmake_command(set-file-dne ${CMAKE_COMMAND} -S ${source} --project-file dne.cmake)
  run_cmake_command(set-file-multi ${CMAKE_COMMAND} -S ${source} --project-file 1 --project-file 2)
  run_cmake_command(set-file-none ${CMAKE_COMMAND} -S ${source} --project-file)

  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/other)
  run_cmake_command(set-file ${CMAKE_COMMAND} -S ${source} --project-file other.cmake)
  run_cmake_command(remembers-file ${CMAKE_COMMAND} -S ${source})
  run_cmake_command(cant-change-file ${CMAKE_COMMAND} -S ${source} --project-file another.cmake)
endblock()
