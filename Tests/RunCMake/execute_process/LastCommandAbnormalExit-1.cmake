execute_process(COMMAND "${EXIT_CRASH_EXE}"
  COMMAND ${CMAKE_COMMAND} -E true
  RESULT_VARIABLE result
  )

if(result EQUAL "0")
  execute_process(COMMAND "${EXIT_CRASH_EXE}"
    COMMAND ${CMAKE_COMMAND} -E true
    COMMAND_ERROR_IS_FATAL LAST
    )
endif()
