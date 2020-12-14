execute_process(COMMAND ${CMAKE_COMMAND} -E true
  COMMAND ${CMAKE_COMMAND} -E sleep 10
  COMMAND ${CMAKE_COMMAND} -E true
  TIMEOUT 1
  RESULT_VARIABLE result
)

if(NOT result EQUAL "0")
  execute_process(COMMAND ${CMAKE_COMMAND} -E true
    COMMAND ${CMAKE_COMMAND} -E sleep 10
    COMMAND ${CMAKE_COMMAND} -E true
    TIMEOUT 1
    COMMAND_ERROR_IS_FATAL ANY
    )
endif()
