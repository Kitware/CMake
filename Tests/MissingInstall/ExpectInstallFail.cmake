execute_process(
  COMMAND ${CMAKE_COMMAND}
    --build .
    --target install ${SI_CONFIG}
  RESULT_VARIABLE RESULT
  OUTPUT_VARIABLE OUTPUT
  ERROR_VARIABLE ERROR
)

if(NOT "${RESULT}" GREATER "0")
  message(FATAL_ERROR "install should have failed")
endif()
