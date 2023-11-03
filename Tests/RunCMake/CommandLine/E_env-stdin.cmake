execute_process(
  COMMAND ${CMAKE_COMMAND} -E echo "Hello world"
  COMMAND ${CMAKE_COMMAND} -E env ${PRINT_STDIN_EXE}
  )
