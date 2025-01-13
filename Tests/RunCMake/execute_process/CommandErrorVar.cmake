# Ignores var when RESULT_VARIABLE is used
set(CMAKE_EXECUTE_PROCESS_COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND ${CMAKE_COMMAND} -E false
  RESULT_VARIABLE result
)

# Ignores var when RESULTS_VARIABLE is used
execute_process(COMMAND ${CMAKE_COMMAND} -E false
  RESULTS_VARIABLE results
)

# Ignores var when argument supplied
execute_process(COMMAND ${CMAKE_COMMAND} -E false
  COMMAND_ERROR_IS_FATAL NONE
)

# Value of NONE has no effect
set(CMAKE_EXECUTE_PROCESS_COMMAND_ERROR_IS_FATAL NONE)
execute_process(COMMAND ${CMAKE_COMMAND} -E false)

# Error on invalid value
set(CMAKE_EXECUTE_PROCESS_COMMAND_ERROR_IS_FATAL BAD_VALUE)
execute_process(COMMAND ${CMAKE_COMMAND} -E false)
