execute_process(COMMAND "${Python_EXECUTABLE}" -c
    "import os; os.kill(os.getpid(),11)"
  COMMAND ${CMAKE_COMMAND} -E true
  COMMAND_ERROR_IS_FATAL ANY
  )
