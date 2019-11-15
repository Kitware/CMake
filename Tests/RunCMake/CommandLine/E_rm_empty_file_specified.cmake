execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm ""
  RESULT_VARIABLE actual_result
  )

if(NOT "${actual_result}" EQUAL "1")
  message(SEND_ERROR "cmake -E rm \"\" should have returned 1, got ${actual_result}")
endif()
