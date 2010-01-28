# Block just as long as timeout.cmake would if it were not killed.
execute_process(COMMAND ${Timeout})

# Verify that the log is empty, which indicates that the grandchild
# was killed before it finished sleeping.
file(READ "${Log}" LOG)
if(NOT "${LOG}" STREQUAL "")
  message(FATAL_ERROR "${LOG}")
endif()
