execute_process(COMMAND ${CMAKE_COMMAND} -E true
  COMMAND "${Python_EXECUTABLE}" -c
    "import os; os.kill(os.getpid(),11)"
  RESULT_VARIABLE result
  )

if(NOT result EQUAL "0")
  execute_process(COMMAND ${CMAKE_COMMAND} -E true
    COMMAND "${Python_EXECUTABLE}" -c
      "import os; os.kill(os.getpid(),11)"
    COMMAND_ERROR_IS_FATAL LAST
    )
endif()
