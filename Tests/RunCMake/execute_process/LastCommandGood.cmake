execute_process(COMMAND ${CMAKE_COMMAND} -E true
  COMMAND ${CMAKE_COMMAND} -E false
  COMMAND ${CMAKE_COMMAND} -E false
  COMMAND ${CMAKE_COMMAND} -E true
  RESULT_VARIABLE result
  )

if(result EQUAL "0")
  execute_process(COMMAND ${CMAKE_COMMAND} -E true
    COMMAND ${CMAKE_COMMAND} -E false
    COMMAND ${CMAKE_COMMAND} -E false
    COMMAND ${CMAKE_COMMAND} -E true
    COMMAND_ERROR_IS_FATAL LAST
    )
endif()
