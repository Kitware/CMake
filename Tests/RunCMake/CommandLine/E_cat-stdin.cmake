file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ll.txt" "ll")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rld.txt" "rld")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E echo_append "H"
  COMMAND ${CMAKE_COMMAND} -E cat - # Read stdin from - arg
  )
execute_process(
  COMMAND ${CMAKE_COMMAND} -E echo_append "e"
  COMMAND ${CMAKE_COMMAND} -E cat # Read stdin when no args
  )
execute_process(
  COMMAND ${CMAKE_COMMAND} -E echo_append "o wo"
  COMMAND ${CMAKE_COMMAND} -E cat "${CMAKE_CURRENT_BINARY_DIR}/ll.txt" - "${CMAKE_CURRENT_BINARY_DIR}/rld.txt"
  )
