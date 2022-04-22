execute_process(COMMAND "${Python_EXECUTABLE}" -c
    "import os; os.kill(os.getpid(),11)"
  COMMAND ${CMAKE_COMMAND} -E true
  RESULT_VARIABLE result
  )

if(result EQUAL "0")
  execute_process(COMMAND "${Python_EXECUTABLE}" -c
      "import os; os.kill(os.getpid(),11)"
    COMMAND ${CMAKE_COMMAND} -E true
    COMMAND_ERROR_IS_FATAL LAST
    )
endif()
