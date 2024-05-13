execute_process(COMMAND ${CMAKE_COMMAND} -E true
  COMMAND "${EXIT_CRASH_EXE}"
  RESULT_VARIABLE result
  )

if(NOT result EQUAL "0")
  execute_process(COMMAND ${CMAKE_COMMAND} -E true
    COMMAND "${EXIT_CRASH_EXE}"
    COMMAND_ERROR_IS_FATAL LAST
    )
endif()
